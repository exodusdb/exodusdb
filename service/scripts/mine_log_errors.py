#!/usr/bin/env python3
"""Mine Exodus request XML logs for *unexpected technical* Response failures.

Walks a parent directory for *.xml and *.xml.gz, extracts <Response> text,
keeps only responses that look like framework/engine failures (Var*, stacks,
System Error, missing lib, …) — not everyday business non-OK messages
("brand cannot be found", validation, etc.).

Prints sorted unique messages (whitespace collapsed). Optional --count.

Usage:
  mine_log_errors.py /root/hosts/c2comms/logs
  mine_log_errors.py /root/hosts --count
  mine_log_errors.py /root/hosts --all          # every non-OK (noisy)
"""

from __future__ import annotations

import argparse
import gzip
import re
import sys
from collections import Counter
from pathlib import Path

RESPONSE_OPEN = re.compile(r"<Response\b[^>]*>", re.IGNORECASE)
RESPONSE_CLOSE = re.compile(r"</Response>", re.IGNORECASE)

# Framework / engine signals — not product validation prose.
TECHNICAL = re.compile(
	r"""(?ix)
	\bVar[A-Z][A-Za-z]+\b          # VarUnassigned, VarDBException, …
	| \bSystem\s+Error\b
	| \bINTERNAL\s+ERROR\b
	| \bERROR\s+NO:
	| function\s+cannot\s+be\s+found\s+in\s+lib
	| \bsegfault\b
	| \bAborted\b
	| \bassert(ion)?\b
	# stack frame: "5: foo.cpp:315:" or "foo.cpp:315:"
	| (?:^|\s)\d+:\s+\S+\.(?:cpp|h|hpp|cc):\d+
	| \b\w+\.(?:cpp|h|hpp|cc):\d+:
	"""
)

# Volatile noise inside otherwise identical technical dumps
VOLATILE = [
	(re.compile(r"\bcursor\d+(?:_\d+)+\b"), "cursor*"),
	(re.compile(r"\bDECLARE\s+\w+\b"), "DECLARE cursor*"),
	(re.compile(r"\bTHREADNO\s+\d+\b", re.I), "THREADNO *"),
	(re.compile(r"\bpid\s*=\s*\d+\b", re.I), "pid=*"),
]


def open_text(path: Path):
	if path.suffix == ".gz" or path.name.endswith(".xml.gz"):
		return gzip.open(path, "rt", encoding="utf-8", errors="replace")
	return path.open("rt", encoding="utf-8", errors="replace")


def is_success(body: str) -> bool:
	s = body.strip()
	return s == "OK" or s.startswith("OK ")


def is_technical(body: str) -> bool:
	"""True if body smells like engine/framework failure, not business msg_."""
	if is_success(body) or not body.strip():
		return False
	return TECHNICAL.search(body) is not None


def is_any_non_ok(body: str) -> bool:
	return not is_success(body) and bool(body.strip())


def normalize(body: str) -> str:
	"""Collapse whitespace + strip volatile ids so the same bug dedupes."""
	s = re.sub(r"\s+", " ", body.strip())
	for rx, repl in VOLATILE:
		s = rx.sub(repl, s)
	return s


def iter_log_files(parent: Path):
	for p in sorted(parent.rglob("*")):
		if not p.is_file():
			continue
		n = p.name
		if n.endswith(".xml") or n.endswith(".xml.gz"):
			yield p


def iter_response_bodies(path: Path):
	inside = False
	parts: list[str] = []
	with open_text(path) as f:
		for line in f:
			if not inside:
				m = RESPONSE_OPEN.search(line)
				if not m:
					continue
				rest = line[m.end() :]
				cm = RESPONSE_CLOSE.search(rest)
				if cm:
					yield rest[: cm.start()]
					continue
				inside = True
				parts = [rest]
			else:
				cm = RESPONSE_CLOSE.search(line)
				if cm:
					parts.append(line[: cm.start()])
					yield "".join(parts)
					inside = False
					parts = []
				else:
					parts.append(line)
	if inside and parts:
		yield "".join(parts)


def main(argv: list[str] | None = None) -> int:
	ap = argparse.ArgumentParser(
		description=(
			"Unique technical failures from Exodus XML request logs "
			"(Var*/System Error/stacks — not business non-OK)."
		)
	)
	ap.add_argument(
		"parent",
		type=Path,
		help="Parent directory to walk for *.xml / *.xml.gz",
	)
	ap.add_argument(
		"--all",
		action="store_true",
		help="Include every non-OK response (business noise too)",
	)
	ap.add_argument(
		"--count",
		action="store_true",
		help="Prefix each line with occurrence count",
	)
	ap.add_argument(
		"--raw",
		action="store_true",
		help="Do not collapse whitespace / volatile ids for dedupe",
	)
	ap.add_argument(
		"-q",
		"--quiet",
		action="store_true",
		help="No progress on stderr",
	)
	args = ap.parse_args(argv)

	parent = args.parent
	if not parent.is_dir():
		print(f"not a directory: {parent}", file=sys.stderr)
		return 2

	keep = is_any_non_ok if args.all else is_technical
	counts: Counter[str] = Counter()
	nfiles = 0
	nresp = 0
	nkept = 0

	for path in iter_log_files(parent):
		nfiles += 1
		if not args.quiet:
			print(f"# {path}", file=sys.stderr)
		try:
			for body in iter_response_bodies(path):
				nresp += 1
				if not keep(body):
					continue
				nkept += 1
				key = body.strip() if args.raw else normalize(body)
				if key:
					counts[key] += 1
		except OSError as e:
			print(f"# skip {path}: {e}", file=sys.stderr)

	for text in sorted(counts):
		if args.count:
			print(f"{counts[text]}\t{text}")
		else:
			print(text)

	if not args.quiet:
		mode = "all-non-OK" if args.all else "technical"
		print(
			f"# files={nfiles} responses={nresp} kept={nkept} "
			f"unique={len(counts)} mode={mode}",
			file=sys.stderr,
		)
	return 0


if __name__ == "__main__":
	sys.exit(main())
