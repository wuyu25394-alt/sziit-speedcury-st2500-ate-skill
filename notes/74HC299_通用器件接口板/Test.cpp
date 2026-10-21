// ============================================================================
//  74HC299 —— 8 位通用移位寄存器（三态输出）测试程序
//  平台：加速科技 MINI-ATE（ST2500 / ST2516）
//  接口板：通用器件接口板（需自行用杜邦线搭建测试电路）
//
//  资料来源：
//    · 器件参数：Nexperia 74HC299 数据手册 Rev.5（2019-01-17）
//    · 工程体系与 API：ic-testing-notes 技能（ST2500 工程文件体系 / API 手册 V2.2）
//    · 代码风格：CD4511_v1 模板工程
//
//  ⚠️ 本工程名 HC299_v1，属课程体系外的第 7 个器件，限值由数据手册推导，
//     任何相冲突之处以实测 + 数据手册为准。详见 README 说明。
// ============================================================================

#include "interface.h"

// ---------------------------------------------------------------------------
//  引脚与测试机资源对应关系（须与 HC299_v1.sig 一致）
//  ---------------------------------------------------------------------------
//    Pin 1  S0      -> DIO 1:1:1     模式选择
//    Pin 2  OE1     -> DIO 1:1:2     三态使能，低有效
//    Pin 3  OE2     -> DIO 1:1:3     三态使能，低有效
//    Pin 4  IO6     -> DIO 1:1:4     并行数据 I/O
//    Pin 5  IO4     -> DIO 1:1:5
//    Pin 6  IO2     -> DIO 1:1:6
//    Pin 7  IO0     -> DIO 1:1:7
//    Pin 8  Q0      -> DIO 1:1:8     串行输出（普通输出）
//    Pin 9  MR      -> DIO 1:1:9     异步主复位，低有效
//    Pin 10 GND     -> 地            不经测试机
//    Pin 11 DSR     -> DIO 1:1:10    右移串行输入
//    Pin 12 CP      -> DIO 1:1:11    时钟，上升沿
//    Pin 13 IO1     -> DIO 1:1:12
//    Pin 14 IO3     -> DIO 1:1:13
//    Pin 15 IO5     -> DIO 1:1:14
//    Pin 16 IO7     -> DIO 1:1:15
//    Pin 17 Q7      -> DIO 1:1:16    串行输出（普通输出）
//    Pin 18 DSL     -> DIO 1:1:17    左移串行输入
//    Pin 19 S1      -> DIO 1:1:18    模式选择
//    Pin 20 VCC     -> DPS 1:1:0     器件电源 5 V
//
//  信号组（须与 HC299_v1.grp 一致，组间成员不重叠）：
//    INPUT_PINS  = S0, S1, MR, CP, DSR, DSL      6 个纯输入（IIL 测量对象）
//    CTRL_PINS   = OE1, OE2                      2 个三态使能
//    IO_PINS     = IO0..IO7                      8 个双向数据口
//    Q_PINS      = Q0, Q7                        2 个串行输出
//    OUT_PINS    = IO_PINS + Q_PINS  = 10 个     所有输出（VOH/VOL 对象）
//    ALL_PINS    = INPUT_PINS + CTRL_PINS + IO_PINS + Q_PINS
//                = 6 + 2 + 8 + 2 = 19 个          全部 DIO（OS 对象）
//
//  参数（ParamNumber）累计顺序 —— 必须与 .tmf 的 ResultAnalysis 完全一致：
//    OSN_T  0  ~ 18   (19 个：ALL_PINS 顺序)
//    OSP_T  19 ~ 37   (19 个)
//    IIL_T  38 ~ 43   ( 6 个：INPUT_PINS)
//    VOH_T  44 ~ 53   (10 个：OUT_PINS)
//    VOL_T  54 ~ 63   (10 个：OUT_PINS)
//    IOZ_T  64 ~ 71   ( 8 个：IO_PINS)
//    IDD_T  72        ( 1 个)
//    FUNC_T 73        ( 1 个)
// ---------------------------------------------------------------------------


