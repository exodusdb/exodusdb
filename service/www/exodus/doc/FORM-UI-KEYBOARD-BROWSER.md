# FORM-UI-KEYBOARD-BROWSER

**Location:** `service/www/exodus/doc/` (web programming docs)  
**Audience:** maintainers and AI assistants working on form UI key policy  
**See also:** [FORM-UI-PHILOSOPHY.md](./FORM-UI-PHILOSOPHY.md), [FORM-UI-TYPES.md](./FORM-UI-TYPES.md), `../3/exodus/helpkeyboard.htm` (in-app list), [PROGRAMMERS_OVERVIEW.md](./PROGRAMMERS_OVERVIEW.md)

Technical UI note: what the **browser** usually does vs what **Exodus** does on a form page (conflicts, intentional steals, leave-alone).  
Not a duplicate of Gate A architecture — that stays in PROGRAMMERS_OVERVIEW / AGENTS.

**Scope:** typical desktop Chrome/Firefox. On Mac, **Cmd** ≈ **Ctrl** unless noted.  
**“Left alone”** = Exodus form chrome does not `preventDefault` it (the browser or OS may still handle it).

---

## How to regenerate / refresh this list

Compare common browser shortcuts with live key paths in:

- `service/www/3/exodus/scripts/dbform.js` (form keydown, Alt pan, digit accesskeys)
- `service/www/3/exodus/scripts/client.js` (browser zoom leave-alone, modal wheel, popstate)
- `service/www/3/exodus/helpkeyboard.htm` (in-app product shortcut list)

### Agent prompt (reproduce)

```text
List all typical browser keys with a column for browser and exodus.
Cross-check form/client key handling (dbform.js, client.js) and helpkeyboard.htm.
Include history, tabs, zoom, F-keys, Alt+letter, wheel. Note wide-form Alt+Left/Right.
Update FORM-UI-KEYBOARD-BROWSER.md (technical UI doc family).
```

### Source inventory (shell)

From `~/exodus`:

```bash
# Form + client: Alt/Ctrl, F-keys, cancel paths
rg -n "altKey|ctrlKey|metaKey|keycode ==|F5|F6|F7|F8|F9|F12|preventDefault|exocancelevent|history|zoom|accesskey|browser_chrome|scroll_viewport" \
  service/www/3/exodus/scripts/dbform.js \
  service/www/3/exodus/scripts/client.js

# In-app product shortcut list
rg -n "Alt\+|Ctrl\+|F[0-9]|Esc|Enter|Tab" service/www/3/exodus/helpkeyboard.htm

# Explicit “leave to browser” comments
rg -n "LEFT TO THE BROWSER|Browser chrome|history back|form_any_wide_layout|scroll_viewport" \
  service/www/3/exodus/scripts/client.js \
  service/www/3/exodus/scripts/dbform.js
```

After material key-policy changes, re-run the inventory (or the agent prompt) and update this file; also update `helpkeyboard.htm` when the **in-app** shortcut list changes.

*Last inventory: 2026-08-07.*

---

## Navigation & history

| Keys | Browser (typical) | Exodus |
|------|-------------------|--------|
| **Alt+←** | History **back** | **Not wide:** history back. **Wide form** (`.exodusform-wide`): pan left ~1 screen — see [FORM-UI-WIDE-NARROW.md](./FORM-UI-WIDE-NARROW.md) |
| **Alt+→** | History **forward** | **Not wide:** history forward. **Wide:** pan right |
| **Alt+↑ / Alt+↓** | (varies; rarely page nav) | **Pan** up/down ~1 screen (Alt+↓ on a drop-down still opens options) |
| **Backspace** (not in text) | Old IE: back; modern: usually nothing | **Owned:** field leave / typeahead / decide, etc. — not browser back |
| **Browser Back / gesture** | Leave page | **Normal page:** back. **Confirm/decide open:** closes popup (`popstate`), does **not** leave page |
| **Alt+Home** | Browser home (some) | **First field** on the form |
| **Alt+End** | — | **Last field** on the form |
| **Home / End** | Caret / page | Field caret / table / form nav (context) |
| **Ctrl+Home / Ctrl+End** | Top/bottom of page | In tables: first/last row–column |

## Tabs, windows, address bar

