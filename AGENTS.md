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

- **Commit after each completed turn** of substantive work so history is easy to bisect/backtrack. Prefer small, descriptive commits over large batches.
- **Do not push** unless the user asks.
- Squash only when the user asks.

## Principles (short)

- HTM is a **seed**; dbform rewrites the DOM (e.g. `align='T'` → contenteditable `<span>` with `min-width = exoduslength × 7`).
- **Pane** owns the outer edge; **grid borders** belong on **`td`**, not `tr`. Do not use page `tr { border-bottom }` on `exodusform` pages.
- **Embedded groups** live in host cells (`:has(> TABLE[exodusgroupno])`); a line “under” a group is often the **outer** row, not the inner table.
- Prefer **dict** / **HTM column class** / **page-local CSS** over global `!important` hacks.
- Find **root cause** (page `<style>`, inline dbform styles, pane rules) before adding suppressions.
- **Craftsmanship:** obvious local fix over thoughtless scaffolding (see global `~/.grok/AGENTS.md` change style §5).

### Auto-open a record on form load

Set **`gparameters.key`** before dbform’s post-init open (dict build or `form_postinit`). dbform then `opendoc`s it. Do **not** use `setTimeout`/`focuson` hacks. Examples: `users_dict.js` (`gparameters.key = gusername`), `systemconfiguration_dict.js` / `form_postinit` (`'SYSTEM.CFG'`). Optional: `di.defaultvalue` on the key field for New/clear defaults only.

## Framework stay-out (default)

- **Do not edit framework code** (`dbform.js`, `client.js`, core form/event/gate machinery) unless the user has **explicitly instructed that in the last few turns** of this conversation.
- Prefer fixing product code (`colors.js`, page dicts/HTM, `users.js`, etc.). If a bug looks framework-wide, **say so and wait** — do not “fix” it by changing F7, `gpreviousvalue`, `validateupdate`, or popup contracts.
- Popup cancel without `validateupdate` is **standard Exodus practice**; product popups must not break that (e.g. do not wipe mid-edit field text to paper over face-preview bugs).

## Light framework changes (dbform / client)

Only when the user has **just** asked for a light framework adaptation of keyboard, focus, tab order, or form actions:

- **`focusdirection` is a dumb DOM walker**, not a form model. Policy is the skip/accept list. Prefer **one more accepted target** (e.g. form-action SPANs on Tab only via `gkeycode`) over wrap helpers, last-field oracles, or parallel routers.
- Do **not** trust `gfinalinputelement` / `gstartelement` identity alone after `form_postinit` (pages like search reorder rows; “last field” is “walker finds nothing else before wrap”).
- Key-specific behaviour already uses **`gkeycode`** (e.g. Enter skips buttons). Extend that pattern: Tab-only vs Enter/arrows, not a new code path.
- Form actions are **SPANs** (`.menubutton` / `.graphicbutton` + `exodusonclick`), not native `<button>`s — the walker already visits them if the skip list allows.
- If the first idea needs a new named helper + special-case call site, **pause** and re-read the existing walker/filter first.

### Gate A / DOM events (do not layer hacks)

Invariants when touching focus, click, `gblockevents`, or `#uiblockerdiv`:

- **Gate A** = exclusive async *business* flight (`exodus_begin`). `form_blockevents` prevents a *second flight*, not browser defaults for the control that just focused.
- **Avoid parallel starts at the source.** Do not call `exodus_begin` while another flight is airborne. Nested `await` inside the current flight is correct; a second *commencement* is not. Capture/sync handlers outside Gate A (e.g. digit accesskeys) must **not** start a flight if `g_exodus_flow` is set — return/ignore. Optional background uses `exodus_begin_if_idle` (quiet skip). Required work that must run after land uses `exodus_begin_when_idle`.
- **Parallel start → visible `systemerror`, not silent ignore.** With `queue_max = 0`, a conflicting `exodus_begin` shows a Gate A conflict dialog (debug signal). Do not “fix” races by teaching Gate A to swallow conflicts; fix the caller so it never takes off while busy.
- **Focus is not modal.** Do not mount `#uiblockerdiv` on focus/activate flights (native `<select>` dies under a full-page overlay).
- **Same gesture:** focus runs *before* the click of that click. Cancelling that click → SELECT focus-only on first press. Allow native SELECT activation through while blocked; still do not start a new flight.
- **Do not blur+refocus** a control that already holds `document.activeElement` (closes open listboxes). `focuson` / `focuson2` only re-assert when focus was lost.

### Menubar / form icons (theme2)

- **One scale for all:** `--exodus-ui-icon-size` in `global.css` is the **only** size knob (outer box; scales with text). Retune once → all menubar/form icons.
- **SVG assets are full-bleed 16×16** (art to the edges of the viewBox). Default: no empty border in the file. Do **not** add per-file `scale(…)` wrappers or per-control CSS padding/mask hacks for mass.
- **Exception — perceptually large glyphs** (dense shapes that read oversized next to siblings, e.g. solid X / Close): may include **padding in the SVG** (e.g. 2 units inset on a 16 canvas). That is the only place for that inset — not CSS.
- **`--exodus-ui-icon-glyph` stays `1`** (fill the outer box). Painted `<img>` padding and mono `mask-size` both follow it; do not use glyph as a per-icon mass control.
- Colours stay in the SVGs; overall display size does not.

### Raster → SVG migration (multi-turn)

**Menubar / form chrome:** already SVG (menu, refresh, logout, theme, company, New…nav, zoom, calendar, row add/delete). Delete obsolete PNG/GIF twins only when no HTM/JS refs remain.

**Next batches (in order):**
1. Login: `eye.png` / `eye-slash.png` (+ index.html)
2. Dialog chrome: `xpinfo.gif`, `xpquestion.gif`, `xpwarning.gif`, `xpcritical.gif`
3. Leftover form GIF: `smallopen.gif` if still referenced
4. Neosys page icons: `play.png`, `pencil.png`, `page_copy.png`, address/detail icons, logos as needed
5. Purge unused rasters after refs are gone

### Deferred: framework mental model write-up

User asked (2026-07) for a stored **end-to-end mental model** of Gate A/B, focus, form validation, and DOM events so future work is not terra nullius / spaghetti. **Remind the user** to run that analysis session when they have bandwidth; do not invent parallel “AI patch” layers meanwhile. Natural home: extend `PROGRAMMERS_OVERVIEW.md` §3 (or a short sibling) from real call paths, not speculation.

## Neosys modules

Agency/finance/jobs/media HTM under `~/neosys/web/3/` is symlinked into `service/www/3/` via `~/neosys/merge_web_modules`. Module pages use the same Exodus framework paths (`../exodus/scripts/client.js`, etc.).