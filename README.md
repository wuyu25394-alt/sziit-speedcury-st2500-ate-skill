# 集成电路测试知识库 · 也是一个 Agent Skill

> **一句话**：把加速科技 **ST2516 / ST2500** 测试平台的整套课程资料（24 份 PDF、1 份 API 编程手册 CHM、6 个完整 ATE 工程），整理成 **27 篇中文笔记**，并打包成一个**可以被 AI 直接加载的技能（Skill）**。
>
> **两种用法**：① 给人读（照着下面的导航表翻）；② 装成 Skill 给 AI 用（见「安装」一节，一条命令）。
>
> **来源**：本仓库是对加速科技 / 芯星计划课程资源的整理与摘录。**原始资料著作权归各权利人所有，请勿商用**，详见 [NOTICE.md](NOTICE.md)。

| | |
|---|---|
| 笔记规模 | **27 篇**，约 **85 万字节**（57 万字符） |
| 覆盖原始资料 | 24 份 PDF（含 **433 页**教学课件）、1 份 API 编程手册（CHM）、1 份 DOCX、**6 个完整 ATE 工程** |
| 实训案例 | CD4511、HK24C02、LM358、TPS73625、HK32F031F4P6、ADC0832（**47 条 FuncList** 逐条分析） |
| 技能名 | `ic-testing-notes` |
| 技能入口 | [`SKILL.md`](SKILL.md) |

---

## 一、安装：把它装成 AI 技能

本仓库根目录就是**技能包本体**（`SKILL.md` + 全部笔记），因此「克隆到技能目录」即可，**不需要移动或复制任何文件**。

### 1.1 DeepSeek Harness（DSH）

```powershell
git clone https://github.com/wuyu25394-alt/sziit-speedcury-st2500-ate-skill "$env:USERPROFILE\.dsh\skills\ic-testing-notes"
```

装完**新建一个会话**（或重启 DSH），技能目录里就会出现 `ic-testing-notes`，描述为"集成电路测试（ATE）知识库与实操手册…"。

### 1.2 其他支持 Agent Skills 约定的工具

`SKILL.md` + YAML frontmatter（`name` / `description`）是通用的 Agent Skills 约定。DSH 除此之外还会扫描 `~/.agents/skills`；其他工具的技能目录请以各自文档为准（例如 Claude Code 用 `~/.claude/skills`）：

```powershell
git clone https://github.com/wuyu25394-alt/sziit-speedcury-st2500-ate-skill "$env:USERPROFILE\.agents\skills\ic-testing-notes"
```

### 1.3 验证安装

```powershell
# 文件在不在
Test-Path "$env:USERPROFILE\.dsh\skills\ic-testing-notes\SKILL.md"

# frontmatter 是否正常（应输出 name: ic-testing-notes）
Get-Content "$env:USERPROFILE\.dsh\skills\ic-testing-notes\SKILL.md" -TotalCount 2
```

> ⚠️ **前提**：`SKILL.md` **必须以 `---` 作为文件第一个字符**，frontmatter 才能被解析。用编辑器另存时**不要加 UTF-8 BOM**，也不要在开头插空行或注释。

### 1.4 更新与卸载

```powershell
# 更新
git -C "$env:USERPROFILE\.dsh\skills\ic-testing-notes" pull

# 卸载：删掉目录即可
Remove-Item -Recurse -Force "$env:USERPROFILE\.dsh\skills\ic-testing-notes"
```

### 1.5 装完之后能问什么

技能生效后，可以直接问这类问题（AI 会自己去读对应笔记）：

- 「SMU 是什么？它和 DPS 有什么区别？」
- 「帮我写一个 CD4511 的开短路测试函数，用 FIMV 模式」
- 「HK32F031F4P6 的 VOH/VOL 为什么要先输入 PassCode？」
- 「`.tmf` 和 `.pat` 有什么区别？我改了限值为什么没生效？」
- 「我的漏电流测试结果全是几百微安，可能是什么问题？」
- 「用这道题考我：直流参数测试都有哪些项，各自的测试条件是什么？」

### 1.6 不走代理访问（GitHub 直连不通时）

**背景**：`github.com` 在国内常直连不通（git 会报 `Connection was reset`）。以下方式**实测可用**，按推荐度排序。

