# Exodus Web Framework Programmer's Manual

**Location:** `service/www/exodus/doc/` (web programming docs)  
**Runtime code:** `service/www/3/exodus/scripts/` (`client.js`, `dbform.js`, …)  
**See also:** [FORM-UI-PHILOSOPHY.md](./FORM-UI-PHILOSOPHY.md), [FORM-UI-TYPES.md](./FORM-UI-TYPES.md), [FORM-UI-NUMBER.md](./FORM-UI-NUMBER.md), [FORM-UI-KEYBOARD-BROWSER.md](./FORM-UI-KEYBOARD-BROWSER.md)  
**Not here:** C++/var/exoprog — repo-root `doc/`

This document describes how to use the client-side JavaScript web framework in the Exodus system (version 3 UI). The framework provides modal dialogs, an `async`/`await` cooperative event model, database access, dictionary-driven forms, security, uploads, and more. It is designed to work with the Exodus backend via the `xhttp.php` bridge (or file mode for local testing).

The framework originated in an era of older browsers and cooperative generators; it now uses native `async`/`await` with a single exclusive event gate. It follows Revelation/Pick multivalue conventions with special delimiter characters.

**Important design principles (per current implementation):**
- Security ultimately rests on the backend + a valid PHP session once established.
- Custom "tokens" (`database*username*`) are **namespace keys** for supporting multiple concurrent logins/users/databases within one PHP session cookie. They are **not** an authentication gate by themselves.
- Submitted paths/filenames (the "internal path") are generally not treated as confidential.
- Filename lengths are limited (currently 128 characters in upload handling).
- Server-side scripts enforce HTTPS for production sites.

## 1. Overview and Architecture

- **Core file:** `client.js` — Must be included **first**. Core globals, Gate A/B, `exodblink`, `exoui_showmodaldialog`, security, cookies, utilities, string/array prototypes.
- **Form automation:** `dbform.js` + helpers in `db.js` — Dictionary-driven (`dict_*`) CRUD forms, MV groups, validation, buttons.
- **Communication bridge:** `xhttp.php` (and .asp variants). Client sends XML (`<token>`, `<request>`, `<data>`); bridge writes temp files; backend processes and responds via `.1`/`.2`/`.3` files.
- **Async model (Gate A):** Event handlers and deferred async work enter via `exodus_begin` (exclusive owner; `g_exodus_flow_queue_max = 0` means **no queuing** — second start while airborne is a **visible `systemerror`**, not a silent skip). Prefer `async function myfunc() { ... await someOperation() ... }` and `await` inside the same flight. Do **not** start free-floating async that does DB/UI work. Avoid parallel starts at the source (check `g_exodus_flow` / use `exodus_begin_if_idle` for optional work).
- **Wait/Cancel (Gate B):** Only intentional second stack — in-DOM Wait/Cancel on the modal blocker while Gate A is blocked on `db.send` XHR. No main-line form dbio from Gate B.
- **Public commencement API (only three):** `exodus_begin` (business), `exodus_begin_if_idle` (background), `exodus_begin_waitcancel` (Wait/Cancel). No fourth entry path. `startAsyncFlow` is a deprecated alias of `exodus_begin`.
- **Flight log:** Quiet by default. Enable with `?logflights=1` or `glogflights=true` in the console (`[exodus flight] TAKEOFF|LANDING|SKIP|…`).
- **No generators:** `function*` / `yield*` are not supported. Use `async`/`await` only. Accidental generators fail with `systemerror` (stage 6).
- **UI conventions:** Modal dialogs, `class="exodusform"` tables, input `id`s matching dictionary codes, heavy use of `gparameters`.
- **Data delimiters:** `rm`, `fm`, `vm`, `sm`, `tm`, `stm` (and their regex versions).
- **Security model:** PHP sessions (the real auth) + namespaced tokens. `exoui_security('TASK')`. Once a valid session exists, the web layer trusts it.
- **Globals:** `gusername`, `gdataset`, `gparameters`, `greadonlymode`, many others set from cookies or passed in.

The scripts support both browser HTTP mode and `file://` local mode (for development/testing).

## 2. Including the Framework

### Recommended script ordering in `<head>`

