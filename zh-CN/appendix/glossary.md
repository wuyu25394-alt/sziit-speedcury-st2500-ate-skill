# 附录 A　术语总表

全书用词以这张表为准。按主题分组，每条给中英对照与一句话说明；缩写第一次在正文出现时，都按「中文（English，缩写）」的格式写。

---

## A.1 测试阶段与设备

| 缩写 | 英文全称 | 中文 | 说明 |
|---|---|---|---|
| **ATE** | Automatic Test Equipment | 自动测试设备 / 测试机 | 整机系统，含测试机本体、外围设备、接口板与程序 |
| **DUT** | Device Under Test | 被测器件 | 当前被夹在夹具里的那一颗芯片 |
| **CP** | Circuit Probing | 晶圆测试 / 中测 | 在整片晶圆上逐颗裸片测 |
| **FT** | Final Test | 成品测试 / 成测 | 封装完成后测成品 |
| **Prober** | — | 探针台 | 中测设备，负责移动晶圆、告知坐标、给失效品打墨点 |
| **Handler** | — | 分选机 | 成测设备，负责自动送料、按结果分等级 |
| **PIB** | Probe Interface Board | 探针接口板 | 中测用治具 |
| **Probe Card** | — | 针卡 | 把测试机资源引到晶圆裸片的焊盘上 |
| **DIB** | Device Interface Board | 负载板 / Load Board | 成测用治具，把资源引到插座 |
| **Socket** | — | 测试座 | 成测时固定芯片引脚 |
| **Docking** | — | 对接 | 测试机与探针台/分选机的机械与信号连接 |
| **Die** | — | 裸片 / 晶粒 | 晶圆上的一颗芯片 |
| **EOT** | End of Test | 测试结束信号 | 测试机通知外围设备可以分选 |
| **Site** | — | 并测工位 | 一次同时测几颗；本平台最多 1024 |

---

## A.2 产业链

| 缩写 | 全称 | 说明 |
|---|---|---|
| **IDM** | Integrated Device Manufacturer | 设计、制造、封测、销售全包 |
| **Fabless** | — | 只做设计，不建厂 |
| **Foundry** | — | 晶圆代工 |
| **OSAT** | Outsourcing Assembly and Test | 委外封装与测试 |
| **EDA** | Electronic Design Automation | 电子设计自动化工具 |
| **IP** | Intellectual Property | 可复用的电路模块授权 |

---

## A.3 测试机硬件与资源

| 术语 | 含义 |
|---|---|
| **SCB** | 主控板（System Control Board），管上下电、按键、电压温度监控与风扇 |
| **DFB32** | 主力业务板，一块板上集成 DIO、PPMU、BPMU、DPS、RVS、CBIT、AWG、DGT、TMU 九类模拟前端模块 |
| **MWB8** | 音频板，低频高精度的 AWG 与 DGT |
| **SMU20** | SMU 板卡型号，独立的 4 通道高压电源板 |
| **AFE** | Analog Front End，模拟前端模块，业务板上按功能划分的电路单元 |
| **Adapter** | 机头，级联系统里第几台；最多 8 个 |
| **FBID / Slot** | 机头内的业务板编号，Slot1～Slot5 |
| **Channel** | AFE 模块内的通道号，**从 0 开始** |
| **Online / OffLine** | 板卡在位 / 未在位 |

**通道寻址**：`Adapter : FBID(Slot) : Channel`，例如 `1:1:6` 表示 1 号机头、Slot1、该资源的 6 号通道。**不含资源类型层**，资源类型由 `.sig` 的 `AFEType` 单独指定。

---

## A.4 资源类型（`AFEType` 可选值）

