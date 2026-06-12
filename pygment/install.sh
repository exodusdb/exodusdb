#!/bin/bash
set -euxo pipefail

###############################################################################
# ROBUST METHOD FOR UBUNTU LTS (including 20.04 and future releases)
#
# Recommended one-time preparation on any Ubuntu LTS (20.04, 22.04, 24.04+):
#     apt-get update
#     apt-get install python3-pip python3-wheel --no-install-recommends
#
# This is the most robust approach across Ubuntu LTS releases because:
#   - It uses the pip that Canonical ships and supports for that exact release.
#   - With --no-install-recommends the installed footprint is tiny
#     (only python3-pip + python3-wheel). No build-essential or python3-dev.
#   - The script below automatically avoids --break-system-packages on old pip
#     versions (20.04 ships pip ~20.x which does not support the flag).
#
# Do NOT rely primarily on `python3 -m ensurepip` for Ubuntu containers.
# The distro python3-pip + wheel is more stable long-term.
###############################################################################

# Prefer the paths commonly used on these containers.
if [ -x /usr/bin/python3 ]; then
    PYTHON=/usr/bin/python3
else
    PYTHON=python3
fi

if [ -x /usr/bin/pip3 ]; then
    PIP=/usr/bin/pip3
else
    PIP="$PYTHON -m pip"
fi

# If pip is not available, give clear instructions using the minimal
# recommended apt command (with --no-install-recommends).
if ! "$PIP" --version >/dev/null 2>&1; then
    echo "pip is not available."
    echo ""
    echo "On Ubuntu LTS, install the minimal supported set with:"
    echo "    apt-get update"
    echo "    apt-get install python3-pip python3-wheel --no-install-recommends"
    echo ""
    echo "Then re-run this script."
    exit 1
fi

echo "Using PYTHON=$PYTHON"
echo "Using PIP=$PIP"

# Determine whether --break-system-packages is needed.
# This flag was added in pip 23.0. On 20.04 the packaged pip is too old
# and does not recognise it (and does not need it).
pip_major=$("$PIP" --version | awk '{print $2}' | cut -d. -f1)
if [ "$pip_major" -ge 23 ] 2>/dev/null; then
    BREAK_FLAG="--break-system-packages"
else
    BREAK_FLAG=""
fi

echo "=== [1/4] Discovering and removing any previous exodus lexer installation (idempotent) ==="

"$PYTHON" - <<'PYEOF'
import site, os, shutil, glob

print("Searching for previous installations...")

roots = list(site.getsitepackages()) + [site.getusersitepackages()]

for base in ("/usr/local/lib", "/usr/lib"):
    for py in glob.glob(os.path.join(base, "python*")) + glob.glob(os.path.join(base, "python3*")):
        roots.append(os.path.join(py, "dist-packages"))
        roots.append(os.path.join(py, "site-packages"))

candidates = []
for r in set(roots):
    if not r or not os.path.isdir(r): continue
    for pat in ("*exodus*lexer*", "*exodus_cpp*", "exodus_lexers*", "exodus-lexers*"):
        for p in glob.glob(os.path.join(r, pat)):
            if os.path.exists(p):
                candidates.append(p)

for base in ("/usr/local/lib", "/usr/lib"):
    for p in glob.glob(os.path.join(base, "python*", "dist-packages", "exodus_lexers-*.egg")):
        if os.path.exists(p):
            candidates.append(p)

seen = set()
for p in candidates:
    if p in seen: continue
    seen.add(p)
    print(f"  Removing: {p}")
    try:
        if os.path.isdir(p):
            shutil.rmtree(p, ignore_errors=True)
        else:
            os.remove(p)
    except Exception as e:
        print(f"    Warning: {e}")

for base in ("/usr/local/lib", "/usr/lib"):
    for py in glob.glob(os.path.join(base, "python*")) + glob.glob(os.path.join(base, "python3*")):
        for pth in glob.glob(os.path.join(py, "dist-packages/easy-install.pth")) + \
                   glob.glob(os.path.join(py, "site-packages/easy-install.pth")):
            if os.path.isfile(pth):
                try:
                    with open(pth) as f:
                        lines = f.readlines()
                    new = [ln for ln in lines if "exodus" not in ln.lower()]
                    if len(new) != len(lines):
                        with open(pth, "w") as f:
                            f.writelines(new)
                        print(f"  Cleaned: {pth}")
                except Exception as e:
                    print(f"    pth warning: {e}")

print("Cleanup phase finished.")
PYEOF

echo "=== [2/4] pip uninstall (belt and suspenders) ==="
$PIP uninstall -y exodus_lexers exodus-lexers 2>/dev/null || true

echo "=== [3/4] Modern installation ==="
$PIP install . --force-reinstall --no-deps $BREAK_FLAG

echo "=== [4/4] Self-test ==="
"$PYTHON" - <<'PYEOF'
import sys
from pygments.lexers import get_lexer_by_name

try:
    lexer = get_lexer_by_name('exoduscpp')
    print("Lexer name :", lexer.name)
    print("Aliases    :", lexer.aliases)
    print("Module     :", lexer.__module__)
    print("Class      :", type(lexer).__name__)

    if "exodus" in lexer.__module__.lower() or "ExodusCpp" in str(type(lexer)):
        print("\n✅ SUCCESS: Custom ExodusCppLexer is active.")
    else:
        print("\n❌ Not the custom lexer")
        sys.exit(1)
except Exception as e:
    print("❌ Verification failed:", e)
    sys.exit(1)

import os, subprocess
if os.path.isfile("./test.sh"):
    print("\n--- Running ./test.sh ---")
    res = subprocess.run(["./test.sh"], capture_output=True, text=True)
    print(res.stdout)
    if res.returncode != 0:
        print(res.stderr)
PYEOF

echo "=== Install complete (idempotent, minimal footprint) ==="