// ===========================================================================
//  一、公共辅助函数
//  注意：本平台 DPS 只支持 FV 模式（FVMV / FVMI / FNMV），不支持 FIMV/FIMI。
//        凡"强流测压"必须用 ppmu；凡"供电"用水 dps。
// ===========================================================================

// 上下电：VCC = 5 V
void PowerOnDevice(double dbVolt)
{
    dps.Signal("VCC").SetMode("FVMI")
        .VoltForce(0.0)
        .CurrRange(0.5)              // 先大量程，防上电冲击
        .CurrClamp(0.5, -0.5)
        .Execute();
    dps.Signal("VCC").Connect();
    sys.DelayUs(8000);               // 机械 relay 稳定
    dps.Signal("VCC").SetMode("FVMI")
        .VoltForce(dbVolt)
        .CurrRange(0.5)
        .CurrClamp(0.5, -0.5)
        .Execute();
    sys.DelayUs(2000);
}

// 下电与全通道复位
void PowerOffDevice()
{
    // 各 DIO 先拉到 0 V，避免带电拔插
    ppmu.Signal("ALL_PINS").SetMode("FVMI")
        .VoltForce(0.0)
        .CurrRange(40e-3)
        .VoltClamp(6.0, -2.0)
        .Execute();
    sys.DelayUs(1000);
    // 转成"不驱动"状态收尾
    ppmu.Signal("ALL_PINS").SetMode("FNMV").Execute();
    sys.DelayUs(1000);
    ppmu.Signal("ALL_PINS").DisConnect();

    dps.Signal("VCC").SetMode("FVMI")
        .VoltForce(0.0).CurrRange(0.5).CurrClamp(0.5, -0.5)
        .Execute();
    sys.DelayUs(1000);
    dps.Signal("VCC").DisConnect();
    sys.DelayUs(8000);
}

// 数字电平设置：5 V 逻辑
void SetLevel(double dbVih)
{
    pinlevel.Signal("ALL_PINS").SetMode("DIO")
        .SetVih(dbVih)               // 5.0
        .SetVil(0.0)
        .SetVoh(dbVih * 0.7)         // 3.5  判高门限
        .SetVol(dbVih * 0.3)         // 1.5  判低门限
        .VoltClamp(6.0, -2.0)
        .Execute();
    sys.DelayUs(1000);
}

// 把控制脚强制到指定电平（用 PPMU 的 FVMI，强压测流、不关心电流）
void ForceCtrl(const char* strSig, double dbVolt)
{
    ppmu.Signal(strSig).SetMode("FVMI")
        .VoltForce(dbVolt)
        .CurrRange(40e-3)
        .VoltClamp(6.0, -2.0)
        .Execute();
}


// ===========================================================================
//  二、DC 参数测试项
// ===========================================================================

// ---------------------------------------------------------------------------
//  0. OSN_T —— 负向开短路：所有 DIO 抽 -100 uA，测二极管压降
//     期望 -1.2 ~ -0.2 V（与 CD4511 案例同口径）
//     ⚠️ 测前 VCC 必须置 0 V，否则电源脚保护二极管会引入额外通路
// ---------------------------------------------------------------------------
USER_CODE void OSN_T()
{
    vector<ST_MEAS_RESULT> OSN_RESULT;

TEST_BEGIN
    PowerOnDevice(0.0);              // VCC = 0 V

    ppmu.Signal("ALL_PINS").Connect();
    // 先全部拉到 0 V，消除残余电荷
    ppmu.Signal("ALL_PINS").SetMode("FVMV")
        .VoltForce(0.0)
        .CurrRange(40e-3)
        .VoltClamp(2.0, -2.0)
        .Execute();
    sys.DelayUs(2000);

    // 抽电流 -100 uA，测电压
    ppmu.Signal("ALL_PINS").SetMode("FIMV")
        .CurrForce(-100e-6)
        .CurrRange(100e-6)
        .VoltClamp(2.0, -2.0)
        .Execute();
    sys.DelayUs(2000);

    ppmu.Measure(OSN_RESULT);
    ppmu.Signal("ALL_PINS").SetMode("FNMV").Execute();
    ppmu.Signal("ALL_PINS").DisConnect();
    PowerOffDevice();

    binObj.CheckResultAndBin(0, OSN_RESULT, 19);

TEST_ERROR
    binObj.HandlerException(0);
TEST_END
}

