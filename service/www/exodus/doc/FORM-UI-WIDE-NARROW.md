# FORM-UI-WIDE-NARROW

**Location:** `service/www/exodus/doc/` (web programming docs)

**AIM and method** of Exodus **narrow vs extreme-wide** form layout (dbform + `global.css` + `client.js`).

Related:

- [FORM-UI-PHILOSOPHY.md](./FORM-UI-PHILOSOPHY.md) — pane/frame model, no flash
- [FORM-UI-TYPES.md](./FORM-UI-TYPES.md) — field categories (code vs free-text) and soft max
- Code: `../3/exodus/global.css` (soft ceiling, nest hosts, `.exoform-wide`)
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
| Form table | `.exoformpane > TABLE.exoform` has **`max-width: calc(100vw - 2rem)`** — soft ceiling |
| Form table width | **Not** forced `max-content` on non-wide forms (auto under the ceiling) |
| Nested hosts (no HTM `width=`) | **`max-width: 100%` only** — no `width: max-content` |
| Nested hosts opt-in | HTML **`width="100%"`** for line grids that must fill the data column |
| Free-text SPAN | Paint: **`width/max 100%`**, **`pre-wrap`**, fold under cell; **`style.maxWidth` stays `100%`** |
| Attribute `exomaxwidth="30ch"` | Stored as **wide soft-max candidate only** — not applied as style until wide |

### Wide (`.exoform-wide` only)

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
band = already .exoform-wide
     ? soft_ceiling − hysteresis (~64px)   // stay-wide: need a clearer fit to leave
     : soft_ceiling

wantWide =
    skeleton_width(free-text floors @ 6ch, table width max-content)
      > band
    OR
    (record displayed
     AND form already at soft ceiling
     AND free-text column is crushed below its preferred width)
    OR
    sprawl scrollWidth past ceiling (enter) / past band (leave)

Leave-wide sprawl check: temporarily clear .exoform-wide (and 30ch soft max),
measure scrollWidth, restore. Do not trust scrollWidth under wide-only CSS
(max-content / cell min-width max-content / 30ch) — that understates sprawl
and causes narrow↔wide thrash (e.g. myjobs after filter).

if wantWide:
    add .exoform-wide
    apply free-text soft max (30ch when exomaxwidth set)
else:
    remove .exoform-wide
    free-text style.maxWidth = 100%   // fold under cell / ceiling
