#!/bin/bash
set -euo pipefail
:
: pgexodus_dev.sh [--no-pull]
:
: Checkout pgexodus at the commit pinned by this exodus branch, then switch
: to local master with origin/master upstream so you can commit and push.
:
:	install.sh only does a standard submodule checkout, detached HEAD at the pin.
:	Run this script when you need to develop in pgexodus.
:
:	--no-pull  stay at the pinned commit, only attach master branch for pushing
:
EXODUS_DIR="$(cd "$(dirname "$0")" && pwd)"
PGEXODUS_DIR="$EXODUS_DIR/pgexodus"
PULL=1
[[ ${1:-} == --no-pull ]] && PULL=0
[[ $# -eq 0 || ${1:-} == --no-pull ]] || { echo "Usage: $0 [--no-pull]" >&2; exit 2; }
:
cd "$EXODUS_DIR"
git submodule sync --recursive
git submodule update --init --recursive
:
cd "$PGEXODUS_DIR"
git fetch origin
:
if [[ $PULL -eq 1 ]]; then
	if git show-ref --verify --quiet refs/heads/master; then
		git switch master
	else
		git switch -c master --track origin/master
	fi
	git pull --ff-only origin master
else
:	Attach master at the commit pinned by this exodus branch, do not advance to origin/master
	git switch -C master
fi
:
git branch --set-upstream-to=origin/master master 2>/dev/null || true
:
echo "pgexodus: branch $(git rev-parse --abbrev-ref HEAD) at $(git rev-parse --short HEAD)"
git status -sb
echo
echo "After committing in pgexodus, update the exodus submodule pointer:"
echo "  cd $EXODUS_DIR && git add pgexodus && git commit"