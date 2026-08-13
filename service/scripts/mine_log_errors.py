#!/usr/bin/env python3
"""Mine Exodus request XML logs for unusual <Response> bodies.

Walks a parent directory for *.xml and *.xml.gz, extracts Response text,
drops *normal* responses, prints the rest sorted and deduplicated.

Normal (dropped):
  - OK / OK …
  - NOT OK / NOT OK …
  - everyday not-found / does-not-exist style messages

Everything else is kept (System Error, Var*, stacks, odd Error:, …).

Usage:
  mine_log_errors.py /root/hosts/c2comms/logs
  mine_log_errors.py /root/hosts --count
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

# Everyday validation phrases (case-insensitive substring on short prose).
NORMAL_PHRASES = (
	"cannot be found",
	"could not be found",
	"does not exist",
	"do not exist",
	"is missing",
	"not found",
	"already exists",
	"is required",
	"not authorised",
	"not authorized",
	"access refused",
	"no record",
	"lock not authorised",
	"lock not authorized",
	"your lock expired",
)

VOLATILE = [
	(re.compile(r"\bcursor\d+(?:_\d+)+\b"), "cursor*"),
	(re.compile(r"\bDECLARE\s+\w+\b"), "DECLARE cursor*"),
]


def open_text(path: Path):
	if path.suffix == ".gz" or path.name.endswith(".xml.gz"):
		return gzip.open(path, "rt", encoding="utf-8", errors="replace")
	return path.open("rt", encoding="utf-8", errors="replace")


def looks_like_stack_or_engine(s: str) -> bool:
	if re.search(r"\.(?:cpp|h|hpp|cc):\d+", s):
		return True
	if re.search(r"\bVar[A-Z][A-Za-z]+\b", s):
		return True
	if re.search(r"\bSystem\s+Error\b", s, re.I):
		return True
	return False


def is_normal(body: str) -> bool:
	"""True if routine success / soft fail / not-found prose — drop it."""
	s = body.strip()
	if not s:
		return True

	if s == "OK" or s.startswith("OK "):
		return True
	if s == "NOT OK" or s.startswith("NOT OK "):
		return True

	# Engine failures always keep, even if text also says "not found"
	if looks_like_stack_or_engine(s):
		return False

	# Strip optional "Error:" wrapper for phrase checks
	msg = s
	if msg.startswith("Error:"):
		msg = msg[6:].strip()
		if not msg:
			return True

	# Short prose validation only (long / multi-paragraph → keep)
	if len(msg) > 300:
		return False

	low = msg.lower()
	return any(p in low for p in NORMAL_PHRASES)


def normalize(body: str) -> str:
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
			"Unique non-normal Response texts from Exodus XML request logs "
			"(drops OK / NOT OK / not-found style)."
		)
	)
	ap.add_argument(
		"parent",
		type=Path,
		help="Parent directory to walk for *.xml / *.xml.gz",
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
				if is_normal(body):
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
		print(
			f"# files={nfiles} responses={nresp} kept={nkept} unique={len(counts)}",
			file=sys.stderr,
		)
	return 0


if __name__ == "__main__":
	sys.exit(main())
