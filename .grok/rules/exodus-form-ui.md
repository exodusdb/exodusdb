# Exodus form UI

When the task touches form layout, CSS, borders, width, tabs, repeating groups, or embedded tables:

1. Read `service/www/3/exodus/FORM-UI-PHILOSOPHY.md` before proposing changes.
2. See also `AGENTS.md` at the repo root.

Do not use `tr { border-bottom }` on exodusform pages, global `!important` width hacks, or HTM `<input>` styles that dbform discards (flowing fields become `<span>`).