# Exodus form UI — layout philosophy

**Location:** `exodus/service/www/3/exodus/`  
**Audience:** maintainers and AI assistants working on Exodus web forms  
**Discovery:** pointed to from `~/exodus/AGENTS.md`, `~/neosys/AGENTS.md`, and `.grok/rules/exodus-form-ui.md` in those repos  
**See also:** `scripts/PROGRAMMERS_OVERVIEW.md`, `exodus/doc/forms.htm`, `global.css`

This note captures *strategic* UI conventions — how forms are built at runtime, how borders and width behave, and what to avoid. It is not a duplicate of the programmer's manual.

---

## 1. HTM is a seed; the live DOM is different

Pages ship static HTM plus dicts (`*_dict.js`). **`dbform.js` rewrites the DOM on load.** Always reason about the **runtime** tree, not the source file alone.

Common transforms:

| Source HTM | After `dbform.js` |
|------------|-------------------|
| `<input id="FIELD">` with dict `align='T'` | Contenteditable `<span>` (flowing text) |
| Nested `<table>` with `exodusgroupno` fields | `TABLE[exodusgroupno]` / `#exodusgroupN` |
| Top-level `TABLE.exodusform` | Often wrapped in `div.exodusformpane` by `exoduswrapformpanes()` in `client.js` |

**Implication:** inline styles on `<input>` in HTM (e.g. `max-width`) are often **discarded** when the field becomes a `<span>`. Dict properties and page CSS must target the **final** element (`.exodusid_FIELD`, column `td` classes, etc.).

---

## 2. Layer responsibilities (do not fight them)

```
div.exodusformpane          ← outer rounded frame (owns the outside edge)
  TABLE.exodusform          ← grid; table element border stripped inside pane
    tr / td                 ← cell grid lines (td borders, not tr)
      TABLE[exodusgroupno]  ← embedded repeating group (nested)
```

### Pane (`exodusformpane`)

- Inserted by `exoduswrapformpanes()` / merged by `exoduscoalesceformpanes()` (sibling panes separated only by `<br>` merge into one shell).
- Centered pages: `div[align="center"] > .exodusformpane { width: max-content; margin: auto; }` — pane sizes to **content**, not viewport.
- Pane **strips** outer table border and removes **last row cell** bottom borders so the pane border is the only bottom edge.

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

### Embedded groups (`TABLE[exodusgroupno]` inside a form cell)

- Host cell gets `border: none` and `padding: 0` via `:has(> TABLE[exodusgroupno])` in `global.css`.
- `exodusclear_embeddedtable_hostborders()` in `client.js` also clears inline borders on host cell/row.
- Inner group table often has **no** `exodusform` class and **no** `<thead>` (e.g. ledgerprint account picker).
- Inner cells are **not** direct children of the outer grid — they do **not** get the outer `> tbody > tr > td` border rule unless you add rules explicitly.

When a line appears “under” an embedded group, inspect the **parent outer `<tr>`** and page-local CSS before blaming the group table.

---

## 3. Flowing text fields (`align='T'`)

Set by `exodus_dict_text()` in `db.js`. `dbform.js` converts `INPUT` → `SPAN` and applies:

```javascript
element.style.minWidth = (exoduslength * 7) + 'px';
element.style.display = 'block';  // unless link/popup
element.style.whiteSpace = 'pre-wrap';
```

**Width symptoms:** a table looks “too wide” or “stuck left” because **column minimums** sum above the viewport, not because the pane chooses `width: 100%`. `margin: auto` cannot center content wider than the viewport.

**Prefer native knobs (in order):**

1. **Dict** — `di.length` (lowers min-width floor; keeps flowing).
2. **HTM** — `class` on the column `td` (thead + template tbody row; cloned rows inherit).
3. **Page-local CSS** — cap column: `table-layout: fixed` + `td` width + `min-width: 0 !important` on `.exodusid_FIELD` spans (must override dbform inline `min-width`).