| Keys | Browser | Exodus |
|------|---------|--------|
| **Ctrl+T / Ctrl+N / Ctrl+W** | New tab / window / close | Generally **left alone** (old Ctrl+N block is commented out) |
| **Ctrl+Tab / Ctrl+Shift+Tab** | Switch tabs | Left alone (browser/OS) |
| **Ctrl+L / Alt+D / F6\*** | Focus address bar | **\*F6 = Link** on forms (related screen) — **not** address bar |
| **Ctrl+Shift+T** | Reopen closed tab | Left alone |
| **Alt+Tab** | OS window switch | OS — not app |

## Zoom & find

| Keys | Browser | Exodus |
|------|---------|--------|
| **Ctrl++ / Ctrl+- / Ctrl+0** | Zoom in/out/reset | **Left alone** (capture only stops app path) |
| **Ctrl+wheel** | Zoom | **Left alone** (except colour popup, which owns Ctrl+wheel) |
| **Ctrl+F** | Find in page | Generally left alone |
| **Ctrl+P** | Print dialog | **Alt+P** = Print/Send form; Ctrl+P often still browser print |

## Refresh & devtools

| Keys | Browser | Exodus |
|------|---------|--------|
| **F5 / Ctrl+R** | Reload | **F5** uses form reload path (Mac key-bubble fix) |
| **Ctrl+Shift+R** | Hard reload | Usually left alone |
| **F12** | DevTools | **EXODUS user only:** debug step/log; otherwise browser |
| **Ctrl+Shift+I / J** | DevTools / console | Left alone |

## Edit / clipboard

| Keys | Browser | Exodus |
|------|---------|--------|
| **Ctrl+C / X / V / A / Z** | Copy/cut/paste/select all/undo | Mostly **native** in fields; some popups allow copy explicitly |
| **Ctrl+Enter** | Send (some sites) | **Save / OK / commit** |
| **Enter** | Submit / newline | **Field leave / next** (textarea: newline unless Ctrl/Shift) |
| **Tab / Shift+Tab** | Focus cycle | **Next/prev field** |
| **Esc** | Stop / close UI | **Close / cancel / dismiss** |
| **Space** | Activate focused control | Field/space; **decide multi:** toggle only (no next row) |
| **Del** | Delete | Field delete; **decide multi:** restore entry checks (with F8) |

## Function keys (high conflict)

| Keys | Browser | Exodus |
|------|---------|--------|
| **F1** | Help | (browser/OS often) |
| **F2** | Rename (OS) | **Edit field** (in-place) |
| **F3** | Find next | — |
| **F5** | Reload | Reload (see above) |
| **F6** | Cycle chrome / address | **Link** (related record) |
| **F7** | Caret browsing (Firefox) | **Popup** (options) |
| **F8** | (devtools/script) | **Close / cancel**; decide multi: **restore entry** checks |
| **F9** | — | **Save / OK** |
| **F10** | Menu bar focus | Usually browser/OS |
| **F11** | Fullscreen | Browser |

## Alt+letter (forms)

| Keys | Browser | Exodus |
|------|---------|--------|
| **Alt+M** | accesskey / menu | **Menu** |
| **Alt+N / O / S / C / E / L / P** | accesskey / new/open… | **New / Open / Save / Close / Edit / List / Print** |
| **Alt+G** | — | **Logout** |
| **Alt+R** | Reload (some) | **Refresh cache** |
| **Alt+Y** | — | **Copy down** (table) |
| **Alt+0…9** | Tab strip / toolbar (varies) | Forms **with digit accesskeys:** digits **claimed** (even unused cancelled so browser does not steal) |

## Wheel

| Gesture | Browser | Exodus |
|---------|---------|--------|
| **Plain wheel** | Page scroll | Normal scroll; **modal open:** page scroll blocked; **decide:** step options |
| **Ctrl+wheel** | Zoom | Zoom (except colour popup) |

## Summary — intentional “steals”

| Exodus owns | Browser loses (in that context) |
|-------------|-------------------------------|
| F6 / F7 / F8 / F9 | Address cycle, FF caret browse (F7), etc. |
| Alt+letter form chrome | Matching accesskeys / some Alt shortcuts |
| Alt+0–9 (digit-accesskey forms) | Tab/address shortcuts on those digits |
| Alt+↑↓ always; Alt+←→ when **wide** | History for ←→ only when **not** wide |
| Backspace in form/decide | Old “navigate back” behaviour |
| Back while popup open | Leave page |

---

