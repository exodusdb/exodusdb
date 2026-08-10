# FORM-UI-WIDE-NARROW

**Location:** `service/www/exodus/doc/` (web programming docs)

**AIM and method** of Exodus **narrow vs extreme-wide** form layout (dbform + `global.css` + `client.js`).

Related:

- [FORM-UI-PHILOSOPHY.md](./FORM-UI-PHILOSOPHY.md) — pane/frame model, no flash
- [FORM-UI-TYPES.md](./FORM-UI-TYPES.md) — field categories (code vs free-text) and soft max
- Code: `../3/exodus/global.css` (soft ceiling, nest hosts, `.exodusform-wide`)
- Code: `../3/exodus/scripts/client.js` (`form_update_wide_layout`, `form_table_set_wide`)
- Code: `../3/exodus/scripts/dbform.js` (free-text paint, `exomaxwidth` attribute only)

---

## Concept (start here)

A **narrow** form is one that **fits within a screen**. In it, text fields can **expand until the form fills the screen**, and **then the text folds**. Whether a form is narrow or wide can **change if the user zooms in or out**.

A **wide** form is one that **does not fit within a screen**. Text fields **expand up to ~30 characters (30ch)** and **then fold**.

The two strategies give the **best UI depending on form and screen size**.

---

## AIM

1. **Narrow (default):** Form fits the screen under a **viewport soft ceiling**. Free-text grows with available width, then **folds** so long typing does not force horizontal sprawl.
2. **Wide (exception):** Form cannot fit (many fixed columns / skeleton past the ceiling, or free-text crushed in a multi-col grid). Allow the form past the ceiling (horizontal pan / Alt+←→). Empty-length free-text **soft-max ~30ch**, then fold — so description columns do not monopolise the grid.

**What is not the aim**

- Treating **30ch** as the fold rule for **narrow** forms (narrow fold is “fill screen, then wrap”).
- “Fixing” nest hug / stretched tables by putting **`width: max-content`** on nested host tables (exotable / exogroup / fieldstrip). That is a **repeated bad regression**: free-text loses its width budget and stops folding.

---

## Method (two modes)

### Narrow (default)

| Piece | Behaviour |
|--------|-----------|
| Form table | `.exodusformpane > TABLE.exodusform` has **`max-width: calc(100vw - 2rem)`** — soft ceiling |
| Form table width | **Not** forced `max-content` on non-wide forms (auto under the ceiling) |
| Nested hosts (no HTM `width=`) | **`max-width: 100%` only** — no `width: max-content` |
| Nested hosts opt-in | HTML **`width="100%"`** for line grids that must fill the data column |
| Free-text SPAN | Paint: **`width/max 100%`**, **`pre-wrap`**, fold under cell; **`style.maxWidth` stays `100%`** |
| Attribute `exomaxwidth="30ch"` | Stored as **wide soft-max candidate only** — not applied as style until wide |

### Wide (`.exodusform-wide` only)

| Piece | Behaviour |
|--------|-----------|
| Who decides | **`form_update_wide_layout()`** in `client.js` (not CSS alone) |
| When | Skeleton preferred width **>** soft ceiling, **or** record open and free-text **crushed** under the ceiling |
| Form table | `max-width: none; width: max-content` |
| Free-text cells | CSS `min-width: max-content` on host tds |
| Free-text soft max | **`form_table_apply_freetext_wide_max(table, true)`** → empty-length spans get **`style.maxWidth` from `exomaxwidth` (30ch)** |
| Leaving wide | Must re-apply **`style.maxWidth = 100%`** — never leave a stale 30ch on narrow |

---

## Decision algorithm (method detail)

```
soft_ceiling = 100vw − 2rem   (current zoom)

wantWide =
    skeleton_width(free-text floors @ 6ch, table width max-content)
      > soft_ceiling
    OR
    (record displayed
     AND form already at soft ceiling
     AND free-text column is crushed below its preferred width)

if wantWide:
    add .exodusform-wide
    apply free-text soft max (30ch when exomaxwidth set)
else:
    remove .exodusform-wide
    free-text style.maxWidth = 100%   // fold under cell / ceiling
```

**30ch is not “fold on narrow screens.”** Fold on narrow is **`max-width: 100%` + pre-wrap + soft ceiling**. 30ch is only a **soft cap in wide mode** so multi-col grids stay usable.

---

## Forbidden regressions (checklist before any width CSS/JS change)

1. **Do not** set `width: max-content` or `min-width: max-content` on  
   `TABLE.exodusform TABLE.exotable|TABLE[exogroupno]|TABLE.exodus-fieldstrip`  
   without `[width]` — kills free-text fold budget.
2. **Do not** set free-text `style.maxWidth = 30ch` at dbform paint — only the wide applier may do that, and only when `.exodusform-wide` is present.
3. **Do not** “fix” a stretched nest by max-content on all nests; prefer remove spurious HTM `width=100%`, or opt-in `width=100%` only where a line grid needs fill.
4. Before changing form `width:` on non-wide tables, re-read competing bugs **A** (sibling stretch) and **B** (free-text fold) in `global.css` comments on `.exodusformpane > TABLE.exodusform`.

---

## Where the knobs live

| Concern | File / symbol |
|---------|----------------|
| Soft ceiling | `global.css` → `.exodusformpane > TABLE.exodusform { max-width: calc(100vw - 2rem) }` |
| Nest hosts (no max-content) | `global.css` → nested `TABLE.exotable` / `[exogroupno]` / `.exodus-fieldstrip` |
| Wide class + free-text min-width | `global.css` → `.exodusform-wide` |
| Decide wide / apply 30ch | `client.js` → `form_update_wide_layout`, `form_table_set_wide`, `form_table_apply_freetext_wide_max` |
| Free-text paint + `exomaxwidth` attr | `dbform.js` → SPAN `fieldStyle === 'text'` |
| Field style axis | `dbform.js` / `db.js` → `exostyle` code \| text \| number |

---

## Keyboard note

When **wide**, Alt+← / Alt+→ **pan** the form (~1 screen). When **not wide**, those keys stay browser history. See [FORM-UI-KEYBOARD-BROWSER.md](./FORM-UI-KEYBOARD-BROWSER.md).