#### 方式 A（最推荐，长期稳定）：镜像到 Gitee 等国内托管

Gitee 直连无阻。把仓库推一份过去后，`clone` / `pull` / `push` **全部不需要代理**：

```powershell
# 1) 先在 https://gitee.com 新建一个空仓库（不要勾选"使用 README 初始化"）
# 2) 加一个国内远程并推送
cd D:\集成电路测试
git remote add gitee https://gitee.com/<你的用户名>/ic-testing-notes.git
git push -u gitee main

# 3) 之后从国内源克隆，不需要任何代理
git clone https://gitee.com/<你的用户名>/ic-testing-notes.git
```

日常同步：

```powershell
git pull gitee main      # 从国内源拉（不需要代理）
git push gitee main      # 推到国内源（不需要代理）
git push origin main     # 推回 GitHub（需要代理）
```

> **GitCode**（`gitcode.com`）同样直连可用，做法完全一致。

#### 方式 B（零配置，立刻可用）：公共反代作为**只读**远程

```powershell
# 直接克隆
git clone https://ghproxy.net/https://github.com/wuyu25394-alt/sziit-speedcury-st2500-ate-skill.git
# 备选反代：https://ghfast.top/https://github.com/wuyu25394-alt/sziit-speedcury-st2500-ate-skill.git
```

加成第二个远程，之后拉取都走反代：

```powershell
git remote add mirror https://ghproxy.net/https://github.com/wuyu25394-alt/sziit-speedcury-st2500-ate-skill.git
git remote set-url --push mirror no_push    # ★ 必须：禁止经第三方推送
git pull mirror main
```

> ⚠️ **只用于读。** 公共反代会看到你的请求内容；**绝不要通过它推送**——那等于把你的 GitHub 凭据交给第三方。上面第二行的 `--push no_push` 就是为此。
> 反代是第三方服务，**可用性无保证**，随时可能失效或限速。长期稳定请用方式 A。

#### 方式 C（单文件浏览/下载）：jsDelivr CDN

不需要 git，直接在浏览器取任意文件：

```
https://cdn.jsdelivr.net/gh/wuyu25394-alt/sziit-speedcury-st2500-ate-skill@main/README.md
https://cdn.jsdelivr.net/gh/wuyu25394-alt/sziit-speedcury-st2500-ate-skill@main/SKILL.md
https://cdn.jsdelivr.net/gh/wuyu25394-alt/sziit-speedcury-st2500-ate-skill@main/01-理论基础/1.4-术语速查表（含SMU详解）.md
```

#### 实测结论（不走代理）

| 可直连 | 不可直连 |
|---|---|
| `raw.githubusercontent.com`、**Gitee**、**GitCode**、**`cdn.jsdelivr.net`**、**`ghproxy.net`**、**`ghfast.top`** | **`github.com` 本体**、`gh-proxy.com`、`github.moeyy.xyz`、`bgithub.xyz`、gitmirror 系（DNS 解析失败） |

#### 本仓库已配好的远程

| 远程名 | 地址 | 用途 | 需要代理 |
|---|---|---|---|
| `origin` | `github.com/wuyu25394-alt/sziit-speedcury-st2500-ate-skill` | 主仓库，可拉可推 | **需要** |
| `mirror` | `ghproxy.net/...` | **只读**镜像，`fetch` / `pull` | **不需要**（push 已禁用） |

一条命令拉最新（不需要代理）：

```powershell
git pull mirror main
```

> 同样适用于技能安装目录：`git -C "$env:USERPROFILE\.dsh\skills\ic-testing-notes" pull mirror main`

#### 排错：推送报 `Could not connect to server ... proxy 127.0.0.1`

说明本地代理（Clash / FlClash 的 `127.0.0.1:7890`）**当前没在运行**。两条路：

```powershell
# A. 在 Clash / FlClash 里把代理打开，然后正常推送
git push origin main

# B. 代理没开、但 github.com 恰好能直连时，临时绕开代理配置推送
git -c http.https://github.com.proxy= push origin main
```

> 代理是否可用、github.com 能否直连，**随网络环境变化**（实测同一台机器一天内两种状态都出现过）。所以上面那两条命令都记着；`mirror` 远程不受影响，**读取永远不需要代理**。

