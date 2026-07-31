---
name: exodus-remotecall
description: >
  Login to Exodus and post remotecalls via xhttp.php using ~/test.exodus.sh
  (curl + PHP session cookies; password only from a passfile, never in a shell
  variable or logs). Use when testing live VAL/SELECT/typeahead/EXECUTE against
  c2comms or any host dataset, diagnosing backend responses, or /exodus-remotecall.
---

# Exodus remotecall (live login + post)

## Rule (non-negotiable)

- **Password never in shell vars, never in logs, never printed.**
- Source: passfile only — default `~/.ssh/<dataset>`, fallback `~/.ssh/exodus` (`chmod 600`).
- Use **awk** to substitute a `$pass` placeholder into a temp body file (as `~/test.exodus.sh` does). Do not `PASS=$(cat …)` or `echo` the body after substitution in tool output if it still holds the secret.
- Prefer running **`~/test.exodus.sh`** over reinventing curl.

## Smoke test (script)

```bash
# args: <fqdn> [https_port=443] [dataset=test] [username=exodus] [passfile]
~/test.exodus.sh c2comms.hosts.neosys.com 44325 c2comms
```

Script path: logs in, then posts a sample `SELECT CURRENCIES … XML 3`. Check for `<response>OK</response>`.

On this host, Apache may also serve **`d-c2comms.hosts.neosys.com`** on 443; use the FQDN/port that reaches the live site you are testing.

## Custom REQUEST after login

1. Run the script’s login half (or full script once) so you have a **cookie jar**.
2. Or login once yourself following the script’s template (passfile → rawdatafile via awk).
3. Post with the **same cookies** (no password on later calls):

```bash
# cookie_file from a prior successful login (script uses /tmp/<dataset>.<port>.$$.cookies.txt)
# request: FM-separated fields as | then replace | → %0D; spaces in SQL as %20
api_url="https://<fqdn>:<port>/3/exodus/scripts/xhttp.php"
dataset=c2comms
username=exodus   # must match login

# Example: typeahead-shaped VAL.SCHEDULE with allownew 00
# CACHE\rEXECUTE\rAGENCY\rVAL.SCHEDULE\rSCHEDULES\rHRSFR2016UAERAD1\rREF_NO\r\r\r00
req='CACHE|EXECUTE|AGENCY|VAL.SCHEDULE|SCHEDULES|HRSFR2016UAERAD1|REF_NO|||00'
req=${req//|/%0D}
body="<root><token>${dataset}*${username}*</token><timeout></timeout><request>${req}</request><data></data></root>"

curl -sk "$api_url" --cookie "$cookie_file" \
  -H 'Content-Type: text/plain;charset=UTF-8' \
  --data-raw "$body"
```

Inspect: `<response>OK</response>` vs error text; `<data>` size/shape (XML multi-hit vs empty vs full record).

## Pair with journalctl

After a post, confirm the service saw it:

```bash
journalctl --since "2 min ago" --no-pager | rg 'VAL\.SCHEDULE|HRSFR|SELECT.*SCHEDULES'
```

Multi-hit VAL path should show a follow-on `SELECT SCHEDULES  WITHOUT YEAR_PERIOD…` (select2 materialize). Exact-key short-circuit alone is fast OK **without** that SELECT.

## Do not

- Empty-password file drops under `data/<dataset>/~*.1`
- Guess credentials or log passfiles
- Rely only on journalctl when you need the **response body**

## Reference

- Full implementation: **`~/test.exodus.sh`**
- Always-on note: **`~/.grok/AGENTS.md`** → “Live Exodus remotecall”
