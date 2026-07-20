# Exodus — notes for AI assistants

## Form / web UI work (read first)

Before changing **form layout**, **CSS**, **borders**, **width**, **tabs**, **repeating groups**, or **embedded tables**, read:

**`service/www/3/exodus/FORM-UI-PHILOSOPHY.md`**

That file explains runtime DOM (dbform, panes, `td` vs `tr` borders, flowing spans, embedded `exodusgroupno` tables) and lists anti-patterns to avoid.

## Framework reference

| Topic | Location |
|-------|----------|
| Form UI philosophy | `service/www/3/exodus/FORM-UI-PHILOSOPHY.md` |
| JS API / architecture | `service/www/3/exodus/scripts/PROGRAMMERS_OVERVIEW.md` |
| Form CSS | `service/www/3/exodus/global.css` |
| Form automation | `service/www/3/exodus/scripts/dbform.js` |
| Pane wrap | `service/www/3/exodus/scripts/client.js` (`exoduswrapformpanes`) |
| Dev docs (HTML) | `service/www/exodus/doc/forms.htm` |

## Git / commits

Do **not** run `git commit` (or push) unless the user clearly asks — e.g. “commit”, “commit this”, “ok commit”. Approval of a **change** (“ok”, “looks good”, “go ahead”) is not commit approval. When work is done but unstated, report status and ask before committing.

## Principles (short)

- HTM is a **seed**; dbform rewrites the DOM (e.g. `align='T'` → contenteditable `<span>` with `min-width = exoduslength × 7`).
- **Pane** owns the outer edge; **grid borders** belong on **`td`**, not `tr`. Do not use page `tr { border-bottom }` on `exodusform` pages.
- **Embedded groups** live in host cells (`:has(> TABLE[exodusgroupno])`); a line “under” a group is often the **outer** row, not the inner table.
- Prefer **dict** / **HTM column class** / **page-local CSS** over global `!important` hacks.
- Find **root cause** (page `<style>`, inline dbform styles, pane rules) before adding suppressions.

## Light framework changes (dbform / client)

When the user asks to *lightly* adapt keyboard, focus, tab order, or form actions:

- **`focusdirection` is a dumb DOM walker**, not a form model. Policy is the skip/accept list. Prefer **one more accepted target** (e.g. form-action SPANs on Tab only via `gkeycode`) over wrap helpers, last-field oracles, or parallel routers.
- Do **not** trust `gfinalinputelement` / `gstartelement` identity alone after `form_postinit` (pages like search reorder rows; “last field” is “walker finds nothing else before wrap”).
- Key-specific behaviour already uses **`gkeycode`** (e.g. Enter skips buttons). Extend that pattern: Tab-only vs Enter/arrows, not a new code path.
- Form actions are **SPANs** (`.menubutton` / `.graphicbutton` + `exodusonclick`), not native `<button>`s — the walker already visits them if the skip list allows.
- If the first idea needs a new named helper + special-case call site, **pause** and re-read the existing walker/filter first.

## Neosys modules

Agency/finance/jobs/media HTM under `~/neosys/web/3/` is symlinked into `service/www/3/` via `~/neosys/merge_web_modules`. Module pages use the same Exodus framework paths (`../exodus/scripts/client.js`, etc.).