// ---------------------------------------------------------------------------
//  1. OSP_T —— 正向开短路：所有 DIO 灌 +100 uA
//     期望 0.2 ~ 1.2 V
// ---------------------------------------------------------------------------
USER_CODE void OSP_T()
{
    vector<ST_MEAS_RESULT> OSP_RESULT;

TEST_BEGIN
    PowerOnDevice(0.0);

    ppmu.Signal("ALL_PINS").Connect();
    ppmu.Signal("ALL_PINS").SetMode("FVMV")
        .VoltForce(0.0).CurrRange(40e-3).VoltClamp(2.0, -2.0)
        .Execute();
    sys.DelayUs(2000);

    ppmu.Signal("ALL_PINS").SetMode("FIMV")
        .CurrForce(+100e-6)
        .CurrRange(100e-6)
        .VoltClamp(2.0, -2.0)
        .Execute();
    sys.DelayUs(2000);

    ppmu.Measure(OSP_RESULT);
    ppmu.Signal("ALL_PINS").SetMode("FNMV").Execute();
    ppmu.Signal("ALL_PINS").DisConnect();
    PowerOffDevice();

    binObj.CheckResultAndBin(19, OSP_RESULT, 19);   // 起始 ParamNumber = 19

TEST_ERROR
    binObj.HandlerException(0);
TEST_END
}

// ---------------------------------------------------------------------------
//  2. IIL_T —— 输入低电平漏电流
//     6 个纯输入脚 S0, S1, MR, CP, DSR, DSL 强压 0 V，测电流
//     手册 II：±1 uA（VCC = 6 V，25 ℃）；本测 VCC = 5 V
//     ⚠️ 先大量程再切 2 uA 量程，防上电/电容瞬态超量程
// ---------------------------------------------------------------------------
USER_CODE void IIL_T()
{
    vector<ST_MEAS_RESULT> IIL_RESULT;

TEST_BEGIN
    PowerOnDevice(5.0);

    ppmu.Signal("INPUT_PINS").Connect();
    ppmu.Signal("INPUT_PINS").SetMode("FVMI")
        .VoltForce(0.0)
        .CurrRange(40e-3)            // 第一步：大量程
        .VoltClamp(6.0, -2.0)
        .Execute();
    sys.DelayUs(3000);

    ppmu.Signal("INPUT_PINS").SetMode("FVMI")
        .VoltForce(0.0)
        .CurrRange(2e-6)             // 第二步：切小量程
        .VoltClamp(6.0, -2.0)
        .Execute();
    sys.DelayUs(3000);

    ppmu.Measure(IIL_RESULT);
    ppmu.Signal("INPUT_PINS").SetMode("FNMV").Execute();
    ppmu.Signal("INPUT_PINS").DisConnect();
    PowerOffDevice();

    binObj.CheckResultAndBin(38, IIL_RESULT, 6);    // 起始 19+19 = 38

TEST_ERROR
    binObj.HandlerException(0);
TEST_END
}

