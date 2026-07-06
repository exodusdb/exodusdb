#!/usr/bin/env python3
"""Report executable yield*/function* left in web JS/HTML after async migration.

Skips comments (//, /* */, block), framework files with intentional legacy
markers, and documentation. Exit code 1 if any executable issues found.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

WEB_ROOTS = [
    Path(__file__).resolve().parents[1] / "service/www/3/exodus",
    Path(__file__).resolve().parents[2] / "neosys/web/3",
]

SKIP_FILES = {"PROGRAMMERS_OVERVIEW.html"}

SKIP_REL_PATHS = {
    "scripts/client.js",
    "scripts/dbform.js",
    "scripts/gds.js",
    "scripts/exodus.js",
}

YIELD_RE = re.compile(r"yield\s*\*")
FUNCSTAR_RE = re.compile(r"function\s*\*")
EXODUS_ONCLICK_RE = re.compile(r'exodusonclick\s*=\s*["\']yield\s*\*')
SETTIMEOUT_RE = re.compile(r"exodussettimeout\s*\(\s*['\"]yield\s*\*")
ONLOAD_RE = re.compile(r'onload\s*=\s*["\']return\s+yield\s*\*')
DI_STRING_RE = re.compile(
    r"(di\.(popup|validation|defaultvalue|functioncode)|"
    r"\.validation\s*=|\.popup\s*=|\.defaultvalue\s*=|\.functioncode\s*=).*yield\s*\*"
)


def strip_js_comments(text: str) -> list[str]:
    """Return lines with block and line comments removed."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    out_lines: list[str] = []
    for line in text.splitlines():
        if "//" in line:
            line = line.split("//", 1)[0]
        out_lines.append(line)
    return out_lines


def classify_line(raw: str) -> list[str]:
    if "//" in raw:
        code = raw.split("//", 1)[0]
    else:
        code = raw
    if not code.strip():
        return []

    kinds: list[str] = []
    if FUNCSTAR_RE.search(code):
        kinds.append("function*")
    if YIELD_RE.search(code):
        kinds.append("yield*")
    if EXODUS_ONCLICK_RE.search(raw):
        kinds.append("exodusonclick")
    if SETTIMEOUT_RE.search(raw):
        kinds.append("exodussettimeout")
    if ONLOAD_RE.search(raw):
        kinds.append("onload")
    if DI_STRING_RE.search(raw):
        kinds.append("dict-string")
    return kinds


def scan_file(path: Path) -> list[tuple[int, list[str], str]]:
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = strip_js_comments(text)
    findings: list[tuple[int, list[str], str]] = []
    for i, line in enumerate(lines, 1):
        stripped = line.strip()
        if not stripped or stripped.startswith("*"):
            continue
        kinds = classify_line(line)
        if kinds:
            findings.append((i, kinds, stripped[:140]))
    return findings


def should_skip(path: Path, root: Path) -> bool:
    if path.name in SKIP_FILES:
        return True
    try:
        rel = path.relative_to(root).as_posix()
    except ValueError:
        return False
    return rel in SKIP_REL_PATHS


def main(argv: list[str]) -> int:
    roots = [Path(p) for p in argv[1:]] if len(argv) > 1 else WEB_ROOTS
    roots = [r for r in roots if r.is_dir()]

    all_findings: dict[Path, list] = {}
    by_kind: dict[str, int] = {}

    for root in roots:
        if not root.exists():
            continue
        for path in sorted(root.rglob("*")):
            if path.suffix.lower() not in {".htm", ".html", ".js"}:
                continue
            if should_skip(path, root):
                continue
            findings = scan_file(path)
            if findings:
                all_findings[path] = findings
                for _, kinds, _ in findings:
                    for k in kinds:
                        by_kind[k] = by_kind.get(k, 0) + 1

    print("=== yield*/function* migration verification ===\n")
    print(f"Files with executable issues: {len(all_findings)}")
    for k, v in sorted(by_kind.items()):
        print(f"  {k}: {v} line(s)")

    for root in roots:
        files = [p for p in all_findings if str(p).startswith(str(root))]
        if not files:
            continue
        print(f"\n--- {root} ---")
        for path in sorted(files):
            rel = path.relative_to(root)
            items = all_findings[path]
            print(f"\n{rel} ({len(items)} line(s))")
            for lineno, kinds, snippet in items[:10]:
                print(f"  L{lineno} [{','.join(kinds)}] {snippet}")
            if len(items) > 10:
                print(f"  ... +{len(items) - 10} more")

    return 1 if all_findings else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))