**Avoid:** `max-width` on HTM `<input>` only; commenting out `exodus_dict_text()` to “shrink” a field; global `!important` churn in `global.css` when a page/dict fix exists.

---

## 4. Top-level vs embedded group tables

| | Top-level group (e.g. authorisation Users) | Embedded group (e.g. ledgerprint accounts) |
|---|--------------------------------------------|-----------------------------------------------|
| Position | Direct child of pane / centered div | Nested in outer form `td` |
| Class | `TABLE.exodusform` + `exodusgroupno` | Often only `exodusid_exodusgroupN` |
| Headings | Usually has `<thead>` | Often **no** `<thead>` |
| Width / centering | `max-content` pane | Host cell `border: none`; width from content |
| Typical width issue | Span `min-width` × many columns | Less common; border/row-separator issues more common |

---

## 5. Debugging checklist (before adding CSS hacks)

1. **Inspect the element that owns the line or width** — Styles panel → winning `border-bottom` / `min-width` rule and **which stylesheet**.
2. **Read page-local `<style>` in the `.htm`** — bare `tr` / `table` selectors are a frequent root cause.
3. **Compare source HTM vs Elements panel** — ids, tag names (`input` vs `span`), `exodusgroupno`, pane wrapper.
4. **Check dict** — `align`, `length`, `groupno`, `exodus_dict_text()` vs plain input.
5. **Pane coalescence** — two tables with only `<br>` between them become **one** pane; tabs/panels prevent that.
6. **Prefer root cause** — remove conflicting page CSS or use the dict/HTM knob; do not stack global suppressions.

---

## 6. Anti-patterns (treat as technical debt)

- `tr { border-bottom: … }` on forms that use `exodusformpane`
- Global overrides to “fix” one page’s width or borders
- `width: 50%` on HTM expecting it to beat span `min-width` or `max-content` panes
- `!important` on `min-width` without a column/`td` strategy
- Assuming overflow/centering bugs before checking **`exoduslength × 7`** on flowing spans
- Suppressing embedded-group `tr` borders globally instead of fixing the outer row / page CSS

---

## 7. Key files

| File | Role |
|------|------|
| `global.css` | Form face, pane shell, td grid, embedded host-cell rules, pane edge stripping |
| `scripts/dbform.js` | Dict bind, group tables, span min-width, insert/delete row, `form_filter` |
| `scripts/db.js` | `exodus_dict_text`, `dictrec`, dict helpers |
| `scripts/client.js` | `exoduswrapformpanes`, `exoduscoalesceformpanes`, `exodusclear_embeddedtable_hostborders` |
| `template.htm` | Minimal form page pattern |

**Vertical alignment** — `global.css` sets `vertical-align: top` on all `TABLE.exodusform`, `TABLE.exodustable`, and `TABLE[exodusgroupno]` cells and rows (including nested tables). Do not add inline `valign` or `vertical-align` in HTM unless a page needs a deliberate exception.

**Cell padding** — em-based in `global.css`, three layers: (1) **outer grid** `--exodus-form-cell-padding-*` on direct `exodusform > tr > td` (ledgerprint, filters); (2) **plain nested tables** and (3) **group/exodustable** via `--exodus-form-nested-cell-padding-*`. Do not hard-code px padding unless a page truly needs an exception.

---

## 8. Worked examples (from real fixes)

**Tabs (authorisation):** wrap each top-level table in a tab panel div so panes are not coalesced; both tables stay in DOM (`display:none` panel); page-local CSS re-centres `.exodusformpane` inside panels.

**Column width (authorisation KEYS):** `authorisation_keys_col` on `td` + `table-layout: fixed` + `.exodusid_KEYS, .exodusid_IP_NUMBERS { min-width: 0 !important; max-width: 100%; }` — flowing text preserved via dict.

**Spurious border (ledgerprint):** removed page `tr { border-bottom }`; grid uses global `td` borders; pane owns bottom edge.

---

*Last updated from authorisation + ledgerprint layout work (asyncjs branch). Extend this file when a non-obvious convention is discovered — keep it short and strategic.*