---

## 二、给人读：内容导航

### 📘 01-理论基础

| 文件 | 内容 |
|---|---|
| [1.1 半导体产业与测试概论](01-理论基础/1.1-半导体产业与测试概论.md) | 产业链四环节、产业数据、IC 测试定义、CP/FT 流程、术语表、测试程序开发流程、五类岗位、测试机发展史、加速科技产品线 |
| [1.2 直流参数测试原理与方法](01-理论基础/1.2-直流参数测试原理与方法.md) | ★ **理论核心**：OS 连接性测试、ESD 保护二极管、串行/并行测试、漏电流 IIL/IIH、IDD、VOH/VOL、VIH/VIL、**五种测量模式** |
| [1.3 交流参数测试与功能测试原理](01-理论基础/1.3-交流参数测试与功能测试原理.md) | Tpd、建立/保持/上升/下降时间；Pattern 定义与 8 个符号、Pattern 生成（真值表/WGL/STIL/VCD）、时序四概念、Pin Level 设定 |
| [1.4 术语速查表（含 SMU 详解）](01-理论基础/1.4-术语速查表（含SMU详解）.md) | ★ **词典**：全课程术语 + **SMU 详解（8.1~8.11 节）** + 缩略语 A–Z |

### 🔧 02-测试机与硬件资源

| 文件 | 内容 |
|---|---|
| [2.1 ST2500 测试平台硬件总览](02-测试机与硬件资源/2.1-ST2500测试平台硬件总览.md) | 平台组成、机箱与指示灯、主控板、DFB32 业务板、**AFE 资源规格**、SMU 专题、通道编号、接口板与线缆、32 脚 DUT 插槽定义、系统校准、安全告警 |
| [2.2 电源模块与测量方法（课件图表）](02-测试机与硬件资源/2.2-电源模块与测量方法（课件图表）.md) | 整机规格、**5 种电源资源规格对比大表**、模式支持矩阵、4 线/2 线连接、Gang 规则、API |
| [2.3 DIO 数字模块详解（课件图表）](02-测试机与硬件资源/2.3-DIO数字模块详解（课件图表）.md) | DIO 指标、Timing 波表与 **WFC 沿状态 13 项定义**、微指令 1~20、PinLevel 7 项 |

### 💻 03-软件开发与工程配置

| 文件 | 内容 |
|---|---|
| [3.1 软件环境与 IDE 操作流程](03-软件开发与工程配置/3.1-软件环境与IDE操作流程.md) | 运行环境、四个登录入口、开发流程、Datalog 导出、AFE 调试工具 |
| [3.2 工程文件体系详解](03-软件开发与工程配置/3.2-工程文件体系详解.md) | `.job/.res/.sig/.grp/.tim/.pat/.tmf/Test.cpp/interface.h/Makefile` 逐字段说明 |
| [3.3 API 编程手册与测量模式](03-软件开发与工程配置/3.3-API编程手册与测量模式.md) | `USER_CODE` 与三段式宏、17 个顶层 API 对象方法表、微指令、**五种测量模式**、数据类型、报错 |
| [3.4 编程手册附录-API 对象索引](03-软件开发与工程配置/3.4-编程手册附录-API对象索引.md) | CHM 文件 → API 对象映射表 |

### 🧪 04-实训平台与硬件接线

| 文件 | 内容 |
|---|---|
| [4.1 实训平台与通用接口板使用指南](04-实训平台与硬件接线/4.1-实训平台与通用接口板使用指南.md) | 从零上手 11 步、**32 针 DUT 插槽信号表全文**、5 个案例模块资源配置与继电器表、上下电顺序、**六款案例逐款接线方法** |

### 🔬 05-实训案例

