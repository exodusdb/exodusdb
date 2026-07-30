# Exodus — notes for AI assistants

## Form / web UI work (read first)

Before changing **form layout**, **CSS**, **borders**, **width**, **tabs**, **repeating groups**, or **embedded tables**, read:

**`service/www/3/exodus/FORM-UI-PHILOSOPHY.md`**

That file explains runtime DOM (dbform, panes, `td` vs `tr` borders, flowing spans, embedded `exogroupno` tables) and lists anti-patterns to avoid.

## Framework reference

| Topic | Location |
|-------|----------|
| Form UI philosophy | `service/www/3/exodus/FORM-UI-PHILOSOPHY.md` |
| JS API / architecture | `service/www/3/exodus/scripts/PROGRAMMERS_OVERVIEW.md` |
| Form CSS | `service/www/3/exodus/global.css` |
| Form automation | `service/www/3/exodus/scripts/dbform.js` |
| Pane wrap | `service/www/3/exodus/scripts/client.js` (`exoduswrapformpanes`) |
| Dev docs (HTML) | `service/www/exodus/doc/forms.htm` |

## Quality model (debt allowed → clean on **review** / **squash**)

Exodus is **production** shared code. Wrong event/gate/popup changes cost **~10×** later. Net speed = allow discovery, force cleanup at named gates.

### While building

- **KISS at the seam:** one more case on the real machine (walker, startevent, icon map). Not a new parallel policy.
- **Blast radius first** on `client.js` / `dbform.js` / Gate A / confirm-decide / focus: smallest change; name what else can break; never “fix” by swallowing whole key classes.
- **Technical debt is allowed for discovery** — thrash, try A then B, short WIP. Keep it local and reversible.
- **Commit freely** after substantive turns (small, descriptive; easy to bisect). Checkpoints are **not** a claim the path is finished. Do **not** push unless asked.

### Quality gates (user need not re-explain)

| User says | Do this |
|-----------|---------|
| **`review`** (plain: “review”, “review changes”, “review this”) | **Primary gate.** Scope net work (uncommitted + unpushed, or area just discussed). Check **KISS + maintainability + blast radius**. **Fix cheap debt** same turn. Report: checked / fixed / open. Commit fixes if substantive. |
| **squash** / related / big-bang / clean history | Run the **same gate**, fix debt, **then** rewrite history so the surviving commit is the clean outcome. |
| Formal **`/review`** skill, PR #, “PR review” | Read-only subagent / GitHub path only — not the plain **`review`** gate. |

**DRY is secondary and often loses to KISS.** True identical contract in many places → one seam. Similar-looking code with small differences → **prefer local copies** plus a short comment (`// Similar: pathA, pathB`) over a shared helper that papers over tiny differences. Parallel sites stay independently maintainable without a search-and-hope abstraction; false DRY (confirm vs decide Esc) is anti-KISS and high blast radius.

Plain **`review`** / **squash** must not wait for the user to also say “KISS” or “clean up debt.”

## Principles (short)

- HTM is a **seed**; dbform rewrites the DOM (e.g. `align='T'` → contenteditable `<span>` with `min-width = exoduslength × 7`).
- **Pane** owns the outer edge; **grid borders** belong on **`td`**, not `tr`. Do not use page `tr { border-bottom }` on `exodusform` pages.
- **Embedded groups** live in host cells (`:has(> TABLE[exogroupno])`); a line “under” a group is often the **outer** row, not the inner table.
- **Multivalue rows (`groupno > 0`):** `cloneNode` copies **attributes**, not **listeners**. Do not attach `input`/`change` only on the template row — use **document/table delegation** or attributes the form already re-reads. See FORM-UI-PHILOSOPHY § “Multivalue rows”.
- **Dirty leave (Cancel / Esc / F8-clear):** default **Discard / Cancel** when `gchangesmade` for unbound forms too (no modal exception). Bound locked leave still uses Save/Discard/Cancel on the record. Later: per-form opt-out for light dialogs (e.g. settings) if needed.
- Prefer **dict** / **HTM column class** / **page-local CSS** over global `!important` hacks.
- Find **root cause** (page `<style>`, inline dbform styles, pane rules) before adding suppressions.
- **Craftsmanship:** obvious local fix over thoughtless scaffolding (see global `~/.grok/AGENTS.md` change style §5).

