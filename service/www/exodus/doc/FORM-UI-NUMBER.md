# FORM-UI-NUMBER — `[NUMBER…]` / `[DECIMAL…]` / `[INTEGER]`

**Location:** `service/www/exodus/doc/`  
**Runtime:** `service/www/3/exodus/scripts/exodus.js` (`NUMBER`, `DECIMAL`, `INTEGER`, `exoround`), `db.js` (`exo_dict_number` / `_integer` / `_decimal`), `gds.js` / `dbform.js` (bind, `oconvertvalue`, `getvalue` / `getvalue_internal`)  
**Tests:** `service/www/3/exodus/scripts/test_number.js`  
**See also:** [FORM-UI-TYPES.md](./FORM-UI-TYPES.md), [PROGRAMMERS_OVERVIEW.md](./PROGRAMMERS_OVERVIEW.md)

---

## 1. What they are

| Conversion | Role |
|------------|------|
| **`[NUMBER,…]`** | Full numeric **ICONV/OCONV** for bound fields and true **external** form — **fully formatted** amounts (see §5) |
| **`[DECIMAL,…]`** | Same ndecs path as NUMBER OCONV but **plain** (no thousands; `.` decimal only) |
| **`[INTEGER]`** | Plain, **0 decimal places** (counts/days/sequences). Optional zero-suppress: **`[INTEGER,Z]`** / **`[INTEGER,0Z]`**. With min/max keep the decimals slot: **`[INTEGER,0,min,max]`** (not `[INTEGER,min,max]` — that would mis-bind min into the max slot) |

There is **no** conversion name `AMOUNT`. Amount fields use NUMBER (often `CURRENCY` / `BASE` / `NDECS`).

`ROUND` is a **legacy alias** for `DECIMAL` — prefer `DECIMAL` / `[DECIMAL,…]`.

| Direction | Meaning |
|-----------|---------|
| **ICONV** | external → internal (always strips grouping; validates min/max) |
| **OCONV** | internal → external |

**Internal (gds):** plain ASCII e.g. `1000.00` or `1000.00USD` (`.` decimal, **no** thousands).  
**External (NUMBER OCONV / screen):** fully formatted per company BASEFMT — e.g. `1,000.00` or `1.000,00` (and unit suffix when `CURRENCY`/`UNIT`).

---

## 2. Dict helpers (bag only)

All via `exo_dict_number` in `db.js`. Prefer helpers over raw `dictrec(…, '[NUMBER…]')` or `di.conversion = '[NUMBER…]'`.

```js
exo_dict_number(di, { decimals: 'CURRENCY' })           // [NUMBER,…] amounts (grouping)
exo_dict_number(di, { decimals: 'CURRENCY', signed: true })  // → min SIGNED (any neg)
exo_dict_number(di, { signed: true, max: 100 })         // → min -100, max 100 (no SIGNED token)
exo_dict_number(di, { signed: true })                   // → min SIGNED
exo_dict_integer(di, { max: 100 })                      // plain 0-dp counts
exo_dict_integer(di)                                    // [INTEGER]
exo_dict_decimal(di, { decimals: 2 })                   // plain fractional [DECIMAL,…]
exo_dict_number(di)                                     // same as {}
```

| `opts` key | Default if omitted | Role |
|------------|--------------------|------|
| `decimals` | `''` | digit / `BASE` / `NDECS` / `CURRENCY` / `UNIT` / `nZ` / combos (`'NDECS,CURRENCY'`). **Empty is special** — see below |
| `min` / `max` | `''` | ICONV limits; empty min → default **≥ 0** unless `SIGNED` / `signed: true` |
| `signed` | omit | `true` when **min omitted**: numeric **max** → min = **−max** (symmetric; normal min/max rules); else min slot **SIGNED**. **Ignored if `min` is set.** |
| `plain` | `false` | `true` → `[DECIMAL,…]`, or **`[INTEGER]`** / **`[INTEGER,0,min,max]`** when `decimals` is 0; omit → `[NUMBER,…]` |

**Shims**

| Helper | Defaults if unset | Result kind |
|--------|-------------------|-------------|
| `exo_dict_integer` | `decimals: 0`, `plain: true` | counts / days / sequences |
| `exo_dict_decimal` | `plain: true` | plain fractional (no thousands) |
| `exo_dict_number` | (none) | amounts unless `plain` |

