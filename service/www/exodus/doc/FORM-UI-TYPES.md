# FORM-UI-TYPES

**Location:** `service/www/exodus/doc/` (web programming docs)

Exodus bound form fields: **theory** (clean categories) and **practice** (dictitem × paint × DOM as implemented).

Primary code: `../3/exodus/scripts/db.js` (`dictrec`, `exo_dict_*`), `dbform.js` (paint, `copydictitem`), wide soft max in `client.js` + `global.css`.

Related:

- [FORM-UI-PHILOSOPHY.md](./FORM-UI-PHILOSOPHY.md)
- **[FORM-UI-NUMBER.md](./FORM-UI-NUMBER.md)** — `[NUMBER…]` / `[DECIMAL…]`
- **[FORM-UI-WIDE-NARROW.md](./FORM-UI-WIDE-NARROW.md)** — AIM + method of narrow free-text fold vs extreme-wide (`.exoform-wide`, 30ch soft max, nest width rules)

---

# Theory

A small closed set of field categories. Each has one width / fold story. Paint and wide layout should key off these, not re-intersect type × readonly × contenteditable × class × align × lowercase × length at every step.

## Ideal categories

| Category | How you know (intent) | Width / fold behaviour |
|----------|----------------------|-------------------------|
| **Code** | uppercase short key-like text | Fixed / hug / nowrap — no free-text soft max |
| **Entry free-text** | flowing text, editable | Fill cell; soft max when wide (empty length → 30ch default) |
| **Display free-text** | flowing text, not editable (readonly F or name-like S) | Same soft max as entry free-text; no edit chrome |
| **Fixed INPUT** | length-bound non-T field | Glyph × length paint on INPUT |
| **Textarea** | multi-line free text | TEXTAREA cols/rows — separate from SPAN soft max |
| **Date / time / period** | calendar / time helpers | Usually fixed length INPUT + popup |
| **Number** | NUMBER conversion | INPUT / display; not free-text soft max |
| **Choice** | select / radio / checkbox | Own widget layout |

### Free-text (theory)

**Free-text** = flowing foldable text (not a code).

Then only:

- **entry** vs **display** (editable or not)

One soft-max rule for both: narrow fit cell (max 100%); wide empty-length soft max ~30ch (or explicit length policy).

### Chrome (theory, orthogonal)

popup/link: real icon | pad (`''`) | suppress (`false`) | omit.  
Does not define text category.

### Design goal

Set category (or text-role + entry/display) once at dict or first paint. Drive white-space, soft max, fill, class, and host rules from that.

---

# Practice

What the code **actually** does: overlapping axes, not a single category id.

## 1. Dictitem axes (source)

From `dictrec` + helpers + common overrides:

| Axis | Values | Set by |
|------|--------|--------|
| **type** | `F` (file/data), `S` (symbolic/calc), also `B` buttons etc. | `dictrec(..., type, ...)` |
| **align** | `''` / `L` / `R` / `T` | `dictrec` arg, or helper (`dict_text`→T, `dict_date`/`dict_number`→L/R by group) |
| **lowercase** | unset / `true` / `false` | `dict_text`→true if unset; `dict_code`→false; date/time sometimes true; default paint for S/SELECT→true |
| **length** | empty / N | `dictrec` arg, helper, or after-call `di.length=` |
| **readonly** | unset / true / msg string | dict / security |
| **rows** | 1+ | `dict_text` / `dict_textarea` |
| **popup / link** | omit / non-empty / `''` / `false`/`null` | pad vs icon vs suppress |
| **conversion** | e.g. `[DATE…]`, `[NUMBER…]` | helpers |
| **radio / checkbox** | option string | helpers / manual |
| **required, groupno, fieldno, …** | various | validation / grid |

Helpers that matter for layout:

| Helper | align | lowercase | length | Notes |
|--------|-------|-----------|--------|-------|
| `exo_dict_text` | **T** | true if unset | clear unless arg | free-text intent |
| `exo_dict_code` | T via text | **false** | default 30 | code intent |
| `exo_dict_textarea` | T | true if unset | keep or set | stays TEXTAREA |
| `exo_dict_date` | L or R (group) | optional | 10 | calendar popup; same group rule as number |
| `exo_dict_number` | L or R (group) | — | — | conversion |
| `exo_dict_time` | L | — | 5 | |

HTM starts as `<input datafld>` or `<span datafld>` or `<textarea>` / `<select>`.

## 2. Paint pipeline (order that multiplies axes)

Rough order in `formfunctions_onload`:

1. **INPUT + align T → replace with SPAN** (before most styling)
2. **copydictitem** → `exo*` attributes (`validpropnames`; no dual) (`false`/`null` → `''` except popup/link removed)
3. radio/checkbox expand
4. **SPAN white-space** from `dictitem.lowercase === false` → nowrap vs pre-wrap
5. **Free-text soft max** if SPAN + align T + lowercase ≠ false
6. **contenteditable** if type F SPAN + !readonly
7. type S → tabIndex -1; F7/F6 chrome; glue S after chrome
8. textAlign from align
9. INPUT length → size; TEXTAREA cols/rows
10. default lowercase for S/SELECT if missing; uppercase style if not lowercase
11. **class**: editable F → `clsNotRequired` / `clsRequired`; else → **`clsReadOnly`**
12. **`form_apply_input_field_width`** — **INPUT only** (not SPAN)

Then wide layout (separate) — full AIM/method: **[FORM-UI-WIDE-NARROW.md](./FORM-UI-WIDE-NARROW.md)**:

- `exomaxwidth` → max 30ch when wide (`form_table_apply_freetext_wide_max`)
- CSS: wide `td:has(> contenteditable | clsReadOnly | align=T)` → `min-width: max-content`
- Narrow: free-text folds under form soft ceiling (`max-width: 100%` on span; **not** 30ch)

## 3. DOM result axes (what you can observe)

| DOM / style | Meaning |
|-------------|---------|
| tag | INPUT / SPAN / TEXTAREA / SELECT / radio / checkbox |
| `exotype` | F / S / … |
| `exoalign` | L / R / T / … |
| `exolowercase` | `true` / `false` / `''` |
| `exolength` | empty or N |
| `exoreadonly` | present or not |
| `contenteditable` | true only entry free-text (and similar) |
| class | `clsRequired` / `clsNotRequired` / **`clsReadOnly`** |
| `exomaxwidth` | `30ch` or absent |
| display / width | block+100% (entry free-text) vs inline-block (display free-text) vs INPUT fixed px |
| white-space | pre-wrap (fold) vs nowrap (code) |
| field row | `.exofieldrow` = `.exofieldchrome` (F7/F6/pad only) + host |
| wide host | cell may get `min-width: max-content` |

**Important mismatch:** “display” is not one flag. It is roughly  
`readonly` **or** `type S` **or** not contenteditable **or** `clsReadOnly` — four overlapping signals.

## 4. Real field kinds (product intent × current DOM)

Grouped by **intent**; sub-rows = combinations that still behave differently.

### A. Entry free-text (flowing, editable)

| Dict | DOM end state | Width |
|------|---------------|--------|
| type F, `dict_text` (or align T + lowercase true), !readonly, empty length | INPUT→SPAN, contenteditable, pre-wrap, **block 100%**, `exomaxwidth=30ch`, clsNotRequired/Required | narrow 100%; wide soft max 30ch |
| same + length N | same but **no** exomaxwidth | soft max off; length is hint only on SPAN |

### B. Display free-text (flowing, not editable)

| Dict | DOM | Width |
|------|-----|--------|
| type F, align T free-text, **readonly** | SPAN, **no** contenteditable, pre-wrap, **inline-block**, exomaxwidth if empty length, **clsReadOnly** | soft max like A if empty length |
| type **S**, `dict_text` (names) | SPAN S, tabIndex -1, same free-text soft max path, clsReadOnly, often HTML display | same |
| type F free-text, no dict_text (bare F) | may stay non-T or incomplete free-text | **not** free-text soft max |

### C. Codes (align T, no fold)

| Dict | DOM | Width |
|------|-----|--------|
| `dict_code` or align T + lowercase **false**, often length 30 | SPAN if started INPUT+T, **nowrap**, no exomaxwidth (lowercase false), often uppercase | not free-text soft max; no INPUT length paint on SPAN |
| entry vs readonly code | contenteditable or not; clsNotRequired vs clsReadOnly | same fold=off |

### D. Fixed INPUT (not align T)

| Dict | DOM | Width |
|------|-----|--------|
| type F, align L/R or blank, length N, !readonly | stays **INPUT**, size=N, **form_apply_input_field_width** → width=min=max px | fixed glyph×N |
| same + readonly | INPUT / clsReadOnly, tab -1 | fixed if length set |
| no length | INPUT, no fixed paint | content / browser default |

### E. Dates / times / periods

| Dict | DOM | Width |
|------|-----|--------|
| `dict_date` etc., align L, length, popup calendar | usually INPUT (not T), fixed length paint, F7 calendar | fixed ~length |

### F. Numbers

| Dict | DOM | Width |
|------|-----|--------|
| `dict_number`, align L or R, conversion NUMBER | INPUT or span display; not free-text soft max | INPUT length path if length set; else content |

### G. Multi-line free text

| Dict | DOM | Width |
|------|-----|--------|
| `dict_textarea` | **TEXTAREA** (not SPAN soft max), align T, lowercase true | cols from length/size; separate from exomaxwidth |

