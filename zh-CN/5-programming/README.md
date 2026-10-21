# 第 5 部　写测试程序

前面几部讲的都是「想清楚」，这一部讲「写出来」。

测试机的软件有一个特点：它把一件事拆成了七种文件。为什么拆？因为改动频率不一样——换一个引脚分配只需改 `.sig`，换一个时序只需改 `.tim`，而 C++ 代码一次都不用动。理解了这套拆分逻辑，配置和代码就不再是一堆要背的格式。

| 章节 | 你会读到 |
|---|---|
| [第 16 章　工程文件体系：七种文件各管什么](16-project-files/README.md) | `.job` `.tmf` `.sig` `.grp` `.res` `.tim` `.pat` 的分工与依赖 |
| [第 17 章　Test.cpp：用户代码怎么被调用](17-test-cpp/README.md) | DLL 导出、生命周期函数、`CheckResultAndBin`、异常与 Bin |
| [第 18 章　七种测量模式：强制什么、测什么](18-measurement-modes/README.md) | 每种模式强制什么、测什么、该配哪个钳位 |
| [第 19 章　调试与排错：连不上、不生效、对不上](19-debugging/README.md) | 连接失败、DLL 不更新、参数个数不一致、读数离谱 |

---

[← 第 4 部　数字与功能测试](../4-digital-tests/README.md) ｜ [目录](../README.md) ｜ [第 6 部　六个实训案例 →](../6-case-studies/README.md)