| 文件 | 案例 | 测试项 | 亮点 |
|---|---|---|---|
| [5.0 六案例总览与横向对比](05-实训案例/5.0-六案例总览与横向对比.md) | — | — | ★ **先看这个**：难度排序、资源覆盖矩阵、共同套路、测试项总表、**课件 vs 工程限值冲突** |
| [5.1 CD4511 数字译码器](05-实训案例/5.1-CD4511数字译码器.md) | CD4511 | 7 | 入门；BCD→七段真值表；CMOS 拉电流特性 |
| [5.2 HK24C02 存储芯片](05-实训案例/5.2-HK24C02存储芯片.md) | HK24C02 | 5 | I²C 四种时序；**上拉电阻与 DC 测试的矛盾** |
| [5.3 LM358 运算放大器](05-实训案例/5.3-LM358运算放大器.md) | LM358 | 5 | 虚短/虚断；Uos/Ios/CMRR/Aus/PSRR/PD；差值法与多次平均 |
| [5.4 TPS73625 电源管理芯片](05-实训案例/5.4-TPS73625电源管理芯片.md) | TPS73625 | 8 | VOUT/ICL/VDO/IGND/LDR/LNR/PSRR；电阻替代电流源 |
| [5.5 HK32F031F4P6 MCU](05-实训案例/5.5-HK32F031F4P6-MCU.md) | HK32F031F4P6 | **13** | **PassCode 测试模式**；TMU 测频；11 个 Pattern |
| [5.6 ADC0832 数模混合芯片](05-实训案例/5.6-ADC0832数模混合芯片.md) | ADC0832 | 3 | ★ 最高阶：AWG 激励 + DSIO 高速采集 + **自写 FFT 与 INL/DNL/SNR/ENOB** |
| [5.7 六个工程源码逐项分析](05-实训案例/5.7-六个工程源码逐项分析.md) | 全部 6 个 | — | 逐字段配置 + 代码数值级分析 + 横向对比 |
| [5.8 案例课件图表补充](05-实训案例/5.8-案例课件图表补充.md) | 全部 6 个 | — | 课件 **118 页图片型页面**的逐页转录 |

### 📄 06-芯片数据手册

| 文件 | 内容 |
|---|---|
| [6.1 六款芯片关键参数](06-芯片数据手册/6.1-六款芯片关键参数.md) | 每款：引脚定义表、极限参数、推荐工作条件、**DC 电气特性表**、AC/时序、真值表、**面向 ATE 的测试要点** |

### 🎯 07-实战与自测

| 文件 | 内容 |
|---|---|
| [7.1 从零开发一个测试工程](07-实战与自测/7.1-从零开发一个测试工程.md) | 开机登录 → 建工程 → 配置 → 编码 → 编译加载运行看日志，**完整流程速查卡** |
| [7.2 常见问题与踩坑清单](07-实战与自测/7.2-常见问题与踩坑清单.md) | ★ 概念误区 / 测量陷阱 / 工程 bug / 手册冲突 / **排错决策树** / 12 条铁律 |
| [7.3 自测题库（含答案）](07-实战与自测/7.3-自测题库（含答案）.md) | **52 题**含答案，按章节组织 |

### 📑 附录

| 文件 | 内容 |
|---|---|
| [99-原始资源索引](99-原始资源索引.md) | 原始 226 个文件 → 整理笔记的一一对照表、整理工具与方法 |
| [SKILL.md](SKILL.md) | 技能入口（给 AI 读） |

### 建议学习顺序

```
1.1 概论 → 1.4 术语表（当词典）
   ↓
1.2 直流参数 → 1.3 交流与功能          ← 理论核心
   ↓
2.1 硬件总览 → 2.2 电源模块 → 2.3 DIO  ← 认识设备
   ↓
7.1 从零开发 → 3.2 工程文件 → 3.3 API   ← 学会用软件
   ↓
5.0 总览 → 5.1 CD4511 → 5.2 HK24C02 → 5.3 LM358
        → 5.4 TPS73625 → 5.5 HK32F031F4P6 → 5.6 ADC0832   ← 按难度递增
   ↓
7.2 踩坑清单 → 7.3 自测题库
```

---

## 三、先看这里：SMU 是什么

> 这是本知识库最有代表性的一条内容，完整版见 [1.4 第 8 节](01-理论基础/1.4-术语速查表（含SMU详解）.md)。

**SMU = Source Measure Unit，中文叫「源测量单元」，业内俗称「源表」。** 有两层含义：