### H. SELECT / radio / checkbox

| Dict | DOM | Width |
|------|-----|--------|
| SELECT | SELECT, default lowercase true | own layout; F7 wrap hug |
| radio/checkbox | expanded inputs + labels | not free-text path |

### I. Type S non-text (calcs, HTML)

| Dict | DOM | Width |
|------|-----|--------|
| type S, often functioncode, may dict_text or not | SPAN, dataFormatAs HTML, tab -1, clsReadOnly | free-text soft max **only if** align T + not code |

### J. Chrome (orthogonal to text kind)

| Dict popup/link | DOM |
|-----------------|-----|
| non-empty | `.exofieldrow` (`.exofieldrow-fill` if entry free-text F) = `.exofieldchrome` (icon) + host |
| `''` | pad slot in the chrome strip |
| false/null | attribute removed — no icon, no pad |
| omit | nothing |

Glue: type **S** after code row moves into `.exofieldrow` unless `<br>`.

**Icon ↔ host is not field navigation.** Localised to F6/F7 **click** only:

| Direction | What happens |
|-----------|----------------|
| **Click icon → host** | `form_field_from_chrome_icon`: `data-exo-host` = logical field name → same TR `exoid_*` or `$$`. Callers: `exoui_popup`, `exoui_link`. |
| **Host → icon** | None. Icons are not focus stops. |
| **Keyboard F6 / F7** | Focused **host** only. Icon DOM is not read. |

## 5. Combination matrix that actually drives **width**

Width is not “category X”. It is roughly:

```
IF tag==SPAN AND align==T AND lowercase!==false:
  → free-text branch
     IF empty length → exomaxwidth
     IF type F AND !readonly → entry fill (block 100%)
     ELSE → display (inline-block)
ELSE IF tag==INPUT AND length>0:
  → fixed px width
ELSE:
  → other / content

PLUS when wide:
  apply max-width from exomaxwidth on SPAN[exomaxwidth]
  AND td:has(contenteditable|clsReadOnly|align=T) min-width max-content
```

So the same product idea (“long name”) can hit different paths if any of these differ: type F/S, readonly, align, lowercase, length, started as INPUT vs SPAN, class clsReadOnly, contenteditable.

## 6. Theory categories vs practice signals

| Clean category (theory) | Today inferred from (practice) |
|-------------------------|--------------------------------|
| **Code** | align T + lowercase false (+ often length) |
| **Entry free-text** | align T + not code + F + !readonly + contenteditable |
| **Display free-text** | align T + not code + (readonly F **or** S) + no contenteditable + often clsReadOnly |
| **Fixed INPUT** | INPUT + length + not converted to SPAN |
| **Textarea** | TEXTAREA + dict_textarea |
| **Choice** | radio/checkbox/select |

Display free-text is **three** mechanisms (readonly, type S, clsReadOnly) that mostly agree but not always.

## 7. Certify example (why it was confusing)

| Field | Dict | Category | Why it failed when soft max was entry-only |
|-------|------|----------|---------------------------------------------|
| VEHICLE_NAME | S + dict_text | display free-text | soft max only on editable F |
| BRAND_NAME | S + dict_text | display free-text | same |
| SPEC | F readonly, no dict_text (until fixed) | not free-text | no align-T free-text path |
| AD_MATERIAL | F + dict_text + readonly | display free-text | same exclusion |
| CERTIFICATE_NO | F + dict_text + length 20 | free-text with length → no exomaxwidth | different width story |

## 8. Practice summary

- **Dict** has a few real knobs: type, align, lowercase, length, readonly, rows, popup/link, conversion.
- **DOM** multiplies them: INPUT vs SPAN conversion, contenteditable, three class roles, exomaxwidth, white-space, fill vs inline-block, chrome wrap, wide host CSS.
- **Product kinds** are few (~6–8); **behavioural combinations** are many because paint keys off **intersections**, not a single category id.

### Free-text soft max (current rule of thumb)

Full story: **[FORM-UI-WIDE-NARROW.md](./FORM-UI-WIDE-NARROW.md)**.

| | Rule |
|---|------|
| Who | free-text = exostyle text / align T and not code |
| Empty length | attribute `exomaxwidth=30ch` (candidate only) |
| Narrow | **style** max-width **100%** — fold under soft ceiling; **never** lock to 30ch |
| Wide | style max-width from `exomaxwidth` on all `SPAN[exomaxwidth]` |
| Entry | type F + !readonly → contenteditable, fill cell |
| Display | readonly F or type S → no contenteditable, inline-block + same soft max |
| Codes | nowrap, out of soft max |

Wide host `min-width: max-content` on contenteditable / clsReadOnly / align T cells is a **separate** axis from soft max on the span; do not conflate the two without an explicit decision.
