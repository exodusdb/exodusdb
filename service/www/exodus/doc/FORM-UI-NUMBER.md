# FORM-UI-NUMBER — `[NUMBER…]` / `[ROUND…]`

**Location:** `service/www/exodus/doc/`  
**Runtime:** `service/www/3/exodus/scripts/exodus.js` (`NUMBER`, `ROUND`, `exodusround`), `db.js` (`exodus_dict_number`), `gds.js` / `dbform.js` (bind, `oconvertvalue`, `getvalue` / `getvalue_internal`)  
**See also:** [FORM-UI-TYPES.md](./FORM-UI-TYPES.md), [PROGRAMMERS_OVERVIEW.md](./PROGRAMMERS_OVERVIEW.md)

---

## 1. What they are

| Conversion | Role |
|------------|------|
| **`[NUMBER,…]`** | Full numeric **ICONV/OCONV** for bound fields and true **external** form (amounts: thousands when BASEFMT groups) |
| **`[ROUND,…]`** | Same numeric work as NUMBER OCONV but **plain** (no thousands) — intermediate math **and** non-amount integers (journal no, counts, sequences) |

**Dict helper** (`db.js`): bag only — `exodus_dict_number(di, opts)`.

```js
exodus_dict_number(di, { decimals: 'CURRENCY' })           // [NUMBER,…] amounts
exodus_dict_number(di, { decimals: 0, plain: true })       // [ROUND,…] ids/counts/sequences
exodus_dict_number(di)                                     // same as {}
```

| `opts` key | Default if omitted | Role |
|------------|--------------------|------|
| `decimals` | `''` | digit / `BASE` / `NDECS` / `CURRENCY` / `UNIT` / `nZ` |
| `min` / `max` | `''` | ICONV limits; `min: 'POSITIVE'` |
| `plain` | `false` | `true` → `[ROUND,…]` (no thousands); omit → `[NUMBER,…]` |

**Omit defaults** — do not write `decimals: ''`, `min: ''`, or `plain: false`.  
`exodus_dict_number(di, { min: 0, max: 100 })` not `{ decimals: '', min: 0, max: 100 }`.

Use `plain: true` for non-amounts (JOURNAL_NO, NUMBER_ADS, SEQUENCE, port numbers, day counts, …).

| Direction | Meaning |
|-----------|---------|
| **ICONV** | external → internal (always strips grouping; validates min/max) |
| **OCONV** | internal → external |

**Internal (gds):** plain e.g. `1000.00` or `1000.00USD`.  
**External (screen):** often `1,000.00` or `1.000,00` when company BASEFMT groups.

There is **no** conversion name `AMOUNT`. Amount fields use NUMBER (often `CURRENCY` / `BASE` / `NDECS`).

---

## 2. `display` argument (no global flag)

```js
function NUMBER(mode, value, params, display)
// display default **true**

function ROUND(mode, value, params) {
    return NUMBER(mode, value, params, false)
}
```

| Call | `display` | OCONV result |
|------|-----------|--------------|
| `.exodusoconv('[NUMBER,2]')` / bind / setx | **true** (default) | decimals + **grouping** when BASEFMT ends with `,` |
| `.exodusoconv('[ROUND,2]')` / `ROUND(...)` | **false** | decimals only, plain `1000.00` |

Both ICONV and OCONV **normalize** grouping before parse (accept plain or already-external).  
`display` only controls whether OCONV **re-applies** thousands after decimals.  
**No** `gnumber_oconv_display` / begin/end.

---

## 3. Parameters (same for NUMBER and ROUND)

```text
[NUMBER|ROUND, <decimals>, <min|POSITIVE>, <max>]
```

`CURRENCY` / `UNIT` may appear in any slot.

| Param | Effect |
|-------|--------|
| digit / empty / `NDECS` / `BASE` / `nZ` | **Decimal places** (and zero-suppress) |
| min / max / `POSITIVE` | ICONV limits |
| `CURRENCY` / `UNIT` | amount+unit |

These do **not** select thousands by name. Grouping is:

- **OCONV + display true** + **`gbasefmt` ends with `,`**, and  
- MD vs MC for **which** character is decimal vs thousands.

`gbasefmt` (cookie `bf`) is fixed for the session (company format).

---

## 4. Policy: do not OCONV to round mid-calc

**Bad:** `(a + b).exodusoconv('[NUMBER,2]')` then more math.  
**Good:** `.exodusoconv('[ROUND,2]')` or pure `exodusround(n, 2)` then continue; use **NUMBER** only for real external / field conversion.

Amount fields: omit `plain` (or `plain: false`) so conversion is `[NUMBER,…]` and paint gets grouping. Non-amount integers: `plain: true` → `[ROUND,…]`.

---

## 5. getvalue

`getvalue` is always **external** from the DOM (including NUMBER with thousands/unit).

For storage/math use **`getvalue_internal(element)`** = `getvalue` + ICONV when `exodusconversion` is `[…]`. Do not hide ICONV inside `getvalue` (that broke “did the field change?” for grouping-only edits).

---

## 6. Source map

| Concern | Where |
|---------|--------|
| `NUMBER` / `ROUND` | `exodus.js` |
| Dict helper | `db.js` → `exodus_dict_number(di, opts)` → `[NUMBER,…]` or `[ROUND,…]` if `plain` |
| Pure numeric round | `exodus.js` → `exodusround` |
| BASEFMT | `client.js` → `gbasefmt`, `gthousands_regex` |
| DOM read | `dbform.js` → `getvalue` (external), `getvalue_internal` (ICONV) |
