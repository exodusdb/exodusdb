# FORM-UI-NUMBER — `[NUMBER…]` / `[DECIMAL…]`

**Location:** `service/www/exodus/doc/`  
**Runtime:** `service/www/3/exodus/scripts/exodus.js` (`NUMBER`, `DECIMAL`, `INTEGER`, `exodusround`), `db.js` (`exodus_dict_number`), `gds.js` / `dbform.js` (bind, `oconvertvalue`, `getvalue` / `getvalue_internal`)  
**See also:** [FORM-UI-TYPES.md](./FORM-UI-TYPES.md), [PROGRAMMERS_OVERVIEW.md](./PROGRAMMERS_OVERVIEW.md)

---

## 1. What they are

| Conversion | Role |
|------------|------|
| **`[NUMBER,…]`** | Full numeric **ICONV/OCONV** for bound fields and true **external** form (amounts: thousands when BASEFMT groups) |
| **`[DECIMAL,…]`** | Same as NUMBER OCONV but **plain** (no thousands); may have fractional places |
| **`[INTEGER]`** | Plain, **0 decimal places** (counts/days/sequences). Optional zero-suppress: **`[INTEGER,Z]`** / **`[INTEGER,0Z]`**. With min/max keep the decimals slot: **`[INTEGER,0,min,max]`** (not `[INTEGER,min,max]` — that would mis-bind min into the max slot) |

**Dict helpers** (`db.js`): bag only — all via `exodus_dict_number`.

```js
exodus_dict_number(di, { decimals: 'CURRENCY' })           // [NUMBER,…] amounts (grouping)
exodus_dict_number(di, { decimals: 'CURRENCY', signed: true })  // allow negatives (min SIGNED)
exodus_dict_integer(di, { max: 100 })                      // plain 0-dp counts
exodus_dict_integer(di)                                    // [INTEGER]
exodus_dict_decimal(di, { decimals: 2 })                   // plain fractional [DECIMAL,…]
exodus_dict_number(di)                                     // same as {}
```

| `opts` key | Default if omitted | Role |
|------------|--------------------|------|
| `decimals` | `''` | digit / `BASE` / `NDECS` / `CURRENCY` / `UNIT` / `nZ` |
| `min` / `max` | `''` | ICONV limits; empty min → default **≥ 0** unless `SIGNED` / `signed: true` |
| `signed` | omit | `true` → min slot `SIGNED` (allow negatives) when min omitted |
| `plain` | `false` | `true` → `[DECIMAL,…]`, or **`[INTEGER]`** / **`[INTEGER,0,min,max]`** when `decimals` is 0; omit → `[NUMBER,…]` |

**Omit defaults** — do not write `decimals: ''`, `min: ''`, or `plain: false`.  
Prefer `exodus_dict_number(di, { max: 100 })` over `{ min: 0, max: 100 }` (default ≥ 0).

Use **integer** for counts/days/sequences; **decimal** for plain fractional; **number** for amounts (grouping when BASEFMT groups).

| Direction | Meaning |
|-----------|---------|
| **ICONV** | external → internal (always strips grouping; validates min/max) |
| **OCONV** | internal → external |

**Internal (gds):** plain e.g. `1000.00` or `1000.00USD`.  
**External (screen):** often `1,000.00` or `1.000,00` when company BASEFMT groups.

There is **no** conversion name `AMOUNT`. Amount fields use NUMBER (often `CURRENCY` / `BASE` / `NDECS`).

---

## 2. Grouping vs plain (conversion name)

Call via conversion strings / shims — not by hand-passing extra args into `NUMBER`:

| Call | OCONV |
|------|--------|
| `.exodusoconv('[NUMBER,2]')` / bind / setx | decimals + **grouping** when BASEFMT ends with `,` |
| `.exodusoconv('[DECIMAL,2]')` / `DECIMAL(...)` | decimals only, plain `1000.00` |
| `.exodusoconv('[INTEGER]')` / `INTEGER(...)` | **0 dp**, plain (no thousands) |
| `ROUND(...)` | legacy alias for `DECIMAL` |

Both ICONV and OCONV **normalize** grouping before parse (accept plain or already-external).  
How DECIMAL/INTEGER ask NUMBER for plain / 0 dp is an **implementation detail** (not a public conversion parameter).  
**No** `gnumber_oconv_display` / begin/end.

---

## 3. Parameters (same slots for NUMBER, DECIMAL, INTEGER)

```text
[NUMBER|DECIMAL|INTEGER, <decimals>, <min>, <max>]
```

Examples: `[NUMBER,2]`, `[DECIMAL,2,0,100]`, `[INTEGER]`, `[INTEGER,Z]`, `[INTEGER,0,0,999999]`.

`CURRENCY` / `UNIT` may appear in any slot.

| Param | Effect |
|-------|--------|
| digit / empty / `NDECS` / `BASE` / `nZ` | **Decimal places** (and zero-suppress). INTEGER forces 0 at runtime; with min/max keep a `0` in this slot so min/max stay in the right positions. |
| min / max | ICONV numeric limits |
| `CURRENCY` / `UNIT` | amount+unit |

These do **not** select thousands by name. Grouping is:

- **NUMBER OCONV** + **`gbasefmt` ends with `,`**, and  
- MD vs MC for **which** character is decimal vs thousands.

`gbasefmt` (cookie `bf`) is fixed for the session (company format).

---

## 4. Policy: do not OCONV to round mid-calc

**Bad:** `(a + b).exodusoconv('[NUMBER,2]')` then more math.  
**Good:** `.exodusoconv('[DECIMAL,2]')` or pure `exodusround(n, 2)` then continue; use **NUMBER** only for real external / field conversion.

Amount fields: omit `plain` (or `plain: false`) so conversion is `[NUMBER,…]` and paint gets grouping. Non-amount integers: `plain: true` + `decimals: 0` → `[INTEGER]` (ndecs forced in shim).

---

## 5. getvalue

`getvalue` is always **external** from the DOM (including NUMBER with thousands/unit).

For storage/math use **`getvalue_internal(element)`** = `getvalue` + ICONV when `exodusconversion` is `[…]`. Do not hide ICONV inside `getvalue` (that broke “did the field change?” for grouping-only edits).

---

## 6. Source map

| Concern | Where |
|---------|--------|
| `NUMBER` / `DECIMAL` / `INTEGER` | `exodus.js` |
| Dict helper | `db.js` → `exodus_dict_number(di, opts)` → `[NUMBER,…]`, or `[DECIMAL,…]` / `[INTEGER]` / `[INTEGER,0,min,max]` when `plain` |
| Pure numeric round | `exodus.js` → `exodusround` |
| BASEFMT | `client.js` → `gbasefmt`, `gthousands_regex` |
| DOM read | `dbform.js` → `getvalue` (external), `getvalue_internal` (ICONV) |
