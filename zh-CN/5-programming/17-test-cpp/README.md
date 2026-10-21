# 第 17 章　Test.cpp：用户代码怎么被调用

> 📍 **测试旅程位置**
>
> - 前一站：七种配置文件各就各位
> - **本章：看唯一那份要人手写的 C++ 代码，平台到底在什么时候调用它**
> - 下一站：第 18 章把 API 里那七种测量模式一次讲透

---

## 平台是宿主，代码是插件

一份工程里，只有 `Test.cpp` 是人手写的，其余七份都是配置。它被编成动态库 `Test.dll`：`Build` 时由编译工具调 `Makefile` 完成，运行时由平台按名字调用。

这个形状解释三件事。

**为什么没有 `main()`。** 开始和结束不由代码决定。`Load` 之后，Runner 按 `.tmf` 排好的顺序逐项调用，代码只是被叫到的那一段。

**为什么函数名不能随便改。** `interface.h` 里有 `#define USER_CODE extern "C" __declspec(dllexport)`，测试项都写成 `USER_CODE void xxx()`，于是按 C 名字导出、不做修饰，平台拿着 `.tmf` 里 `FuncName` 的字符串就能找到同名函数。

**为什么改完代码要重编译。** `.sig`、`.tim`、`.tmf` 是运行时读的，保存就生效；`Test.dll` 是编译产物，改过 `.cpp` 必须 `Unload` → `Build` → 重新 `Load`。

在 `.tmf` 里删掉一个测试项，`Test.cpp` 里的函数体不会被删，只是被注释掉——按名字找不到，就不调用。

> [!NOTE]
> **📖 名词｜测试项（Test Item / Func）**
>
> - **定义**：测试流程里的一个步骤，在 `.tmf` 里占一行 Func，对应 `Test.cpp` 里的一个函数。
> - **现场用法**：「OS 那一项挂了，先看它读回来的是多少伏。」
> - **相关名词**：Test Flow、FuncList、Sbin、Datalog
> - **常见混淆**：和「一次测量」混。一个测试项里可以测 17 个引脚、上报 17 个参数，但只算一个 Func。

---

## 一个测试函数的骨架

```cpp
USER_CODE void OSN_TEST() {
TEST_BEGIN
    ppmu.Signal("OSPINS_GRP").SetMode("FIMV").CurrForce(-100.0e-6).CurrRange(100.0e-6).VoltClamp(2, -2).Execute();
    vector<ST_MEAS_RESULT> MEAS_RESULT;
    ppmu.Measure(MEAS_RESULT);
    binObj.CheckResultAndBin(0, MEAS_RESULT, 17);
TEST_ERROR
    binObj.HandlerException(0);
TEST_END
}
```

`TEST_BEGIN` 是正常流程的起点，同时开了一个异常捕获域；`TEST_END` 收尾；夹在中间的 `TEST_ERROR` 只在前面某句抛异常时才执行。三段位置固定。

`TEST_BEGIN` 到 `TEST_ERROR` 之间放业务：选信号、连接、设模式与量程、下发、延时、测量、判限。`TEST_ERROR` 到 `TEST_END` 之间只放一句 `HandlerException`——异常一抛，`TEST_BEGIN` 到出错点之间的收尾语句全被跳过，这时候 DUT 可能还带着电。

`HandlerException` 的参数只有两个取值：`0` 在工厂模式退出 ATE 系统、在开发模式停止测试；`1` 跳过当前函数、恢复运行环境、继续跑下一个。多数工程写 `0`，让问题停在第一现场。

> [!TIP]
> **🔧 Know-how｜异常段不写业务，收尾写在正常路径上**
>
> - **情境**：某个测试项偶发失败，Text Log 里只留一句异常，看不出是哪一步炸的。
> - **做法**：把断电、断开继电器这些收尾动作放在函数正常路径的末尾，`TEST_ERROR` 段只留 `binObj.HandlerException(0)`。
> - **为什么**：异常抛出后，出错点到 `TEST_ERROR` 之间的语句一条都不执行，指望它收尾等于没写。
> - **没做会怎样**：下一项开始时 DUT 还带着上一项的电，读数全乱，看着像「这一项坏了」，其实是上一项没收拾干净。
> - **相关章节**：[第 19 章 调试与排错](../19-debugging/README.md)

---

## 参数从哪里来

代码里那几个数字全部来自 `.tmf`。`binObj.CheckResultAndBin(0, MEAS_RESULT, 17)` 说的是：从这个 Func 的第 `0` 个子项起，拿向量里的 17 个值，逐个和对应子项的 `Min` / `Max` 比。

- **起始号**是子项的 `Param ID`，从 0 起。界面上第一个子项编号是 1000，代码里就是 `0`，第二个 1001 对应 `1`。
- **数量**是这次上报几个子项，必须与该 Func 在 `.tmf` 里实际挂的子项数一致。
- 也可以按名字定位：`CheckResult(site, "PA4", ...)` 里的字符串就是 `ParamName`。

还有一层隐式约定：结果向量第 i 个元素，对应从起始号数第 i 个子项。`.sig` 里信号在组内的排列、`.tmf` 里子项的顺序、`Measure()` 回来的顺序，三者必须一致。错位不会报错，只会把 A 脚的电压拿去比 B 脚的限值——看起来一片全 fail，其实一个都没测错。

