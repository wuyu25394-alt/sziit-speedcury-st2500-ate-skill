// ============================================================================
// 74HC138 测试程序（Test.cpp）
// ----------------------------------------------------------------------------
// 本文件由 MINI-ATE Runner 加载，按 74HC138.tmf 的 FuncList 顺序调用各
// USER_CODE 函数；每个函数就是一个测试项，函数内部完成：
//   上电 -> 连接资源 -> 设置模式/电平 -> 测量 -> 判 Bin -> 下电/断开
// 测试项与 TMF 的对应关系：
//   OSN/OSP  开短路      ICC 静态电流     IIH/IIL 输入漏电
//   VOH/VOL  输出高低电平                   FUNC/DEMUX/FUNC_LIM/FUNC_2V/FUNC_6V 功能
//   Tpd      传输延时（TMU）                Tt 输出转换时间（TMU）
//   SetPassBin 全部通过后置 Pass Bin
// ============================================================================
#include "interface.h"
// 包含 ATE 公共接口：dps/ppmu/pinlevel/dio/tmu/binObj/sys 等全局对象都在这里声明
#include <iostream>
// 标准输入输出，ProjectInit 等钩子里用 cout 打印提示
#include <string>
// 标准字符串，用于测试项名称/错误信息
#include <vector>
// vector 容器，存放 Measure 返回的测量结果和 Pattern 的失败结果

// 使用 std 命名空间，省去 std:: 前缀（与仓库其他工程保持一致）
using namespace std;

// 站点信息：SITE 当前站点号，SITE_ID 站点 ID，ACTIVE_SITE_NUM 并测站点数；单站测试固定为 0
unsigned long SITE = 0, SITE_ID = 0, ACTIVE_SITE_NUM = 0;

// 74HC138 管脚数量定义
// 输入脚数量：A0/A1/A2 三个地址 + E1/E2/E3 三个使能，共 6 个
const int INPUT_PIN_NUM  = 6;   // A0,A1,A2,E1,E2,E3
// 输出脚数量：Y0~Y7 共 8 个（74HC138 是 3-8 译码器）
const int OUTPUT_PIN_NUM = 8;   // Y0 ~ Y7
// I/O 总数 = 输入 6 + 输出 8 = 14，OS 测试会对这 14 个脚逐个测
const int IO_PIN_NUM     = 14;  // INPUT_PIN_NUM + OUTPUT_PIN_NUM

// VOL 测试使用的 Pattern 与对应输出管脚
// VOL 测试用的 8 个 Pattern 名称：每个 Pattern 只让对应的 Yn 输出低电平
const char* VOL_PAT[OUTPUT_PIN_NUM] = {"VOL_Y0","VOL_Y1","VOL_Y2","VOL_Y3",
									   "VOL_Y4","VOL_Y5","VOL_Y6","VOL_Y7"};
// VOL_SIG 与 VOL_PAT 一一对应：VOL_PAT[n] 选通时，VOL_SIG[n] 就是低电平脚
const char* VOL_SIG[OUTPUT_PIN_NUM] = {"Y0","Y1","Y2","Y3","Y4","Y5","Y6","Y7"};

// Tpd 测试使用的 DIO 通道号（与 74HC138.sig 中的定义一致）
// A0 在 .sig 中映射到 DIO 通道 0，Tpd 测量时把它接到 TMU0
const unsigned long A0_DIO_CH  = 0;   // A0  -> DIO_0
// E1 映射到 DIO 通道 3（E1 低有效，是“同相”路径）
const unsigned long E1_DIO_CH  = 3;   // E1  -> DIO_3
// E3 映射到 DIO 通道 5（E3 高有效，是“反相”路径）
const unsigned long E3_DIO_CH  = 5;   // E3  -> DIO_5
// Y0 映射到 DIO 通道 6，Tpd/Tt 都固定测 Y0，接到 TMU1
const unsigned long Y0_DIO_CH  = 6;   // Y0  -> DIO_6

// 工程复用钩子：换站点/换 job 时 Runner 会调用，这里只打印
USER_CODE void ProjectReuse() {
  // 打印函数名，便于在 Console 里确认调用顺序
  cout << "ProjectReuse" << endl;
}

// 工程初始化钩子：加载 .job 后调用一次，可在这里做全局初始化
USER_CODE void ProjectInit() {
  // 打印函数名，确认工程已初始化
  cout << "ProjectInit" << endl;
}

// 工程复位钩子：测试结束/异常退出时调用，用于释放资源
USER_CODE void ProjectReset() {
  // 打印函数名，确认复位被调用
  cout << "ProjectReset" << endl;
}

// 站点初始化钩子：每个站点开始测试前调用
USER_CODE void site_init() {
  // 打印函数名，确认站点初始化
  cout << "site_init" << endl;
}

// 站点复位钩子：站点测试结束后调用
USER_CODE void site_reset() {
  // 打印函数名，确认站点复位
  cout << "site_reset" << endl;
}

// Wafer 结束回调：探针台测完一片时由 proberObj 触发
void EndOfWafer() {
  // 打印提示，实际产线可在这里做数据落盘/换片
  cout << "EndOfWafer" << endl;
}

// Lot 结束回调：一整批测完时由 proberObj 触发
void EndOfLot() {
  // 打印提示，实际产线可在这里做批次统计
  cout << "EndOfLot" << endl;
}

// 注册 Wafer 结束检查：把 EndOfWafer 函数指针交给探针台对象
inline void Enable_EOW_Func() {
  // proberObj.EnableWaferEndCheck 绑定回调，测完一片自动调用 EndOfWafer
  proberObj.EnableWaferEndCheck(EndOfWafer);
}

