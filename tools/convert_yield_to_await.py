#!/usr/bin/env python3
"""Convert generator syntax to async/await in JS source code.

Preserves yield* inside string literals and comments (dynamic exodus code).
Skips lines marked with /* yield */ before function*.
"""

import re
import sys
from pathlib import Path


def _regex_prefix_ok(text: str, i: int) -> bool:
    """Heuristic: is '/' at i the start of a regex literal?"""
    j = i - 1
    while j >= 0 and text[j] in " \t\r\n":
        j -= 1
    if j < 0:
        return True
    prev = text[j]
    if prev in "(=[,;:!&|?{+-*%^~<>":
        return True
    # return / throw / case / in / typeof etc.
    return text[j - 3 : j + 1].endswith("return") or text[j - 5 : j + 1].endswith("throw")


def convert_js(text: str) -> str:
    out = []
    i = 0
    n = len(text)

    def peek(k=0):
        return text[i + k] if i + k < n else ""

    while i < n:
        ch = text[i]

        # regex literal
        if ch == "/" and peek(1) not in ("/", "*") and _regex_prefix_ok(text, i):
            out.append("/")
            i += 1
            in_class = False
            while i < n:
                c = text[i]
                if c == "\\":
                    out.append(c)
                    if i + 1 < n:
                        out.append(text[i + 1])
                        i += 2
                    else:
                        i += 1
                    continue
                if c == "[" and not in_class:
                    in_class = True
                elif c == "]" and in_class:
                    in_class = False
                out.append(c)
                i += 1
                if c == "/" and not in_class:
                    while i < n and text[i] in "gimsuy":
                        out.append(text[i])
                        i += 1
                    break
            continue

        # line comment
        if ch == "/" and peek(1) == "/":
            j = text.find("\n", i)
            if j == -1:
                out.append(text[i:])
                break
            out.append(text[i : j + 1])
            i = j + 1
            continue

        # block comment
        if ch == "/" and peek(1) == "*":
            j = text.find("*/", i + 2)
            if j == -1:
                out.append(text[i:])
                break
            block = text[i : j + 2]
            out.append(block)
            i = j + 2
            continue

        # string literals
        if ch in ("'", '"', "`"):
            quote = ch
            j = i + 1
            while j < n:
                c = text[j]
                if c == "\\":
                    j += 2
                    continue
                if c == quote:
                    j += 1
                    break
                j += 1
            out.append(text[i:j])
            i = j
            continue

        # /* yield */ protected function* (legacy marker)
        if text.startswith("/* yield */", i):
            out.append("/* yield */")
            i += len("/* yield */")
            # copy following whitespace
            while i < n and text[i] in " \t":
                out.append(text[i])
                i += 1
            continue

        # function * name(...) -> async function name(...)
        m = re.match(r"function\s*\*", text[i:])
        if m:
            out.append("async function")
            i += m.end()
            continue

        # yield * expr -> await expr
        m = re.match(r"yield\s*\*", text[i:])
        if m:
            out.append("await")
            i += m.end()
            continue

        out.append(ch)
        i += 1

    return "".join(out)


def _convert_html_attributes(text: str) -> str:
    """Convert yield* to await in exodus HTML event attribute values."""

    def repl_attr(m: re.Match) -> str:
        attr, quote, body = m.group(1), m.group(2), m.group(3)
        new_body = re.sub(r"yield\s*\*", "await", body)
        return f"{attr}={quote}{new_body}{quote}"

    # exodusonclick, exodusonchange, etc.
    return re.sub(
        r'\b(exodus\w+)=(["\'])(.*?)\2',
        repl_attr,
        text,
        flags=re.DOTALL,
    )


def _convert_dynamic_code_strings(js: str) -> str:
    """Convert yield* to await in exodus dynamic code strings (dict attrs, timers)."""
    js = re.sub(r"(=\s*)'return\s+yield\s*\*", r"\1'return await ", js)
    js = re.sub(r'(=\s*)"return\s+yield\s*\*', r'\1"return await ', js)
    js = re.sub(r"(=\s*)'yield\s*\*", r"\1'await ", js)
    js = re.sub(r'(=\s*)"yield\s*\*', r'\1"await ', js)
    js = re.sub(
        r"(exodussettimeout\s*\(\s*)'yield\s*\*",
        r"\1'await ",
        js,
        flags=re.IGNORECASE,
    )
    js = re.sub(
        r"(exodussetinterval\s*\(\s*)'yield\s*\*",
        r"\1'await ",
        js,
        flags=re.IGNORECASE,
    )
    return js


def _convert_script_blocks(text: str) -> str:
    """Run convert_js on each <script>...</script> block."""

    def repl_script(m: re.Match) -> str:
        body = convert_js(m.group(2))
        body = _convert_dynamic_code_strings(body)
        return m.group(1) + body + m.group(3)

    return re.sub(
        r"(<script\b[^>]*>)(.*?)(</script>)",
        repl_script,
        text,
        flags=re.DOTALL | re.IGNORECASE,
    )


def _add_async_to_functions_with_await(text: str) -> str:
    """Prefix async on plain functions whose bodies contain await."""
    funcs = []
    for m in re.finditer(r"^(async )?function (\w+)", text, re.M):
        funcs.append((m.start(), m.group(2), bool(m.group(1))))
    funcs.append((len(text), "END", True))

    inserts = []
    for i in range(len(funcs) - 1):
        start, name, is_async = funcs[i]
        end = funcs[i + 1][0]
        chunk = text[start:end]
        if not is_async and re.search(r"\bawait\b", chunk):
            # insert 'async ' before 'function' at this declaration
            inserts.append(start)

    if not inserts:
        return text

    out = []
    prev = 0
    for pos in inserts:
        out.append(text[prev:pos])
        out.append("async ")
        prev = pos
    out.append(text[prev:])
    return "".join(out)


def convert_html(text: str) -> str:
    text = _convert_script_blocks(text)
    text = _add_async_to_functions_with_await(text)
    text = _convert_html_attributes(text)
    return text


def convert_file(path: Path) -> bool:
    original = path.read_text(encoding="utf-8", errors="ignore")
    if path.suffix.lower() in (".htm", ".html"):
        converted = convert_html(original)
    else:
        converted = convert_js(original)
        converted = _add_async_to_functions_with_await(converted)
    if converted != original:
        path.write_text(converted, encoding="utf-8")
        return True
    return False


def main(paths):
    for path in paths:
        p = Path(path)
        if convert_file(p):
            print(f"converted: {p}")
        else:
            print(f"unchanged: {p}")


if __name__ == "__main__":
    main(sys.argv[1:])