### Auto-open a record on form load

Set **`gparameters.key`** before dbform’s post-init open (dict build or `form_postinit`). dbform then `opendoc`s it. Do **not** use `setTimeout`/`focuson` hacks. Examples: `users_dict.js` (`gparameters.key = gusername`), `systemconfiguration_dict.js` / `form_postinit` (`'SYSTEM.CFG'`). Optional: `di.defaultvalue` on the key field for New/clear defaults only.

## Framework stay-out (default)

- **Do not edit framework code** (`dbform.js`, `client.js`, core form/event/gate machinery) unless the user has **explicitly instructed that in the last few turns** of this conversation.
- Prefer fixing product code (`colors.js`, page dicts/HTM, `users.js`, etc.). If a bug looks framework-wide, **say so and wait** — do not “fix” it by changing F7, `gpreviousvalue`, `validateupdate`, or popup contracts.
- Popup cancel without `validateupdate` is **standard Exodus practice**; product popups must not break that (e.g. do not wipe mid-edit field text to paper over face-preview bugs).

## Build and install (basic exodus lib / CLI)

Core library, CLI tools, and tests live under `~/exodus` (this tree). They are **not** the service/neosys `.so` plugins.

**Do not skip tests before install.** Alias **`m`** is the full cycle; prefer it, or run the same steps by hand in order from `~/exodus/build`:

| Step | Command | What it does |
|------|---------|----------------|
| **1. Build** | `ninja` | Compile lib/cli (and gendoc post-build side effects such as refreshing `var.1` / `testing_var.h.cpp` when gendoc is linked). |
| **2. Test** | `CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=\`nproc\` ninja test` | Run the test suite. **Required before install** — do not `ninja install` after a failed or skipped test run. |
| **3. Install** | `ninja install` | Install lib/cli into the prefix (typically `/usr/local`). Only after step 2 passes. |

One-liner (same as alias **`m`**):

```bash
(cd /root/exodus/build && ninja && CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=`nproc` ninja test && ninja install)
```

| Optional | Command / note |
|----------|----------------|
| **Docs** | From `~/exodus/build`: **`ninja doc`** (or `ninja gendoc`). Regenerates `exodus/var.1` (man) and `exodus/var.htm` (HTML) via `cli/gendoc` + `doc/gendoc_helper.sh`. **Not** part of default `ninja` / `m`; see `doc/README.md`. |
| **Header doc pipeline** | gendoc only picks up **contiguous `//` comments immediately before a public function declaration** (plus section titles like `///// Section :`), not arbitrary comments elsewhere in the header. Those blocks → **pseudo-markdown** → **man** (`var.1`) / **HTML** (`var.htm`). Labels (`param:` / `return:` / …), `* ` bullets. Backticked examples are extracted into **`test/testing_var.h.cpp`** and must **compile and run**. Odd backtick counts abort gendoc. |
| **Header doc audience** | Those pre-function doc comments are **for app programmers using the library**, not library-maintainer notes. Must be **readable at a glance**. Omit anything not instantly clear; **gloss over complexity** so the main point lands. Cross-link: e.g. perform `return` only says it is main’s return or what stop()/abort() determine — details live on stop/abort. Prefer prose `//` over backticks for control-flow that is hard to run as tests. |

After a successful **build → test → install**, the basic package is on the install path. That alone does **not** rebuild or redeploy service/neosys application libraries.

## C++ style: prefer `var` member functions (FYI)

Free functions in `exofuncs.h` (`quote`, `squote`, `oconv`, `osshell`, …) mostly **forward to members**. The library and apps are written in a **member-first** style because `var` is used heavily in chains and the same names exist on both sides.

**Prefer members** in service, neosys, cli, and new tests:

| Prefer | Avoid (when a receiver is already a `var`) |
|--------|--------------------------------------------|
| `x.quote()` / `x.squote()` | `quote(x)` / `squote(x)` |
| `x.oconv(fmt)` | `oconv(x, fmt)` |
| `x.field(sep, n)` | `field(x, sep, n)` |
| `cmd.osshell()` | `osshell(cmd)` |

**Survey (approx. live `.cpp` call sites, free vs `.member`):** members dominate for `quote`, `field`, `len`, `trim`, `ucase`/`lcase` in service and neosys. Free `oconv`/`iconv` remain common in neosys (Basic-heritage / multi-arg expressions). Free `osshell` is still common for string literals (`osshell("which …")`); use `cmd.osshell()` when the command is already in a `var`. Tests mix both more freely.

Mutators follow the same idea: **`x.squoter()`** (in place) vs **`x.squote()`** (returns a new quoted `var`). Prefer the form that matches local style in the file you are editing.

Do **not** mass-convert free calls for style alone; use members for **new** code and when touching a line.

## Service / neosys C++ (compile + deploy to live)

Editing `.cpp` under `service/src/` or `~/neosys/src/` (or their `.dat` sources) is **not** enough for live. Live `serve_*` loads **`.so` plugins** from live lib dirs (`LD_LIBRARY_PATH` typically includes `/usr/local/live/lib` and `$EXO_HOME/lib`).

| Step | Command / note |
|------|----------------|
| **1a. One file** | From the source directory: `compile foo.cpp` (installs to `$EXO_HOME/lib` e.g. `~/lib/libfoo.so`). Compile every library you changed (and dependents if needed). |
| **1b. Everything** | **`~/exodus/service/src/compall`** builds all **exodus service** `.cpp` + installs/syncs **`.dat`**. **`~/neosys/compall`** runs that plus **`~/neosys/src/compall`** (all neosys). Required for a full rebuild of application libs/dat after broad changes or a fresh tree (not covered by alias `m`). |
| **2. Deploy to live** | **`cd ~/exodus/service && ./copyall CONFIRM`** — required to fully deploy: rsync `~/bin,lib,dat,inc` → `~/live` and `/usr/local/{bin,lib,…}` → `/usr/local/live` with **`--whole-file`** so `dlopen` sees real updates, and touch live dirs so **`_live` services** restart/reload. Without `CONFIRM` = dry run only. |
| **3. Do not** | Hand-copy a single `.so` as the primary path (easy to miss a dir). Do not claim “live” after compile-only. |

- **Web (JS/CSS/HTM)** is separate: served from the tree / merge paths; no `copyall` for browser assets.
- After `./copyall CONFIRM`, live services pick up new libs (script also touches live dirs so listen restarts when configured). If behaviour still looks old, check which `serve_*` unit and its `LD_LIBRARY_PATH`.

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
- **Background is not modal.** `exodus_begin_if_idle` (expression2 poll, keepalive, relock) must not mount `#uiblockerdiv` / `overflow:hidden` either — that flashed page scrollbars every 250ms on wide forms (mediadiary).
- **Same gesture:** focus runs *before* the click of that click. Cancelling that click → SELECT focus-only on first press. Allow native SELECT activation through while blocked; still do not start a new flight.
- **Do not blur+refocus** a control that already holds `document.activeElement` (closes open listboxes). `focuson` / `focuson2` only re-assert when focus was lost.

### DOMUI events while a popup is open (strategy)

`#exodusconfirmdiv` and other exclusive UI are **product-owned** for the shell lifetime. Do not re-grow a three-path maze (startevent + document_onkeydown “belt” + div handler all half-implementing Enter).

**Ownership (code in `client.js`):**