```html
<!DOCTYPE html>
<html>
<head>

  <!-- 1. Configuration (before client.js) -->
  <script type="text/javascript">
    // For persisted data forms
    var gdatafilename = 'YOURDICTNAME';   // dict_ function will be dict_YOURDICTNAME
    // var gmodule = 'YOURMODULE';        // for module-style (program) processing instead of direct file save

    // Optional: EXODUSlocation if scripts are not in the expected relative place
    // if (typeof EXODUSlocation == 'undefined') EXODUSlocation = '../exodus/';
  </script>

  <!-- 2. Core framework (MUST be first real script include) -->
  <script type="text/javascript" src="../exodus/scripts/client.js"></script>

  <!-- 3. Form support (for data entry screens) -->
  <script type="text/javascript" src="../exodus/scripts/dbform.js"></script>

  <!-- 4. Common utilities (dates, amounts, prototypes, etc.) -->
  <script type="text/javascript" src="../exodus/scripts/exodus.js"></script>

  <!-- 5. Dictionary definition (MUST come after client.js) -->
  <script type="text/javascript">
    function dict_YOURDICTNAME() {
      var dict = [];
      var din = -1;
      var di;
      var groupn = 0;

      // Key field (only for file forms using gdatafilename)
      di = dict[++din] = dictrec('ID', 'F', 0);

      di = dict[++din] = dictrec('NAME', 'F', 1, 'Full Name');

      // Multi-value group
      ++groupn;
      di = dict[++din] = dictrec('LINES', 'F', 10, 'Line Items', groupn);
      di = dict[++din] = dictrec('QTY', 'F', 11, '', groupn);

      return dict;
    }
  </script>

  <title>My Form</title>
</head>
<body>
  ...
  <input id="ID" />
  <input id="NAME" />
  ...
</body>
</html>
```

**Order is critical:**
1. Config globals.
2. `client.js`.
3. Dictionary definition script.
4. (Optional) Other framework or app scripts.

Common additional includes (from examples):
- `../general/scripts/general.js`
- Various `_dict.js` files for specific modules.

## 3. Async/await, Gate A, and Dialogs

### Basic usage

```js
async function myFunction() {
  var ok = await exoui_okcancel('Are you sure?', 1);
  if (!ok) return;

  if (!(await db.send())) {
    return await exoui_invalid(db.response);
  }

  var choice = await exoui_decide('Select', dataArray);
}
```

### Sync shell vs async flight (house rules)

Two kinds of functions — do not mix their jobs:

| Kind | Naming | May do | Must not do |
|------|--------|--------|-------------|
| **Sync shell** | Prefer `*_sync` for DOM attribute targets (`onclick=`, `onload=`, `onblur=`) | Pure DOM paint, `exodus_begin…` kickoff, cancel event | `await`, `db.send`, dialogs, `gds.setx`, any real work |
| **Async flight** | `async function …` (entered only via Gate A) | `await` confirm / `db.send` / `gds.setx` / form hooks | Free-run outside Gate A; start a second flight without `exodus_begin` |

```js
// Sync shell — HTML may call this; only kicks Gate A
function save_onclick_sync() {
  void exodus_begin(save_onclick, 'save_onclick')
}

// Async flight — all real work lives here
async function save_onclick() {
  if (!(await savedoc())) return false
  return true
}
```

- **`setvalue(el, v)`** — sync DOM paint only (no conversion/validation).
- **`await gds.setx(id, recn, v)`** — data store + conversion + validation (async; must `await` inside a flight).
- Bare call of an async function **without** `await` is a bug (you get a Promise, not a result).
- Nested `await` inside one flight is correct; a second *commencement* while busy is a **bug signal** (`queue_max = 0` → `systemerror` dialog). Do not treat that dialog as something to silence in Gate A — stop initiating the second start.

DOM events, HTM `*_sync` bridges, and deferred work all enter **Gate A** (`exodus_begin`). Nested `await` stays on that one flight. `g_exodus_flow_queue_max` is wait-list capacity only: **0 = no queuing** (current); raise later if multi-flight wait is wanted.

**Parallel starts:** avoid at the source. Capture/sync code outside a flight must not call `exodus_begin` while `g_exodus_flow` is set (return/ignore, or use `exodus_begin_if_idle` / `exodus_begin_when_idle`). Gate A does **not** quietly drop conflicting takeoffs — races surface as error messages on purpose so bad call sites get fixed.

Optional background work (session keepalive, relock) uses `exodus_begin_if_idle` — **skip** if busy (never uses the queue).

### No `function*` / `yield*`

Generators are **removed** (stage 6). Framework and app modules use `async`/`await` only. Form hooks (`form_predeleterow`, etc.) and dict `functioncode` must be async or sync — not generators.