// 注册 Lot 结束检查：把 EndOfLot 函数指针交给探针台对象
inline void Enable_EOL_Func() {
  // proberObj.EnableLotEndCheck 绑定回调，测完一批自动调用 EndOfLot
  proberObj.EnableLotEndCheck(EndOfLot);
}

/*
 * 功能类 Pattern 的通用执行流程：
 *   上电 -> 设置 DIO 电平 -> 运行 Pattern 比对 -> 下电 -> 记录结果
 * paramID 对应 TMF 中该测试项的 ParamNumber。
 */
static void FunctionalPatternTest(const char* patName, double vcc,
								  double vih, double vil,
								  double voh, double vol,
								  unsigned long paramID)
{
	// 存放 Pattern 比较结果：每个元素对应一个站点/一次比较的 pass/fail
	vector<ST_DIO_RESULT> result;

	//CONNECT / POWER_ON
	// 连接 DPS 的 VCC 通道，准备给芯片供电
	dps.Signal("VCC").Connect();
	// 等待 2ms，让继电器/电源稳定后再设置电压
	sys.DelayUs(2000);

	// DPS 设为 FVMV（强制电压、测量电流）：电压=vcc，电流档 0.5A，钳位 ±0.5A
	dps.Signal("VCC").SetMode("FVMV").VoltForce(vcc)
									 .CurrRange(0.5)
									 .CurrClamp(0.5, -0.5)
									 .Execute();
	// 等待 2ms，让 VCC 上升到目标值、芯片内部复位完成
	sys.DelayUs(2000);

	// 把 14 个 I/O 设为 DIO 模式，并设置输入高/低、输出比较高/低和电压钳位
	pinlevel.Signal("IO_PINS").SetMode("DIO")
							  .SetVih(vih)
							  .SetVil(vil)
							  .SetVoh(voh)
							  .SetVol(vol)
							  .VoltClamp(vcc + 0.5, -2)
							  .Execute();
	// 等待 2ms，让 pinlevel 生效、输出稳定
	sys.DelayUs(2000);

	//运行 Pattern
	// 运行指定 Pattern，超时 10s；Test() 会驱动输入并比较输出
	dio.Pattern(patName).SetTimeOut(10000).Test();
	// 取回 Pattern 的失败结果（空表示全部通过）
	dio.Pattern(patName).GetFailResult(result);
	// 等待 2ms，确保 Pattern 执行完、结果已锁存
	sys.DelayUs(2000);

	//POWER_OFF
	// 下电前先把所有 I/O 驱动归零，避免带电拔插
	pinlevel.Signal("IO_PINS").SetMode("DIO")
							  .SetVih(0)
							  .SetVil(0)
							  .SetVoh(0)
							  .SetVol(0)
							  .VoltClamp(0.5, -0.5)
							  .Execute();
	// 等待 2ms，让 I/O 放电到 0V
	sys.DelayUs(2000);

	// 把 VCC 强制到 0V，给芯片断电
	dps.Signal("VCC").SetMode("FVMV").VoltForce(0)
									 .CurrRange(0.5)
									 .CurrClamp(0.5, -0.5)
									 .Execute();
	// 等待 2ms，让 VCC 放电完成
	sys.DelayUs(2000);

	//DISCONNECT
	// 断开 DPS 连接，释放 VCC 通道
	dps.Signal("VCC").DisConnect();
	// 等待 2ms，确保继电器完全断开
	sys.DelayUs(2000);

	// 把 Pattern 结果交给 binObj 判 Bin：paramID 对应 TMF 的参数号
	binObj.CheckResultAndBin(paramID, result);
}

/*
 * 传输延时测量：
 *   patName   : 带 loop_inf 的延时 Pattern
 *   inDioCh   : 被测输入管脚对应的 DIO 通道，绑定到 TMU0（主通道）
 *   inSlope   : 输入触发沿
 *   outSlope  : 输出触发沿（Y0 固定绑定 TMU1）
 *   paramID   : TMF 中的 ParamNumber
 * 测量时序：进入 loop_inf 的 Hold 状态后启动 TMU，测完退出 Hold。
 */
static void MeasureTpd(const char* patName, unsigned long inDioCh,
					   TMU_SLOPE_TYPE inSlope, TMU_SLOPE_TYPE outSlope,
					   unsigned long paramID)
{
	// 存放 TMU 测到的延时值（单位由 API 决定，本工程按 ns 判）
	vector<ST_MEAS_RESULT> timeResult;
	// Hold 标志：Pattern 跑到 loop_inf 时会挂起，true 表示已进入测量窗口
	bool bHold = false;

	//TMU0 接被测输入管脚，TMU1 接 Y0
	// TMU0（主通道）绑定被测输入脚所在的 DIO 通道
	tmu.SetupTmuOutputCh(1, 1, 0, inDioCh);
	// TMU1（从通道）固定绑定 Y0 的 DIO 通道 6
	tmu.SetupTmuOutputCh(1, 1, 1, Y0_DIO_CH);

	//运行延时 Pattern，等待进入 Hold 状态
	// 运行带 loop_inf 的延时 Pattern，跑到 loop_inf 会停在 Hold 状态
	dio.Pattern(patName).SetTimeOut(10000).Run();
	// 等待 1ms，让 Pattern 进入 Hold、电平稳定
	sys.DelayUs(1000);
	// 查询 Hold 状态；true 才能启动 TMU 测量
	dio.Pattern(patName).GetHoldStatus(bHold);

	// 只有进入 Hold 才测量，否则直接走失败分支
	if(bHold)
	{
		// 配置 PinToPinDelay：主/从触发电平都设 2.25V（4.5V 的一半）
		tmu.PinToPinDelay().Signal("TMU_PINS")
						   .SetTrigLevel(2.25, 2.25)
						   .SetTrigSlope(inSlope, outSlope)
						   .SetTimeOut(100)
						   // TMU 电压钳位 6V/-2V，保护输入
						   .SetVoltClamp(6, -2)
						   // Execute 下发 TMU 配置
						   .Execute();
		// 等待 1ms，让 TMU 配置生效
		sys.DelayUs(1000);

		// 启动测量：等待主通道触发沿，再等从通道触发沿，记录时间差
		tmu.PinToPinDelay().StartMeas();
		// 取回测量结果（为空表示超时/没抓到沿）
		tmu.PinToPinDelay().GetMeasResult(timeResult);

		// 退出 Hold，让 Pattern 继续跑完并 stop
		dio.Pattern(patName).SetHoldExit();
	}
	// 等待 Pattern 完全结束，释放 DIO 资源
	dio.Pattern(patName).WaitEnd();

	//未进入 Hold 或未测到数据时，写入超限值判失败
	// 没进入 Hold 或 TMU 没返回结果时，构造一个超限值
	if(timeResult.empty())
	{
		// 构造失败结果结构体
		ST_MEAS_RESULT stFail;
		// 信号名写 Tpd，便于日志定位
		stFail.strSigName = "Tpd";
		// 站点号 0（单站）
		stFail.ulSite     = 0;
		// 写 9999（远大于 38ns 上限），保证判失败
		stFail.dbValue    = 9999;
		// 把失败结果放入向量，走统一的判 Bin 流程
		timeResult.push_back(stFail);
	}

	// 按 paramID 判 Bin；这里只有 1 个参数（一个延时值）
	binObj.CheckResultAndBin(paramID, timeResult, 1);
}