> [!TIP]
> **🔧 Know-how｜给一项加引脚，三处一起改**
>
> - **情境**：某个测试项原来测 17 个脚，现在要多测一个。
> - **做法**：`.tmf` 里加一个子项，`.sig` 里把新信号放进同一个组并对齐顺序，再把代码里的数量从 17 改成 18。
> - **为什么**：判定是按位置对号入座的，不是按名字。谁排在第 18 位，第 18 个限值就比谁。
> - **没做会怎样**：多出来的那个值被丢掉，或者整段错位一格，良品被判成坏品。
> - **相关章节**：[第 16 章 工程文件体系](../16-project-files/README.md)

---

## 结果怎么变成 Bin

**判据在配置里，判定在代码里。**

`.tmf` 里每个 Func 记着 `Sbin` / `Hbin` / `Results`，每个子项记着 `Min` / `Max` / `Unit`；代码只负责把结果交上去。比限值、分档、写 Datalog 都由 `CheckResultAndBin` 一次完成。

结果向量有两种，同一个接口都收：电参数用 `vector<ST_MEAS_RESULT>`，成员是 `strSigName`、`ulSite`、`dbValue`；图形用 `vector<ST_DIO_RESULT>`，成员是 `ulSite`、`bPass`。

`Results` 决定判完往哪走：`Pass`、`Fail` 分别表示这一项通过、失败时被分到它记的 Bin，`None` 是不管对错都继续。整条流程走完、中途没被 `Fail` 分出去，最后落到 `SetPassBin`，里面只有一句 `binObj.SetBin(1)`：分 Bin 后执行 `Site_Reset`、清空当前有效 Site，一个回合结束。

> [!NOTE]
> **📖 名词｜结果向量（ST_MEAS_RESULT / ST_DIO_RESULT）**
>
> - **定义**：平台预定义的两种结果容器，分别装电参数值与图形比较结果，每个 Site 一条。
> - **现场用法**：「Measure 回来几个值，就交几个上去，别自己数。」
> - **相关名词**：`dbValue`、`bPass`、`ulSite`、Datalog
> - **常见混淆**：和 `ST_DIO_RECORD` 混。RECORD 里是 `bFail`，`true` 表示失效，极性与 `bPass` 正好相反。

---

## 一次完整调用的时序

**选 site → 按 Func 顺序调函数 → 配资源 → 测量 → 判 Bin → SetPassBin → site_reset**

1. 在 `.tmf` 里勾选测试项，设 `LoopCount` 与 `DoAll`；在 Site Configuration 里选要跑的 site。
2. 点 Run（`Ctrl+R`），Runner 按 `.tmf` 里 Func 的排列顺序，依次到 `Test.dll` 里找同名函数。
3. 进 `TEST_BEGIN` 之后：`Signal()` 选信号，`Connect()` 闭合继电器，`SetMode()` 定模式，再设量程、钳位、强制值，最后 `Execute()` 下发。
4. `sys.DelayUs` 等稳定，再 `Measure()` 取电参数，或 `dio.Pattern("...").Test()` 跑图形并 `GetFailResult()`。
5. 结果交给 `CheckResultAndBin`，按 `.tmf` 的限值判、按该 Func 的 `Sbin` / `Hbin` 分。
6. 全部项走完，`SetPassBin` 执行 `binObj.SetBin(1)`。
7. `site_reset` 做复位：断开继电器、强制值归零、必要时 `DisConnect`。
8. Datalog 一行记录一次测试：`SBin`、`SiteTime`、`Unit`、`Min`、`Max`，失败值标红。

第 3 到第 5 步任何一句抛异常，都会跳到 `TEST_ERROR`，这一项不留结果，也不分 Bin。

---

## 本章重点

1. 平台是宿主，代码是插件：`Test.cpp` 编成 `Test.dll`，函数靠 `USER_CODE`（即 `extern "C" __declspec(dllexport)`）按名字导出，没有 `main()`。
2. 每个测试项是一个被 `TEST_BEGIN` / `TEST_ERROR` / `TEST_END` 包住的函数，正常流程写在 BEGIN 之后，异常处理写在 ERROR 段。
3. 参数不在代码里定义，在 `.tmf` 里：起始子项号、数量、顺序三者对齐，`CheckResultAndBin` 才不会判错项。
4. 判据在配置、判定在代码：`Min` / `Max` / `Sbin` / `Hbin` 都写在 `.tmf`，代码只负责把结果交上去。
5. 改过 `Test.cpp` 必须 `Unload` → `Build` → 重新 `Load`，否则跑的还是旧 DLL。

---

## 延伸阅读

- 《ST2500 API 编程手册 V2.2》`14_API_BinDef.html`（`CheckResultAndBin` 的重载清单）
- 加速科技培训课件《测试案例-电源管理芯片（TPS73625）》
- 笔记库：[3.3 API 编程手册与测量模式](../../../notes/03-软件开发与工程配置/3.3-API编程手册与测量模式.md)

---

[← 上一章：第 16 章 工程文件体系：七种文件各管什么](../16-project-files/README.md) ｜ [目录](../../README.md) ｜ [下一章：第 18 章 七种测量模式：强制什么、测什么 →](../18-measurement-modes/README.md)