**Do not** use free-running `setTimeout(async () => …)` or rely on `exosettimeout('await myfunc()')` as a second event system. Prefer `await myfunc()` inside the current handler. If you must defer after the current flight (e.g. next `opendoc`, popup → openrecord), use:

```js
exodus_begin_when_idle(myfunc, 'label', { delay_ms: 10 })
```

That retries until Gate A is free, then `exodus_begin`. If still busy after 30s → **systemerror** (visible). Do not one-shot `setTimeout` → `exodus_begin` (with `queue_max = 0` that used to **silently skip**).

Required `exodus_begin` while another flight is airborne (and the wait list is full) also **systemerror**s. Optional background work uses `exodus_begin_if_idle` (silent skip is intentional).

**`form_postread` vs `form_postdisplay`:** `opendoc2` runs `form_postread` → `gds.load` → `form_postdisplay`. Anything that needs bound DOM rows (`form_filter`, per-row `exosetreadonly`, signature/logo images) belongs in `form_postdisplay`, not same-flight `await` from `form_postread`.

### Modal Dialogs

```js
// Open
var params = {
  key: theKey,
  readonly: true,
  mycustom: 'value'
};
var returnedValue = await exoui_showmodaldialog('somedialog.htm', params);

// In the dialog page, read:
if (gparameters.key) { ... }

// Close and return value(s) to caller
exoui_windowclose('a result string');
exoui_windowclose(['multiple', 'values']);
```

Key functions:
- `exoui_showmodaldialog(url, arguments)`
- `exoui_windowclose(returnValue)`
- `exodus_setchildwin_returnvalue(...)`

Dialogs are the primary navigation/composition mechanism.

## 4. Database Access (`exodblink` / XMLHTTP)

`client.js` provides `exodblink`: browser pages set `db.request` (and optional `db.data`), then `await db.send()`. Transport is XMLHTTP to `scripts/xhttp.php` in normal browser mode (file mode is legacy).

**Field separators are the real CR character** (ASCII 13). In JS source that is the two-character escape `\r` inside a string literal:

```js
var db = new exodblink();   // auto-picks XMLHTTP or file mode

// Real CR between fields — not the two characters backslash + "r"
db.request = 'EXECUTE\rGENERAL\rMYCOMMAND\rparam1\rparam2';
db.data    = 'optional payload (multivalue ok)';

if (!(await db.send())) {
  return await exoui_invalid(db.response);
}

var resultData = db.data;
var message    = db.response;
```

HTML attributes and text inputs do **not** interpret `\r`. A value like `SELECT\rCURRENCIES\r…` typed in a box is literal backslash-r and arrives as one unknown request unless you convert first, e.g. `cmd.replace(/\\r/g, '\r')` (see `test.htm` custom command). Prefer building requests in JS with real `\r` escapes, as the rest of the framework does.

Typical request shapes (same CR separators throughout):

| Shape | Example use |
|--------|-------------|
| `SELECT\rfile\rsortselect\rcollist\rXML` | Dropdowns, app lists (`general.js` currencies, etc.) |
| `SELECT\rfile\rsortselect\rcollist\rXML\rmaxnrecs` | Optional 6th field = max records |
| `CACHE\rSELECT\rfile\rsort\rcollist\rXML\rmaxnrecs` | `exofilepopup` (CACHE prefix is stripped server-side) |
| `SELECT\rfile\r\rRECORD` | Read records by key list in `db.data` |
| `EXECUTE\rmodule\rcommand\r…` | Backend programs |
| `READ\rfile\rkey` / `CACHE\rREAD\r…` | Single record |
| `LOGIN\r…`, `TEST`, `LOOPBACK`, `KEEPALIVE` | Session / probes |

On success `db.send()` is truthy; `db.data` holds the body (often XML for SELECT), `db.response` holds the status/message. Login / logout helpers exist on the dblink object.

### Every `db.send` must have a failure path

**Rule:** no request is success-only. Callers handle `!(await db.send())` (or equivalent): `exoui_invalid`, typeahead miss/hide, `return false`, etc. “This is only typeahead / quiet / background — ignore fail” is a bug.

**Session lost (`Please login`)** is one failure mode handled **inside** dblink for **all** sends (main `db` and typeahead private/`quiet` link) — same path:

1. `send` calls `login()` still inside the send loop; on success the **same request is retried**.
2. **Resume login as {user}?** (`exoui_yesno`, default Yes):
   - **Yes** — cookie auto-login if remembered, else **modal** `index.html`; then retry.
   - **No** — full-page `index.html`, return false (async unwind; no `force_an_exit` throw under evaluate). Post-login start is index logic (often `users.htm`), not return-to-form.
3. Typeahead `quiet` = no modal **shield** per keystroke only — **not** skip reauth or skip handling `!send` after fail.

**Typeahead parallel dblink:** uniquely uses a **private** `exodblink` (`form_typeahead_dblink` / `gform_typeahead_db`) so client-side main `db`/`gds` is not blocked by keystroke I/O. It still shares the **same PHP session cookie**. PHP would serialize concurrent requests on that session unless `xhttp.php` releases the lock early (`session_write_close()` before the long `.1` poll for non-LOGIN). So typeahead and main-channel traffic are **queued/interlaced on the server** under one session identity — parallel client channels, not a separate login. Reauth and failure paths stay universal.

**Hard case (one Gate A):** Resume confirm / modal login are interactive UI **nested under** `await db.send` (and often typeahead → evaluate), with automatic retry. Confirm/modal depth and orphan force-cancel looking like No are real risks. Prefer fixing call sites and failure paths over a second gate or silent swallow.

Code: `client.js` — `exodblink_login`, both send transports’ `Please login` branches; typeahead `form_typeahead_dblink()`; `xhttp.php` session release.

Cookies are used heavily for dataset, username, globals (`exogetcookie2`, `exosetcookie`).

## 5. Dictionary-Driven Forms (The Main Pattern)

### Dictionary Definition

Return an array of `dictrec` objects. Field IDs in HTML must match the codes.

```js
function dict_MYFILE() {
  var dict = [];
  var din = -1;
  var di;

  di = dict[++din] = dictrec('ID', 'F', 0);                    // key
  di = dict[++din] = dictrec('NAME', 'F', 1, 'Full Name', '', '', '', '', '', '', 40);

  var group = 1;
  di = dict[++din] = dictrec('ITEMS', 'F', 10, 'Items', group);
  di = dict[++din] = dictrec('QTY',   'F', 11, '', group);

  return dict;
}
```

`dictrec(code, type, fieldno, title, group, keypart, x, conversion, functioncode, align, length)`

Many helpers exist in `db.js`:
- `exo_dict_text(di, length)`
- `exo_dict_date(di)`
- `exo_dict_amount(di)`
- `exo_dict_emailaddress(di)`
- `exo_dict_url(di, protocol)`
- Validation functions (`exo_val_*`) that you can assign to `di.validation = 'await myfunc()'`

You can attach:
- `di.validation`
- `di.conversion` (code;description pairs)
- `di.checkbox`
- `di.popup`
- `di.required`, `di.readonly`, `di.length`, `di.maxlength`, `di.validcharacters`, etc.

### Form HTML

Use plain inputs with matching `id`s. The framework wires everything (validation on change, MV handling, buttons in `#formbuttons`, save logic, etc.).

See `empty_dataform.htm`, `empty_fileform.htm`, and `template.htm` for complete minimal examples.

### File Forms vs Module Forms

- **File form**: Use `gdatafilename`. Include a field 0 (key). Saves directly to the file.
- **Module form**: Use `gmodule = 'PROGRAMNAME'`. Usually no field 0; the backend program processes the data.

## 6. Security

```js
if (!(await exoui_security('EDIT CUSTOMERS'))) {
  return await exoui_invalid(gmsg);
}
```

Tasks come from the AUTHORISATION file. Many backend `EXECUTE GENERAL` calls also enforce authorisation.

Login flow populates the PHP session (via `xhttp.php`) and cookies. The web layer then trusts the presence of `_username` (and related) entries.

**Note (per current design):** The custom tokens are namespace identifiers, not security tokens. Authentication is carried by the PHP session.

## 7. File Uploads

```js
var params = {
  database: gdataset,
  filename: 'JOBS',
  key: recordKey,
  versionno: version,
  allowablefileextensions: 'jpg,png,pdf',
  minheight: 50,
  maxheight: 200
};

var returnedTargetFilename = await exoui_showmodaldialog('../exodus/upload.htm', params);
```

The upload posts to `upload.php` (which now safely derives its own redirect target). The result is the target filename that was written.

See `upload.js`, `upload.htm`, and `upload2.htm`.

