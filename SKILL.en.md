🌐 **English** ｜ [简体中文](SKILL.md)

# IC Testing Knowledge Base (ATE / AccelTest ST2516·ST2500)

> This is the English edition of [`SKILL.md`](SKILL.md). The skill definition file used by agent runtimes is still `SKILL.md` (Chinese); this page is for English readers who want to understand and navigate the knowledge base.

## What this skill is

A **knowledge base for IC testing (ATE)**, re-organized by a student who read through all the course material (24 PDFs, 1 API programming manual in CHM, 6 complete ATE projects).

The repository has two layers — keep them apart when using it:

| Layer | Location | What it is |
|---|---|---|
| **Book** | `zh-CN/` | A 28-chapter open textbook, organized as "follow one chip"; good for reading in order or for answering concept/flow questions |
| **Notes** | `notes/` | 28 dense fact sheets (~870 KB) with heavy tables and page references; good for looking up exact fields, limits and code values |

**All content lives at relative paths inside this skill directory** — no network access needed. Read it with `read` / `grep`; the files are Chinese Markdown.

## How to use it: locate the file by task

| Task | Read first |
|---|---|
| Unfamiliar concept/term (including **SMU**, PPMU, BPMU, DPS, SBIN/HBIN, WFC…) | `notes/01-理论基础/1.4-术语速查表（含SMU详解）.md` |
| Want to see "how the book tells it", or a complete explanatory thread | `zh-CN/README.en.md` (English TOC) → `zh-CN/README.md` (full text) |
| DC parametric tests (OS, IIL/IIH, IDD, VOH/VOL, VIH/VIL) — principles and methods | `notes/01-理论基础/1.2-直流参数测试原理与方法.md` |
| Op-amp principles and parameters (Vos/Ib/Ios/AOL/CMRR/PSRR/GBW/SR, virtual short, UA741/LM358) | `notes/01-理论基础/1.5-运算放大器原理与模拟参数测试.md` |
| AC parameters / functional test (Pattern, Timing, Strobe, PinLevel) | `notes/01-理论基础/1.3-交流参数测试与功能测试原理.md` |
| Industry background, CP/FT flow, how a test program is developed | `notes/01-理论基础/1.1-半导体产业与测试概论.md` |
| Tester hardware, boards, resource specs, channel numbering, calibration | `notes/02-测试机与硬件资源/2.1-ST2500测试平台硬件总览.md` |
| Power resource comparison (DPS/BPMU/PPMU/RVS/**SMU**), measurement-mode matrix, Gang | `notes/02-测试机与硬件资源/2.2-电源模块与测量方法（课件图表）.md` |
| DIO digital module specs, WFC edge definitions, micro-instructions | `notes/02-测试机与硬件资源/2.3-DIO数字模块详解（课件图表）.md` |
| Writing/running test programs: IDE operations, build, load, run | `notes/03-软件开发与工程配置/3.1-软件环境与IDE操作流程.md`, `notes/07-实战与自测/7.1-从零开发一个测试工程.md` |
| Field-level meaning of project files (`.job/.tmf/.sig/.grp/.res/.tim/.pat`) | `notes/03-软件开发与工程配置/3.2-工程文件体系详解.md` |
| API manual, measurement modes, data types, error handling | `notes/03-软件开发与工程配置/3.3-API编程手册与测量模式.md` |
| Training platform / interface board wiring, per-case wiring for the six cases | `notes/04-实训平台与硬件接线/4.1-实训平台与通用接口板使用指南.md` |
| Six-case overview, test item lists, difficulty ranking, cross-comparison | `notes/05-实训案例/5.0-六案例总览与横向对比.md` |
| A specific case (CD4511 / HK24C02 / LM358 / TPS73625 / HK32F031F4P6 / ADC0832) | `notes/05-实训案例/5.1` ~ `5.6-*.md` |
| 74HC138 extended case (3-to-8 decoder, VOH/VOL, Tpd/Tt, virtual ATE environment) | `notes/05-实训案例/5.9-74HC138三线译码器（扩展案例）.md` |
| 74HC138 fully annotated source (1089 lines, in TMF order) | `notes/05-实训案例/74HC138/Test.cpp` |
| 74HC595 extended case (shift/latch/tri-state, IOZ, known-issue review) | `notes/05-实训案例/5.10-74HC595移位寄存器（扩展案例）.md` |
| Field-by-field analysis of project source (`.sig`/`.grp`/`.tim`/`.tmf` + code values) | `notes/05-实训案例/5.7-六个工程源码逐项分析.md` |
| Transcriptions of slide image pages (test item tables, pin tables, truth tables, flowcharts) | `notes/05-实训案例/5.8-案例课件图表补充.md` |
| Pinout / absolute maximum ratings / DC characteristics / truth tables / ATE test points for the six chips | `notes/06-芯片数据手册/6.1-六款芯片关键参数.md` |
| Troubleshooting, pitfalls, conflicting limits | `notes/07-实战与自测/7.2-常见问题与踩坑清单.md` |
| Practice questions | `notes/07-实战与自测/7.3-自测题库（含答案）.md` (52 questions with answers) |
| Mapping from original materials to notes | `notes/99-原始资源索引.md` |

## Hard facts you must know before answering

**1. SMU has two meanings.** General meaning = Source Measure Unit — source and meter in one, four-quadrant operation. On this platform it means one class of board in `.sig`, model **SMU20**, an **independent 4-channel high-voltage power board** (`ChType = Supply`), variants `SMU_GANG2/3/4`. Specs: voltage **1/5/10/20 V, four ranges (max 20 V)**, current **±500 mA (2 A in gang)**, voltage accuracy **±(0.05% + 0.5 mV) to ±(0.05% + 10 mV)**, current **7 ranges from 1 µA to 500 mA**.

**The essential difference from a DPS**: **a DPS can only force voltage (no FIMV/FIMI), while an SMU supports all five modes and is the only resource on the platform that supports the `FVMIMV`/`FIMVMI` hybrid modes and both clamp types** — that is why it is called a Source **Measure** Unit and not a Power Supply. See `notes/01-理论基础/1.4-…md` section 8 (8.1–8.11), or `zh-CN/2-instrumentation/07-smu/README.md`.

**2. There are seven measurement modes (not five, and never write "FMIM").** `FVMV`, `FVMI` (force voltage, measure current — for leakage/IDD), `FVMIMV`, `FIMV` (force current, measure voltage — for OS/VOH/VOL), `FIMI`, `FIMVMI`, `FNMV` (force nothing, measure voltage). Mnemonic: **to measure voltage use FIMV, to measure current use FVMI**; hybrid modes are SMU-only.

**3. How to read a channel number like `1:1:6`.** The authoritative definition is **`Adapter` (test head, 1-8) : `FBID/Slot` (business board, 1-5) : `Channel` (0-based)**, with **no "resource type" layer** (the resource type is given separately by the `AFEType` field in `.sig`). Some training-project documents write it as "board:resource:channel", which **is wrong**. Channel ranges: DPS 0-3, DIO 0-31, TMU 0-3, CBIT 0-7, AWG 0, DGT 0.

**4. Two iron rules (functional test).**
① **Never treat a DUT input pin as an output pin under test** — when a pattern does not care about an input pin, **do not give `X`** (`X` is the output mask state and makes the tester connect that pin to the comparator); give a definite `0`/`1` or use `P` to hold.
② **Never connect the tester's driver unit to a DUT output pin** — two sources fight, and **the one with less current is severely damaged**.

**5. When slides and the project disagree, the project `.tmf` wins.** Several inconsistencies have been found; the most typical: **the ADC0832 slides write ENOB as 12–16 bit while the project uses 7–8 bit** — the ADC0832 is an 8-bit ADC and `ENOB=(SINAD−1.76)/6.02`, so 12–16 bit is **physically impossible**; that slide column was carried over from a 12/16-bit ADC template. There are also CD4511 differences in OS/VOH/IIL limits. Full comparison in `notes/05-实训案例/5.0-…md` section 6.2.

**6. Real issues previously found in projects** (good code-review exercises): TPS73625's `LDR_TEST` is missing `.Execute()` (the "V/mV dimensional mismatch" claim in `VDO_TEST` was **disproved by on-board testing in 2026-11** — the platform auto-converts by the `.tmf` `Unit` field: mV ×1000, mA ×1000, uA ×1e6); ADC0832's `.tmf` declares 6 static parameters while the code judges only 5; HK32F031F4P6's `OSN_TEST` mixes group names and `PLL_TEST` judges a bin with an empty result vector.

## Source reliability and citation discipline

- Notes carry **manual page numbers / slide page numbers / project file names** so they can be traced back; **keep those citations** when quoting.
- Some content comes from **visual reading of image-only pages** (222 of the 433 pages across 12 slide decks are images; the `HK24C02` and `CD4511BE` datasheets are image-only PDFs). Notes mark uncertain values as "(图不清)" — **do not present such numbers as certain**.
- All discovered conflicts are **listed without picking a winner**; when answering about a conflicted item, **state that sources disagree and give each source's value**, not just one.
- **For key parameters, the chip vendor datasheet is authoritative**; this knowledge base is a secondary compilation.

## Limitations (do not answer beyond these)

- Covers only **AccelTest ST2516/ST2500 + MINI-ATE**; it does **not** represent the equivalent naming and toolchains of other ATE vendors (Advantest/Teradyne).
- The notes layer only has parameters for **six course chips plus the 74HC138/74HC595 extended cases** (CD4511, HK24C02, LM358, TPS73625, HK32F031F4P6, ADC0832, 74HC138, 74HC595); other devices require their own datasheets.
- Does not contain the original PDF/CHM/DOCX files, nor project binaries (`Test.dll`); does not contain the measured decimals from log screenshots.
- Copyright of the original materials belongs to **AccelTest / Xinxing Plan / the respective chip vendors** — see `NOTICE.md`. **Cite the source when reusing this material; commercial use is not allowed.**
