#!/usr/bin/env python3
"""Report simple, reproducible C++ source metrics (not a full parser)."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIRS = (ROOT / "src",)
EXTENSIONS = {".cpp", ".h", ".hpp"}
CONTROL = re.compile(r"\b(if|for|while|case|catch)\b|&&|\|\|")
FUNCTION = re.compile(
    r"(?:^|\n)[\w:\<\>~*&\s]+\([^;{}]*\)\s*(?:const\s*)?(?:noexcept\s*)?\{",
    re.MULTILINE,
)

files = sorted(p for base in SOURCE_DIRS for p in base.rglob("*")
               if p.is_file() and p.suffix in EXTENSIONS)
loc = 0
functions = []
for path in files:
    source = path.read_text(encoding="utf-8", errors="replace")
    lines = source.splitlines()
    loc += sum(1 for line in lines if line.strip() and not line.lstrip().startswith(("//", "/*", "*")))
    for match in FUNCTION.finditer(source):
        start = match.end() - 1
        depth = 0
        end = start
        for end in range(start, len(source)):
            if source[end] == "{":
                depth += 1
            elif source[end] == "}":
                depth -= 1
                if depth == 0:
                    break
        body = source[start:end + 1]
        complexity = 1 + len(CONTROL.findall(body))
        line = source.count("\n", 0, match.start()) + 1
        functions.append((complexity, str(path.relative_to(ROOT)), line))

print(f"C++ source files: {len(files)}")
print(f"Non-blank, non-comment-leading source lines (approx.): {loc}")
print("Approximate cyclomatic complexity (top 10 detected function bodies):")
for complexity, path, line in sorted(functions, reverse=True)[:10]:
    print(f"  {complexity:>3}  {path}:{line}")
print("Note: lexical estimate only; templates, lambdas, macros and nested braces can affect results.")
