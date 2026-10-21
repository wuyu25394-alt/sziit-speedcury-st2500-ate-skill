#!/usr/bin/env python3
"""检查这本书的结构与链接。

用法：
    python3 scripts/check_book.py            # 检查全部
    python3 scripts/check_book.py --quiet    # 只报错

检查项：
  1. 相对链接是否指向存在的文件
  2. 每章是否有标题、📍 位置框、本章重点、延伸阅读、导航栏
  3. 提示框里的字段是否写成了 `> - ` 列表项
  4. 每个部 / 章目录下是否有 README.md
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

BOOK_DIR = ROOT / "zh-CN"
ROOT_FILES = [
    "README.md",
    "INSTALL.md",
    "CONTRIBUTING.md",
    "STYLE_GUIDE.md",
    "KNOWN-ISSUES.md",
    "NOTICE.md",
    "SKILL.md",
    "templates/chapter-template.md",
]

LINK_RE = re.compile(r"\[[^\]]*\]\(\s*<?([^)\s>]+)>?\s*\)")
SKIP_PREFIX = ("http://", "https://", "mailto:", "#", "tel:")

BOX_FIELDS = (
    "定义",
    "现场用法",
    "相关名词",
    "常见混淆",
    "情境",
    "做法",
    "为什么",
    "没做会怎样",
    "相关章节",
)
BOX_FIELD_RE = re.compile(
    r"^> \*\*(?:" + "|".join(BOX_FIELDS) + r")\*\*"
)
CHAPTER_DIR_RE = re.compile(r"^\d{2}-")


def iter_markdown() -> list[Path]:
    files: list[Path] = []
    for name in ROOT_FILES:
        p = ROOT / name
        if p.exists():
            files.append(p)
    if BOOK_DIR.exists():
        files.extend(sorted(BOOK_DIR.rglob("*.md")))
    return files


def strip_code_blocks(text: str) -> str:
    """把 ``` 围栏与行内 `代码` 换掉，保留行号。"""
    out: list[str] = []
    in_fence = False
    for line in text.splitlines():
        if line.lstrip().startswith("```"):
            in_fence = not in_fence
            out.append("")
            continue
        if in_fence:
            out.append("")
        else:
            out.append(re.sub(r"`[^`]*`", "``", line))
    return "\n".join(out)


def is_template(path: Path) -> bool:
    return path.parent.name == "templates"


def check_links(path: Path) -> list[str]:
    errors: list[str] = []
    text = strip_code_blocks(path.read_text(encoding="utf-8"))
    for lineno, line in enumerate(text.splitlines(), 1):
        for target in LINK_RE.findall(line):
            if target.startswith(SKIP_PREFIX):
                continue
            if not target or target.startswith("{{"):
                continue
            clean = target.split("#", 1)[0]
            if not clean:
                continue
            resolved = (path.parent / clean).resolve()
            try:
                resolved.relative_to(ROOT)
            except ValueError:
                errors.append(f"{rel(path)}:{lineno} 链接跑到仓库外：{target}")
                continue
            if not resolved.exists():
                errors.append(f"{rel(path)}:{lineno} 链接失效：{target}")
    return errors


def check_chapter(path: Path) -> list[str]:
    errors: list[str] = []
    raw = path.read_text(encoding="utf-8")
    text = strip_code_blocks(raw)
    lines = text.splitlines()

    if not lines or not lines[0].startswith("# 第 "):
        errors.append(f"{rel(path)} 第一行应为「# 第 N 章　标题」")

    if "📍" not in text:
        errors.append(f"{rel(path)} 缺 📍 测试旅程位置框")
    if "## 本章重点" not in text:
        errors.append(f"{rel(path)} 缺「## 本章重点」")
    if "## 延伸阅读" not in text:
        errors.append(f"{rel(path)} 缺「## 延伸阅读」")
    if "｜ [目录]" not in text:
        errors.append(f"{rel(path)} 缺导航栏（… ｜ [目录](...) ｜ …）")

    note_count = text.count("> [!NOTE]")
    tip_count = text.count("> [!TIP]")
    if note_count < 2:
        errors.append(f"{rel(path)} 名词框只有 {note_count} 个（至少 2）")
    if tip_count < 2:
        errors.append(f"{rel(path)} Know-how 框只有 {tip_count} 个（至少 2）")

    for lineno, line in enumerate(lines, 1):
        if BOX_FIELD_RE.match(line):
            errors.append(
                f"{rel(path)}:{lineno} 框内字段必须是列表项，改成「> - **…**」"
            )
    return errors


def check_dirs() -> list[str]:
    errors: list[str] = []
    if not BOOK_DIR.exists():
        return ["找不到 zh-CN/ 目录"]
    for part in sorted(p for p in BOOK_DIR.iterdir() if p.is_dir()):
        if not (part / "README.md").exists():
            errors.append(f"{rel(part)}/ 缺 README.md（部首页）")
        for chapter in sorted(p for p in part.iterdir() if p.is_dir()):
            if not CHAPTER_DIR_RE.match(chapter.name):
                continue
            if not (chapter / "README.md").exists():
                errors.append(f"{rel(chapter)}/ 缺 README.md（章正文）")
    return errors


def check_hygiene(path: Path) -> list[str]:
    """粗查几种常见的排版问题。"""
    errors: list[str] = []
    text = path.read_text(encoding="utf-8")
    if text.startswith("\ufeff"):
        errors.append(f"{rel(path)} 文件带 UTF-8 BOM")
    if "<!-- source:" in text:
        errors.append(f"{rel(path)} 残留参考仓库的 source 注释")
    for lineno, line in enumerate(text.splitlines(), 1):
        if "\t" in line:
            errors.append(f"{rel(path)}:{lineno} 出现制表符")
        if line.rstrip() != line:
            errors.append(f"{rel(path)}:{lineno} 行尾有多余空格")
    return errors


def rel(path: Path) -> str:
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return str(path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--quiet", action="store_true")
    args = parser.parse_args()

    files = iter_markdown()
    errors: list[str] = []

    errors.extend(check_dirs())

    chapter_count = 0
    for path in files:
        if not is_template(path):
            errors.extend(check_links(path))
        errors.extend(check_hygiene(path))
        parent = path.parent
        if (
            path.name == "README.md"
            and parent.parent.name
            and CHAPTER_DIR_RE.match(parent.name)
        ):
            chapter_count += 1
            errors.extend(check_chapter(path))

    if not args.quiet:
        print(f"扫描 {len(files)} 个 Markdown 文件，{chapter_count} 章。")

    if errors:
        print(f"\n发现 {len(errors)} 个问题：\n")
        for e in errors:
            print("  - " + e)
        return 1

    print("全部通过。")
    return 0


if __name__ == "__main__":
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    raise SystemExit(main())