**① 通用本义**：把「精密源」（可强制电压或电流）与「精密测量」（可测电压或电流）集成在同一通道的仪器单元，核心是**四象限工作**。所有直流参数测试在仪器层面都是「用 SMU 干活」。

**② 本平台的具体资源**（`.sig` 文件 `AFEType` 里的一类板卡）：

| 项目 | 规格 |
|---|---|
| 板卡型号 | **SMU20**（独立电源板卡，与 DFB32、MWB8 并列） |
| 通道数 | **4**（SMU0~3），4 线连接 |
| **电压** | **1 V / 5 V / 10 V / 20 V 四档，最大 20 V**（全平台最高 → 手册称它"高压"） |
| **电流** | **±500 mA；Gang 模式最大 2 A** |
| **电压精度** | **±(0.05%+0.5 mV) ~ ±(0.05%+10 mV)**（全平台最好） |
| **电流档位** | **7 档：1 µA / 10 µA / 100 µA / 1 mA / 10 mA / 100 mA / 500 mA** |
| 变体 | `SMU`、`SMU_GANG2/3/4` |

**最关键的一点**：**DPS 只能 Force Voltage（不支持 FIMV/FIMI，不能当电流源），而 SMU 五种模式全支持，并且是全平台唯一支持 `FVMIMV` / `FIMVMI` 混合模式、两种钳位都支持的资源** —— 这才是它叫 "Source **Measure** Unit" 而不是 "Power Supply" 的实证。

> 💡 **整理时的一个发现**：使用手册里 SMU 只出现 5 处、只有一句定义、**没有任何电气指标**；完整规格其实藏在**教学课件的一张图片表**里。同一门课的不同资料必须交叉对照。

---

## 四、已知的资料口径冲突（重要）

整理过程中发现原始资料**自相矛盾或明显笔误**，已如实记录、**未擅自修改**。使用时请特别注意：

| 冲突项 | 情况 |
|---|---|
| **课件限值 vs 工程 `.tmf` 限值** | CD4511 的 OS / VOH / IIL、TPS73625 的 OS 等多处不一致。**以工程 `.tmf` 为准** |
| **ADC0832 的 ENOB** | 课件写 **12~16 bit**，工程写 **7~8 bit**。ADC0832 是 8 位 ADC，**12~16 bit 物理上不可能** —— 课件是从 12/16 位 ADC 模板沿用未改 |
| CD4511 引脚定义 | 课件引脚表（6=BI、7=LT）与同一课件的文字（4=BI、3=LT）**互相矛盾** |
| HK32F031F4P6 的 VIH/VIL | 手册有**两套规格**（0.9·VDD 与线性公式），VDD=3.3 V 时分别为 **2.97 V** 与 **1.546 V**，差异极大，**需向原厂确认** |
| HK24C02 的 AC 规格 | 首页声称 5 V 时 1 MHz，但 AC 表**只给 1.8 V / 2.5 V 两档**，**不可外推** |
| LM358 的 VOS | **B 版 ±3 mV / 基础版 ±7 mV**，限值须按型号 + 温度档分档 |
| 工程代码 bug | TPS73625 `VDO_TEST` 量纲不匹配、`LDR_TEST` 漏 `.Execute()`；ADC0832 声明 6 个参数只判 5 个；HK32F031 组名混用、`PLL_TEST` 空结果向量 |
| 图片型 PDF | `HK24C02` 与 `CD4511BE` 两份数据手册的电气特性页**是图片、无文字层**，只读文本会完全丢失参数 |

**结论性建议**：**不要盲信单一资料**。冲突时优先工程 `.tmf`；数值违反物理常识时相信物理；两套规格并存时问原厂。

---

## 五、技能包结构

本仓库根目录即技能包，`SKILL.md` 提供 AI 入口，其余是它引用的知识本体：

```
sziit-speedcury-st2500-ate-skill/          ← 克隆到 skills/ 下就是这个技能
├── SKILL.md                      ← ★ 技能入口（frontmatter: name + description）
├── README.md                     ← 本文件（给人看）
├── NOTICE.md                     ← 资料来源与使用声明
├── 01-理论基础/           4 篇
├── 02-测试机与硬件资源/    3 篇
├── 03-软件开发与工程配置/  4 篇
├── 04-实训平台与硬件接线/  1 篇
├── 05-实训案例/           9 篇
├── 06-芯片数据手册/        1 篇
├── 07-实战与自测/          3 篇
├── 99-原始资源索引.md
├── 00-索引.md ~ 05-为什么需要片选CSB.md   ← 早前一次会话留下的 6 个文件（见下）
├── .gitignore
└── .gitattributes
```

