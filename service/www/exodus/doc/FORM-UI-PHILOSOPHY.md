# Exodus form UI — layout philosophy

**Location:** `service/www/exodus/doc/` (web programming docs)  
**Audience:** maintainers and AI assistants working on Exodus web forms  
**Discovery:** pointed to from `~/exodus/AGENTS.md`, `~/neosys/AGENTS.md`, and `.grok/rules/exodus-form-ui.md`  
**See also:** [PROGRAMMERS_OVERVIEW.md](./PROGRAMMERS_OVERVIEW.md), [forms.htm](./forms.htm), `../3/exodus/global.css`, [FORM-UI-TYPES.md](./FORM-UI-TYPES.md), **[FORM-UI-WIDE-NARROW.md](./FORM-UI-WIDE-NARROW.md)** (narrow free-text fold vs extreme-wide / 30ch; multivalue col0 `width="1%"` nest-fill), [FORM-UI-KEYBOARD-BROWSER.md](./FORM-UI-KEYBOARD-BROWSER.md)  
**Not here:** C++/var/exoprog docs live under repo-root `doc/`

This note captures *strategic* UI conventions — how forms are built at runtime, how borders and width behave, and what to avoid. It is not a duplicate of the programmer's manual.

---

## 1. HTM is a seed; the live DOM is different

Pages ship static HTM plus dicts (`*_dict.js`). **`dbform.js` rewrites the DOM on load.** Always reason about the **runtime** tree, not the source file alone.

Common transforms:

| Source HTM | After `dbform.js` |
|------------|-------------------|
| `<input id="FIELD">` with dict `align='T'` | Contenteditable `<span>` (flowing text) |
| Nested `<table>` with `exogroupno` fields | `TABLE[exogroupno]` / `#exogroupN` |
| Top-level `TABLE.exodusform` | Often wrapped in `div.exodusformpane` by `exoduswrapformpanes()` in `client.js` |

**Implication:** inline styles on `<input>` in HTM (e.g. `max-width`) are often **discarded** when the field becomes a `<span>`. Dict properties and page CSS must target the **final** element (`.exodusid_FIELD`, column `td` classes, etc.).

---

## 2. Layer responsibilities (do not fight them)

```
div.exodusformpane          ← outer rounded frame (owns the outside edge)
  TABLE.exodusform          ← grid; table element border stripped inside pane
    tr / td                 ← cell grid lines (td borders, not tr)
      TABLE[exogroupno]  ← embedded repeating group (nested)
```

### Pane (`exodusformpane`)

- Inserted by `exoduswrapformpanes()` (one shell per sibling *run* of top-level forms with only `<br>`/whitespace between; same rules as coalesce). Pre-authored panes left alone; `exoduscoalesceformpanes()` still merges those if needed.
- **No flash of bare/wrong layout:** until `html.exodus-panes-ready` (set after wrap), `TABLE.exodusform` / `.exodusformpane` use `visibility: hidden` (`global.css`). Forms stay measurable during onload; first paint is not an unmerged narrow shell.
- Pane: `width: max-content` only — **follow contents**; border encloses every form. Do not `max-width` / `overflow-x` the **pane**.
- Form tables inside the pane may use `max-width: calc(100vw − …)` so flexible text columns wrap when the form would exceed the screen; the pane still tracks that (capped) table width.
- Pane **strips** outer table border and removes **last row cell** bottom borders so the pane border is the only bottom edge.
- **Manual side-by-side** (layout table with two+ form columns): table cells default to middle vertical-align — set `vertical-align: top` on that layout row (or cells) in the HTM. Not a pane/wrap concern.
- **Do not** author page-local `exodusformpane` wrappers just to avoid flash — framework handles reveal.

### Grid borders live on **`td`**, not **`tr`**

```css
/* global.css — direct children of exodusform */
.exodusform > tbody > tr > td { border: 1px lightgrey solid; }

/* inside pane — last row td bottoms cleared */
.exodusformpane > TABLE.exodusform > tbody > tr:last-child > * { border-bottom: none; }
```

**Anti-pattern:** page CSS like `tr { border-bottom: … }`. It bypasses pane last-row logic, draws a full-width line under embedded groups (on the **outer** `tr`, not the inner group `tr`), and duplicates the bottom edge on the last row.

