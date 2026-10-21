# 把这本书装成一个 AI 技能

本仓库根目录同时就是**技能包本体**（`SKILL.md` + 全部笔记），因此「克隆到技能目录」即可，不需要移动或复制任何文件。

技能名：`ic-testing-notes`

---

## 一、安装

### 1.1 DeepSeek Harness（DSH）

```powershell
git clone https://github.com/wuyu25394-alt/ic-testing-notes-groupA "$env:USERPROFILE\.dsh\skills\ic-testing-notes"
```

装完**新建一个会话**（或重启 DSH），技能目录里就会出现 `ic-testing-notes`。

### 1.2 其他支持 Agent Skills 约定的工具

`SKILL.md` + YAML frontmatter（`name` / `description`）是通用的 Agent Skills 约定。DSH 除此之外还会扫描 `~/.agents/skills`；其他工具的技能目录请以各自文档为准（例如 Claude Code 用 `~/.claude/skills`）：

```powershell
git clone https://github.com/wuyu25394-alt/ic-testing-notes-groupA "$env:USERPROFILE\.agents\skills\ic-testing-notes"
```

---

## 二、验证

```powershell
# 文件在不在
Test-Path "$env:USERPROFILE\.dsh\skills\ic-testing-notes\SKILL.md"

# frontmatter 是否正常（应输出 name: ic-testing-notes）
Get-Content "$env:USERPROFILE\.dsh\skills\ic-testing-notes\SKILL.md" -TotalCount 2
```

> [!IMPORTANT]
> **前提**：`SKILL.md` **必须以 `---` 作为文件第一个字符**，frontmatter 才能被解析。用编辑器另存时**不要加 UTF-8 BOM**，也不要在开头插空行或注释。

---

## 三、更新与卸载

```powershell
# 更新
git -C "$env:USERPROFILE\.dsh\skills\ic-testing-notes" pull

# 卸载：删掉目录即可
Remove-Item -Recurse -Force "$env:USERPROFILE\.dsh\skills\ic-testing-notes"
```

---

## 四、不走代理访问

GitHub 直连不稳时，有三条路可以不用代理：

| 方式 | 地址 | 能力 |
|---|---|---|
| Gitee 镜像 | 自行在 Gitee 导入本仓库 | 可读写，需注册 |
| 公共反代 | `https://ghproxy.net/https://github.com/wuyu25394-alt/ic-testing-notes-groupA` | **只读**，可 `clone` / `pull`，不能 `push` |
| jsDelivr CDN | `https://cdn.jsdelivr.net/gh/wuyu25394-alt/ic-testing-notes-groupA@main/` | 只读单文件，适合取某一篇 |

> [!CAUTION]
> 公共反代与 CDN 都是第三方服务，它们能看到你请求了什么 URL。**不要通过它们传输凭据**，也不要把 token 写进 clone 地址。

---

## 五、推送报错怎么办

Git 在代理没开的时候会直连失败，报 `Failed to connect to github.com:443 over proxy 127.0.0.1 after 1 ms`。两种处理：

```powershell
# 一次性绕开代理配置
git -c http.https://github.com.proxy= push origin main

# 或者长期配置（代理常开时）
git config --global http.https://github.com.proxy http://127.0.0.1:7890
```

`gh` 命令不读 Windows 系统代理，需要单独设环境变量：

```powershell
$env:HTTPS_PROXY = 'http://127.0.0.1:7890'
$env:HTTP_PROXY  = 'http://127.0.0.1:7890'
```
