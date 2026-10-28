# 74HC138 逐句注释源码

本目录保存 `74HC138\Test.cpp` 的**逐句注释版**，用于对照 [5.9 笔记](../5.9-74HC138三线译码器（扩展案例）.md) 阅读。

## 文件

| 文件 | 说明 |
|---|---|
| `Test.cpp` | 1089 行，约 34 KB；在原始 871 行代码上插入约 290 条整行中文注释，**代码逻辑一字未改** |
| `README.md` | 本说明 |

## 注释约定

- 每条注释都以 `//` 整行插入，不修改任何语句、缩进和空行；
- 注释分三层：
  1. **文件头**：整个测试程序的调用关系；
  2. **函数头**：测试项的目的、条件、限值；
  3. **语句级**：每个 `Connect / SetMode / VoltForce / Measure / CheckResultAndBin` 的作用与参数含义；
- 多行链式调用只在**第一条**前写总注释，链内参数用行内/行前短注释补充。

## 建议阅读顺序（与 TMF 一致）

| 顺序 | 函数 | 看什么 |
|---|---|---|
| 1 | `FunctionalPatternTest` | 功能测试的公共骨架：上电 → 设电平 → 跑 Pattern → 下电 → 判 Bin |
| 2 | `OSN` / `OSP` | OS 为什么必须 VCC=0、±100 µA 与 ±1.2 V 判据 |
| 3 | `ICC` | 为什么先 20 mA 充电再切 20 µA 精测 |
| 4 | `VOH` / `VOL` | 电流方向：VOH 抽 −4 mA，VOL 灌 +4 mA |
| 5 | `IIH` / `IIL` | FVMI 强制 6 V / 0 V 测漏电 |
| 6 | `FUNC` / `DEMUX` / `FUNC_LIM` / `FUNC_2V` / `FUNC_6V` | 同一个 Pattern 在不同 VCC/电平下的复用 |
| 7 | `MeasureTpd` / `Tpd` | TMU 主从通道、loop_inf Hold、同相/反相斜率 |
| 8 | `MeasureTt` / `Tt` | 10%/90% 阈值与 RiseTime/FallTime |
| 9 | `SetPassBin` | 全部通过后置 Pass Bin |

## 关键点速查

| 测试 | 关键代码 | 说明 |
|---|---|---|
| OS | `SetMode("FIMV").CurrForce(±100e-6)` | 强制电流、测电压；VCC 必须 0 V |
| ICC | `FVMI 6V, 20mA -> 20uA` | 先大后小，避开上电瞬态 |
| VOH | `FIMV CurrForce(-4e-3)` | 电流流出 DUT，测 3.84~4.4 V |
| VOL | `FIMV CurrForce(+4e-3)` | 电流流入 DUT，测 0~0.33 V |
| IIH/IIL | `FVMI VoltForce(6/0)` | 强制电压、测电流，±1 µA |
| Tpd | `PinToPinDelay` + `loop_inf` | TMU0→输入、TMU1→Y0，0~38 ns |
| Tt | `RiseTime/FallTime` | 0.45 V / 4.05 V，0~19 ns |

## 编译验证

```powershell
cd D:\MINI-ATE\workspace\74HC138
make
```

注释版已用 `g++ -std=c++17 -Wall` 编译通过，生成 `Test.dll`；注释只影响可读性，不改变测试逻辑。

## 与 5.9 笔记的关系

- 笔记讲**为什么**：器件、TMF 限值、Pattern、踩坑；
- 本目录讲**每一行在做什么**：逐句对照代码；
- 两者配合，适合作为“从工程源码学 ATE 测试开发”的入门材料。