**Omit defaults** — do not write `decimals: ''`, `min: ''`, `plain: false`, or redundant `min: 0` (empty min already means ≥ 0).

```js
// Prefer
exo_dict_number(di, { max: 100 })
// Not
exo_dict_number(di, { min: 0, max: 100 })
```

**Empty / omitted `decimals` (rare — intentional only)**  

When the decimals slot is empty, NUMBER ICONV/OCONV sets ndecs from the **current value’s** fractional digit count (keep entered places / “dynamic” width), not from company NDECS/BASE.  

- **Omit `decimals` only when that dynamic / keep-entered behaviour is intentional** (unusual for bound amounts).  
- Prefer an explicit choice: `0`, `2`, `'NDECS'`, `'BASE'`, `'CURRENCY'`, etc.  
- Do **not** omit just to “use the default” for normal money fields — default empty ≠ NDECS.

**Choose the helper by meaning**

| Kind | Helper | Typical bag |
|------|--------|-------------|
| Counts, days, sequences, line nos | `exo_dict_integer` | `{ max: n }` if needed |
| Amounts (grouping when BASEFMT groups) | `exo_dict_number` | `{ decimals: 'CURRENCY' }` / `'BASE'` / `'NDECS'` |
| Allow negatives (credits, journals, some estimates) | same + | `signed: true` |
| Plain fractional (no thousands) | `exo_dict_decimal` | `{ decimals: 2 }` or `NDECS` |

---

## 3. ICONV min / max / SIGNED (load-bearing)

Implemented in `NUMBER` (`exodus.js`). DECIMAL/INTEGER call into the same path.

| Rule | Behaviour |
|------|-----------|
| **Empty min** | Value must be **≥ 0** |
| **min `SIGNED`** | Clears the min numeric test; **negatives allowed** |
| **Numeric min** | Value must be ≥ min |
| **Empty max** | No upper bound |
| **Numeric max** | Value must be ≤ max (same empty+numeric gate as min — so **`max: 0` works**) |
| **`POSITIVE` keyword** | **Retired** — use numeric `min: 0` or rely on default ≥ 0 |

Bag → conversion string (`exo_dict_number`):

| Bag | Conversion min/max slots |
|-----|--------------------------|
| `signed: true`, no min, **no** max | min = `SIGNED`; `CURRENCY`/`UNIT` from decimals go **after** min/max (e.g. `[NUMBER,,SIGNED,,CURRENCY]`) |
| `signed: true`, no min, **numeric max** | min = **−max**, max unchanged (e.g. `{ signed: true, max: 100 }` → `…,-100,100`) — **no** `SIGNED` token; standard min/max ICONV |
| `min` set | `signed` ignored |

Prefer **`signed: true`** at the bag over raw `min: 'SIGNED'` or hand-written floors.

---

## 4. Parameters (same slots for NUMBER, DECIMAL, INTEGER)

```text
[NUMBER|DECIMAL|INTEGER, <decimals>, <min>, <max>]
```

Examples: `[NUMBER,2]`, `[DECIMAL,2,0,100]`, `[INTEGER]`, `[INTEGER,Z]`, `[INTEGER,0,0,999999]`, `[NUMBER,,SIGNED,,CURRENCY]`.

`CURRENCY` / `UNIT` are flags after decimals/min/max (`exo_dict_number` emits them last). NUMBER trims them with compact — mid-list still shifts slots (see tests); prefer trailing.

| Param | Effect |
|-------|--------|
| digit / empty / `NDECS` / `BASE` / `nZ` | **Decimal places** (and zero-suppress). INTEGER forces 0 at runtime; with min/max keep a `0` in this slot so min/max stay in the right positions. |
| min / max | ICONV numeric limits (`SIGNED` in min = allow neg) |
| `CURRENCY` / `UNIT` | amount+unit |

Parameter tokens (`CURRENCY`, decimals, min, max) do **not** choose thousands by name. Grouping and `,.` / `.,` separators are **only** a NUMBER OCONV + BASEFMT concern (§5).

---

## 5. NUMBER OCONV = fully formatted external (`,.` / `.,`)