```

**30ch is not “fold on narrow screens.”** Fold on narrow is **`max-width: 100%` + pre-wrap + soft ceiling**. 30ch is only a **soft cap in wide mode** so multi-col grids stay usable. It must not drive a false leave-wide.

---

## Multivalue col0 `width="1%"` (current nest-fill mechanism)

**Where:** `dbform.js` sets HTML `width="1%"` on group lead-in cells (ins/del tbody, thead chrome, tfoot spacer) classed `.exogroup_col0`.

This is a **deliberate layout side effect**, not “make the first column 1% of the table for looks.” Document it before “fixing” it.

### What it does *not* do

- Does **not** set the nest table’s `style.width` or `width="100%"`.
- Computed table width is still ordinary auto table layout + nest `max-width: 100%` from CSS.
- Free-text SPANs still paint as today (`width`/`maxWidth` **100% of cell**, `pre-wrap`); the `1%` is **not** painted on the span.

### What it does (theory of narrow free-text expand + fold)

Think of two stages:

**(a) Wide / narrow detection can still treat the form as narrow** when fixed columns fit the soft ceiling.

Skeleton measure floors free-text at **6ch** and prefers content width. The col0 `1%` is a **column** percentage, not nest `width: 100%` on the table element. Live layout and skeleton are not the same as “always force viewport-wide nests,” so the form is **not** forced into `.exoform-wide` merely because free-text should grow. Narrow forms keep free-text **`style.maxWidth = 100%`** (of cell), not the wide **30ch** soft max.

**(b) After that, free-text has a real capability to grow under the screen, then fold.**

Free-text already wants **100% of its cell** (akin to, but **not** the same as, setting nest `width: 100%` as the product knob). Without a definite nest/host width budget, those cells stay content-skinny and never “expand across the form.”

HTML **`width="1%"` on a column** makes the browser give the multivalue / nest table a **definite used width ≈ the host TD** (fill the data column). Then:

1. Free-text cells get that budget → spans **expand** with available width under the form **soft ceiling**.  
2. Long text **starts folding** (`pre-wrap` + `max-width: 100%` of cell + form `max-width: calc(100vw − 2rem)`) so typing prefers to stay **on screen**.  
3. If folding is **not** enough (many fixed columns, true extreme-wide), the form is allowed **past** the right edge (`.exoform-wide`, pan / Alt+←→) and free-text may take the **30ch** soft max instead of monopolising the grid.

So: **`1%` is the current back door that gives free-text its expand-then-fold budget on multivalue nests**, while still allowing narrow detection to keep **30ch off** until the form is truly wide. Secondary effect: col0 stays narrow when the nest is host-wide (without `1%`, that lead-in column often absorbs free space).

### Why obvious alternatives are not drop-in replacements

| Alternative | Why it fails or is incomplete alone |
|-------------|-------------------------------------|
| Remove `1%` only | Nest left-packs (not fill 100%) → free-text cells stay skinny → **no** expand-under-ceiling on line grids. |
| Nest `width: max-content` | **Forbidden** — free-text loses fold budget; sprawl or false wide → 30ch. |
| Nest CSS/HTML `width: 100%` as default | Honest fill, but skeleton measure can resolve % toward the **viewport** → false `.exoform-wide` → free-text stuck at **30ch** unless skeleton **temporarily** ignores nest fill. |
| Per-page HTM `width="100%"` on every line grid | Same fill idea; scales poorly; still needs skeleton care if used widely. |
| Style col0 with nowrap/valign only (no width) | Does **not** stop col0 exploding when the table is host-wide; only a **width** rule (`1%` today) does. |
| CSS `width: 0` on `.exogroup_col0` while keeping HTML `1%` for fill | Can **fight** the percentage contribution and kill host fill. |

Any real replacement must cover **both**: (1) nest host-width budget for free-text expand/fold, (2) col0 chrome min — and must not break **(a)** narrow detection / **(b)** 30ch only when truly wide.

### Compact multivalue (no free-text) — two halves

**Terms (do not use “hug” alone — ambiguous):**

| Term | Meaning |
|------|---------|
| **Fill** | Nest stretches to the **host TD** (~100% of data cell). Driven by col0 `width="1%"`. Free-text can expand/fold. Values (esp. R-aligned) can sit far from ins/del. |
| **Left-pack** (aka “hug content”) | Nest is only as wide as its columns need — **left-aligned pack, not fill 100%** of the host. Ins/del stay next to the values. Implemented as omit `1%` + `style.width = max-content`. |

Compact strips (short codes / integers) must **left-pack**, not fill (e.g. R-aligned `JOURNAL_NO` stuck far from ins/del when filled).

**Framework (`form_group_needs_nest_fill` in dbform) — simple rule:**

- **Fill** only if the group has **free-text data entry** (`type F` + text host via `form_field_exostyle` / `exo_dict_text`).  
- **Not** display `S`, not code/number/checkbox.  
- Otherwise **left-pack**: omit `1%` and set nest `style.width = max-content` (second half: without it, auto width under a wide host TD still takes available width ≈ fill).

Sources: `dict.groups[g]`, else dict index walk.

### Nest left-pack vs HTM hardcode

**Default:** do **not** hardcode nest width. Framework auto left-packs when the group has no free-text entry; auto-fills when it does.

**Last resort only** — if auto still fills wrongly and left-pack is required:

```html
<table class="exotable" style="width: max-content">
```

- Means **left-pack** (size-to-content / not fill 100% of host), not a vague “hug”.  
- **Do not** use nest `max-content` on free-text line grids (fold regression).  
- Opt-in **full width fill** remains HTML `width="100%"` on the nest.  
- Comment on the table when you hardcode, and prefer removing the hardcode once auto is enough.

**Does the algorithm honour hardcode?** Yes (`form_table_hardcoded_width` / `form_table_hardcode_is_hug` in dbform):

| HTM hardcode | Effect |
|--------------|--------|
| `style="width: max-content"` (or fit/min-content) | Force **left-pack**: no col0 `1%`; do not rewrite `style.width` |
| `width="100%"` or other `width=` | Leave `style.width` alone; **fill/`1%` still follows free-text entry** |
| none | Auto: free-text entry → fill `1%`; else left-pack `style.width = max-content` |

---

## Forbidden regressions (checklist before any width CSS/JS change)

1. **Do not** set `width: max-content` or `min-width: max-content` on  
   `TABLE.exoform TABLE.exotable|TABLE[exogroupno]|TABLE.exofieldstrip`  
   without `[width]` — kills free-text fold budget.
2. **Do not** set free-text `style.maxWidth = 30ch` at dbform paint — only the wide applier may do that, and only when `.exoform-wide` is present.
3. **Do not** “fix” a stretched nest by max-content on all nests; prefer remove spurious HTM `width=100%`, or opt-in `width=100%` only where a line grid needs fill.
4. Before changing form `width:` on non-wide tables, re-read competing bugs **A** (sibling stretch) and **B** (free-text fold) in `global.css` comments on `.exoformpane > TABLE.exoform`.
5. **Do not** remove multivalue col0 `width="1%"` without an explicit replacement for nest host-fill **and** a plan for wide/narrow (see section above). Col0-only “polish” without a width rule does not replace it.

---

## Where the knobs live

| Concern | File / symbol |
|---------|----------------|
| Soft ceiling | `global.css` → `.exoformpane > TABLE.exoform { max-width: calc(100vw - 2rem) }` |
| Nest hosts (no max-content) | `global.css` → nested `TABLE.exotable` / `[exogroupno]` / `.exofieldstrip` |
| Wide class + free-text min-width | `global.css` → `.exoform-wide` |
| Decide wide / apply 30ch | `client.js` → `form_update_wide_layout`, `form_table_set_wide`, `form_table_apply_freetext_wide_max` |
| Free-text paint + `exomaxwidth` attr | `dbform.js` → SPAN `fieldStyle === 'text'` |
| Field style axis | `dbform.js` / `db.js` → `exostyle` code \| text \| number |
| Multivalue col0 `width="1%"` (nest fill side effect) | `dbform.js` → group bind (`.exogroup_col0`); theory above |

---

## Keyboard note

When **wide**, Alt+← / Alt+→ **pan** the form (~1 screen). When **not wide**, those keys stay browser history. See [FORM-UI-KEYBOARD-BROWSER.md](./FORM-UI-KEYBOARD-BROWSER.md).