/*
 * 时序类测试公共上电流程（VCC = 4.5V）
 */
static void TimingTestPowerOn()
{
	// 连接 VCC，准备上电
	dps.Signal("VCC").Connect();
	// 等待 2ms
	sys.DelayUs(2000);

	// VCC 强制 4.5V（74HC138 交流参数测试的标准电压）
	dps.Signal("VCC").SetMode("FVMV").VoltForce(4.5)
									 .CurrRange(0.5)
									 .CurrClamp(0.5, -0.5)
									 .Execute();
	// 等待 2ms，电源稳定
	sys.DelayUs(2000);

	// 设置 14 个 I/O 的 DIO 电平：VIH=4.5、VIL=0、VOH=3.15、VOL=1.35
	pinlevel.Signal("IO_PINS").SetMode("DIO")
							  .SetVih(4.5)
							  .SetVil(0)
							  // 输出比较电平 3.15/1.35 对应 4.5V 的 70%/30%，即手册 VIH/VIL 极限
							  .SetVoh(3.15)
							  .SetVol(1.35)
							  .VoltClamp(5, -2)
							  .Execute();
	// 等待 2ms，电平稳定
	sys.DelayUs(2000);
}

/*
 * 时序类测试公共下电流程
 */
static void TimingTestPowerOff()
{
	// 下电前先把 I/O 归零
	pinlevel.Signal("IO_PINS").SetMode("DIO")
							  .SetVih(0)
							  .SetVil(0)
							  .SetVoh(0)
							  .SetVol(0)
							  .VoltClamp(0.5, -0.5)
							  .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// VCC 强制 0V
	dps.Signal("VCC").SetMode("FVMV").VoltForce(0)
									 .CurrRange(0.5)
									 .CurrClamp(0.5, -0.5)
									 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// 断开 VCC
	dps.Signal("VCC").DisConnect();
	// 等待 2ms，确保断开
	sys.DelayUs(2000);
}

/*
 * 输出转换时间测量（tt = tTLH / tTHL，测量点 10% ~ 90% VCC）
 * 使用 DELAY_A0 Pattern 使 Y0 持续翻转，TMU1 接 Y0
 */
static void MeasureTt()
{
	// 存放上升时间 tTLH 的结果
	vector<ST_MEAS_RESULT> TTLH_RESULT;
	// 存放下降时间 tTHL 的结果
	vector<ST_MEAS_RESULT> TTHL_RESULT;
	// Hold 标志，DELAY_A0 进入 loop_inf 后为 true
	bool bHold = false;

	//TMU1 接 Y0
	// Tt 只需要从通道：TMU1 接 Y0
	tmu.SetupTmuOutputCh(1, 1, 1, Y0_DIO_CH);

	// 运行 DELAY_A0，让 Y0 持续翻转，进入 Hold
	dio.Pattern("DELAY_A0").SetTimeOut(10000).Run();
	// 等待 1ms
	sys.DelayUs(1000);
	// 查询 Hold 状态
	dio.Pattern("DELAY_A0").GetHoldStatus(bHold);

	// 进入 Hold 才启动 RiseTime/FallTime 测量
	if(bHold)
	{
		//上升时间：10% VCC -> 90% VCC
		// 配置 RiseTime：从 10% VCC 到 90% VCC（4.5V 时是 0.45V -> 4.05V）
		tmu.RiseTime().Signal("TMU2")
					  .SetThresholds(0.45, 4.05)
					  .SetSampleSize(10)
					  .SetArmSize(0)
					  .SetTimeOut(100)
					  .SetVoltClamp(6, -2)
					  .Execute();
		// 等待 1ms
		sys.DelayUs(1000);
		// 启动上升时间测量
		tmu.RiseTime().StartMeas();
		// 取回 tTLH
		tmu.RiseTime().GetMeasResult(TTLH_RESULT);

		//下降时间：90% VCC -> 10% VCC
		// 配置 FallTime：从 90% VCC 到 10% VCC（4.05V -> 0.45V）
		tmu.FallTime().Signal("TMU2")
					  .SetThresholds(4.05, 0.45)
					  .SetSampleSize(10)
					  .SetArmSize(0)
					  .SetTimeOut(100)
					  .SetVoltClamp(6, -2)
					  .Execute();
		// 等待 1ms
		sys.DelayUs(1000);
		// 启动下降时间测量
		tmu.FallTime().StartMeas();
		// 取回 tTHL
		tmu.FallTime().GetMeasResult(TTHL_RESULT);

		// 退出 Hold，让 Pattern 继续跑完
		dio.Pattern("DELAY_A0").SetHoldExit();
	}
	// 等待 Pattern 结束
	dio.Pattern("DELAY_A0").WaitEnd();

	//未进入 Hold 或未测到数据时，写入超限值判失败
	// tTLH 没测到时构造失败值
	if(TTLH_RESULT.empty())
	{
		// 构造失败结果
		ST_MEAS_RESULT stFail;
		// 信号名 TTLH
		stFail.strSigName = "TTLH";
		// 站点 0
		stFail.ulSite     = 0;
		// 9999 远大于 19ns 上限
		stFail.dbValue    = 9999;
		// 放入结果向量
		TTLH_RESULT.push_back(stFail);
	}
	// tTHL 没测到时同样构造失败值
	if(TTHL_RESULT.empty())
	{
		// 构造失败结果
		ST_MEAS_RESULT stFail;
		// 信号名 TTHL
		stFail.strSigName = "TTHL";
		// 站点 0
		stFail.ulSite     = 0;
		// 9999 判失败
		stFail.dbValue    = 9999;
		// 放入结果向量
		TTHL_RESULT.push_back(stFail);
	}

	// tTLH 对应 Param 0
	binObj.CheckResultAndBin(0, TTLH_RESULT, 1);
	// tTHL 对应 Param 1
	binObj.CheckResultAndBin(1, TTHL_RESULT, 1);
}