Current handling includes a 128-character limit on the combined directory+filename.

## 8. Common Utilities and Prototypes

Many convenience methods are added to `String` and `Array` (primarily in `exodus.js` and `client.js`):

```js
str.exofield(delim, n)
str.exoconvert(fromChars, toChars)
str.exocapitalise()
str.exotrim([char])
str.exoquote()
str.exoxmlquote()
arr.exolocate(item)
arr.exounique()
arr.exosum()
arr.exojoin(...)
arr.exotrim()
... many more (xlate, sum, invert, etc.)
```

See `exodus.js` and searches for `exodus` prototype methods.

Other frequent utilities:
- `await exoui_invalid(msg)`
- `await exoui_note(msg)`
- `await exoui_warning(msg)`
- `await exoui_okcancel(msg, default)`
- `await exoui_decide(question, data, ...)`
- `exoui_windowclose(value)`
- `exodus_begin(asyncFn, 'label')` — **public #1** Gate A business (`systemerror` if busy when `queue_max` is 0)
- `exodus_begin_if_idle(asyncFn, 'label')` — **public #2** optional background; skip if busy
- `exodus_begin_waitcancel(source)` — **public #3** Gate B Wait/Cancel only (from uiblocker)
- `$$('id')` or `$$('classname')` — element lookup

## 9. Sessions, Login, and the Token Model

- Real authentication is via PHP sessions + backend login.
- The custom token (`database*username*`) is a **namespace** inside the session so one cookie can hold state for multiple users/databases.
- `xhttp.php` handles the actual LOGIN (sends credentials to backend, only creates the session entry on success).
- `upload.php` and other pages check for the presence of `_username` entries.

Once a valid session exists, the web framework treats the user as authenticated for that namespace.

## 10. Error Handling and User Messages

Preferred pattern:

```js
if (!(await someOperation())) {
  return await exoui_invalid(db.response || 'Something went wrong');
}
```

Helpers:
- `exoui_invalid(msg)` — usually stops and shows error
- `exoui_note(msg)`
- `exoui_warning(msg)`

## 11. Best Practices & Gotchas

- **Order matters** — config → client.js → dict definition → HTML.
- Use `await` for anything that waits (dialogs, `db.send`, form hooks) **inside** Gate A.
- Prefer `gparameters` over URL query strings when opening dialogs.
- Match input `id`s exactly to dictionary codes.
- Field 0 is special (key).
- Use groups in the dictionary for multivalue sets.
- **MV row clone:** `cloneNode` keeps attributes, not `addEventListener` handlers. Prefer document/table **delegation** (or attributes) over per-element listeners on group fields — see [FORM-UI-PHILOSOPHY.md](./FORM-UI-PHILOSOPHY.md) § Multivalue rows.
- The framework does a lot of auto-wiring — follow the template patterns.
- 128 character combined path+filename limit (enforced in upload handling).
- Internal paths/filenames passed around are generally not considered confidential.
- Debug code remains in the tree (no separate prod build). Use with care.
- Test both HTTP and `file://` modes if doing local development.
- For module processing vs direct file save, understand `gmodule` vs `gdatafilename`.

## 12. Key Files Quick Reference

| File                  | Purpose                                      |
|-----------------------|----------------------------------------------|
| `client.js`           | Core framework, Gate A/B, dblink, dialogs, security, utilities |
| `dbform.js`           | Form lifecycle, MV handling, save/load, buttons |
| `db.js`               | dictrec() + many dict_ and val_ helpers     |
| `exodus.js`           | Dates, amounts, string/array prototypes     |
| `login.js`            | Login UI                                    |
| `upload.js` / `upload.htm` / `upload2.htm` | File attachment support |
| `xhttp.php`           | HTTP bridge to backend                      |
| `server.js` / `server0.js` | Server/file-mode helpers                 |
| `authorisation*.js`, `users*.js` | Security and user admin screens        |

**Excellent starting templates:**
- `empty_dataform.htm`
- `empty_fileform.htm`
- `template.htm`

Study real forms (`users.htm`, `authorisation.htm`, `codepage.htm`, etc.) for patterns.

---

This manual is a living draft based on the code, templates, and usage patterns in the `scripts` directory. For the absolute latest behavior, refer to the source and the empty templates.

If you need expansions (more examples for MV groups, gds, printing, specific validators, backend EXECUTE command list, etc.), let me know what sections to flesh out.