| 类型 | 中文 | 一句话 |
|---|---|---|
| **DIO** | 数字输入/输出 | 输出 Pattern、比较输入电平；通道数最多 |
| **PPMU** | 每引脚测量单元 | 挂在每个 DIO 通道上，做开短路与漏电；两线 |
| **BPMU** | 板载测量单元 | 精度高于 PPMU，两种钳位都有 |
| **DPS** | 器件电源 | 给芯片供电；**不支持强制电流**；可 Gang 并联 |
| **RVS** | 参考电压源 | 只给参考电压，无钳位，单档 50 mA |
| **SMU** | 源测量单元 | 独立高压板卡，源测一体、四象限；精度最高 |
| **CBIT** | 继电器控制 | 控制 DUT 板上的继电器，切换送到引脚上的资源 |
| **AWG** | 任意波形发生器 | 生成模拟激励，如 PSRR 项叠加的纹波 |
| **DGT** | 数字采集 | 采集模拟波形 |
| **TMU** | 时间测量单元 | 测频率、周期、脉宽 |
| **Gang** | 并联 | 多个同类通道并联输出更大电流，只能并相邻通道 |

---

## A.5 工程文件

| 文件 | 内容 | 谁维护 |
|---|---|---|
| `.job` | 工程入口，串起其余文件 | 平台生成 |
| `.res` | 资源自检结果，决定能选哪些 `AFEType` | 平台自检生成 |
| `.sig` | 引脚与资源定义（含 `AFEType` 与通道三元组） | 人编辑 |
| `.grp` | 引脚分组 | 人编辑 |
| `.tim` | 时序：周期、边沿、驱动与比较的时机 | 人编辑 |
| `.pat` | 图形（Pattern）：每一拍的输入与期望输出 | 人编辑或转换生成 |
| `.tmf` | 测试流程：Site 层 + Func 层，含测试项编号、名字、Bin 与限值 | 人编辑 |
| `Test.cpp` | 用户代码，编译成 `Test.dll` | 人写 |
| `interface.h` | 代码与平台对接的声明 | 平台提供 |

---

## A.6 测量模式

命名规则：`F` 表示 Force（强制），`M` 表示 Measure（测量），后面的 `V` / `I` / `N` 分别表示电压、电流、不测。

| 模式 | 强制 | 测量 | 该配的钳位 |
|---|---|---|---|
| `FVMV` | 电压 | 电压 | 电流 |
| `FVMI` | 电压 | 电流 | 电流 |
| `FIMV` | 电流 | 电压 | 电压 |
| `FIMI` | 电流 | 电流 | 电压 |
| `FNMV` | 不强制 | 电压 | — |
| `FVMIMV` | 电压 + 钳位 | 电流 | 两者 |
| `FIMVMI` | 电流 + 钳位 | 电压 | 两者 |

**注意**：没有 `FMIM` 这种写法。混合模式 `FVMIMV` / `FIMVMI` 只有 SMU 支持；DPS 不支持任何 `FIM` 开头的模式。

---

## A.7 直流参数

| 缩写 | 含义 | 典型条件 |
|---|---|---|
| **OS** | Open/Short，开短路 | 灌电流读电压，看二极管压降 |
| **IIL** | Input Low Current，输入低电平漏电流 | 输入给 0 V，测流出的电流 |
| **IIH** | Input High Current，输入高电平漏电流 | 输入给 VDD，测流入的电流 |
| **IOZ** | 三态输出漏电流 | 输出置高阻，测漏电 |
| **VOL** | Output Low Voltage，输出低电平 | 灌规定电流时测电压 |
| **VOH** | Output High Voltage，输出高电平 | 拉规定电流时测电压 |
| **IOL** | Output Low Current，低电平灌电流能力 | 强制低电平，测能灌多少 |
| **IOH** | Output High Current，高电平拉电流能力 | 强制高电平，测能拉多少 |
| **IDD / ICC** | 电源电流 | 规定电压下测总电流，分静态与动态 |
| **IGND** | 地引脚电流 | 判断地回路是否正常 |

---

## A.8 模拟与电源参数