// ---------------------------------------------------------------------------
//  3. VOH_T —— 输出高电平电压
//     技法：用"并行置数"把 8 个 I/O 全置 1，不改向量就能测 VOH
//        MR=0 -> 清 0；MR=1, S1=H, S0=H -> 并行置数；CP 上升沿锁存
//     条件：抽 -20 uA（手册 VOH 的测试条件），要求 >= 4.4 V（@5 V）
// ---------------------------------------------------------------------------
USER_CODE void VOH_T()
{
    vector<ST_MEAS_RESULT> VOH_RESULT;

TEST_BEGIN
    PowerOnDevice(5.0);

    // 1) 先让 8 个 I/O 作为"输入"驱动全 1，再置数进去
    ppmu.Signal("IO_PINS").Connect();
    ppmu.Signal("IO_PINS").SetMode("FVMI")
        .VoltForce(5.0)
        .CurrRange(40e-3)
        .VoltClamp(6.0, -2.0)
        .Execute();
    ForceCtrl("OE1", 0.0);           // 使能输出（低有效）
    ForceCtrl("OE2", 0.0);
    ForceCtrl("MR",  0.0);           // 复位
    ForceCtrl("S1",  5.0);
    ForceCtrl("S0",  5.0);           // 并行置数模式
    sys.DelayUs(3000);
    ForceCtrl("MR",  5.0);           // 解除复位
    sys.DelayUs(1000);
    ForceCtrl("CP",  0.0);
    sys.DelayUs(1000);
    ForceCtrl("CP",  5.0);           // 上升沿：I/On -> Qn，全 1 装入
    sys.DelayUs(3000);
    ForceCtrl("CP",  0.0);
    sys.DelayUs(1000);

    // 2) 松开 I/O 的驱动，改为比较/测量
    ppmu.Signal("IO_PINS").SetMode("FNMV").Execute();
    sys.DelayUs(2000);

    // 3) 抽 -20 uA 测 VOH
    ppmu.Signal("OUT_PINS").SetMode("FIMV")
        .CurrForce(-20e-6)
        .CurrRange(100e-6)
        .VoltClamp(6.0, 0.0)
        .Execute();
    sys.DelayUs(3000);

    ppmu.Measure(VOH_RESULT);
    ppmu.Signal("OUT_PINS").SetMode("FNMV").Execute();
    ppmu.Signal("OUT_PINS").DisConnect();
    PowerOffDevice();

    binObj.CheckResultAndBin(44, VOH_RESULT, 10);   // 含 IO0..7 + Q0,Q7

TEST_ERROR
    binObj.HandlerException(0);
TEST_END
}

// ---------------------------------------------------------------------------
//  4. VOL_T —— 输出低电平电压
//     技法：MR 清零后保持（MR=1, S1=L, S0=L -> hold），全 0 输出
//     条件：灌 +20 uA，要求 <= 0.1 V（@5 V）
// ---------------------------------------------------------------------------
USER_CODE void VOL_T()
{
    vector<ST_MEAS_RESULT> VOL_RESULT;

TEST_BEGIN
    PowerOnDevice(5.0);

    ppmu.Signal("IO_PINS").Connect();
    ForceCtrl("OE1", 0.0);
    ForceCtrl("OE2", 0.0);
    ForceCtrl("MR",  0.0);           // 异步复位 -> 全 0
    sys.DelayUs(3000);
    ForceCtrl("MR",  5.0);           // 释放复位
    ForceCtrl("S1",  0.0);
    ForceCtrl("S0",  0.0);           // 保持模式，锁住全 0
    sys.DelayUs(3000);

    ppmu.Signal("OUT_PINS").SetMode("FIMV")
        .CurrForce(+20e-6)
        .CurrRange(100e-6)
        .VoltClamp(6.0, -1.0)
        .Execute();
    sys.DelayUs(3000);

    ppmu.Measure(VOL_RESULT);
    ppmu.Signal("OUT_PINS").SetMode("FNMV").Execute();
    ppmu.Signal("OUT_PINS").DisConnect();
    PowerOffDevice();

    binObj.CheckResultAndBin(54, VOL_RESULT, 10);

TEST_ERROR
    binObj.HandlerException(0);
TEST_END
}

