#!/usr/bin/env python3
"""Mine Exodus request XML logs for non-OK <Response> bodies.

Walks a parent directory for *.xml and *.xml.gz, extracts Response text,
keeps those that look like errors (not starting with OK), then prints
sorted unique messages (one per line after whitespace collapse).

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

# Multi-line stacks live inside a single <Response>...</Response>
RESPONSE_OPEN = re.compile(r"<Response\b[^>]*>", re.IGNORECASE)
RESPONSE_CLOSE = re.compile(r"</Response>", re.IGNORECASE)


def open_text(path: Path):
	if path.suffix == ".gz" or path.name.endswith(".xml.gz"):
		return gzip.open(path, "rt", encoding="utf-8", errors="replace")
	return path.open("rt", encoding="utf-8", errors="replace")


def is_error_response(body: str) -> bool:
	"""Listen success responses start with OK (alone or 'OK …')."""
	s = body.strip()
	if not s:
		return False
	if s == "OK" or s.startswith("OK "):
		return False
	# e.g. "OK\n…" unlikely; also "OKSESSIONID" not used
	return True


def normalize(body: str) -> str:
	"""Collapse whitespace so multi-line stacks dedupe cleanly."""
	return re.sub(r"\s+", " ", body.strip())


def iter_log_files(parent: Path):
	for p in sorted(parent.rglob("*")):
		if not p.is_file():
			continue
		n = p.name
		if n.endswith(".xml") or n.endswith(".xml.gz"):
			yield p


def iter_response_bodies(path: Path):
	"""Stream Response bodies from one file (handles multi-line content)."""
	buf = ""
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
					# rare: another open on same line — ignore for simplicity
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
		# truncated file; still emit what we have
		yield "".join(parts)


def main(argv: list[str] | None = None) -> int:
	ap = argparse.ArgumentParser(
		description="Extract unique error-like Response texts from Exodus XML request logs."
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
		help="Do not collapse whitespace (dedupe on exact text)",
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

	for path in iter_log_files(parent):
		nfiles += 1
		if not args.quiet:
			print(f"# {path}", file=sys.stderr)
		try:
			for body in iter_response_bodies(path):
				nresp += 1
				if not is_error_response(body):
					continue
				key = body.strip() if args.raw else normalize(body)
				if key:
					counts[key] += 1
		except OSError as e:
			print(f"# skip {path}: {e}", file=sys.stderr)

	# Stream sorted unique (by text); optional count
	for text in sorted(counts):
		if args.count:
			print(f"{counts[text]}\t{text}")
		else:
			print(text)

	if not args.quiet:
		print(
			f"# files={nfiles} responses={nresp} unique_errors={len(counts)}",
			file=sys.stderr,
		)
	return 0


if __name__ == "__main__":
	sys.exit(main())
