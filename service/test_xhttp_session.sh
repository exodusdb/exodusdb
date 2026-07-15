#!/bin/bash
# Test LOGIN session persistence via xhttp.php (run on the server having the problem).
# Usage: test_xhttp_session.sh HOST DATASET USER PASSWORD [PORT]
# Example: test_xhttp_session.sh starlit.hosts.neosys.com starlit EXODUS secret 44323

set -euo pipefail

HOST=${1:?host}
DATASET=${2:?dataset}
USER=${3:?user}
PASS=${4:?password}
PORT=${5:-443}
TOKEN="${DATASET}*${USER}*"
JAR=$(mktemp /tmp/xhttp_session_test.XXXXXX)
trap 'rm -f "$JAR"' EXIT

BASE="https://${HOST}:${PORT}/3/exodus/scripts/xhttp.php"
LOGIN_REQ="LOGIN%0D${USER}%0D${PASS}%0D${DATASET}%0D%0Ddefault"
TASKS_REQ="EXECUTE%0DGENERAL%0DGETTASKS%0DNOT"

post() {
	local req=$1
	local curl_args=(-sk)
	# On the server itself, force loopback when HOST is the local FQDN
	if [[ ${LOCAL:-} == 1 ]]; then
		curl_args+=(--resolve "${HOST}:${PORT}:127.0.0.1")
	fi
	curl "${curl_args[@]}" "$BASE" \
		-X POST -H 'Content-Type: text/xml' \
		-b "$JAR" -c "$JAR" \
		-d "<root><token>${TOKEN//\*/%2A}</token><timeout></timeout><request>${req}</request><data></data></root>"
}

echo "=== LOGIN ==="
LOGIN_XML=$(post "$LOGIN_REQ")
echo "$LOGIN_XML" | python3 -c "import sys,urllib.parse; print(urllib.parse.unquote(sys.stdin.read()))" | head -5
echo "Cookie jar:"
cat "$JAR" | grep -v '^#' || true

echo
echo "=== GETTASKS (same PHPSESSID) ==="
TASKS_XML=$(post "$TASKS_REQ")
echo "$TASKS_XML" | python3 -c "import sys,urllib.parse; s=urllib.parse.unquote(sys.stdin.read()); print(s[:500])"

if echo "$TASKS_XML" | grep -q 'Please login'; then
	echo
	echo "FAIL: session not established after LOGIN"
	exit 1
fi

if echo "$TASKS_XML" | grep -q '<result>1</result>'; then
	echo
	echo "OK: GETTASKS succeeded — PHP session persisted after LOGIN"
	exit 0
fi

echo
echo "FAIL: unexpected GETTASKS response"
exit 1