// ---------------------------------------------------------------------------
//  5. IOZ_T —— 三态输出关断漏电流（74HC299 特有，必测）
//     条件：OE1 = 5 V（HIGH 关断），S1/S0 = 1/1 并行置数，I/O 脚加 5 V
//     期望：|I| <= 5 uA（手册 IOZ，VCC = 6 V；本测 5 V）
//     ⚠️ 这是三态器件最容易漏测的一项 —— 关不断或关断漏电大都出在这里
// ---------------------------------------------------------------------------
USER_CODE void IOZ_T()
{
    vector<ST_MEAS_RESULT> IOZ_RESULT;

TEST_BEGIN
    PowerOnDevice(5.0);

    // 关断输出：OE1 拉高
    ForceCtrl("OE1", 5.0);
    ForceCtrl("OE2", 0.0);           // OE1 高即可关断（二者是 OR 关系）
    ForceCtrl("S1",  5.0);
    ForceCtrl("S0",  5.0);           // 置数模式
    sys.DelayUs(3000);

    // I/O 脚加 5 V，测流入电流（不能低于 VCC 之外的范围）
    ppmu.Signal("IO_PINS").Connect();
    ppmu.Signal("IO_PINS").SetMode("FVMI")
        .VoltForce(5.0)
        .CurrRange(2e-6)
        .VoltClamp(6.0, -2.0)
        .Execute();
    sys.DelayUs(4000);

    ppmu.Measure(IOZ_RESULT);
    ppmu.Signal("IO_PINS").SetMode("FNMV").Execute();
    ppmu.Signal("IO_PINS").DisConnect();
    PowerOffDevice();

    binObj.CheckResultAndBin(64, IOZ_RESULT, 8);

TEST_ERROR
    binObj.HandlerException(0);
TEST_END
}

// ---------------------------------------------------------------------------
//  6. IDD_T —— 静态电源电流
//     条件：输入全到 VCC 或 GND、无负载；手册 ICC <= 8 uA（@6 V, 25 ℃）
// ---------------------------------------------------------------------------
USER_CODE void IDD_T()
{
    vector<ST_MEAS_RESULT> IDD_RESULT;

TEST_BEGIN
    // 先把所有输入置于确定电平（不悬空），否则 IDD 会偏大
    PowerOnDevice(5.0);
    ForceCtrl("MR",  5.0);
    ForceCtrl("S1",  0.0);
    ForceCtrl("S0",  0.0);           // 保持模式，内部无翻转
    ForceCtrl("CP",  0.0);
    ForceCtrl("DSR", 0.0);
    ForceCtrl("DSL", 0.0);
    sys.DelayUs(3000);

    // 切到小电流量程测 IDD
    dps.Signal("VCC").SetMode("FVMI")
        .VoltForce(5.0)
        .CurrRange(5e-6)
        .CurrClamp(5e-6, -5e-6)      // 先设 CurrRange 再设 CurrClamp
        .Execute();
    sys.DelayUs(5000);

    dps.Signal("VCC").Measure(IDD_RESULT);
    PowerOffDevice();

    binObj.CheckResultAndBin(72, IDD_RESULT, 1);

TEST_ERROR
    binObj.HandlerException(0);
TEST_END
}


// ===========================================================================
//  三、功能测试项（Pattern）
// ===========================================================================

// ---------------------------------------------------------------------------
//  7. FUNC_T —— 功能向量：置数 / 右移 / 左移 / 保持 / 复位 / 三态
//     向量文件：pattern\func.csv     时序表：WFT1（见 HC299_v1.tim）
// ---------------------------------------------------------------------------
USER_CODE void FUNC_T()
{
    vector<ST_DIO_RESULT> FUNC_RESULT;

TEST_BEGIN
    PowerOnDevice(5.0);
    SetLevel(5.0);

    dio.Timing().Load("D:\\MINI-ATE\\workspace\\HC299_v1\\HC299_v1.tim");
    dio.Pattern("FUNC").Load("D:\\MINI-ATE\\workspace\\HC299_v1\\pattern\\func.csv");
    sys.DelayUs(2000);

    dio.Pattern("FUNC").SetTimeOut(20000).Run().WaitEnd();
    dio.Pattern("FUNC").GetFailResult(FUNC_RESULT);

    dio.Pattern("FUNC").UnLoad();
    dio.Timing().UnLoad();
    PowerOffDevice();

    binObj.CheckResultAndBin(73, FUNC_RESULT, 1);

TEST_ERROR
    binObj.HandlerException(0);
TEST_END
}


// ===========================================================================
//  四、Pass 置位
// ===========================================================================
USER_CODE void SetPassBin()
{
TEST_BEGIN
    binObj.SetBin(1);
TEST_ERROR
    binObj.HandlerException(0);
TEST_END
}