**If lighter row separators are needed:** style **`td`** on non-last rows only, e.g.  
`.exodusformpane > table.exodusform > tbody > tr:not(:last-child) > td { border-bottom-color: #f0f0f0; }`

### Embedded groups (`TABLE[exogroupno]` inside a form cell)

- Host cell gets `border: none` and `padding: 0` via `:has(> TABLE[exogroupno])` in `global.css`.
- `exodusclear_embeddedtable_hostborders()` in `client.js` also clears inline borders on host cell/row.
- Inner group table often has **no** `exodusform` class and **no** `<thead>` (e.g. ledgerprint account picker).
- Inner cells are **not** direct children of the outer grid — they do **not** get the outer `> tbody > tr > td` border rule unless you add rules explicitly.

When a line appears “under” an embedded group, inspect the **parent outer `<tr>`** and page-local CSS before blaming the group table.

### Multivalue rows: do not attach listeners to group fields

`gds.addrow` / `insertrow` / `bindgroup` build lines with **`cloneNode(true)`**. That copies **attributes** (`exodusfieldno`, `exodusonchange`, `exoduspopup`, …) but **not** `addEventListener` handlers.

**Do not** wire product or framework behaviour only via per-element listeners on `groupno > 0` fields in `formfunctions_onload` — after insert/delete/Ctrl+D the new row looks bound but silent.

**Prefer (in order):**

1. **Document / table delegation** — one listener; filter on attributes (e.g. `form_oninput` on `document` for fields with `exodusfieldno` / `exodusonchange`).
2. **Attributes + existing form path** — `exodusonclick`, `exoduspopup`, dict `validation` / `onchange` already re-read from the element.
3. **Reattach in `setupnewrow` only if unavoidable** — that is the clone seam; keep it tiny and attribute-driven.

Same trap applies to any experimental live-typing / typeahead work on multivalue codes.

---

## 3. Flowing text fields (`align='T'`)

Set by `exodus_dict_text()` in `db.js`. `dbform.js` converts `INPUT` → `SPAN` and applies:

```javascript
element.style.minWidth = spanlen + 'ch';  // length floor (ch)
element.style.display = 'block';  // unless link/popup
element.style.whiteSpace = 'pre-wrap';
element.style.overflowWrap = 'break-word';
```

**Framework width model:** pane = content width (always). Soft-cap **form tables** at ~viewport (`max-width: calc(100vw − 2rem)`), not the pane. Contenteditable: `max-width: 100%` of cell + `overflow-wrap`. No HTM hard-coded widths for this.

**Width symptoms:** a table looks “too wide” or “stuck left” because **column minimums** sum above the viewport, not because the pane chooses `width: 100%`. `margin: auto` cannot center content wider than the viewport.

**Prefer native knobs (in order):**

1. **Dict** — `di.length` (lowers min-width floor; keeps flowing).
2. **HTM** — `class` on the column `td` (thead + template tbody row; cloned rows inherit).
3. **Page-local CSS** — only if a page needs a tighter cap: `table-layout: fixed` + column width + `min-width: 0 !important` on `.exodusid_FIELD` (must override dbform inline `min-width`).

**Avoid:** `max-width` on HTM `<input>` only; commenting out `exodus_dict_text()` to “shrink” a field; page `!important` that fights the global 100% pane cap without a reason.

---

## 4. Top-level vs embedded group tables

| | Top-level group (e.g. authorisation Users) | Embedded group (e.g. ledgerprint accounts) |
|---|--------------------------------------------|-----------------------------------------------|
| Position | Direct child of pane / centered div | Nested in outer form `td` |
| Class | `TABLE.exodusform` + `exogroupno` | Often only `exodusid_exogroupN` |
| Headings | Usually has `<thead>` | Often **no** `<thead>` |
| Width / centering | `max-content` pane | Host cell `border: none`; width from content |
| Typical width issue | Span `min-width` × many columns | Less common; border/row-separator issues more common |

---

## 5. Debugging checklist (before adding CSS hacks)