/*
 * OSN：所有管脚对 GND 的 ESD 二极管开路/短路测试
 * VCC = 0V，向 IO_PINS 灌 -100uA 电流，测量管脚电压
 * 正常：-1.2V ~ -0.2V
 */
USER_CODE void OSN() {
	TEST_BEGIN
	//建立储存结果向量
	// 存放 14 个脚的 OSN 电压结果
	vector<ST_MEAS_RESULT> OSN_RESULT;
	//管脚连接
	// 连接 VCC（OS 测试时 VCC 要拉到 0V）
	dps.Signal("VCC").Connect();
	// 连接 14 个 I/O 的 PPMU，准备灌电流
	ppmu.Signal("IO_PINS").Connect();
	// 等待 5ms，让连接稳定
	sys.DelayUs(5000);
	//VCC Clear
	// VCC 强制 0V：OS 测试必须断电，否则保护二极管会引入额外通路
	dps.Signal("VCC").SetMode("FVMV").VoltForce(0)
									 .CurrRange(0.5)
									 .CurrClamp(0.5, -0.5)
									 .Execute();
	// 等待 5ms，VCC 放电完成
	sys.DelayUs(5000);
	//IO_PINS Clear
	// 先把 14 个 I/O 强制 0V，清掉残存电荷
	ppmu.Signal("IO_PINS").SetMode("FVMV").VoltForce(0)
										  .CurrRange(40e-6)
										  .Execute();
	// 等待 5ms
	sys.DelayUs(5000);
	//IO_PINS 灌 -100uA 电流
	// PPMU 设为 FIMV：对 14 个脚同时抽 -100uA（负向）
	ppmu.Signal("IO_PINS").SetMode("FIMV").CurrForce(-100e-6)
										  // 电流档 200uA，覆盖 100uA
										  .CurrRange(200e-6)
										  // 电压钳位 ±2V，防止开路时电压跑飞
										  .VoltClamp(2, -2)
										  .Execute();
	// 等待 5ms，让电流建立
	sys.DelayUs(5000);
	//测量电压
	// 测量 14 个脚的电压，结果按 IO_PINS 顺序排列
	ppmu.Signal("IO_PINS").Measure(OSN_RESULT);
	//IO_PINS 电流清零
	// 测完把电流源归零，避免下电时继续灌流
	ppmu.Signal("IO_PINS").SetMode("FIMI").CurrForce(0)
										  .CurrRange(200e-6)
										  .VoltClamp(2, -2)
										  .Execute();
	// 等待 5ms
	sys.DelayUs(5000);

	//断开连接
	// 断开 VCC
	dps.Signal("VCC").DisConnect();
	// 断开 PPMU
	ppmu.Signal("IO_PINS").DisConnect();
	// 等待 5ms
	sys.DelayUs(5000);

	// 14 个脚逐个判 Bin：Param 0~13 对应 IO_PINS 顺序
	binObj.CheckResultAndBin(0,OSN_RESULT,IO_PIN_NUM);

	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * OSP：所有管脚对 VCC 的 ESD 二极管开路/短路测试
 * VCC = 0V，向 IO_PINS 拉 100uA 电流，测量管脚电压
 * 正常：0.2V ~ 1.2V
 */
USER_CODE void OSP() {
	TEST_BEGIN
	//建立储存结果向量
	// 存放 14 个脚的 OSP 电压结果
	vector<ST_MEAS_RESULT> OSP_RESULT;
	//管脚连接
	// 连接 VCC
	dps.Signal("VCC").Connect();
	// 连接 14 个 I/O 的 PPMU
	ppmu.Signal("IO_PINS").Connect();
	// 等待 5ms
	sys.DelayUs(5000);
	//VCC Clear
	// VCC 强制 0V
	dps.Signal("VCC").SetMode("FVMV").VoltForce(0)
									 .CurrRange(0.5)
									 .CurrClamp(0.5, -0.5)
									 .Execute();
	// 等待 5ms
	sys.DelayUs(5000);
	//IO_PINS Clear
	// 14 个 I/O 先归零
	ppmu.Signal("IO_PINS").SetMode("FVMV").VoltForce(0)
										  .CurrRange(40e-6)
										  .Execute();
	// 等待 5ms
	sys.DelayUs(5000);
	//IO_PINS 拉 100uA 电流
	// PPMU FIMV：对 14 个脚同时灌 +100uA（正向）
	ppmu.Signal("IO_PINS").SetMode("FIMV").CurrForce(100e-6)
										  // 电流档 200uA
										  .CurrRange(200e-6)
										  // 电压钳位 ±2V
										  .VoltClamp(2, -2)
										  .Execute();
	// 等待 5ms
	sys.DelayUs(5000);
	//测量电压
	// 测量 14 个脚电压
	ppmu.Signal("IO_PINS").Measure(OSP_RESULT);
	//IO_PINS 电流清零
	// 电流归零
	ppmu.Signal("IO_PINS").SetMode("FIMI").CurrForce(0)
										  .CurrRange(200e-6)
										  .VoltClamp(2, -2)
										  .Execute();
	// 等待 5ms
	sys.DelayUs(5000);

	//断开连接
	// 断开 VCC
	dps.Signal("VCC").DisConnect();
	// 断开 PPMU
	ppmu.Signal("IO_PINS").DisConnect();
	// 等待 5ms
	sys.DelayUs(5000);

	//比较
	// 14 个脚逐个判 Bin
	binObj.CheckResultAndBin(0,OSP_RESULT,IO_PIN_NUM);

	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * ICC：静态电源电流
 * VCC = 6V，输入全部为低电平（译码器禁止，输出全高，无负载）
 * 正常：0 ~ 8uA
 */
USER_CODE void ICC() {
	TEST_BEGIN

	// 存放 ICC 电流结果（只有 1 个参数）
	vector<ST_MEAS_RESULT> ICC_RESULT;

	// 连接 VCC
	dps.Signal("VCC").Connect();
	// 等待 2ms
	sys.DelayUs(2000);

	//先大电流档位给 VCC 充电
	// FVMI：强制 6V、先用量程 20mA 给 VCC 电容快速充电
	dps.Signal("VCC").SetMode("FVMI").VoltForce(6)
									 .CurrRange(20e-3)
									 .CurrClamp(20e-3,-20e-3)
									 .Execute();
	// 等待 2ms，充电完成
	sys.DelayUs(2000);

	//输入管脚配置为 DIO，全部驱动低电平
	// 把 14 个 I/O 设为 DIO 并全部驱动低电平，保证输入状态确定
	pinlevel.Signal("IO_PINS").SetMode("DIO")
							  .SetVih(6)
							  .SetVil(0)
							  .SetVoh(4.2)
							  .SetVol(1.8)
							  .VoltClamp(6, -2)
							  .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	//运行 ICC Pattern，使输入状态确定
	// 运行 ICC Pattern：让 A/E 全部为低，译码器禁止、输出全高、无负载
	dio.Pattern("ICC").SetTimeOut(10000).Test();
	// 等待 5ms，让电流稳定
	sys.DelayUs(5000);

	//切换到小电流档位测量
	// 切到 20uA 小量程精测（先大后小，避免上电瞬态）
	dps.Signal("VCC").SetMode("FVMI").VoltForce(6)
									 .CurrRange(20e-6)
									 .CurrClamp(20e-6,-20e-6)
									 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// 测量 VCC 电流，即 ICC
	dps.Signal("VCC").Measure(ICC_RESULT);
	// 等待 5ms
	sys.DelayUs(5000);

	//POWER_OFF
	// 下电前 I/O 归零
	pinlevel.Signal("IO_PINS").SetMode("DIO")
							  .SetVih(0)
							  .SetVil(0)
							  .SetVoh(0)
							  .SetVol(0)
							  .VoltClamp(0.5, -0.5)
							  .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// VCC 强制 0V
	dps.Signal("VCC").SetMode("FVMV").VoltForce(0)
									 .CurrRange(20e-6)
									 .CurrClamp(20e-6,-20e-6)
									 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// 断开 VCC
	dps.Signal("VCC").DisConnect();

	// ICC 只有 1 个参数
	binObj.CheckResultAndBin(0,ICC_RESULT,1);

	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * VOH：输出高电平
 * VCC = 4.5V，输出全高（E1 = 1 禁止译码），每个输出灌 -4mA
 * 正常：3.84V ~ 4.4V
 */
USER_CODE void VOH() {
	TEST_BEGIN
	// 存放 8 个输出的 VOH 电压
	vector<ST_MEAS_RESULT> VOH_RESULT;
	// 存放 VOH Pattern 的比较结果
	vector<ST_DIO_RESULT>  VOH_PAT_RESULT;

	//CONNECT
	// 连接 VCC
	dps.Signal("VCC").Connect();
	// 等待 2ms
	sys.DelayUs(2000);

	//POWER_ON
	// VCC 强制 4.5V
	dps.Signal("VCC").SetMode("FVMV").VoltForce(4.5)
									 .CurrRange(0.5)
									 .CurrClamp(0.5,-0.5)
									 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// 设置 I/O 电平：VIH=4.5、VIL=0、VOH=3.15、VOL=1.35
	pinlevel.Signal("IO_PINS").SetMode("DIO")
							  .SetVih(4.5)
							  .SetVil(0)
							  .SetVoh(3.15)
							  .SetVol(1.35)
							  .VoltClamp(4.5, -2)
							  .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	//输出全高，并记录 Pattern 比较结果
	// 运行 VOH Pattern：E1=1 禁止译码，8 个输出全高
	dio.Pattern("VOH").SetTimeOut(10000).Test();
	// 取 Pattern 比较结果
	dio.Pattern("VOH").GetFailResult(VOH_PAT_RESULT);
	// 等待 5ms
	sys.DelayUs(5000);

	//连接 PPMU 并拉 -4mA 电流
	// 连接 8 个输出的 PPMU
	ppmu.Signal("OUTPUT_PINS").Connect();
	// 等待 2ms
	sys.DelayUs(2000);

	// FIMV：对 8 个输出同时抽 -4mA（电流流出 DUT）
	ppmu.Signal("OUTPUT_PINS").SetMode("FIMV").CurrForce(-4e-3)
										  // 电流档 20mA
										  .CurrRange(20e-3)
										  // 电压钳位 5V/-2V
										  .VoltClamp(5, -2)
										  .Execute();
	// 等待 5ms，让输出建立
	sys.DelayUs(5000);

	// 测量 8 个输出的电压
	ppmu.Signal("OUTPUT_PINS").Measure(VOH_RESULT);
	// 等待 5ms
	sys.DelayUs(5000);

	//Pattern 比较失败时，VOH 测量结果无效，强制判失败
	// 如果 Pattern 比较失败，说明输出状态不对，VOH 测量无意义
	if(!VOH_PAT_RESULT.empty() && !VOH_PAT_RESULT[0].bPass)
	{
		// 把 8 个结果全部改成 999，强制判失败
		for(size_t i = 0; i < VOH_RESULT.size(); i++)
			// 999 远大于 4.4V 上限
			VOH_RESULT[i].dbValue = 999;
	}

	//POWER_OFF
	// PPMU 电流归零
	ppmu.Signal("OUTPUT_PINS").SetMode("FIMI").CurrForce(0)
										  .CurrRange(20e-3)
										  .VoltClamp(2, -2)
										  .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// I/O 电平归零
	pinlevel.Signal("IO_PINS").SetMode("DIO")
							  .SetVih(0)
							  .SetVil(0)
							  .SetVoh(0)
							  .SetVol(0)
							  .VoltClamp(0.5, -0.5)
							  .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// VCC 强制 0V
	dps.Signal("VCC").SetMode("FVMV").VoltForce(0)
									 .CurrRange(0.5)
									 .CurrClamp(0.5, -0.5)
									 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	//DISCONNECT
	// 断开 VCC
	dps.Signal("VCC").DisConnect();
	// 断开 PPMU
	ppmu.Signal("OUTPUT_PINS").DisConnect();
	// 等待 2ms
	sys.DelayUs(2000);

	// 8 个输出逐个判 Bin
	binObj.CheckResultAndBin(0,VOH_RESULT,OUTPUT_PIN_NUM);

	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * VOL：输出低电平
 * VCC = 4.5V，逐个地址译码，对当前为低电平的输出灌 4mA 电流
 * 正常：0V ~ 0.33V
 */
USER_CODE void VOL() {
	TEST_BEGIN

	// 存放当前被测脚的 VOL 电压（每脚 1 个结果）
	vector<ST_MEAS_RESULT> VOL_RESULT;
	// 存放当前 VOL Pattern 的比较结果
	vector<ST_DIO_RESULT>  VOL_PAT_RESULT;

	//CONNECT
	// 连接 VCC
	dps.Signal("VCC").Connect();
	// 等待 2ms
	sys.DelayUs(2000);

	//POWER_ON
	// VCC 强制 4.5V
	dps.Signal("VCC").SetMode("FVMV").VoltForce(4.5)
									 .CurrRange(0.5)
									 .CurrClamp(0.5, -0.5)
									 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// 设置 I/O 电平
	pinlevel.Signal("IO_PINS").SetMode("DIO")
							  .SetVih(4.5)
							  .SetVil(0)
							  .SetVoh(3.15)
							  .SetVol(1.35)
							  .VoltClamp(4.5, -2)
							  .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	//逐个输出测试 VOL
	// 逐个输出测 VOL：n=0..7 对应 Y0..Y7
	for(int n = 0; n < OUTPUT_PIN_NUM; n++)
	{
		//译码使 Yn 输出低电平
		// 运行 VOL_PAT[n]：该 Pattern 只让 Yn 输出低，其余输出高
		dio.Pattern(VOL_PAT[n]).SetTimeOut(10000).Test();
		// 取 Pattern 比较结果
		dio.Pattern(VOL_PAT[n]).GetFailResult(VOL_PAT_RESULT);
		// 等待 2ms
		sys.DelayUs(2000);

		// 连接当前被测脚 Yn 的 PPMU（单脚）
		ppmu.Signal(VOL_SIG[n]).Connect();
		// 等待 2ms
		sys.DelayUs(2000);

		// FIMV：对 Yn 灌 +4mA（电流流入 DUT）
		ppmu.Signal(VOL_SIG[n]).SetMode("FIMV").CurrForce(4e-3)
											  // 电流档 20mA
											  .CurrRange(20e-3)
											  // 电压钳位 ±2V
											  .VoltClamp(2, -2)
											  .Execute();
		// 等待 5ms
		sys.DelayUs(5000);

		// 测量 Yn 的电压
		ppmu.Signal(VOL_SIG[n]).Measure(VOL_RESULT);
		// 等待 2ms
		sys.DelayUs(2000);

		//Pattern 比较失败时，该脚 VOL 测量结果无效，强制判失败
		// 如果该脚 Pattern 比较失败，VOL 结果无效
		if(!VOL_PAT_RESULT.empty() && !VOL_PAT_RESULT[0].bPass && !VOL_RESULT.empty())
			// 把结果改成 999，强制判失败
			VOL_RESULT[0].dbValue = 999;

		// 电流归零
		ppmu.Signal(VOL_SIG[n]).SetMode("FIMI").CurrForce(0)
											  .CurrRange(20e-3)
											  .VoltClamp(2, -2)
											  .Execute();
		// 等待 2ms
		sys.DelayUs(2000);

		// 断开该脚 PPMU
		ppmu.Signal(VOL_SIG[n]).DisConnect();
		// 等待 2ms
		sys.DelayUs(2000);

		//结果按 ParamNumber = n 与 TMF 中的 Yn 对应
		// Param n 对应 TMF 中的 Yn
		binObj.CheckResultAndBin(n,VOL_RESULT,1);
	}

	//POWER_OFF
	// 循环结束，I/O 电平归零
	pinlevel.Signal("IO_PINS").SetMode("DIO")
							  .SetVih(0)
							  .SetVil(0)
							  .SetVoh(0)
							  .SetVol(0)
							  .VoltClamp(0.5, -0.5)
							  .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// VCC 强制 0V
	dps.Signal("VCC").SetMode("FVMV").VoltForce(0)
									 .CurrRange(0.5)
									 .CurrClamp(0.5, -0.5)
									 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	//DISCONNECT
	// 断开 VCC
	dps.Signal("VCC").DisConnect();
	// 等待 2ms
	sys.DelayUs(2000);

	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * IIH：输入高电平漏电流
 * VCC = 6V，输入管脚强制 6V，测量电流
 * 正常：-1uA ~ 1uA
 */
USER_CODE void IIH() {
	TEST_BEGIN

	// 存放 6 个输入脚的 IIH 电流
	vector<ST_MEAS_RESULT> IIH_RESULT;

	//CONNECT
	// 连接 VCC
	dps.Signal("VCC").Connect();
	// 连接 6 个输入脚的 PPMU
	ppmu.Signal("INPUT_PINS").Connect();
	// 等待 2ms
	sys.DelayUs(2000);

	// VCC 强制 6V（IIH 的手册测试电压）
	dps.Signal("VCC").SetMode("FVMV").VoltForce(6)
									 .CurrRange(0.5)
									 .CurrClamp(0.5,-0.5)
									 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// FVMI：把 6 个输入脚强制到 6V，测流入/流出的电流
	ppmu.Signal("INPUT_PINS").SetMode("FVMI").VoltForce(6)
											 // 电流档 20uA
											 .CurrRange(20e-6)
											 // 电压钳位 6V/-2V
											 .VoltClamp(6, -2)
											 .Execute();
	// 等待 5ms，让漏电稳定
	sys.DelayUs(5000);

	// 测量 6 个输入脚的电流
	ppmu.Signal("INPUT_PINS").Measure(IIH_RESULT);
	// 等待 2ms
	sys.DelayUs(2000);

	//恢复输入电压
	// 测完把输入脚恢复 0V
	ppmu.Signal("INPUT_PINS").SetMode("FVMV").VoltForce(0)
											 .CurrRange(20e-6)
											 .VoltClamp(6, -2)
											 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// VCC 强制 0V
	dps.Signal("VCC").SetMode("FVMV").VoltForce(0)
									 .CurrRange(0.5)
									 .CurrClamp(0.5, -0.5)
									 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// 断开 VCC
	dps.Signal("VCC").DisConnect();
	// 断开 PPMU
	ppmu.Signal("INPUT_PINS").DisConnect();
	// 等待 2ms
	sys.DelayUs(2000);

	// 6 个输入脚逐个判 Bin
	binObj.CheckResultAndBin(0,IIH_RESULT,INPUT_PIN_NUM);

	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * IIL：输入低电平漏电流
 * VCC = 6V，输入管脚强制 0V，测量电流
 * 正常：-1uA ~ 1uA
 */
USER_CODE void IIL() {
	TEST_BEGIN

	// 存放 6 个输入脚的 IIL 电流
	vector<ST_MEAS_RESULT> IIL_RESULT;

	// 连接 VCC
	dps.Signal("VCC").Connect();
	// 连接 6 个输入脚的 PPMU
	ppmu.Signal("INPUT_PINS").Connect();
	// 等待 2ms
	sys.DelayUs(2000);

	// VCC 强制 6V
	dps.Signal("VCC").SetMode("FVMV").VoltForce(6)
									 .CurrRange(0.5)
									 .CurrClamp(0.5,-0.5)
									 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// FVMI：把 6 个输入脚强制到 0V，测漏电
	ppmu.Signal("INPUT_PINS").SetMode("FVMI").VoltForce(0)
											 // 电流档 20uA
											 .CurrRange(20e-6)
											 // 电压钳位 ±2V
											 .VoltClamp(2, -2)
											 .Execute();
	// 等待 5ms
	sys.DelayUs(5000);

	// 测量 6 个输入脚电流
	ppmu.Signal("INPUT_PINS").Measure(IIL_RESULT);
	// 等待 2ms
	sys.DelayUs(2000);

	// VCC 强制 0V
	dps.Signal("VCC").SetMode("FVMV").VoltForce(0)
									 .CurrRange(0.5)
									 .CurrClamp(0.5, -0.5)
									 .Execute();
	// 等待 2ms
	sys.DelayUs(2000);

	// 断开 VCC
	dps.Signal("VCC").DisConnect();
	// 断开 PPMU
	ppmu.Signal("INPUT_PINS").DisConnect();
	// 等待 2ms
	sys.DelayUs(2000);

	// 6 个输入脚逐个判 Bin
	binObj.CheckResultAndBin(0,IIL_RESULT,INPUT_PIN_NUM);

	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * FUNC：功能测试
 * VCC = 4.5V，8 种使能组合 + 8 个地址的完整真值表
 */
USER_CODE void FUNC() {
	TEST_BEGIN
	// 调用公共功能函数：Pattern=FUNC，VCC=4.5V，VIH=4.5/VIL=0，VOH=3.15/VOL=1.35，Param 0
	FunctionalPatternTest("FUNC", 4.5, 4.5, 0, 3.15, 1.35, 0);
	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * DEMUX：解复用功能测试
 * VCC = 4.5V，E1 作为数据输入，E2 = 0、E3 = 1
 * 每个地址下 E1 为低时对应输出为低，E1 为高时全部输出为高
 */
USER_CODE void DEMUX() {
	TEST_BEGIN
	// 调用公共功能函数：Pattern=DEMUX，其余条件与 FUNC 相同，Param 0
	FunctionalPatternTest("DEMUX", 4.5, 4.5, 0, 3.15, 1.35, 0);
	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * FUNC_LIM：输入阈值余量功能测试
 * VCC = 4.5V，Vih = 3.15V（VIH min）、Vil = 1.35V（VIL max）
 * 在手册极限输入电平下验证译码功能
 */
USER_CODE void FUNC_LIM() {
	TEST_BEGIN
	// 调用公共功能函数：用极限输入电平 VIH=3.15、VIL=1.35 跑 FUNC，验证最坏情况
	FunctionalPatternTest("FUNC", 4.5, 3.15, 1.35, 3.15, 1.35, 0);
	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * FUNC_2V：低电源电压功能测试
 * VCC = 2.0V（手册推荐最低工作电压）
 */
USER_CODE void FUNC_2V() {
	TEST_BEGIN
	// 调用公共功能函数：VCC 降到 2.0V，电平按比例设为 2.0/0/1.4/0.6
	FunctionalPatternTest("FUNC", 2.0, 2.0, 0, 1.4, 0.6, 0);
	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * FUNC_6V：高电源电压功能测试
 * VCC = 6.0V（手册推荐最高工作电压）
 */
USER_CODE void FUNC_6V() {
	TEST_BEGIN
	// 调用公共功能函数：VCC 升到 6.0V，电平按比例设为 6.0/0/4.2/1.8
	FunctionalPatternTest("FUNC", 6.0, 6.0, 0, 4.2, 1.8, 0);
	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * Tpd：传输延时（An / E3 / E1 -> Y0）
 * VCC = 4.5V，TMU0 接被测输入，TMU1 接 Y0
 * 参数：0 TPHL_A  1 TPLH_A  2 TPHL_E3  3 TPLH_E3  4 TPHL_E1  5 TPLH_E1
 * 正常：0 ~ 38ns
 */
USER_CODE void Tpd() {
	TEST_BEGIN

	//CONNECT / POWER_ON
	// 时序测试公共上电：VCC=4.5V，I/O 电平 4.5/0/3.15/1.35
	TimingTestPowerOn();

	//An -> Yn（反相：输入上升，输出下降 / 输入下降，输出上升）
	// A0 -> Y0 的 TPHL：A0 上升（POS），Y0 下降（NEG），写 Param 0
	MeasureTpd("DELAY_A0", A0_DIO_CH, TMU_SLOPE_POS, TMU_SLOPE_NEG, 0);
	// A0 -> Y0 的 TPLH：A0 下降（NEG），Y0 上升（POS），写 Param 1
	MeasureTpd("DELAY_A0", A0_DIO_CH, TMU_SLOPE_NEG, TMU_SLOPE_POS, 1);

	//E3 -> Yn（反相）
	// E3 -> Y0 的 TPHL：E3 上升（POS），Y0 下降（NEG），写 Param 2
	MeasureTpd("DELAY_E3", E3_DIO_CH, TMU_SLOPE_POS, TMU_SLOPE_NEG, 2);
	// E3 -> Y0 的 TPLH：E3 下降（NEG），Y0 上升（POS），写 Param 3
	MeasureTpd("DELAY_E3", E3_DIO_CH, TMU_SLOPE_NEG, TMU_SLOPE_POS, 3);

	//E1 -> Yn（同相：输入上升，输出上升 / 输入下降，输出下降）
	// E1 -> Y0 的 TPLH：E1 上升（POS），Y0 上升（POS），写 Param 4
	MeasureTpd("DELAY_E1", E1_DIO_CH, TMU_SLOPE_POS, TMU_SLOPE_POS, 4);
	// E1 -> Y0 的 TPHL：E1 下降（NEG），Y0 下降（NEG），写 Param 5
	MeasureTpd("DELAY_E1", E1_DIO_CH, TMU_SLOPE_NEG, TMU_SLOPE_NEG, 5);

	//POWER_OFF / DISCONNECT
	// 时序测试公共下电：I/O 归零、VCC 归零、断开
	TimingTestPowerOff();

	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

/*
 * Tt：输出转换时间（Y0）
 * VCC = 4.5V，测量点 10% ~ 90% VCC
 * 参数：0 TTLH（上升时间）  1 TTHL（下降时间）
 * 正常：0 ~ 19ns
 */
USER_CODE void Tt() {
	TEST_BEGIN

	//CONNECT / POWER_ON
	// 时序测试公共上电
	TimingTestPowerOn();

	// 测量 Y0 的上升/下降时间，结果写 Param 0/1
	MeasureTt();

	//POWER_OFF / DISCONNECT
	// 时序测试公共下电
	TimingTestPowerOff();

	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}

USER_CODE void SetPassBin() {
	TEST_BEGIN

	// 全部测试项通过后，把最终 Bin 置为 1（Pass）
	binObj.SetBin(1);

	TEST_ERROR
	binObj.HandlerException(0);
	TEST_END
}
