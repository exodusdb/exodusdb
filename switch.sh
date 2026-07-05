#!/bin/bash
set -euo pipefail
:
: switch.sh \<branch\> # Wrap git switch in named stash push/pop.
:
: Check can switch
:
	target_branch="${1:?Syntax is ./switch.sh <branch_name>}"
	if ! git show-ref --verify --quiet "refs/heads/$target_branch"; then
		echo "Branch $target_branch does not exist"
		exit 1
	fi
	if [ "$target_branch" = "$(git rev-parse --abbrev-ref HEAD)" ]; then
		echo "Already on $target_branch"
		exit 2
	fi
:
: Push branch specific stash if anything stashable
:
	target_branch="${1:?Syntax is ./switch.sh <branch_name>}"
	branch_name_1="$(git rev-parse --abbrev-ref HEAD)"
	stash_1="switch_sh_$branch_name_1"
	if ! git diff --quiet --exit-code || ! git diff --cached --quiet --exit-code; then
		git stash push -m "$stash_1" || { echo "Stash failed"; return 1; }
	fi
:
: Switch if possible
:
	if ! git switch "$target_branch"; then
		echo "Switch failed"
		# Dont exit
	fi
:
: Pop branch specific stash if present.
: This effectively reverts the initial push in case switch failed.
:
	branch_name_2="$(git rev-parse --abbrev-ref HEAD)"
	stash_2="switch_sh_$branch_name_2"
	stash_ref="$(git stash list | awk -F: -v pat="$stash_2" '$0 ~ pat {print $1; exit}')"
	if [ "$stash_ref" ]; then
		# Note --index option to pop staged changes as well
		# git incomprehensibly doesnt pop staged changes nor even provide a way to set it as the default
		# except manually per user in ~/.gitconfig or per clone in .git/config
		git stash pop --index "$stash_ref"
	fi
:
: Exit