---
name: exodus-remotecall
description: >
  Login to Exodus and post remotecalls via xhttp.php (curl + PHP session cookies;
  password only from a passfile, never in a shell variable or logs). Script lives
  in this skill under scripts/test.exodus.sh. Use when testing live VAL/SELECT/
  typeahead/EXECUTE against c2comms or any host dataset, diagnosing backend
  responses, or /exodus-remotecall.
---

# Exodus remotecall (live login + post)

Canonical script (versioned with this skill — **not** a home-dir one-off):

```text
scripts/test.exodus.sh
```

Resolve path when running (prefer project skill if present):

```bash
# from neosys or exodus repo root
SCRIPT=.grok/skills/exodus-remotecall/scripts/test.exodus.sh
# or absolute
SCRIPT=~/neosys/.grok/skills/exodus-remotecall/scripts/test.exodus.sh
# user install
SCRIPT=~/.grok/skills/exodus-remotecall/scripts/test.exodus.sh
```

## Rule (non-negotiable)

- **Password never in shell vars, never in logs, never printed.**
- Source: passfile only — default `~/.ssh/<dataset>`, fallback `~/.ssh/exodus` (`chmod 600`).
- The script uses **awk** to substitute a `$pass` placeholder into a temp body file. Do not `PASS=$(cat …)` or print the body after substitution if it still holds the secret.
- Prefer running **this skill’s `scripts/test.exodus.sh`** over reinventing curl or empty-password `.1` drops.

## Smoke test

```bash
SCRIPT=~/neosys/.grok/skills/exodus-remotecall/scripts/test.exodus.sh
# args: <fqdn> [https_port=443] [dataset=test] [username=exodus] [passfile]
bash "$SCRIPT" c2comms.hosts.neosys.com 44325 c2comms
```

Logs in, then posts sample `SELECT CURRENCIES … XML 3`. Expect `<response>OK</response>`.

On this host, Apache may also serve **`d-c2comms.hosts.neosys.com`** on 443; use the FQDN/port for the site under test.

## Custom REQUEST after login

1. Run the smoke script once (or its login half) → cookie jar `/tmp/<dataset>.<port>.$$.cookies.txt`.
2. Post further REQUESTs with the **same cookies** (no password):

```bash
api_url="https://<fqdn>:<port>/3/exodus/scripts/xhttp.php"
dataset=c2comms
username=exodus   # must match login
cookie_file=...   # from login

# FM fields as | then | → %0D; spaces in SQL as %20
# Example: VAL.SCHEDULE typeahead allownew 00
req='CACHE|EXECUTE|AGENCY|VAL.SCHEDULE|SCHEDULES|HRSFR2016UAERAD1|REF_NO|||00'
req=${req//|/%0D}
body="<root><token>${dataset}*${username}*</token><timeout></timeout><request>${req}</request><data></data></root>"

curl -sk "$api_url" --cookie "$cookie_file" \
  -H 'Content-Type: text/plain;charset=UTF-8' \
  --data-raw "$body"
```

Inspect: `<response>OK</response>` vs error; `<data>` shape (XML multi-hit vs empty vs full record).

## Pair with journalctl

```bash
journalctl --since "2 min ago" --no-pager | rg 'VAL\.SCHEDULE|HRSFR|SELECT.*SCHEDULES'
```

Multi-hit VAL should log a follow-on `SELECT SCHEDULES  WITHOUT YEAR_PERIOD…`. Exact-key short-circuit alone is fast OK without that SELECT.

## Do not

- Depend on a disposable `~/test.exodus.sh` — use **this skill’s** `scripts/test.exodus.sh`
- Empty-password file drops under `data/<dataset>/~*.1`
- Guess credentials or log passfiles
- Rely only on journalctl when you need the **response body**

## Reference

- Script: **`scripts/test.exodus.sh`** (in this skill)
- Always-on: **`~/.grok/AGENTS.md`** → “Live Exodus remotecall”