1. **Inspect the element that owns the line or width** — Styles panel → winning `border-bottom` / `min-width` rule and **which stylesheet**.
2. **Read page-local `<style>` in the `.htm`** — bare `tr` / `table` selectors are a frequent root cause.
3. **Compare source HTM vs Elements panel** — ids, tag names (`input` vs `span`), `exogroupno`, pane wrapper.
4. **Check dict** — `align`, `length`, `groupno`, `exodus_dict_text()` vs plain input.
5. **Pane coalescence** — two tables with only `<br>` between them become **one** pane; tabs/panels prevent that.
6. **Prefer root cause** — remove conflicting page CSS or use the dict/HTM knob; do not stack global suppressions.

---

## 6. Anti-patterns (treat as technical debt)

- `tr { border-bottom: … }` on forms that use `exodusformpane`
- Global overrides to “fix” one page’s width or borders
- `width: 50%` on HTM expecting it to beat span `min-width` or `max-content` panes
- `!important` on `min-width` without a column/`td` strategy
- Assuming overflow/centering bugs before checking **`exolength`** on flowing spans
- Suppressing embedded-group `tr` borders globally instead of fixing the outer row / page CSS

---

## 7. Key files

| File | Role |
|------|------|
| `global.css` | Form face, pane shell, td/th grid, embedded host-cell rules, pane edge stripping |
| `scripts/dbform.js` | Dict bind, group tables, span min-width, insert/delete row, `form_filter` |
| `scripts/db.js` | `exodus_dict_text`, `dictrec`, dict helpers |
| `scripts/client.js` | `exoduswrapformpanes`, `exoduscoalesceformpanes`, `exodusclear_embeddedtable_hostborders` |
| `template.htm` | Minimal form page pattern |

**Vertical alignment** — `global.css` sets `vertical-align: top` on all `TABLE.exodusform`, `TABLE.exotable`, and `TABLE[exogroupno]` cells and rows (including nested tables). Do not add inline `valign` or `vertical-align` in HTM unless a page needs a deliberate exception.

**Cell padding** — em-based in `global.css`, three layers: (1) **outer grid** `--exodus-form-cell-padding-x/y` on direct `exodusform > tr > td|th` (ledgerprint, filters); (2) **plain nested tables** and (3) **group/exotable** via `--exodus-form-nested-cell-padding-x/y`. Horizontal is **equal L/R** (split of the old all-left inset) so content is not jammed against the right cell border. Do not hard-code px padding unless a page truly needs an exception.

**Label italic** — all `TABLE.exodusform td/th` use `--exodus-form-label-font-style` (default italic), including nested label tables. Bound data fields (`INPUT`/`SPAN.clsRequired` etc.) reset to `--exodus-form-data-font-style` (normal). Do not set `font-style: normal` on nested-table padding rules.

**Prompt cells (`th`)** — Human prompt text is `th` (outer grid *and* nested header strips). Data/controls stay in `td` (never put `datafld` / bound inputs in `th`). All `TABLE.exodusform th` use the same richer face as sticky group thead (OKLCH / lighter mix / DM `#303a5a`). Optional `TABLE.exodus-fieldstrip` is layout only (side-by-side strips); face is the same rule. Sticky *position* stays on `TABLE[exogroupno] > thead` (and decide). Do not invent `exohlabel` / `exovlabel` classes.

**Section title rows** — Long forms (agency/system/timesheet configuration, …) use a full-width prompt `th` whose main content is an `h2` (optional hidden key input ok). No project `h2` stylesheet — size/weight/margins are browser UA (`1.5em` bold, `~0.83em` block margin). Cell face = prompt `th` tint; italic inherits from the form. Do not put a nested data table in the same cell as the section `h2`.

---

## 8. Worked examples (from real fixes)

**Tabs (authorisation):** wrap each top-level table in a tab panel div so panes are not coalesced; both tables stay in DOM (`display:none` panel); page-local CSS re-centres `.exodusformpane` inside panels.

**Column width (authorisation KEYS):** `authorisation_keys_col` on `td` + `table-layout: fixed` + `.exodusid_KEYS, .exodusid_IP_NUMBERS { min-width: 0 !important; max-width: 100%; }` — flowing text preserved via dict.

**Spurious border (ledgerprint):** removed page `tr { border-bottom }`; grid uses global `td` borders; pane owns bottom edge.

---

*Last updated from authorisation + ledgerprint layout work (asyncjs branch). Extend this file when a non-obvious convention is discovered — keep it short and strategic.*