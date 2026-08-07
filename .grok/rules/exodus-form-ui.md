# Exodus form UI

When the task touches form layout, CSS, borders, width, tabs, repeating groups, or embedded tables:

1. Read `service/www/exodus/doc/FORM-UI-PHILOSOPHY.md` before proposing changes.
2. Field kinds (theory + practice): `service/www/exodus/doc/FORM-UI-TYPES.md`.
3. Browser vs form keys: `service/www/exodus/doc/FORM-UI-KEYBOARD-BROWSER.md`.
4. See also `AGENTS.md` at the repo root (web docs under `service/www/exodus/doc/`; C++/var under repo `doc/`).

Do not use `tr { border-bottom }` on exodusform pages, global `!important` width hacks, or HTM `<input>` styles that dbform discards (flowing fields become `<span>`).