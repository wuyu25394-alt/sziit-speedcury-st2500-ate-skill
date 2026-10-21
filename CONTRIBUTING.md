# 贡献指南

谢谢愿意帮忙。这本书还在长，任何一条勘误、一个例子、一张图都有用。

---

## 一、先看看能做什么

| 想做 | 怎么做 |
|---|---|
| 报告错误（写错了、数值不对、链接失效） | 开一条 Issue，用「勘误」模板 |
| 提建议（讲不清、缺例子、顺序不对） | 开一条 Issue，用「内容建议」模板 |
| 直接改内容 | Fork → 改 → 提 Pull Request |
| 补图 | 见下面「关于图」 |

不确定算不算错误就直接开 Issue，不用纠结分类。

---

## 二、改稿之前先读规范

1. [STYLE_GUIDE.md](STYLE_GUIDE.md) —— 用词、排版、章节结构、三种提示框的写法。**必读**。
2. 找一章风格接近的看看，比如 [第 7 章 SMU](zh-CN/2-instrumentation/07-smu/README.md)。
3. 新建一章就从 [模板](templates/chapter-template.md) 开始。

---

## 三、改完自查

```bash
python3 scripts/check_book.py
```

> [!TIP]
> Windows 上有时 `python` 会被应用商店的占位程序接管，敲下去什么都不输出。这时改用 `python3`，或 `py -3 scripts/check_book.py`。

这个脚本会检查：

- 所有相对链接是否指向存在的文件
- 每章是否有标题、📍 位置框、`## 本章重点`、`## 延伸阅读`、导航栏
- 提示框里的字段是否都写成了 `> - ` 列表项

有一条报错就先修完再提 PR。

---

## 四、内容要求

- **事实要能追到出处**。规格数值、模式支持关系、文件名、API 名称，都必须能在 `notes/` 里找到对应；找不到就写量级，不要编。
- **不写公司机密**。只写公开资料与业界通用做法，不写任何公司的内部参数、流程、客户信息。
- **不写成产品手册**。以方法为主角，型号只是举例。
- **脱敏**。引用课件里的具体判据时，说明它来自哪个案例；不要把整张课件表搬进来。

---

## 五、关于图

- 流程图、时序图优先用 **Mermaid**，GitHub 直接渲染，文字跟着正文走。
- 需要矢量图用 **SVG**，放 `images/`，文件名「章节号-描述」，如 `07-smu-four-quadrant.svg`。
- **图上不放文字**。需要标注时用编号，在图下的说明里解释。
- 只用手绘或授权兼容的图，不要直接截课件、书籍、厂商网站的图。
- 还没画的图先用注释占位：`<!-- 图：SMU 四象限示意 -->`。

---

## 六、Pull Request

- 一个 PR 只做一件事。改一章就只改那一章，顺手全局重排会让 review 很难做。
- 标题写清楚：`勘误：第 9 章 IIL 判据单位写错`。
- 描述里写：改了什么、为什么改、依据在哪（引用 `notes/` 里的篇目或原始资料页码）。
- 如果是新增章节，先开 Issue 说一声，避免撞车。

---

## 七、授权

提交内容即表示同意以 [CC BY-NC-SA 4.0](LICENSE) 授权发布。请确认你有权提交这些内容。

---

## 八、协作与更新流程

### 8.1 三种参与方式

| 你的身份 | 能做什么 | 需要什么 |
|---|---|---|
| 任何人（仓库是 public） | clone 下来读、当技能用、fork | **不需要邀请** |
| 被邀请的协作者 | 上面的 + 直接修改并推送 | **`Write` 权限** |
| 只想提建议 | 开 Issue / PR | 不需要权限 |

> 邀请入口：仓库 **Settings → Collaborators and teams → Add people**，角色必须是 **`Write`** 才能推送。
> **`Admin` 权限请谨慎给**——那等于把「改可见性、删仓库、加别人」的能力也交出去。

### 8.2 分支保护现状

`main` 分支目前**没有保护规则**，有 `Write` 权限的人可以直接推，不必走 PR。因此约定：

- **小改动**（错别字、补参数、修链接）→ 直接推 `main`
- **大改动**（重构目录、改 `SKILL.md` 描述、批量改限值）→ **开分支 + PR**
- **绝不** `git push --force` 到 `main`

### 8.3 协作者怎么写

```powershell
# ① 首次：克隆到技能目录（同时也是工作副本）
git clone https://github.com/wuyu25394-alt/sziit-speedcury-st2500-ate-skill `
  "$env:USERPROFILE\.dsh\skills\ic-testing-notes"

# ② 动手前先同步，避免冲突
git -C "$env:USERPROFILE\.dsh\skills\ic-testing-notes" pull --ff-only

# ③ 改文件，然后跑一遍自查
python3 scripts/check_book.py

# ④ 提交并推送
cd "$env:USERPROFILE\.dsh\skills\ic-testing-notes"
git add -A
git commit -m "9: 修正 IIL 判据单位"
git push origin main
```

装完/更新完**新建会话或重启 DSH**，技能才会重新加载。

### 8.4 提交前必须检查的四件事

1. **不提交敏感信息**——API key、token、含用户名的本机绝对路径、Log 里的实测截图。
2. **不提交大文件**——原始 PDF / CHM / 课件、工程二进制（`Test.dll`）。本仓库有意不含这些；真需要放，先讨论并考虑 Git LFS。
3. **保持 `SKILL.md` 的 frontmatter 完好**——它**必须以 `---` 作为文件第一个字符**，不加 BOM，开头不插空行。改坏了整个技能就加载不了。
4. **手册/课件来的数值要带出处**——把图片识读的不确定值写成确定值，是最容易造成误导的一类改动。已知的口径冲突见 [KNOWN-ISSUES.md](KNOWN-ISSUES.md)。

### 8.5 冲突怎么办

```powershell
git pull --rebase origin main     # 把你的改动接到别人之后

# 有冲突时 Git 会停下来告诉你哪些文件冲突
# 手工改好后：
git add <冲突文件>
git rebase --continue
```

不要用 `git push --force` 图省事——那会抹掉别人的提交。

### 8.6 网络：国内需要代理

`github.com` 直连经常不通，建议开代理。仓库的 git 配置已按主机名设了代理：

```
http.https://github.com.proxy = http://127.0.0.1:7890
```

代理没开时会报 `Could not connect to server ... proxy 127.0.0.1`，三种处理：

```powershell
# A. 把代理打开（首选）
git push origin main

# B. 代理没开、但能直连时，临时绕开代理配置
git -c http.https://github.com.proxy= push origin main

# C. 长期不用代理（对当前会话生效）
$env:no_proxy = "github.com"
```

> [!CAUTION]
> 只读镜像（`mirror`，走 ghproxy.net）的 pushurl 已设为 `no_push`，**不能用于推送**。公共反代会看到你的请求内容，通过它推送等于把凭据交给第三方。
