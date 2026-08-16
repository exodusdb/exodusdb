# Exodus form UI

When the task touches form layout, CSS, borders, width, tabs, repeating groups, or embedded tables:

1. Read `service/www/exodus/doc/FORM-UI-PHILOSOPHY.md` before proposing changes.
2. Field kinds (theory + practice): `service/www/exodus/doc/FORM-UI-TYPES.md`.
3. **`[NUMBER…]` / `[DECIMAL…]` / `[INTEGER,0…]`:** `service/www/exodus/doc/FORM-UI-NUMBER.md`.
3. **Narrow vs extreme-wide (AIM + method):** `service/www/exodus/doc/FORM-UI-WIDE-NARROW.md` — free-text fold under soft ceiling; **no** nest `width:max-content`; **30ch only** with `.exoform-wide`.
4. Browser vs form keys: `service/www/exodus/doc/FORM-UI-KEYBOARD-BROWSER.md`.
5. See also `AGENTS.md` at the repo root (web docs under `service/www/exodus/doc/`; C++/var under repo `doc/`).

Do not use `tr { border-bottom }` on exoform pages, global `!important` width hacks, or HTM `<input>` styles that dbform discards (flowing fields become `<span>`).

**Repeated regression:** nested host `width:max-content` or free-text `style.maxWidth=30ch` on narrow forms — see FORM-UI-WIDE-NARROW.md.