> **技能发现规则**（DSH 实现）：技能根目录下，**一层子目录含 `SKILL.md`** 或**裸 `.md` 文件**各算一个技能；子目录内的其他 `.md` **不会被误认**。因此本仓库里 `README.md`、`NOTICE.md` 和 8 个章节目录**都不会**被当成额外技能。

### 关于目录里的 6 个早前文件

`00-索引.md`、`01-SMU与PMU.md`、`02-Si5351A时钟发生器.md`、`03-I2C总线接口.md`、`04-LMX2541控制接口.md`、`05-为什么需要片选CSB.md` 是**本知识库成形之前**由另一次学习会话留下的：

- **`01-SMU与PMU.md`** —— 与本主题高度相关，其要点（四象限、Compliance、SMU 与 PMU 的区别、Force/Sense 四线、时序与吞吐量）**已并入** [1.4 的 8.9~8.11 节](01-理论基础/1.4-术语速查表（含SMU详解）.md)；
- **`03-I2C总线接口.md`** —— 可作为 HK24C02 案例的 I²C 原理补充；
- **`02` / `04` / `05`** —— 属于**另一个项目**（Si5351A / LMX2541 时钟发生器方向），与本知识库无关，**保留原样未做改动**。

---

## 六、它是怎么做出来的

| 步骤 | 工具 | 规模 |
|---|---|---|
| PDF 文本提取 | Python `pypdf` | 24 份 PDF |
| PDF 渲染成图 | Python `pymupdf` | **222 个图片型课件页** + 两份图片型数据手册全部页 |
| 图片内容识读 | 视觉逐页识读 | 表格 / 原理图 / 波形图 / 真值表逐格转录 |
| CHM 解包 | `hh.exe -decompile` | 55 个文件 |
| 中文文档读取 | `python-docx` | 1 份 DOCX |
| 工程源码分析 | 直接读源码 | 6 个工程、**47 条 FuncList** 逐条 |

**准确性分级**：笔记中标注了出处页码/文件名可回溯的内容为可靠；来自图片识读的标注了"（图不清）"；发现的冲突**全部列出、未择一**。

**已知局限**：
- 只覆盖**加速科技 ST2516/ST2500 + MINI-ATE** 这一套平台，不代表其他 ATE 厂商的命名与工具链；
- 只含 **6 款芯片**参数，其他器件需另查原厂数据手册；
- 不含原始 PDF/CHM/DOCX 与工程二进制，也不含 Log 截图中的实测小数；
- 这是**二手整理**，关键参数请以原厂数据手册为准。

---

## 七、许可与引用

- 本仓库是**个人学习笔记的整理成果**，**不用于任何商业用途**；
- 原始资料（加速科技的技术手册与教学课件、各芯片原厂数据手册）的**著作权归其各自权利人所有**；
- **引用或转载时请注明资料来源与权利方**，不要声称对原始内容拥有著作权；
- 如果你是权利人，认为本仓库内容不当或超出合理引用范围，**请联系仓库所有者，会立即删除**；
- 完整声明见 [NOTICE.md](NOTICE.md)。

> ⚠️ **本仓库当前没有 `LICENSE` 文件**。没有许可证 = 默认"保留所有权利"，即**协作者在法律上并未获得修改与再分发的明确授权**。
> 既然要拉人一起维护，建议补一份许可证（例如 **CC BY-NC 4.0**，正好对应上面的「注明来源、不得商用」）。**此项待办。**

---

## 八、协作与更新流程（给协作者）

### 8.1 三种参与方式，先分清

| 你的身份 | 能做什么 | 需要什么 |
|---|---|---|
| **任何人**（仓库是 public） | clone 下来当技能用、读、fork | **不需要邀请** |
| **被邀请的协作者** | 上面的 + **直接修改并推送** | **`Write` 权限** |
| 只想提建议 | 开 Issue / PR | 不需要权限 |