**Any** use of NUMBER for **output** is full external form — not a light pad/round.

That includes:

| Call site | Same rule |
|-----------|-----------|
| `.exooconv('[NUMBER]')` / `.exooconv('[NUMBER,…]')` | Fully formatted OCONV |
| Bound field paint / `setx` with conversion `[NUMBER,…]` | Same |
| `NUMBER('OCONV', value, params)` with default `display` | Same |
| User messages, notes, invalid text built with `.exooconv('[NUMBER,…]')` | Same — intentional screen text |

**Not** “almost plain with optional commas.” Assume the result may contain **thousands separators and a locale decimal** and must not be fed back into arithmetic without ICONV (or avoid NUMBER mid-calc entirely — §6).

### BASEFMT rules (`gbasefmt`, cookie `bf`)

Session company format. OCONV (when grouping is on) uses:

| `gbasefmt` | Thousands | Decimal | Example OCONV of `1000.5` at 2 dp |
|------------|-----------|---------|-------------------------------------|
| **MD…,** (common “Anglo”) | `,` | `.` | `1,000.50` |
| **MC…,** (common “Euro”) | `.` | `,` | `1.000,50` |
| BASEFMT **does not** end with `,` | *(none)* | `.` | `1000.50` (ndecs only; still NUMBER path) |

Implementation sketch (`exodus.js` NUMBER OCONV):

- `useThousands` ⇔ last character of `gbasefmt` is `,`
- `isMC` ⇔ `gbasefmt` starts with `MC` → thousands `.` and decimal `,`; else thousands `,` and decimal `.`
- Integer part gets `\B(?=(\d{3})+(?!\d))` grouping when `useThousands`
- Unit suffix (`CURRENCY` / `UNIT`) is reattached after separators

ICONV **strips** either grouping style before parse (accept plain or already-external).

### Grouping vs plain (conversion name)

| Call | OCONV result shape |
|------|--------------------|
| `.exooconv('[NUMBER,…]')` / bind / setx / messages | **External:** ndecs + **full** `,.` or `.,` formatting when BASEFMT groups |
| `.exooconv('[DECIMAL,…]')` / `DECIMAL(...)` | **Plain:** ndecs only, `.` decimal, **no** thousands — re-entrable for math |
| `.exooconv('[INTEGER]')` / `INTEGER(...)` | **Plain:** 0 dp, no thousands |
| `ROUND(...)` | legacy alias for `DECIMAL` |

How DECIMAL/INTEGER ask NUMBER for plain / 0 dp is an **implementation detail** (`display` / `forced_ndecs` — not a public conversion parameter).  
**No** `gnumber_oconv_display` / begin/end.

**Document every `[NUMBER…]` OCONV as user-facing formatted output.** If you need plain digits for further calc or storage shape, use **`[DECIMAL…]`**, **`[INTEGER]`**, or **`exoround`** — not NUMBER.

---

## 6. Policy: do not OCONV to round mid-calc

**Bad:** `(a + b).exooconv('[NUMBER,2]')` then more math — result may be `1,234.56` / `1.234,56`, not a plain number.  
**Good:** `.exooconv('[DECIMAL,2]')` or pure `exoround(n, 2)` then continue.

Use **NUMBER** only for:

- Bound field conversion (helper → `[NUMBER,…]`)
- True **external** display / print
- **User-facing** strings (warnings, notes, invalid messages) where full formatting is wanted

Amount fields: omit `plain` so conversion is `[NUMBER,…]` and paint gets full external formatting.  
Non-amount integers: `exo_dict_integer` (or `plain: true` + `decimals: 0`) → `[INTEGER]`.

---

## 7. Paint and host (dbform)

Primary axis: **`di.exostyle`** (set by helpers only).

| Helper | `exostyle` | Host |
|--------|------------|------|
| `exo_dict_number` / `_integer` / `_decimal` | `"number"` | content **SPAN** (not fixed INPUT) |

| Axis | Number SPAN behaviour |
|------|------------------------|
| **`di.length`** | **Unused** for code/number SPANs (floor **6ch**, expand). Do not set length to “size” a number field. |
| **`di.align`** | Helper: if unset → **R** when `groupno > 0`, else **L**. Preset (e.g. footer totals) is **kept**. |
| **INPUT width** | NUMBER fields become SPAN; length-based INPUT width rules do not apply. |