| Product | Open signal | Key owner | Form path role |
|--------|-------------|-----------|----------------|
| **Plain confirm** (OK, Yes/No, text, invalid, Wait) | `#exodusconfirmdiv` without `.exodusconfirm_decide` | **Document capture** while open: `exodusconfirm_install_plain_keydown` → `exodusconfirm_keymap` (install in `exodusconfirm2`, uninstall in `finally`) | **Swallow only** (`exodusconfirm_startevent` does not reimplement OK/Cancel) |
| **Decide list** | `.exodusconfirm_decide` / `#decide_table1` | **Handlers on the confirm div** (`decide_document_on*`) | Swallow; **Esc** if focus is outside (bubble never reaches the div) |
| **Colour / calendar** | product open flags | Product helpers / handlers on their DOM | Form isolates; do not invent a second key map in `starteventhandler` |

**Rules:**

1. **Name the owner first** (capture, div, or product helper). Install for shell open; uninstall in `finally`.
2. **Form path while exclusive UI / `gblockevents`:** isolate only — helper contract `null` / `true` / `false` (not open / allow browser e.g. Ctrl+C / swallow form logic). **Never** “sort of” dismiss OK from the form switchboard.
3. **No second belt** that returns `true` when focus is inside the dialog without acting (that made Enter a no-op).
4. **Browser keeps** Ctrl/Cmd ±/0 and Ctrl/Cmd+wheel unless a product deliberately owns them (capture zoom path stays non-`preventDefault`).
5. **`resolvePendingConfirm` is idempotent** — safe if a dual path double-fires once; do not rely on that instead of single ownership.
6. **False DRY:** confirm vs decide Esc/Enter are allowed to differ (focus often outside decide). Prefer local clear paths over one mega-`popup_onkeydown`.
7. Prefer **delete a path** over adding another when two handlers do the same job.

Smoke when touching this: OK-only invalid Enter+click; Yes/No; text input Enter/Esc; decide arrows/Select/Esc; Wait/Cancel if easy.

### Menubar / form icons (theme2)

- **One scale for all:** `--exodus-ui-icon-size` in `global.css` is the **only** size knob (outer box; scales with text). Retune once → all menubar/form icons.
- **SVG assets are full-bleed 16×16** (art to the edges of the viewBox). Default: no empty border in the file. Do **not** add per-file `scale(…)` wrappers or per-control CSS padding/mask hacks for mass.
- **Exception — perceptually large glyphs** (dense shapes that read oversized next to siblings, e.g. solid X / Close): may include **padding in the SVG** (e.g. 2 units inset on a 16 canvas). That is the only place for that inset — not CSS.
- **`--exodus-ui-icon-glyph` stays `1`** (fill the outer box). Painted `<img>` padding and mono `mask-size` both follow it; do not use glyph as a per-icon mass control.
- **Naming:** `role[-variant].svg` (kebab-case). Families: `shell-*`, `record-*`, `file-*`, `nav-*`, `row-*`, `field-*`, `confirm-*`, `media-*`, `sort` / `sort-up` / `sort-down`, `dialog-*`, `login-*`, `theme-*`. Mono masks = one file + CSS tint (no `_lm`/`_dm`); **standard 16×16** like other toolbar icons (glyph inset in the SVG if needed). Painted multi = pair `name_lm.svg` + `name_dm.svg`.
- Colours for mono stay in CSS (`--exodus-icon-*`); painted multi keep fill in the SVG.

### Still raster / webp (page chrome, not toolbar icons)

Address detail PNGs, logos (`exoduslogo.webp`, `login.webp`, …). Toolbar/actions are SVG under the role-based names above.

### Deferred: fuller Gate A/B + form-validation narrative

**DOMUI key ownership while a popup is open** is recorded above (and implemented in `client.js`). Still optional later: a longer end-to-end write-up of Gate A/B, focus, and `validateupdate` in `PROGRAMMERS_OVERVIEW.md` §3 from real call paths — not a second conflicting strategy. Do not invent parallel “AI patch” layers meanwhile.

## Neosys modules

Agency/finance/jobs/media HTM under `~/neosys/web/3/` is symlinked into `service/www/3/` via `~/neosys/merge_web_modules`. Module pages use the same Exodus framework paths (`../exodus/scripts/client.js`, etc.).