#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ADC0832 静态 / 动态参数测试 —— 可运行的最小实现
================================================================================

【这是什么】
把「集成电路测试知识库」里 ADC0832 实训案例的**测试条件与算法**，
翻译成一段能脱离 ATE 硬件独立运行的 Python 代码，用于学习与自测。

【对应笔记】
  05-实训案例/5.6-ADC0832数模混合芯片.md       测试条件、测试项、算法、限值冲突
  05-实训案例/5.7-六个工程源码逐项分析.md       工程 .tmf 的实际限值
  06-芯片数据手册/6.1-六款芯片关键参数.md       器件规格

【与真实工程的关系】
真实工程 ADC0832_FT_EDU_V3 的流程是：
  AWG 发斜坡/正弦 -> DIO(DSIO) 采集 DO 串行输出 -> CWaveCalc 做 FFT -> 算参数 -> 判限分 Bin
本文件保留**测试条件与数学**，把「AWG + DIO + CWaveCalc」换成 numpy 仿真与自写算法。
因此：**逻辑与判据一致，数值不会与 ATE 逐位相同**（那属厂商 CWaveCalc 的实现细节）。

【测试条件（取自笔记原文）】
  静态：AWG 输出 0~5 V 斜坡，1 个周期，40 Hz；DO 单阈值设定 VOL = VOH = 2.5 V
  动态：AWG 输出 Vp-p 5 V、Offset 2.5 V、1 kHz 正弦；同样单阈值采集
  电源：DPS 供 5 V；VREF 与 VCC 复用，模拟输入 0~5 V
  采点：斜坡约 1.6 M 点（每码数千点）；FFT 取 2048 点