| 缩写 | 含义 |
|---|---|
| **VOS** | Input Offset Voltage，输入失调电压 |
| **AVD** | Large Signal Voltage Gain，开环电压增益 |
| **CMRR** | Common Mode Rejection Ratio，共模抑制比 |
| **PSRR** | Power Supply Rejection Ratio，电源抑制比 |
| **VDO** | Dropout Voltage，压差（输入掉到多低输出还稳得住） |
| **IQ** | Quiescent Current，静态电流 |
| **LDO** | Low Dropout Regulator，低压差线性稳压器 |
| **EN** | Enable，使能引脚 |
| **I-V 曲线** | 电流随电压变化的曲线，扫描得到 |

---

## A.9 数字、时序与 ADC 参数

| 缩写 | 含义 |
|---|---|
| **VIH / VIL** | 输入高/低电平门限 |
| **VOH / VOL** | 输出高/低电平（数字判据） |
| **Pattern** | 图形：每一拍的输入与期望输出 |
| **Vector** | 向量：Pattern 里的一拍 |
| **PinLevel** | 引脚电平配置（驱动电平与比较电平） |
| **Timing** | 时序配置（周期、边沿、采样时刻） |
| **tPD** | Propagation Delay，传输延时 |
| **tR / tF** | Rise / Fall Time，上升/下降时间 |
| **tSU / tH** | Setup / Hold Time，建立/保持时间 |
| **fmax** | 最高工作频率 |
| **Shmoo** | 二维 pass/fail 图，横轴电压、纵轴时序或频率 |
| **DNL** | Differential Non-Linearity，微分非线性（一个码宽差多少 LSB） |
| **INL** | Integral Non-Linearity，积分非线性（偏差累积到多少 LSB） |
| **LSB** | Least Significant Bit，最低有效位 |
| **SNR** | Signal-to-Noise Ratio，信噪比 |
| **THD** | Total Harmonic Distortion，总谐波失真 |
| **ENOB** | Effective Number of Bits，有效位数；**受器件物理上限约束** |
| **SAR** | Successive Approximation Register，逐次逼近型 ADC |

---

## A.10 数据与良率

| 术语 | 含义 |
|---|---|
| **Bin** | 按测试结果给芯片分的类别 |
| **HBIN** | Hard Bin，硬件分选箱号 / 等级 |
| **SBIN** | Soft Bin，软件层面的失效原因细分 |
| **Yield** | 良率 = 合格数 / 总数；分一次良率与最终良率 |
| **Datalog** | 每颗芯片留下的测试记录 |
| **STDF** | Standard Test Data Format，量产测试结果的通用数据格式 |
| **Cpk** | 过程能力指数，衡量分布离判据边界有多远 |
| **相关性比对** | 两台机器/两个工位测同一批货，比较结果差异 |
| **Guard Band** | 在手册限值内侧额外留出的安全余量 |

---

## A.11 单位与数量级

| 量 | 单位 | 换算 |
|---|---|---|
| 电压 | V、mV、µV | 1 V = 10³ mV = 10⁶ µV |
| 电流 | A、mA、µA、nA、pA | 1 A = 10³ mA = 10⁶ µA = 10⁹ nA |
| 电阻 | Ω、kΩ、MΩ | 1 MΩ = 10³ kΩ = 10⁶ Ω |
| 电容 | F、pF、nF、µF | 1 µF = 10³ nF = 10⁶ pF |
| 时间 | s、ms、µs、ns | 1 s = 10³ ms = 10⁶ µs = 10⁹ ns |
| 频率 | Hz、kHz、MHz、GHz | 1 GHz = 10³ MHz = 10⁶ kHz |

**读参数时的三个习惯**：先看单位、再看条件、最后看数值。单位错一个数量级，判据就全废了。

---

[← 附录 首页](README.md) ｜ [目录](../README.md) ｜ [附录 B　现场踩坑清单 →](pitfalls.md)
