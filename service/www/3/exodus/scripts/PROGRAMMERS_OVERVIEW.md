# Exodus Web Framework Programmer's Manual

**Location:** `exodus/service/www/3/exodus/scripts/`

This document describes how to use the client-side JavaScript web framework in the Exodus system (version 3 UI). The framework provides modal dialogs, a generator-based asynchronous model (`yield*`), database access, dictionary-driven forms, security, uploads, and more. It is designed to work with the Exodus backend via the `xhttp.php` bridge (or file mode for local testing).

The framework originated in an era of older browsers (IE6+, etc.) and uses cooperative multitasking via JavaScript generators (pre-dating native async/await). It follows Revelation/Pick multivalue conventions with special delimiter characters.

**Important design principles (per current implementation):**
- Security ultimately rests on the backend + a valid PHP session once established.
- Custom "tokens" (`database*username*`) are **namespace keys** for supporting multiple concurrent logins/users/databases within one PHP session cookie. They are **not** an authentication gate by themselves.
- Submitted paths/filenames (the "internal path") are generally not treated as confidential.
- Filename lengths are limited (currently 128 characters in upload handling).
- Server-side scripts enforce HTTPS for production sites.

## 1. Overview and Architecture

- **Core file:** `client.js` — Must be included **first**. Core globals, yield infrastructure, `exodusdblink`, `exodusshowmodaldialog`, security, cookies, utilities, string/array prototypes.
- **Form automation:** `dbform.js` + helpers in `db.js` — Dictionary-driven (`dict_*`) CRUD forms, MV groups, validation, buttons.
- **Communication bridge:** `xhttp.php` (and .asp variants). Client sends XML (`<token>`, `<request>`, `<data>`); bridge writes temp files; backend processes and responds via `.1`/`.2`/`.3` files.
- **Async model:** `function* myfunc() { ... yield* someOperation() ... }`. `yield*` pauses execution until the operation (dialog close, DB response) resumes it via the internal event/yield machinery (`exodus_yield`, `exodus_resume`, etc.).
- **UI conventions:** Modal dialogs, `class="exodusform"` tables, input `id`s matching dictionary codes, heavy use of `gparameters`.
- **Data delimiters:** `rm`, `fm`, `vm`, `sm`, `tm`, `stm` (and their regex versions).
- **Security model:** PHP sessions (the real auth) + namespaced tokens. `exodussecurity('TASK')`. Once a valid session exists, the web layer trusts it.
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
    function* dict_YOURDICTNAME() {
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

## 3. The Yield* Pattern and Dialogs

### Basic usage

```js
function* myFunction() {
  var ok = yield* exodusokcancel('Are you sure?', 1);
  if (!ok) return;

  if (!(yield* db.send())) {
    return yield* exodusinvalid(db.response);
  }

  var choice = yield* exodusdecide('Select', dataArray);
}
```

`yield*` pauses the generator. The framework resumes it when the operation completes (dialog closes, DB response arrives, etc.).

### Modal Dialogs

```js
// Open
var params = {
  key: theKey,
  readonly: true,
  mycustom: 'value'
};
var returnedValue = yield* exodusshowmodaldialog('somedialog.htm', params);

// In the dialog page, read:
if (gparameters.key) { ... }

// Close and return value(s) to caller
exoduswindowclose('a result string');
exoduswindowclose(['multiple', 'values']);
```

Key functions:
- `exodusshowmodaldialog(url, arguments)`
- `exoduswindowclose(returnValue)`
- `exodus_setchildwin_returnvalue(...)`

Dialogs are the primary navigation/composition mechanism.

## 4. Database Access

```js
var db = new exodusdblink();   // auto-picks XMLHTTP or file mode

db.request = 'EXECUTE\rGENERAL\rMYCOMMAND\rparam1\rparam2';
db.data    = 'optional payload (multivalue ok)';

if (!(yield* db.send())) {
  return yield* exodusinvalid(db.response);
}

var resultData = db.data;
var message    = db.response;
```

Common patterns seen in the code:
- `EXECUTE\rGENERAL\r...`
- Direct backend file operations (the backend decides based on the request string).

Login / logout helpers exist on the dblink object.

Cookies are used heavily for dataset, username, globals (`exodusgetcookie2`, `exodussetcookie`).

## 5. Dictionary-Driven Forms (The Main Pattern)

### Dictionary Definition

Return an array of `dictrec` objects. Field IDs in HTML must match the codes.

```js
function* dict_MYFILE() {
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
- `exodus_dict_text(di, length)`
- `exodus_dict_date(di)`
- `exodus_dict_amount(di)`
- `exodus_dict_emailaddress(di)`
- `exodus_dict_url(di, protocol)`
- Validation functions (`exodus_val_*`) that you can assign to `di.validation = 'yield* myfunc()'`

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
if (!(yield* exodussecurity('EDIT CUSTOMERS'))) {
  return yield* exodusinvalid(gmsg);
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

var returnedTargetFilename = yield* exodusshowmodaldialog('../exodus/upload.htm', params);
```

The upload posts to `upload.php` (which now safely derives its own redirect target). The result is the target filename that was written.

See `upload.js`, `upload.htm`, and `upload2.htm`.

Current handling includes a 128-character limit on the combined directory+filename.

## 8. Common Utilities and Prototypes

Many convenience methods are added to `String` and `Array` (primarily in `exodus.js` and `client.js`):

```js
str.exodusfield(delim, n)
str.exodusconvert(fromChars, toChars)
str.exoduscapitalise()
str.exodustrim([char])
str.exodusquote()
str.exodusxmlquote()
arr.exoduslocate(item)
arr.exodusunique()
arr.exodussum()
arr.exodusjoin(...)
arr.exodustrim()
... many more (xlate, sum, invert, etc.)
```

See `exodus.js` and searches for `exodus` prototype methods.

Other frequent utilities:
- `yield* exodusinvalid(msg)`
- `yield* exodusnote(msg)`
- `yield* exoduswarning(msg)`
- `yield* exodusokcancel(msg, default)`
- `yield* exodusdecide(question, data, ...)`
- `exoduswindowclose(value)`
- `exodussettimeout('yield* myfunc()', ms)`
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
if (!(yield* someOperation())) {
  return yield* exodusinvalid(db.response || 'Something went wrong');
}
```

Helpers:
- `exodusinvalid(msg)` — usually stops and shows error
- `exodusnote(msg)`
- `exoduswarning(msg)`

## 11. Best Practices & Gotchas

- **Order matters** — config → client.js → dict definition → HTML.
- Use `yield*` for anything that yields.
- Prefer `gparameters` over URL query strings when opening dialogs.
- Match input `id`s exactly to dictionary codes.
- Field 0 is special (key).
- Use groups in the dictionary for multivalue sets.
- The framework does a lot of auto-wiring — follow the template patterns.
- 128 character combined path+filename limit (enforced in upload handling).
- Internal paths/filenames passed around are generally not considered confidential.
- Debug code remains in the tree (no separate prod build). Use with care.
- Test both HTTP and `file://` modes if doing local development.
- For module processing vs direct file save, understand `gmodule` vs `gdatafilename`.

## 12. Key Files Quick Reference

| File                  | Purpose                                      |
|-----------------------|----------------------------------------------|
| `client.js`           | Core framework, dblink, dialogs, yield, security, utilities |
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