> 邀请入口：仓库 **Settings → Collaborators and teams → Add people**。
> 角色必须是 **`Write`** 才能推送；`Read` 只能看，`Triage` 只能管 issue。
>
> ⚠️ **`Admin` 权限请谨慎给** —— 那等于把"改仓库可见性、删仓库、加别人"的能力也交出去。

### 8.2 当前分支保护状态

**`main` 分支没有保护规则**（`protected: false`），所以有 `Write` 权限的人**可以直接推送，不需要走 PR**。

这带来两个后果，请自觉：

- ✅ 方便：改完直接推，协作者 `pull` 就能看到
- ⚠️ 危险：**推错无法被 review 拦住**，且推坏了历史别人很难恢复

**约定（建议）**：

- **小改动**（错别字、补充参数、修正链接）→ 直接推 `main`
- **大改动**（重构目录、改 `SKILL.md` 的描述、批量改限值）→ **开分支 + PR**，让至少一人看过再合
- **绝不要** `git push --force` 到 `main`

### 8.3 协作者怎么写

```powershell
# ① 首次：克隆到技能目录（同时也是你的工作副本）
git clone https://github.com/wuyu25394-alt/sziit-speedcury-st2500-ate-skill `
  "$env:USERPROFILE\.dsh\skills\ic-testing-notes"

# ② 每次动手前先同步，避免冲突
git -C "$env:USERPROFILE\.dsh\skills\ic-testing-notes" pull --ff-only

# ③ 改文件……（笔记是 Markdown，直接编辑即可）

# ④ 提交并推送
cd "$env:USERPROFILE\.dsh\skills\ic-testing-notes"
git add -A
git commit -m "5.1: 补充 CD4511 课件的另一套限值口径"
git push origin main
```

**装完/更新完记得新建会话或重启 DSH**，技能才会被重新加载。

### 8.4 提交前必须检查的四件事

1. **不要提交敏感信息** —— API key、token、本机绝对路径（含用户名的）、Log 里的实测截图。
   `.gitignore` 已排除常见产物，但它拦不住你手写进去的内容。
2. **不要提交大文件** —— 原始 PDF / CHM / 课件、工程二进制（`Test.dll`）、`node_modules`。
   本仓库**有意不含**这些（见第六节「已知局限」）。真需要放，先问仓库所有者，并考虑 Git LFS。
3. **保持 `SKILL.md` 的 frontmatter 完好** —— 它**必须以 `---` 作为文件第一个字符**，
   **不要加 UTF-8 BOM**，不要在开头插空行。改坏了整个技能就加载不了。
4. **凡是手册/课件来的数值，先声明出处** —— 本仓库的引用纪律是"标注页码/文件名可回溯"（见第七节）。
   把图片识读的不确定值写成确定值，是最容易造成误导的一类改动。

### 8.5 冲突怎么办

两个人改了同一篇笔记时：

```powershell
git pull --rebase origin main     # 把你的改动接到别人之后

# 有冲突时 Git 会停下来并告诉你哪些文件冲突
# 手工改好冲突文件后：
git add <冲突文件>
git rebase --continue
```

**不要**用 `git push --force` 图省事 —— 那会抹掉别人的提交。

### 8.6 网络：国内必须走代理

`github.com` 直连不通（实测），必须开启代理（Clash / FlClash 等，本机端口 **7890**）。
本仓库的 git 配置里已按主机名设了代理：

```
http.https://github.com.proxy = http://127.0.0.1:7890
```

**代理没开时会报**：`Could not connect to server ... proxy 127.0.0.1`

三种处理方式：

```powershell
# A. 把代理软件打开（首选）
git push origin main

# B. 代理没开、但 github.com 恰好能直连时，临时绕开代理配置
git -c http.https://github.com.proxy= push origin main

# C. 长期不用代理：加 no_proxy（对当前会话生效）
$env:no_proxy = "github.com"
```

> ⚠️ 注意：**只读镜像 `mirror`（ghproxy.net）不能用于推送**，它的 pushurl 已被设为 `no_push`。
> 公共反代会看到你的请求内容，**绝不要通过它推送** —— 那等于把凭据交给第三方。