### Align / length after the helper

**After** `exo_dict_integer` / `_number` / `_decimal`:

- Do **not** set `di.align` or `di.exostyle` (helper owns them).
- Do **not** set `di.length` for paint (ignored for number SPAN).

**Exception — align R on non-group / header / tfoot fields:** set **`di.align = 'R'` before** the helper so the helper preserves it (default for `groupno` 0 is L).

```js
// Footer total (group blank) — want R like table money cells
di = dict[++din] = dictrec('TOTAL_AMOUNT', 'S')
di.align = 'R'
exo_dict_number(di, { decimals: 'NDECS' })
```

### dictrec constructor args

Once a field uses a number helper, drop trailing conversion / align / length from `dictrec`:

```js
// Bad (legacy)
di = dict[++din] = dictrec('SEQUENCE', 'F', 3, '', '', '', '', '[INTEGER]', '', 'R', 5)
// Good
di = dict[++din] = dictrec('SEQUENCE', 'F', 3)
exo_dict_integer(di)
```

Indent helper calls at the **same** level as the matching `di = dict[++din] = …` line.

---

## 8. getvalue

`getvalue` is always **external** from the DOM (including NUMBER with thousands/unit).

For storage/math use **`getvalue_internal(element)`** = `getvalue` + ICONV when `exoconversion` is `[…]`. Do not hide ICONV inside `getvalue` (that broke “did the field change?” for grouping-only edits).

---

## 9. Settled contract (do not re-open casually)

| Topic | Rule |
|--------|------|
| Default non-negative | Empty min → **≥ 0** |
| Allow negatives | Bag **`signed: true`** → min `SIGNED`, or min = **−max** when max set (not magic floors) |
| Counts vs amounts | **integer** (plain 0 dp) vs **number** (full external format) vs **decimal** (plain frac) |
| NUMBER OCONV | Always **fully formatted** external (`,.` / `.,` per BASEFMT) — including `.exooconv('[NUMBER…]')` in messages |
| Bags | Omit defaults; no noisy `min: 0` |
| Paint | Helper sets `exostyle` + default align; no post-helper align/length/exostyle |
| Prefer helpers | Do not put `[NUMBER…]` / `[INTEGER…]` in `dictrec` conversion7 for live numeric fields |
| No `POSITIVE` | Use default ≥ 0 or numeric `min: 0` only when you need an explicit floor equal to zero **and** something else is going on (prefer omit) |
| No `UNSIGNED` | Not part of the public API unless explicitly reintroduced |

**Load-bearing NUMBER quirks** (documented in `exodus.js`; do not “fix” without tests): unit peel differs ICONV vs OCONV; `CURRENCY` mid-list shifts min/max; Z + unit zero; JS `0` vs `''` early exit. See comments on `function NUMBER` and `test_number.js`.

---

## 10. Rationalisation status

### Framework (exodus) — strong

| Area | Status |
|------|--------|
| ICONV default ≥ 0 / SIGNED / max gate | Done |
| INTEGER / DECIMAL shims + ROUND alias | Done |
| `exo_dict_number` bag + integer/decimal helpers | Done |
| `signed: true` → min SIGNED | Done |
| Paint: `exostyle number`, length unused, align by groupno | Done |
| Docs + `test_number.js` | Done |

**Framework leftovers (small)**

- Some **exodus product dicts** still call `exo_dict_number({ decimals: 0, min: 0 })` instead of `exo_dict_integer` and omitting `min: 0` (`systemconfiguration`, `parts`, `authorisation`, `colors`, …).
- Helpers under-used **inside** exodus’s own form scripts (API exists; call sites lag).
- Patterns that seed a numeric conversion then **overwrite** with a select list (e.g. year dropdown) are not “number fields” — do not treat as NUMBER migration targets.

### App call sites (neosys and peers) — strong on bound conversions

Typical mature state after the bag migration wave:

| Pattern | Target state |
|---------|----------------|
| `dictrec(…, '[NUMBER…]')` / `[INTEGER…]` / `[DECIMAL…]` | **None** for live numeric fields — use helpers |
| `di.conversion = '[NUMBER…]'` | Prefer helper; rare specials only |
| Allow-neg amounts | `signed: true` on the bag |
| Redundant `min: 0` | Dropped (default ≥ 0) |
| Post-helper `align` / `exostyle` | Dropped |
| Post-helper `length` | Drop (paint-dead for number SPAN); residual sites OK to clear when touched |

**Not the same as** “every right-aligned cell is a NUMBER field.” Many `dictrec(…, 'R', n)` columns are codes, ids, or display chrome **without** a numeric conversion — migrate only when ICONV/OCONV or amount semantics are real.

### Partial / open

| Topic | Notes |
|--------|------|
| Select list + leftover `[INTEGER]` / helper | If `di.conversion = cols` (or `'0:1:2:…'`) **overwrites** a prior numeric conversion, the field is a **select**, not a number. Do **not** add `dict_integer` (it would wipe the list). Drop dead `[INTEGER]` from dictrec; drop stray `exostyle = 'number'` unless paint truly needs it. |
| Helper **after** select assign | `di.conversion = cols` then `exo_dict_integer(di)` makes the helper win — confirm product intent; often a smell. |
| Special raw forms (e.g. `[NUMBER,*]`) | Decide per field: helper bag, keep raw with a comment, or replace. |
| `dict_number({ decimals: 0 })` without plain | May be intentional 0-dp **amount** (grouping). Pure counts → `dict_integer`. Signed 0-dp amounts → often still **number** + `signed: true`, not integer. |
| Legacy `dictrec(…, 'R', length)` next to a helper | Redundant; length unused for number SPAN; align usually already set by helper when `groupno > 0`. Strip when touching the line. |
| Mid-calc | Prefer DECIMAL / `exoround`; NUMBER only for external / messages. |
| Server C++ amount formatting | **Out of scope** of this JS conversion/helper rationalisation. |
| Broad `dict_decimal` adoption | API ready; few call sites until plain fractional fields are walked deliberately. |

### Scorecard (intent)

| Layer | Maturity | Meaning |
|--------|----------|---------|
| Exodus ICONV/OCONV semantics | High | Contract stable; tested |
| Exodus dict helper API | High | One bag path; shims thin |
| Exodus own product dicts | Medium | Still some `decimals:0` + `min:0` style |
| App bound fields on helpers | High | Raw dictrec numeric conversions largely gone |
| App bag style (signed / omit min:0) | High | Matches framework |
| App post-helper paint cleanup | High | align/exostyle; length residual |
| “Every R-aligned value is rationalised” | Low | Not a goal without per-field “is this NUMBER?” |
| Mid-calc / C++ | Partial / separate | Do not claim finished |

---

## 11. Source map

| Concern | Where |
|---------|--------|
| `NUMBER` / `DECIMAL` / `INTEGER` / `ROUND` | `exodus.js` |
| Dict helpers | `db.js` → `exo_dict_number` / `_integer` / `_decimal` |
| Pure numeric round | `exodus.js` → `exoround` |
| BASEFMT | `client.js` → `gbasefmt`, `gthousands_regex` |
| DOM read | `dbform.js` → `getvalue` (external), `getvalue_internal` (ICONV) |
| Number SPAN paint | `dbform.js` — `exostyle` number/code: floor 6ch, length unused |
| Self-check suite | `test_number.js` |

---

## 12. Migration checklist (when touching a field)

1. Is the **live** conversion numeric (`[NUMBER…]` / `[INTEGER…]` / `[DECIMAL…]`), or a select / other overwrite?
2. If numeric: pick **integer** / **number** / **decimal**; add helper **after** `dictrec` (so `groupno` is set).
3. Allow negatives? → `signed: true` (do not invent floors).
4. Drop dictrec conversion7 / align9 / length10 for that field; drop post-helper align / length / exostyle.
5. Footer / header want R with blank group? → `di.align = 'R'` **before** helper.
6. Mid-calc nearby? → DECIMAL or `exoround`, not NUMBER (NUMBER OCONV is fully formatted `,.` / `.,`).
7. User message amounts? → `[NUMBER…]` is correct (full format); do not treat the string as internal.
8. Match indent of the `di =` line.