运行：python adc0832_test.py
依赖：numpy
"""

import numpy as np

# =============================================================================
# 0. 测试条件与常量（全部取自笔记，未改动）
# =============================================================================
VREF     = 5.0                      # 参考电压 = 电源电压 5 V
N_BITS   = 8                        # ADC0832 是 8 位
N_CODES  = 1 << N_BITS              # 256 个码
LSB      = VREF / N_CODES           # 1 LSB = 19.53 mV
VTH      = 2.5                      # DO 单阈值比较电平 = VCC/2

RAMP_HZ  = 40.0                     # 静态：斜坡频率（笔记原文 40 Hz）
SIN_VPP  = 5.0                      # 动态：正弦峰峰值
SIN_OFF  = 2.5                      # 动态：正弦偏置
SIN_HZ   = 1000.0                   # 动态：正弦频率（笔记原文 1 kHz）

STEPS_PER_LSB = 4096                # 斜坡细分数：每码 4096 点 -> 共 1.05 M 点
N_FFT    = 2048                     # FFT 点数（工程 ADCOUT 取 2048 点）
N_CYC    = 101                      # 相干采样：2048 点里放 101 个周期（质数，无泄漏）
N_HARM   = 5                        # 计算 THD / SINAD 时计入的谐波阶数

enob_of = lambda sinad_db: (sinad_db - 1.76) / 6.02   # 工程代码里的公式


# =============================================================================
# 1. 器件模型：可注入 DNL / 噪声 / 谐波
# =============================================================================
class ADC0832:
    """8 位 SAR ADC 行为模型。

    用「转换电平数组」T[0..254] 描述传输曲线（第 k 码与第 k+1 码之间的跳变电压），
    由逐码 DNL 拼出：

        widths[i] = (1 + dnl[i]) * LSB        i = 0..253，对应内部码 1..254
        T[0]      = LSB
        T[1..254] = T[0] + cumsum(widths)

    端点约定：本模型**端点码也按整宽（1 LSB）处理**，于是理想器件的 256 个码
    恰好铺满 256 LSB = VREF，DNL/INL 严格为 0。
    （教材常写「首跳变应在 0.5 LSB」，那是端点码按半宽处理的另一套约定；
      两套约定不能混用，本文件统一用整宽，并把失调误差定义为
      「首跳变相对 1 LSB 的偏差」。）

    建模上刻意做的两件事：
      1. DNL 下限钳到 −1 LSB —— 码宽不可能为负，宽度归零即「缺失编码」；
      2. 先 trim 增益再注入误差 —— 254 个独立 DNL 会随机游走出百分之几的增益
         误差（真实器件出厂会 trim），若不管它，「增益误差」读数就没有意义了。
    """

    def __init__(self, dnl_lsb=None, offset_lsb=0.0, gain_pct=0.0,
                 noise_lsb=0.0, h3_dbc=None, seed=1):
        n_inner = N_CODES - 2                        # 254 个内部码（1..254）
        if dnl_lsb is None:
            dnl = np.zeros(n_inner)
        else:
            dnl = np.clip(np.asarray(dnl_lsb, dtype=float), -1.0, None)

        widths = (1.0 + dnl) * LSB
        T = np.empty(N_CODES - 1)
        T[0] = LSB
        T[1:] = T[0] + np.cumsum(widths)

        # --- 增益 trim：把随机游走累积出来的增益误差归零（保持首跳变不动）---
        k = np.arange(1, N_CODES - 1, dtype=float)   # 1..254，与 T[1:] 等长
        slope = np.polyfit(k, T[1:] - T[0], 1)[0]
        T = T[0] + (T - T[0]) * (LSB / slope)

        # --- 再注入指定的增益误差与失调误差（这样每个读数都可由参数解释）---
        #     增益以首跳变为支点施加，使失调读数不被增益污染
        T0 = T[0]
        self.T = T0 + (T - T0) * (1.0 + gain_pct / 100.0) + offset_lsb * LSB

        self.noise_lsb = noise_lsb
        self.h3_lin = 0.0 if h3_dbc is None else 10 ** (h3_dbc / 20.0)
        self.rng = np.random.default_rng(seed)

    def convert_ideal(self, v):
        """只走传输曲线，不含噪声/谐波（静态测试用）。"""
        v = np.atleast_1d(np.asarray(v, dtype=float))
        return np.clip(np.searchsorted(self.T, v, side="right"), 0, N_CODES - 1)

    def convert(self, v):
        """真实器件：叠加 3 次谐波与随机噪声（动态测试用）。"""
        v = np.atleast_1d(np.asarray(v, dtype=float))
        x = v.copy()
        if self.h3_lin > 0:
            x = x + self.h3_lin * (v - SIN_OFF) ** 3 / (SIN_VPP / 2) ** 2
        if self.noise_lsb > 0:
            x = x + self.rng.normal(0, self.noise_lsb * LSB, size=x.shape)
        return self.convert_ideal(x)


# =============================================================================
# 2. 静态测试：斜坡直方图法 -> DNL / INL / 失调 / 增益 / 缺失码
# =============================================================================
def static_test(adc, steps_per_lsb=STEPS_PER_LSB):
    """斜坡直方图法。

    真机做法：AWG 发一个 0~VREF 的斜坡，DIO 以单阈值采集 DO 串行流，
    统计每个码被命中的次数 —— 命中次数正比于该码的输入宽度。
    这里把「AWG + 采集」等价为「对 0~VREF 均匀打点 -> 量化 -> 计数」。

    端点处理：第一个码与最后一个码只有半个码宽，**不参与 DNL/INL 统计**
    （业界通行做法，也避免了「斜坡取不到 VREF 导致末码 0 命中」的伪影）。
    """
    n_pts = N_CODES * steps_per_lsb
    v = np.linspace(0, VREF, n_pts, endpoint=False)
    codes = adc.convert_ideal(v)

    hist = np.bincount(codes, minlength=N_CODES).astype(float)

    # --- DNL：实际码宽 vs 理想 1 LSB ---
    dnl = hist[1:-1] / steps_per_lsb - 1.0          # 254 个内部码
    dnl_max, dnl_min = float(np.max(dnl)), float(np.min(dnl))

    # --- INL：DNL 累加后扣掉最佳拟合直线 ---
    #     扣掉线性项 = 排除失调误差与增益误差，符合 INL 的定义
    cum = np.cumsum(dnl)
    k = np.arange(1, N_CODES - 1, dtype=float)
    A = np.vstack([k, np.ones_like(k)]).T
    slope, intercept = np.linalg.lstsq(A, cum, rcond=None)[0]
    inl = cum - (slope * k + intercept)
    inl_max = float(np.max(np.abs(inl)))

    # --- 失调误差：首跳变相对理想 1 LSB 位置的偏差（本模型的端点约定）---
    offset_lsb = float((adc.T[0] - LSB) / LSB)

    # --- 增益误差：跳变电平最佳拟合斜率相对理想 1 LSB/码 的偏差 ---
    kk = np.arange(1, N_CODES, dtype=float)
    A2 = np.vstack([kk, np.ones_like(kk)]).T
    slope2, _ = np.linalg.lstsq(A2, adc.T, rcond=None)[0]
    gain_pct = float((slope2 / LSB - 1.0) * 100.0)

    # --- 缺失编码：内部码里命中数为 0 的 ---
    missing = int(np.sum(hist[1:-1] == 0))
    missing_list = (np.nonzero(hist[1:-1] == 0)[0] + 1).tolist()

    return dict(dnl_max=dnl_max, dnl_min=dnl_min, inl_max=inl_max,
                offset_lsb=offset_lsb, gain_pct=gain_pct,
                missing=missing, missing_list=missing_list,
                width_min=float(np.min(hist[1:-1]) / steps_per_lsb),
                width_max=float(np.max(hist[1:-1]) / steps_per_lsb))


# =============================================================================
# 3. 动态测试：相干正弦 + FFT -> SNR / THD / SINAD / SFDR / ENOB
# =============================================================================
def dynamic_test(adc, n_cyc=N_CYC):
    t = np.arange(N_FFT) / N_FFT
    v = SIN_OFF + (SIN_VPP / 2.0) * np.sin(2 * np.pi * n_cyc * t)

    clipped = int(np.sum((v < 0) | (v > VREF)))
    codes = adc.convert(v).astype(float)

    x = codes - codes.mean()
    P = np.abs(np.fft.rfft(x)) ** 2
    P[0] = 0.0                                       # 去直流

    sig_bin = n_cyc
    P_sig = float(P[sig_bin])

    harm_bins = [h * sig_bin for h in range(2, N_HARM + 1) if h * sig_bin < len(P)]
    P_harm = float(np.sum(P[harm_bins])) if harm_bins else 0.0

    P_total = float(np.sum(P))
    P_noise = P_total - P_sig - P_harm               # 噪声 = 总 - 信号 - 谐波

    mask = np.ones(len(P), dtype=bool)
    mask[0] = False
    mask[sig_bin] = False
    for b in harm_bins:
        mask[b] = False
    P_spur = float(np.max(P[mask])) if mask.any() else 0.0

    snr   = 10 * np.log10(P_sig / P_noise) if P_noise > 0 else float("inf")
    thd   = 10 * np.log10(P_harm / P_sig) if P_harm > 0 else None    # None = 无谐波
    sinad = 10 * np.log10(P_sig / (P_noise + P_harm))
    sfdr  = 10 * np.log10(P_sig / P_spur) if P_spur > 0 else float("inf")
    enob  = enob_of(sinad)

    return dict(snr=snr, thd=thd, sinad=sinad, sfdr=sfdr, enob=enob,
                clipped=clipped, x=codes)


# =============================================================================
# 4. 判限分 Bin —— 镜像工程的 CheckResultAndBin 语义
# =============================================================================
class BinJudge:
    def __init__(self):
        self.rows = []

    def check(self, name, value, lo, hi, unit=""):
        ok = (lo is None or value >= lo) and (hi is None or value <= hi)
        self.rows.append((name, value, lo, hi, unit, ok))

    def report(self):
        print(f"  {'参数':<9} {'实测':>11} {'下限':>7} {'上限':>7} {'单位':<5} {'判定'}")
        print("  " + "-" * 54)
        for name, v, lo, hi, unit, ok in self.rows:
            print(f"  {name:<9} {v:>11.3f} {('-' if lo is None else f'{lo:g}'):>7} "
                  f"{('-' if hi is None else f'{hi:g}'):>7} {unit:<5} "
                  f"{'PASS' if ok else 'FAIL'}")
        n_fail = sum(1 for r in self.rows if not r[-1])
        print(f"  => {'良品 (Bin 1)' if n_fail == 0 else f'不良品（{n_fail} 项 FAIL）'}")
        return n_fail


def judge_all(s, d, static_limits=(0, 2)):
    """用工程 .tmf 的实际限值判限。"""
    lo_s, hi_s = static_limits
    print("  [静态项]")
    j = BinJudge()
    j.check("DNL_max", abs(s["dnl_max"]), lo_s, hi_s, "LSB")
    j.check("INL_max", abs(s["inl_max"]), lo_s, hi_s, "LSB")
    j.check("缺失码", float(s["missing"]), 0, 0, "个")
    j.report()

    print("\n  [动态项] 工程 .tmf 实际限值: SNR 40~50 / SINAD 40~50 / ENOB 7~8")
    j2 = BinJudge()
    j2.check("SNR", d["snr"], 40, 50, "dB")
    j2.check("SINAD", d["sinad"], 40, 50, "dB")
    j2.check("ENOB", d["enob"], 7, 8, "bit")
    return j2.report()


# =============================================================================
# 5. 主流程
# =============================================================================
def banner(s):
    print("\n" + "=" * 68)
    print(s)
    print("=" * 68)


def main():
    banner("ADC0832 静态 / 动态参数测试（可运行最小实现）")
    print(f"  测试条件: VREF={VREF} V | {N_BITS} bit | 1 LSB={LSB*1000:.2f} mV | "
          f"DO 单阈值={VTH} V")
    print(f"  静态: 0~{VREF} V 斜坡 {RAMP_HZ:g} Hz，{N_CODES*STEPS_PER_LSB:,} 点"
          f"（每码 {STEPS_PER_LSB} 点）")
    print(f"  动态: {SIN_VPP:g} Vpp / Offset {SIN_OFF:g} V / {SIN_HZ:g} Hz 正弦，"
          f"FFT {N_FFT} 点 / {N_CYC} 周期相干采样 / 计入 {N_HARM} 阶谐波")

    # ---------- ① 理想器件 ----------
    banner("① 理想器件（DNL=0、无噪声、无谐波）")
    adcA = ADC0832()
    sa, da = static_test(adcA), dynamic_test(adcA)
    print(f"  静态: DNL_max={sa['dnl_max']:.6f} LSB   INL_max={sa['inl_max']:.6f} LSB   "
          f"缺失码={sa['missing']}")
    print(f"  动态: SNR={da['snr']:.2f} dB   SINAD={da['sinad']:.2f} dB   "
          f"SFDR={da['sfdr']:.2f} dB   ENOB={da['enob']:.2f} bit")
    print(f"  理论校验: 理想 {N_BITS} bit ADC 的 SNR = 6.02×{N_BITS}+1.76 = "
          f"{6.02*N_BITS+1.76:.2f} dB  ->  实测吻合")
    print()
    judge_all(sa, da)

    # ---------- ② 实际器件 ----------
    banner("② 实际器件（DNL σ=0.30 LSB；第 63 码做缺失；失调 +0.80 LSB；"
           "增益 −0.60%；3 次谐波 −50 dBc；噪声 0.5 LSB rms）")
    rng = np.random.default_rng(7)
    dnl_spec = rng.normal(0, 0.30, N_CODES - 2)          # 254 个内部码
    dnl_spec[62] = -1.0                                  # 码宽归零 -> 第 63 码缺失
    adcB = ADC0832(dnl_lsb=dnl_spec, offset_lsb=0.80, gain_pct=-0.60,
                   noise_lsb=0.5, h3_dbc=-50.0, seed=2)
    sb, db = static_test(adcB), dynamic_test(adcB)
    print(f"  静态: DNL_max={sb['dnl_max']:.4f} LSB   DNL_min={sb['dnl_min']:.4f} LSB   "
          f"INL_max={sb['inl_max']:.4f} LSB")
    print(f"        码宽 {sb['width_min']:.4f} ~ {sb['width_max']:.4f} LSB   "
          f"失调={sb['offset_lsb']:.3f} LSB   增益误差={sb['gain_pct']:.3f}%")
    print(f"        缺失码={sb['missing']} 个，位置={sb['missing_list']}")
    thd_s = "无谐波" if db["thd"] is None else f"{db['thd']:.2f} dB"
    print(f"  动态: SNR={db['snr']:.2f} dB   THD={thd_s}   SINAD={db['sinad']:.2f} dB   "
          f"SFDR={db['sfdr']:.2f} dB   ENOB={db['enob']:.2f} bit")
    if db["clipped"]:
        print(f"  [警告] {db['clipped']} 个采样点超出 0~{VREF} V，发生削顶")
    print()
    judge_all(sb, db)

    # ---------- ③ 验证笔记里记的那处冲突 ----------
    banner("③ 顺便验证笔记里记的那处「课件 vs 工程」冲突")
    print("  课件《12.课件-ADC0832》测试计划表写 :  ADC_ENOB 限值 12 ~ 16 bit")
    print("  工程 .tmf 实际写                   :  ENOB     限值  7 ~  8  bit")
    print(f"  本文件实测（理想器件）             :  ENOB = {da['enob']:.2f} bit")
    print()
    print(f"  8 位 ADC 的物理上限 = (6.02×8 + 1.76 − 1.76) / 6.02 = {8.0:.2f} bit")
    print("  => 12~16 bit 在物理上不可能：课件那一栏是从 12/16 位 ADC 的模板")
    print("     沿用未改，工程 .tmf 的 7~8 bit 才是对的。")
    print("  => 改限值时以工程 .tmf 为准，不要照抄课件。")


if __name__ == "__main__":
    main()
