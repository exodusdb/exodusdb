// Copyright NEOSYS All Rights Reserved.

var gdatatagnames = /(^SPAN$)|(^INPUT$)|(^TEXTAREA$)|(^SELECT$)/
var gtexttagnames = /(^SPAN$)|(^INPUT$)|(^TEXTAREA$)/
var gradiocheckboxtypes = /(^radio$)|(^checkbox$)/
//var nbsp160 = String.fromCharCode(160)

// =============================================================================
// INPUT field width rules (only elements that remain <input> after onload).
// Align-T free text is converted to SPAN first — not covered here.
// Change this table first if behaviour must change; do not add one-off patches.
//
// Scope
//   INCLUDE  INPUT type text/password (and blank type) with exolength, size!=1
//            pure DATE: always (sample '11/11/1111'), length optional
//   INCLUDE  SELECT — maxWidth 60ch only (trap stupid long option titles;
//            does not paint a fixed width from length)
//   INCLUDE  TEXTAREA — separate paint (not form_apply_input_field_width):
//            length → min-width Nch (no length → 6ch floor like free-text);
//            width/max 100% fill cell like entry free-text SPAN. Not fixed cols.
//   EXCLUDE  radio, checkbox, button, submit, image; SPAN
//   EXCLUDE  radio/checkbox; [NUMBER…] fields → SPAN (see paint)
//
// Paint width = sample measured in the field's computed font (after class/font).
// content-box width = minWidth = maxWidth.
// HTML size = length only (table preferred-width hint); never size+2; never size
// as the painted width.
//
// Sample / glyph choice (first match wins)
//   pure DATE — measure "11/11/1111"
//   period   — measure "11/1111" (PERIOD_OF_YEAR / YEAR_PERIOD / FINANCIAL_PERIOD)
//   "8"  digit-ish — TIME / DATE_TIME
//   "0"  average  — exolowercase is set and not "false"
//   "M"  max char — other INPUTs
//
// =============================================================================
// Field style / host (INPUT width + INPUT→SPAN paint below)
//
// ONLY Exodus artifacts — never app modules (Neosys/agency/finance field names,
// product helpers, or “this looks like a client account”).
//
// Primary axis: di.exostyle → DOM attribute "exostyle" (copydictitem; not exodusexostyle).
//   set by style helpers only:
//     exo_dict_code     → "code"   (uppercase SPAN; floor 6ch)
//     exo_dict_text     → "text"   (free-text SPAN; fill + wide soft max)
//     exo_dict_textarea → "text"
//     exo_dict_number   → "number" (SPAN; floor 6ch)
//
// di.length for SPAN paint:
//   text   — wide mode ONLY: empty → attribute exomaxwidth="30ch" (client applies
//            style.maxWidth when .exoform-wide) unless HTM already set exomaxwidth.
//            length set → no soft max. NOT a min-width. Narrow forms: style.maxWidth
//            must stay 100% so free-text folds under form soft ceiling — NEVER lock to 30ch.
//   code / number — unused (floor 6ch either way).
// di.length for TEXTAREA paint: min-width floor (Nch); fill cell for max.
// Align L/R INPUT: form_apply_input_field_width (length / sample).
//
// Free-text fold / wide vs narrow (AIM + method):
//   service/www/exodus/doc/FORM-UI-WIDE-NARROW.md
// Nested TABLE.exotable|exogroup|fieldstrip without [width] must NOT use
// width:max-content in global.css — strips fold budget (HARD RULE there).
// =============================================================================
var gform_input_width_digitconv = /^\[(DATE_TIME|TIME)/
var gform_input_width_puredate = /^\[DATE([,\]]|$)/
// PERIOD_OF_YEAR, YEAR_PERIOD, FINANCIAL_PERIOD, YEARPERIOD (exo_dict_period)
var gform_input_width_periodconv = /\[(PERIOD_OF_YEAR|YEAR_?PERIOD|FINANCIAL_PERIOD)/
var gform_input_width_dateconv = /\[[^\]]*DATE[^\]]*\]/
var gform_input_width_cache = {}
// SELECT: cap used/preferred width so conversion/dropdown option titles
// cannot force the form wider than a normal 60-col field.
var gform_select_max_width = '60ch'
// Sample display date for pure DATE INPUT paint (dd/mm/yyyy style width)
var gform_input_width_date_sample = '11/11/1111'
// Sample period display (mm/yyyy or similar)
var gform_input_width_period_sample = '11/1111'

function form_input_is_pure_date(element) {
	var conv = (element.getAttribute('exoconversion') || '').toUpperCase()
	if (gform_input_width_puredate.test(conv))
		return true
	// Calendar popup without DATE_TIME / TIME conversion
	if ((element.getAttribute('exopopup') || '').indexOf('form_pop_calendar') >= 0
		&& conv.indexOf('DATE_TIME') < 0
		&& !/^\[TIME/.test(conv)
		&& !gform_input_width_periodconv.test(conv))
		return true
	return false
}

function form_input_is_period(element) {
	var conv = (element.getAttribute('exoconversion') || '').toUpperCase()
	return gform_input_width_periodconv.test(conv)
}

// Style class for paint: "code" | "text" | "number" | "".
// Prefer di.exostyle / attribute exostyle (set by style helpers). Fallback only when
// missing so unstyled dicts still work — do not invent app knowledge.
function form_field_exostyle(dictitem, element) {
	var s = ''
	if (dictitem && dictitem.exostyle != null && dictitem.exostyle !== '')
		s = String(dictitem.exostyle)
	else if (element && element.getAttribute)
		s = element.getAttribute('exostyle') || ''
	s = String(s).toLowerCase()
	if (s === 'code' || s === 'text' || s === 'number')
		return s
	// Fallback: axes without style helpers
	if (dictitem) {
		if (dictitem.radio || dictitem.checkbox)
			return ''
		var conv = String(dictitem.conversion != null ? dictitem.conversion : '').toUpperCase()
		// DECIMAL / INTEGER (legacy ROUND) = plain number host
		if (conv.indexOf('[NUMBER') === 0 || conv.indexOf('[DECIMAL') === 0
			|| conv.indexOf('[INTEGER') === 0 || conv.indexOf('[ROUND') === 0)
			return 'number'
		var al = String(dictitem.align || '').toUpperCase()
		if (al.indexOf('T') === 0)
			return (dictitem.lowercase === false) ? 'code' : 'text'
	} else if (element && element.getAttribute) {
		var conv2 = String(element.getAttribute('exoconversion') || '').toUpperCase()
		if (conv2.indexOf('[NUMBER') === 0 || conv2.indexOf('[DECIMAL') === 0
			|| conv2.indexOf('[INTEGER') === 0 || conv2.indexOf('[ROUND') === 0)
			return 'number'
		if (String(element.getAttribute('exoalign') || '').toUpperCase().indexOf('T') === 0) {
			var lc = element.getAttribute('exolowercase')
			// false stored as ""; truthy = free-text
			return (lc && lc !== 'false') ? 'text' : 'code'
		}
	}
	return ''
}

// conversion [NUMBER…] / exostyle number → content SPAN host, not fixed INPUT.
// radio/checkbox never (expanded later).
function form_dictitem_is_number_text(dictitem) {
	if (!dictitem)
		return false
	if (dictitem.radio || dictitem.checkbox)
		return false
	return form_field_exostyle(dictitem, null) === 'number'
}

// INPUT → SPAN: exostyle code|text|number (or fallback align T / [NUMBER…]).
function form_dictitem_wants_text_span(dictitem) {
	if (!dictitem)
		return false
	if (dictitem.radio || dictitem.checkbox)
		return false
	var st = form_field_exostyle(dictitem, null)
	return st === 'code' || st === 'text' || st === 'number'
}

// Build-time constant string for ghost placeholder (not filldefaults).
// Prefer di.placeholder / exoplaceholder. Else constant exodefaultvalue
// ("90" / '90' / bare 90) — only when not required (required defaults still fill).
function form_placeholder_text(element) {
	if (!element || !element.getAttribute)
		return ''
	var ph = element.getAttribute('exoplaceholder')
	if (ph != null && String(ph) !== '')
		return String(ph)
	// Indicator from constant default only if optional — required fields use
	// defaultvalue for filldefaults, not as empty ghost.
	if (element.getAttribute('exorequired'))
		return ''
	var expr = element.getAttribute('exodefaultvalue')
	if (expr == null || expr === '')
		return ''
	expr = String(expr).trim()
	if (expr === '""' || expr === "''")
		return ''
	var m = expr.match(/^"([^"]*)"$/) || expr.match(/^'([^']*)'$/)
	if (m)
		return m[1]
	if (/^-?\d+(\.\d+)?$/.test(expr))
		return expr
	return ''
}

// Ghost text when empty — not the saved value; filldefaults unchanged.
// type F, not readonly; text from form_placeholder_text.
function form_apply_default_placeholder(element) {
	if (!element || !element.getAttribute)
		return
	if (element.getAttribute('exotype') != 'F')
		return
	if (element.getAttribute('exoreadonly'))
		return
	var c = form_placeholder_text(element)
	if (!c)
		return
	if (element.tagName == 'SPAN' && (element.isContentEditable || element.getAttribute('contenteditable')))
		element.setAttribute('data-placeholder', c)
	else if (element.tagName == 'INPUT' && (element.type == 'text' || !element.type))
		element.setAttribute('placeholder', c)
}

function form_input_width_char(element) {
	var conv = (element.getAttribute('exoconversion') || '').toUpperCase()
	// Pure DATE handled by form_apply via sample string — not length×glyph
	if (gform_input_width_digitconv.test(conv))
		return '8'
	if (gform_input_width_dateconv.test(conv))
		return '8'
	if ((element.getAttribute('exopopup') || '').indexOf('form_pop_calendar') >= 0)
		return '8'
	var lc = element.getAttribute('exolowercase')
	if (lc && lc !== 'false')
		return '0'
	return 'M'
}

function form_apply_input_field_width(element) {
	if (!element)
		return
	// Conversion/dropdown SELECT: longest option title can hold the form wide.
	// Cap only — do not invent a fixed width; open list still shows full labels.
	if (element.tagName == 'SELECT') {
		element.style.maxWidth = gform_select_max_width
		return
	}
	if (element.tagName != 'INPUT')
		return
	if (element.type == 'radio' || element.type == 'checkbox'
		|| element.type == 'button' || element.type == 'submit' || element.type == 'image')
		return
	if (element.size == 1)
		return
	var pureDate = form_input_is_pure_date(element)
	var isPeriod = form_input_is_period(element)
	var n = parseInt(element.getAttribute('exolength'), 10)
	if (!pureDate && !isPeriod && !(n > 0))
		return
	var sample
	var cacheKey
	if (pureDate) {
		sample = gform_input_width_date_sample
		cacheKey = 'date:' + sample
	} else if (isPeriod) {
		sample = gform_input_width_period_sample
		cacheKey = 'period:' + sample
	} else {
		var ch = form_input_width_char(element)
		sample = ch.repeat(n)
		cacheKey = ch + '\t' + n
	}
	var cs = getComputedStyle(element)
	var font = [cs.fontStyle, cs.fontVariant, cs.fontWeight, cs.fontSize, cs.fontFamily].join(' ').replace(/\s+/g, ' ').trim()
	var key = font + '\t' + cacheKey
	var px = gform_input_width_cache[key]
	if (!px) {
		var canvas = document.createElement('canvas')
		var ctx = canvas.getContext('2d')
		ctx.font = font
		px = Math.ceil(ctx.measureText(sample).width)
		gform_input_width_cache[key] = px
	}
	var w = px + 'px'
	element.style.boxSizing = 'content-box'
	element.style.width = w
	element.style.minWidth = w
	element.style.maxWidth = w
}

// F7 / F6 / pad-only: one chrome wrap around the field.
// Multivalue nest layout (two halves) — FORM-UI-WIDE-NARROW.md:
//  1) free-text DATA ENTRY → col0 width=1% → FILL host TD (~100%; fold budget)
//  2) otherwise → style width max-content → LEFT-PACK (size to columns, not fill 100%)
//     “hug content” = left-pack, not fill host. 1% alone is not enough under wide host TD.
// Free-text ENTRY only (type F + text host) — not display S.
// HTM hardcode (width= / style.width): max-content|fit|min → force left-pack (no 1%);
// other hardcodes skip auto max-content so we do not clobber. Walk groups[] / index.
function form_table_hardcoded_width(tablex) {
	if (!tablex)
		return ''
	var attr = tablex.getAttribute && tablex.getAttribute('width')
	if (attr != null && String(attr).replace(/\s/g, '') !== '')
		return String(attr)
	var sw = (tablex.style && tablex.style.width) ? String(tablex.style.width) : ''
	return sw
}

function form_table_hardcode_is_hug(w) {
	w = String(w || '').toLowerCase().replace(/\s/g, '')
	return w === 'max-content' || w === 'fit-content' || w === 'min-content'
}

function form_field_is_freetext_entry(di, element) {
	// Display fields never drive nest fill.
	if (di && di.type == 'S')
		return false
	if (element && element.getAttribute && element.getAttribute('exotype') == 'S')
		return false
	if (di && (di.radio || di.checkbox))
		return false
	if (element && element.type && (element.type == 'radio' || element.type == 'checkbox'))
		return false
	// Entry free-text: exostyle text, or align-T free-text fallback (form_field_exostyle).
	return form_field_exostyle(di, element) === 'text'
}

function form_group_needs_nest_fill(groupno) {
	if (typeof gds == 'undefined' || !gds || !gds.dict)
		return false
	var g = Number(groupno)
	var list = gds.dict.groups && gds.dict.groups[g]
	if (list && list.length) {
		for (var i = 0; i < list.length; i++) {
			if (list[i] && form_field_is_freetext_entry(list[i], null))
				return true
		}
		return false
	}
	for (var dictn = 0; dictn < gds.dict.length; dictn++) {
		var di = gds.dict[dictn]
		if (!di || Number(di.groupno) != g)
			continue
		if (form_field_is_freetext_entry(di, null))
			return true
	}
	return false
}

// Field + F6/F7 layout:
//   span.exofieldrow
//     span.exofieldchrome   ← popup / link / pad ONLY
//     host
//     (optional glued type-S name)
// Returns the host (parent is the row). Idempotent.
function form_field_chrome_ensure_wrap(element, dictitem) {
	if (element.parentNode && element.parentNode.classList
		&& element.parentNode.classList.contains('exofieldrow'))
		return element

	var wrapFill = element.tagName == 'SPAN'
		&& element.getAttribute('exotype') == 'F'
		&& form_field_exostyle(dictitem, element) === 'text'

	var row = document.createElement('span')
	row.className = wrapFill ? 'exofieldrow exofieldrow-fill' : 'exofieldrow'

	var chrome = document.createElement('span')
	chrome.className = 'exofieldchrome'

	// replaceChild returns the field; re-parent into row after chrome strip
	element = element.parentNode.replaceChild(row, element)
	row.appendChild(chrome)
	row.appendChild(element)

	if (wrapFill) {
		element.style.flex = '1 1 auto'
		element.style.width = 'auto'
		// Keep same small floor as free-text paint (6ch). Zero here collapses empty
		// FTS/search hosts so typeahead has no anchor (TEXT_XREF).
		element.style.minWidth = '6ch'
	}
	return element
}

// Icon strip for a host already in .exofieldrow (first child .exofieldchrome).
function form_field_chrome_strip(element) {
	var row = element && element.parentNode
	if (!row || !row.classList || !row.classList.contains('exofieldrow'))
		return null
	var chrome = row.firstChild
	if (chrome && chrome.classList && chrome.classList.contains('exofieldchrome'))
		return chrome
	return null
}

// Empty F7/F6 slot: append to host's chrome strip (after form_field_chrome_ensure_wrap).
function form_field_chrome_pad(hostEl, widthCss) {
	var strip = form_field_chrome_strip(hostEl)
	if (!strip)
		return null
	var pad = document.createElement('span')
	pad.className = 'exofieldchrome-pad'
	pad.setAttribute('aria-hidden', 'true')
	pad.style.display = 'inline-block'
	pad.style.flexShrink = '0'
	pad.style.width = widthCss
	pad.style.height = 'var(--exoui-icon-size)'
	strip.appendChild(pad)
	return pad
}

// Icons sit in .exofieldchrome, not beside the host — resolve by field name + same TR (clone-safe).
function form_field_from_chrome_icon(iconEl) {
	if (!iconEl || !iconEl.getAttribute)
		return null
	var hostId = iconEl.getAttribute('data-exo-host')
	if (!hostId)
		return null
	var tr = getancestor(iconEl, 'TR')
	if (tr) {
		var inRow = tr.getElementsByClassName('exoid_' + hostId)
		if (inRow && inRow.length)
			return inRow[0]
	}
	var host = $$(hostId)
	if (!host)
		return null
	if (host.tagName)
		return host
	return host[0] || null
}

// Type S name after a code with F7/F6: append to prior .exofieldrow (host line).
// Skip if a <br> or other real content is between them (e.g. Brand + <br> + name).
function form_glue_name_to_prev_code_chrome(nameEl) {
	if (!nameEl || !nameEl.parentNode)
		return
	var prev = nameEl.previousSibling
	while (prev && prev.nodeType === 3) {
		if (/\S/.test(prev.nodeValue || ''))
			return
		prev = prev.previousSibling
	}
	if (!prev || prev.nodeType !== 1)
		return
	if (prev.tagName === 'BR')
		return
	if (prev.tagName !== 'SPAN')
		return
	if (nameEl.parentNode === prev)
		return
	// Prior sibling is field row (chrome strip + host)
	if (!prev.classList || !prev.classList.contains('exofieldrow'))
		return
	if (!prev.querySelector
		|| !prev.querySelector('.exofieldchrome [isexopopup], .exofieldchrome [isexolink], .exofieldchrome-pad'))
		return
	if (nameEl.style.display === 'block')
		nameEl.style.display = ''
	nameEl.style.flexShrink = '1'
	nameEl.style.minWidth = '0'
	prev.appendChild(nameEl)
}

// Global icons: monochrome {mask,color} via CSS tokens, or painted URL for New/Open/Edit/Delete.
// (exo_icon_spec / colours: client.js + --exoicon-* in global.css)
gnewimage = gimagetheme + 'record-new_lm.svg' // painted multicolour — excluded from mask tint
gopenimage = gimagetheme + 'record-open_lm.svg' // painted page + magnifier
// F7/F6 field chrome: same grey as body text (--exoicon-neutral)
gfindimage = exo_icon_spec('field-find.svg', 'neutral')
gcalendarimage = exo_icon_spec('field-date.svg', 'neutral')
gsaveimage = exo_icon_spec('record-save.svg', 'green')
gsavegreyimage = exo_icon_spec('record-save.svg', 'lightgrey') // inactive Save
// Painted multi-colour (overlapping sheets) — not a single CSS tint
gcopyimage = gimagetheme + (gisdarktheme ? 'record-copy_dm.svg' : 'record-copy_lm.svg')
gcloseimage = exo_icon_spec('record-close.svg', 'red')
greleaseimage = exo_icon_spec('record-release.svg', 'blue') // was #1E97CC
geditimage = gimagetheme + 'record-edit_lm.svg' // painted multicolour
gdeleteimage = gimagetheme + 'record-delete_lm.svg' // painted multicolour
glistimage = exo_icon_spec('file-list.svg', 'darkgrey')
gprintsendimage = exo_icon_spec('file-print.svg', 'darkgrey')
ginsertrowimage = exo_icon_spec('row-insert.svg', 'green')
gdeleterowimage = exo_icon_spec('row-delete.svg', 'red')
gexpandrowimage = exo_icon_spec('row-expand.svg', 'darkgrey')
gsortimage = exo_sortimage()
glinkimage = exo_icon_spec('field-link.svg', 'neutral')
gfirstimage = exo_icon_spec('nav-first.svg', 'blue')
glastimage = exo_icon_spec('nav-last.svg', 'blue')
gnextimage = exo_icon_spec('nav-next.svg', 'blue')
gpreviousimage = exo_icon_spec('nav-prev.svg', 'blue')
// Painted New/Open/Edit/Delete need DM twin files
if (gisdarktheme) {
	gnewimage = gimagetheme + 'record-new_dm.svg'
	gopenimage = gimagetheme + 'record-open_dm.svg'
	gdeleteimage = gimagetheme + 'record-delete_dm.svg'
	geditimage = gimagetheme + 'record-edit_dm.svg'
	gthemeimage = gimagetheme + 'shell-theme_dm.svg'
}

// document.getElementsByTagName('BODY').onload=window_onload

//form function global variables
var gpagenrows = 10
var gkeycode
// Opt-in focus/scroll diagnostics: __form_scroll_log=1, ?scrolllog=1, or localStorage.
function form_scroll_log_on() {
	try {
		if (window.__form_scroll_log === 1 || window.__form_scroll_log === true)
			return true
		if (window.__form_scroll_log === 0 || window.__form_scroll_log === false)
			return false
		if (/(?:^|[?&])scrolllog=1(?:&|$)/.test(String(location.search || '')))
			return true
		if (window.localStorage && localStorage.getItem('gform_scroll_log') == '1')
			return true
	} catch (e) { }
	return false
}
function form_scroll_log_msg() {
	if (!form_scroll_log_on())
		return
	var a = ['[form-scroll]']
	for (var i = 0; i < arguments.length; i++)
		a.push(arguments[i])
	try {
		console.log.apply(console, a)
	} catch (e) {
		try { console.log(a.join(' ')) } catch (e2) { }
	}
}
function form_scroll_el_label(el) {
	if (!el)
		return '(null)'
	var t = el.type || ''
	var id = el.id || ''
	var v = (t == 'radio' || t == 'checkbox') ? (el.value || '') : ''
	return (el.tagName || '?') + (t ? '[' + t + ']' : '') + '#' + id
		+ (v !== '' ? '="' + v + '"' : '')
		+ (el.checked ? '(checked)' : '')
}
var gdictfilename

var gparameters
// Move input parameters from parent windows (windowopen / windowopenkey).
// Done via opener.gwindowopenparameters — not ?param= in the URL (keeps URL cacheable).
// Parent owns bag lifecycle (windowopen always replaces). Child does not null the
// bag so F5 re-reads the same open (a74835d4). Stale keys across forms: only apply
// when bag._openhtm matches this page basename (journals must not inherit a
// vouchers key left after windowopenkey).
// (Pre-a74835d4 one-shot: null after copy — fixed F5 same-form reopen.)
var gwindowopenerparameters = ''
try {
	if (window.opener && window.opener.gwindowopenparameters) {
		var bag = window.opener.gwindowopenparameters
		var here = ''
		try {
			here = String(window.location.pathname || '').replace(/\\/g, '/').split('/').pop().toLowerCase()
		} catch (e2) { }
		var forhtm = bag._openhtm != null ? String(bag._openhtm).toLowerCase() : ''
		// Match: stamped bag for this form. Unstamped (legacy): accept once.
		if (!forhtm || !here || forhtm == here)
			gwindowopenerparameters = bag
	}
}
catch (e) {
}
if (gwindowopenerparameters) {
	for (var paramname in gwindowopenerparameters) {
		if (paramname == '_openhtm')
			continue
		gparameters[paramname] = gwindowopenerparameters[paramname]
	}
}
if (gparameters.readonlymode)
	greadonlymode = true
//gparameters.openreadonly allows edit/release

var gonfocuselement
var gpreviouselement = null
var gnextelement = null
// Caret snapshot for invalid leave → focus return (capture on focusout, before
// the next field steals selection). { el, kind: 'input'|'span', start, end }
var gprevious_sel = null
// Radio/checkbox Esc: value when focus first entered the field (survives live click-validate).
// Not a twin of gpreviousvalue — only Esc while still on that field/group.
// Touched clear on Esc: same as text — gelementthatjustcalledsettouched
// (set after live click commit only when form was untouched before that click).
var g_radio_arrival_anchor = null
var g_radio_arrival_value = ''
var gdependents = []
// insertallrows multi-entry: suspend calcfields so S-field xlate runs once mass
// (grecn null + array.exoxlate) at end or after mid-loop validation fail.
var gcalcfields_suspend = 0
var gcalcfields_pending = null // null | true (all) | [fieldn,…]
var gKeyNodes = false//init will get an array of key nodes if any
// Form OK/Cancel/Save/custom actions: 'top' menubar or 'bottom' under the form.
// Default 'auto': bound → top; unbound → bottom *preferred*.
// Bottom is revoked after full layout if the bar would be off-screen
// (form_keep_action_buttons_on_screen) — never leave Save/OK below the fold.
// Override: gparameters.formbuttonsplace = 'top' | 'bottom' | 'auto' in form_preinit.
var gformbuttonsplace = 'top'
var gkeyexternal = ''//external format of key eg STEVE*1/1/2000
var gkey = ''//internal format of key eg STEVE*12080
var gkeys = []
var gkeyn = 0
var glastkey = ''
var gloaded = false
var gfirstelement
var gfirstnonkeyelement = ''
var gstartelement
var gfinalinputelement
// Digit accesskey → control map (built by form_register_accesskeys). Alt+0…9.
// Complements native HTML accesskey (unreliable) and hardcoded Alt+letter form chrome.
var gformdigitaccesskeys = null
// Capture-phase sync keydown installed once (not via Gate A addeventlistener).
var gformdigitaccesskey_capture_installed = false
// Alt+Up/Down viewport pan always; Alt+Left/Right pan only when form is wide
// (else browser history back/forward). Capture/sync (Gate A undoes scrollBy).
var gform_scroll_viewport_capture_installed = false
// Radio/checkbox: focus on mouseup only (not mousedown). Capture/sync outside Gate A.
var gform_radio_md_target = null
var gform_radio_was_checked = false // optional radio: re-activate clears (click or Space)
var gform_radio_mouseup_focus_installed = false
var gtouched = false//set true in validateupdate exit and delete row (not insert row)
var gelementthatjustcalledsettouched
var gallowsavewithoutchanges = false//allows locked records (with keys) to be saved anyway
var glocked = false//true means record is locked and available for edit
var grelockingdoc = false
var gwindowunloading = false//this seems to be about closing, not unloading and perhaps should be renamed
var gclosing = false
//var gisnewrecord replaced by gds.isnewrecord
var grelocker
var gro
var gmaxgroupno = 0
var gfields = []
var gtables = []
var gsortimages = []
var gdoctitle = document.title
var ginitok = false
var grecn = null
var grows = null
var gfocusonelement
var gcell
var gvalue
var goldvalue
var gorigvalue
var gvalidatingpopup
var gtimers
//var gsavemode replaced by gparameters.savemode
var greadonlymode
var gupdateonlymode
var gpreventcreation//no new
var gpreventdeletion//no delete
var gcalendarscript//1 if needed, 2 if script inserted, 3 if script loaded

//form customisation (must be in a script loaded prior to this dbform script)
//currently CANNOT have any async db or ui
if (typeof (form_preinit) == 'function')
	form_preinit()

//add a script for the data file definition in case not included in the main html
if (typeof gmodule == 'undefined') {
	gmodule = ''
	exosettimeout('formfunctions_onload', 10)
}
if (typeof gdictfilename == 'undefined' || !gdictfilename)
	gdictfilename = gdatafilename
if (typeof gdatafilename == 'undefined' || !gdatafilename)
	gdatafilename = gdictfilename
if (eval('typeof dict_' + gdictfilename + '=="undefined"')) {
	var temp = '<SCR' + 'IPT id=maindict src="scripts/' + gdictfilename.toLowerCase() + '_dict.js"></SCR' + 'IPT>'
	document.writeln(temp)
}

var gds

// Preferred place only. Bottom may be revoked after layout if off-screen.
// gparameters.formbuttonsplace: 'top' | 'bottom' | 'auto' (default auto).
function form_formbuttons_place() {
	var place = (gparameters && gparameters.formbuttonsplace != null && gparameters.formbuttonsplace !== '')
		? String(gparameters.formbuttonsplace).toLowerCase()
		: 'auto'
	if (place === 'auto')
		return gKeyNodes ? 'top' : 'bottom'
	return place === 'bottom' ? 'bottom' : 'top'
}

// Database name + username in the trailing menubar cluster (left of theme/logout).
// Cluster is one float:right flex row with equal gap — see exo_menubar_trailing_cluster.
// Never nest inside #formbuttonsdiv (float:left) — that pins the label to the form actions.
// clear:both must come AFTER all floated menubar children (height for adjust_bodymargin).
function form_place_menubar_session() {
	if (typeof gdatasetname == 'undefined' && typeof gusername == 'undefined')
		return
	if (typeof add_exo_menubar == 'function')
		add_exo_menubar()
	if (!gexo_menubar)
		return

	var old = gexo_menubar.querySelector('.exo_menubar_session')
	if (old)
		old.parentNode.removeChild(old)

	// Remove prior clear divs so we can re-append clear as the last child
	var clears = gexo_menubar.querySelectorAll('.exo_menubar_clear')
	for (var ci = 0; ci < clears.length; ci++)
		clears[ci].parentNode.removeChild(clears[ci])

	// Dataset: "name - code" unless same text (case-insensitive) → name only.
	var dsname = gdatasetname || ''
	var dscode = gdataset || ''
	var dslabel = dsname || dscode
	if (dsname && dscode
		&& String(dsname).toLowerCase() != String(dscode).toLowerCase())
		dslabel = dsname + ' - ' + dscode
	var text = dslabel
	if (gusername)
		text = text ? (text + ' - ' + gusername) : String(gusername)
	if (!text || text === ' - ' || text === '-')
		return

	var span = document.createElement('span')
	span.className = 'exo_menubar_session'
	// Codes are only "xxx" or "xxx_test"
	if (gdataset && String(gdataset).slice(-5) == '_test')
		span.classList.add('exo_menubar_session_test')
	span.appendChild(document.createTextNode(text))

	// First child of trailing cluster: [session | theme | logout]
	var trailing = (typeof exo_menubar_trailing_cluster == 'function')
		? exo_menubar_trailing_cluster()
		: null
	if (trailing) {
		if (trailing.firstChild)
			trailing.insertBefore(span, trailing.firstChild)
		else
			trailing.appendChild(span)
	} else {
		gexo_menubar.appendChild(span)
	}

	var clear = document.createElement('div')
	clear.className = 'exo_menubar_clear'
	gexo_menubar.appendChild(clear)

	// Session-only bars (modal search.htm) and late content must push body below the fixed bar
	if (typeof adjust_bodymargin == 'function')
		adjust_bodymargin()
}

// Form action strip: single live #formbuttonsdiv (no hidden source / visible face clone).
// formbutton_op kept as a thin await wrapper for Alt+letter / key paths.

async function formbutton_op(op, event) {
	if (typeof op != 'function')
		return
	if (arguments.length > 1)
		return await op(event)
	return await op()
}

// Call after form_postdisplay / custom buttons / pane wrap: if the action bar is
// under the form but not fully on-screen, put it in the top menubar.
// Idempotent when already top. client.js re-runs this after exowrapformpanes.
function form_keep_action_buttons_on_screen() {
	if (gformbuttonsplace !== 'bottom')
		return
	var bar = $$('formbuttonsdiv')
	if (!bar)
		return
	var vh = window.innerHeight || document.documentElement.clientHeight || 0
	if (!vh)
		return
	var rect = bar.getBoundingClientRect()
	// Fully visible in the viewport (any part below fold → move up)
	if (rect.bottom <= vh && rect.top >= 0)
		return
	form_move_action_buttons_to_top()
}

// Move #formbuttonsdiv from under the form into the fixed top menubar.
function form_move_action_buttons_to_top() {
	var bar = $$('formbuttonsdiv')
	if (!bar || gformbuttonsplace === 'top')
		return

	gformbuttonsplace = 'top'

	// Unbound Save/Close stay .graphicbutton when moved up (do not flatten).
	// Bound form tools are already .menubutton.

	add_exo_menubar()

	// Drop leftover face node from the old clone architecture (if any)
	var oldface = $$('formbuttonsdiv_face')
	if (oldface && oldface.parentNode)
		oldface.parentNode.removeChild(oldface)

	// #exo_menu is a SPAN — keep formbuttonsdiv a SPAN for valid nesting
	var topbar = bar
	if (bar.tagName !== 'SPAN') {
		topbar = document.createElement('SPAN')
		topbar.id = 'formbuttonsdiv'
		while (bar.firstChild)
			topbar.appendChild(bar.firstChild)
		if (bar.parentNode)
			bar.parentNode.removeChild(bar)
	}
	else {
		topbar.className = ''
		if (topbar.parentNode)
			topbar.parentNode.removeChild(topbar)
	}

	// Mark relocated so CSS can add spacing after Menu (not for native top bars)
	topbar.classList.add('exo_formbuttons_relocated')

	// Order: Menu | form actions (List/…) | trailing. Never left of Menu.
	// Race: form_keep / rAF often runs *after* client.js inserts .hamburger_menu;
	// insertBefore(firstChild) then put List left of Menu. Always park after Menu
	// when present; if Menu is added later it still insertBefore(firstChild) itself.
	var menu = gexo_menubar.querySelector('.hamburger_menu')
	if (menu)
		gexo_menubar.insertBefore(topbar, menu.nextSibling)
	else
		gexo_menubar.insertBefore(topbar, gexo_menubar.firstChild)
	form_place_menubar_session()

	if (typeof adjust_bodymargin == 'function')
		adjust_bodymargin()
}

// Extra form action next to OK/Cancel in #formbuttonsdiv. Call from form_postinit (not raw HTML).
// spec: { id, text, title, accesskey, image, onclick, disabled, insert }
// Element id is id+"button". Click: onclick expression (same as old HTML exo_onclick),
// or omit onclick to use an existing id_onclick (e.g. proforma → proforma_onclick).
function form_add_action_button(spec) {

	if (!spec || !spec.id) {
		systemerror('form_add_action_button()', 'id required')
		return null
	}
	var bar = $$('formbuttonsdiv')
	if (!bar) {
		systemerror('form_add_action_button()', 'formbuttonsdiv not ready')
		return null
	}

	var id = spec.id
	if ($$(id + 'button'))
		return $$(id + 'button')

	// Unbound form extras match Save/Close (raised); bound stay flat menubar tools
	var align = gKeyNodes ? 'left' : 'center'
	bar.insertAdjacentHTML(
		spec.insert === 'start' ? 'afterbegin' : 'beforeend',
		'<span>' + menubuttonhtml(
			id,
			spec.image || '',
			spec.text != null ? spec.text : id,
			spec.title || '',
			spec.accesskey || '',
			align
		) + '</span>'
	)

	var button = $$(id + 'button')
	if (!button)
		return null

	// Optional expression — do not invent/overwrite id_onclick (avoids recursion with e.g. proforma)
	if (spec.onclick) {
		var expr = String(spec.onclick).replace(/^\s+/, '')
		if (expr.slice(0, 6) !== 'await ')
			expr = 'await ' + expr
		var nodes = button.parentNode.querySelectorAll('[exo_onclick]')
		for (var i = 0; i < nodes.length; i++)
			nodes[i].setAttribute('exo_onclick', expr)
	}

	if (!gKeyNodes)
		button.tabIndex = 0
	if (spec.disabled)
		setdisabledandhidden(button, true)
	window[id + 'button'] = button
	// May add a digit accesskey; rebuild map used by Alt+0…9
	form_register_accesskeys()
	return button
}

//'WINDOW LOAD
//''''''''''''

async function formfunctions_onload() {

	wstatus('Initialising, please wait ...')

	//has the problem that clicking on popup images "activates" the surrounding TD!
	//old method is to instate onfocus on all exodus data elements

	//document.body.onactivate=document_onfocus
	//document.body.onfocus='await document_onfocus(event)'
	//document.body.onfocus=document_onfocus
	var activateorfocus = typeof document.body.onactivate == 'undefined' ? 'focus' : 'activate'
	addeventlistener(document.body, activateorfocus, 'document_onfocus')
	// Caret while still on field (validate runs after focus already moved)
	addeventlistener(document.body, 'focusout', 'form_onfocusout_capture')

	gds = new exodatasource
	gds.onreadystatechange = gds_onreadystatechange

	//flag to later events that onload has not finished (set true at end of window_onload)
	ginitok = false

	//make sure not "framed"
	if (window != window.top)
		top.location.href = self.location.href

	//login('formfunctions_onload')

	//form customisation
	if (typeof form_onload == 'function') {
		if (!(await exoevaluate('await form_onload()', 'await form_onload()')))
			return
	}

	//make sure we are logged in
	if (!gusername)
		db.login()

	//done in client.js after db
	//gbasecurr=exogetcookie2('bc')
	//gbasefmt=exogetcookie2('bf')

	//check a parameter
	if (typeof gmodule == 'undefined') {
		systemerror('formfunctions_onload()', 'gmodule is not defined')
		return false //logout('formfunctions_onload - no gmodule')
	}

	//check dictionary function exists
	var temp
	try {
		temp = eval('dict_' + gdictfilename)
	}
	catch (e) {
	}
	if (typeof temp != 'function') {
		systemerror('dict_' + gdictfilename + ' dict function not defined')
		return false //logout('formfunctions_onload - dict function missing')
	}

	var dictfunctionname = 'dict_' + gdictfilename
	var dictarray = await exoevaluate(dictfunctionname + '(gparameters)', 'formfunctions_onload');
	gro = new exorecord(dictarray, gdatafilename)

	gds.dict = gro.dict

	//calendar popup functions
	if (gcalendarscript == 1) {
		var element = document.createElement('script')
		element.src = EXODUSlocation + 'scripts/calendar.js'
		//document.body.insertBefore(element, null)
		//solve htmlfile invalid argument error
		document.body.insertBefore(element, document.body.firstChild)
		//document.writeln('<SCR'+'IPT src="'+EXODUSlocation+'scripts/calendar.js" type="text/javascript"></SCR'+'IPT>')
	}

	//greadonlymode=false

	//set the datasource of all elements
	datasrcelements = []

	var element = document.body
	while (element = nextelement(element)) {

		//skip weird elements
		if (typeof element.getAttribute == 'unknown' || !element.getAttribute)
			continue

		//ensure buttons have tabIndex exo_tabindex_default - to make them come last
		if (element.tagName == 'BUTTON' && !element.tabIndex)
			element.tabIndex = exo_tabindex_default

		//backward compatible with old style datafld attributes
		var datafld = null
		if (!element.id) {
			try {
				datafld = element.getAttribute('datafld')
				if (!datafld)
					datafld = element.getAttribute('datafld')
				if (datafld)
					element.id = datafld
			} catch (e) {
			}
		}

		//force cursor on exotype B for button
		if (element.getAttribute('exotype') == 'B' && element.style)
			element.style.cursor = 'pointer'

		//NB inserting elements within the loop means that the same element
		//may be processed more than once so ensure skip on 2ndtime
		//  if(typeof element.getAttribute('exogroupno')=='undefined'&&element.tagName!='OPTION'&&element.name!=''&&typeof element.name!='undefined')
		//element.name ... gives error "class does not support automation"
		//  if(typeof element.getAttribute('exogroupno')=='undefined'&&element.name&&element.tagName!='OPTION'&&element.name!=''&&typeof element.name!='undefined')
		//  if(typeof element.getAttribute('exogroupno')=='undefined'&&element.tagName!='OPTION'&&element.name!=''&&typeof element.name!='undefined')
		var fieldname = element.id
		if (!fieldname)
			continue

		//if (typeof element.getAttribute('exogroupno')=='undefined'
		//    &&element.tagName!='OPTION'
		//    &&(fieldname=element.name))
		if (element.getAttribute('exogroupno') == null && element.tagName != 'OPTION') {
			//locate the dictionary item otherwise skip the field
			var dictitem = gro.dictitem(fieldname)
			if (!dictitem) {
				//dont error if id not in dictionary because could be non form element but do error if datafld specified
				if (datafld)
					systemerror('formfunctions_onload()', 'Form element ' + exoquote(fieldname) + ' is not in the ' + exoquote(gdictfilename) + ' dictionary\nor is not in the correct group.')
				continue
			}

			//authorisation to make any field read only on bound forms (files)
			//eg CLIENT UPDATE "MARKET"
			//eg CLIENT UPDATE "MARKET CODE"
			if (gKeyNodes) {
				var words = fieldname.split('_')
				for (var wordn = 0; wordn < words.length; ++wordn) {
					if (!(await exoui_security(gdatafilename.exosingular() + ' UPDATE ' + words.slice(0, wordn + 1).join(' ').exoquote()))) {
						dictitem.readonly = gmsg
						break;
					}
				}
			}

			//check that the name is not in use elsewhere (function or other field)
			//try
			//{
			var temp = window[fieldname]
			if (!temp)
				temp = document[fieldname]
			if (temp && temp != element) {
				//TODO should check all and not just ignore previously built radio or checkboxes
				var temp0 = temp[0] ? temp[0] : temp
				if (temp0.type != 'radio' && temp0.type != 'checkbox') {
					systemerror('name ' + exoquote(fieldname) + ' is already in use elsewhere\n' + eval(temp).toString().slice(0, 100) + '\n...')
					continue
				}
			}
			//}
			//catch (e){}

			//convert into DOM node
			element.id = fieldname
			//there is a problem with this for radio boxes as there are multiple elements for one id
			if (element.type != 'radio' && element.type != 'checkbox') {
				element = $$(element.id)
			}
			//element=eval(element.id)

			///log('formfunctions_onload ' + fieldname)

			//dictionary modifications
			//none - currently done in dictrec builder

			// INPUT → SPAN when dict axes say text host: align T or [NUMBER…].
			// Dates/periods/times stay INPUT. Never radio/checkbox (expanded later).
			if (element.tagName == 'INPUT'
				&& (!element.type || element.type == 'text' || element.type == 'password')
				&& form_dictitem_wants_text_span(dictitem)) {

				//replace original element
				var newspan = document.createElement('span')
				element.parentNode.replaceChild(newspan, element)
				newspan.style.marginLeft = element.style.marginLeft

				//copy over attributes
				newspan.tabIndex = element.tabIndex
				//commented out because it prevent setting to -1 if readonly below
				//if (!newspan.tabIndex)
				// newspan.tabIndex=exo_tabindex_default
				if (typeof element.length != 'undefined')
					newspan.length = element.length
				newspan.id = element.id

				//switch over to the new element
				element = newspan
			}

			//copy the dictionary to the field
			//repeated below because cannot seem to merge expando attributes on mac!!!)
			//TODO consider removing lower repetition
			//also repeated for ckeditor spans
			copydictitem(dictitem, element)

			var elementtabindex = element.tabIndex

			//from here on there should be no need for references to the dictitem

			//build radio and check boxes
			if (
				(element.getAttribute('exoradio') && element.type != 'radio')
				||
				(element.getAttribute('exocheckbox') && element.type != 'checkbox')
			) {

				//radio has preference over checkbox
				if (element.getAttribute('exoradio'))
					element.setAttribute('exocheckbox', '')

				//gdefault=await getdefault(element)

				//build html for multiple inputs
				var options = (element.getAttribute('exoradio') ? element.getAttribute('exoradio') : element.getAttribute('exocheckbox')).exosplit(':;')
				var temp = ''
				var elementtype = element.getAttribute('exoradio') ? 'radio' : 'checkbox'
				for (var ii = 0; ii < options.length; ii++) {

					// One nowrap unit: radio/checkbox + label (no break between control and title
					// in narrow cells — e.g. scheduleprint Invoice Type). Label-only nowrap
					// left the input as a separate wrap opportunity.
					temp += '<span style="white-space:nowrap;vertical-align:middle">'
					//build an input item
					temp += '<input type=' + elementtype + ' id=' + fieldname
					temp += ' style="vertical-align:middle"'
					if (element.getAttribute('exoreadonly'))
						temp += ' disabled=true'
					//must be done to group radio boxes
					temp += ' name=' + fieldname
					// mark for mouseup-focus handler (expanded radios have no other marker)
					temp += ' exotype=F'
					// Keep required on each option (original input is replaced by the span)
					if (element.getAttribute('exorequired')
						&& element.getAttribute('exorequired') != 'false')
						temp += ' exorequired=true'
					if (typeof (options[ii][0]) != 'undefined')
						temp += ' value=' + options[ii][0].toString().exoquote()

					//set default but it has to be done again in cleardoc anyway
					//if (gdefault==options[ii][0]) temp+=' checked=true'

					//temp+=' onfocus="return await document_onfocus(event)">'
					temp += '>'

					//postfix the option title
					if (typeof (options[ii][1]) != 'undefined')
						temp += '<span style="vertical-align:middle">' + options[ii][1] + '</span>'
					temp += '</span>'

					//horizontal or vertical
					//if vertical then add <br /> between options
					//but not after last option (so that next input can appear inline with the last option.
					if (element.getAttribute('exohorizontal'))
						temp += '&nbsp;&nbsp;'
					else if (ii < options.length - 1)
						temp += '<br />'

				}

				//replace original element
				var newspan = document.createElement('span')
				newspan.innerHTML = temp
				//element.swapNode(temp)
				element.parentNode.replaceChild(newspan, element)

				//setup onfocus and onclick for all boxes (inputs nested in option wraps)
				var boxes = newspan.getElementsByTagName('input')
				for (var ii = 0; ii < boxes.length; ii++) {
					element = boxes[ii]
					if (element.type == elementtype) {
						element.tabIndex = elementtabindex

						//sadly onfocus doesnt seem to get called on checkboxes in safari 4/chrome 5.0
						//addeventlistener(element, 'focus', 'document_onfocus')
						//addlistener doesnt work on rows since cloning rows doesnt clone listeners
						//addeventlistener(element, 'click', 'onclickradiocheckbox')
						element.setAttribute('exo_onclick', 'await onclickradiocheckbox()')
					}

				}


				//having converted the element to a SPAN/array of INPUT elements ...
				element = newspan
				continue

			}

			var exodropdown = element.getAttribute('exodropdown')
			if (exodropdown) {

				exoassertobject(element, 'formfunctions_onload', 'element (exodropdown)')
				if (typeof exodropdown == 'string')
					exodropdown = exodropdown.split(fm)
				var request = exodropdown[0]
				var colarray = exodropdown[1]
				var noautoselection = exodropdown[2]

				//convert element to a SELECT
				//var temp=document.createElement('select '+element.outerHTML.slice(7)
				var selectelement = document.createElement('select')
				copydictitem(dictitem, selectelement)

				//make sure tabindex is copied over
				selectelement.tabIndex = element.tabIndex

				//indicate that it is a single selection, one line drop down.
				selectelement.size = 1

				//element.swapNode(t)
				element.parentNode.replaceChild(selectelement, element)
				selectelement.id = element.id

				//create all the options of the element
				await exosetdropdown(selectelement, 'CACHE\r' + request, colarray, '', noautoselection)

				element = selectelement
				//    element.innerHTML=element.innerHTML+' '

				element.tabIndex = elementtabindex

			}

			//convert by conversion attribute
			if (element.getAttribute('exoconversion')) {

				var exoconversion = element.getAttribute('exoconversion')

				//conversion is a routine eg [NUMBER] [DATE]
				if (
					typeof exoconversion == 'string'
					&&
					exoconversion.slice(0, 1) == '['
				) {
					//do nothing
				}

				// magic "color": keep bound text INPUT; swatch chrome installed later
				else if (
					typeof exoconversion == 'string'
					&&
					exoconversion.toLowerCase() == 'color'
				) {
					// mark only — colors_install_swatch after popup icon so wrap order is sane
					element.setAttribute('data-exo-color-pending', '1')
				}

				//conversion is an array of options → SELECT
				else {

					//convert element to a SELECT

					var temp = document.createElement('select')
					copydictitem(dictitem, temp)

					temp.size = 1
					//element.swapNode(t)
					element.parentNode.replaceChild(temp, element)
					origid = element.id
					element = temp
					element.id = origid

					setdropdown3(element, element.getAttribute('exoconversion'))

					element.tabIndex = elementtabindex

					addeventlistener(element, 'change', 'form_onchangeselect')
				}
			}

			//onchange for readonly elements
			if (
				element.getAttribute('exoreadonly')
				&&
				!element.getAttribute('disabled')
				&&
				element.tagName && element.tagName == 'SELECT'
			) {
				setdisabledandhidden(element, true)
			}
			// form_oninput is document-delegated (see gform_oninput_delegated below).
			// Per-element listeners are lost on gds cloneNode for multivalue rows.

			//add some events - done on document.body now
			//if (element.tagName.match(gdatatagnames))
			//{
			//}

			//check for duplicate field numbers
			with (dictitem) {
				if (type == 'F' && !dictitem.allowduplicatefieldno) {

					var fieldandwordn = fieldno
					if (keypart)
						fieldandwordn += '.' + keypart
					if (dictitem.wordno)
						fieldandwordn += '.' + wordno

					//skip radio/checkbox elements with the same id
					if (gds.dict.fieldandwordns[fieldandwordn] != element.id) {

						if (typeof gds.dict.fieldandwordns[fieldandwordn] != 'undefined') {
							//throw(new Error(0,exoquote(name)+' duplicate dictionary field and word no '+fieldandwordn+' is not allowed'))
							systemerror('formfunction_onload', exoquote(name) + ' duplicate dictionary name, field no, word no ' + fieldandwordn + ' is not allowed without .allowduplicatefieldno=true\n' + (gds.dict.fieldandwordns[fieldandwordn]))
						}

						//save a field pointer
						gds.dict.fieldandwordns[fieldandwordn] = element.id

					}

				}

			}

			//copy the dictionary to the field (repeated from above!)
			copydictitem(dictitem, element)

			//the first element is considered to be the 'key' field
			//zzz should be field 0
			if (element.getAttribute('exofieldno') == '0') {
				if (!gKeyNodes)
					gKeyNodes = []
				//gKeyNodes[gKeyNodes.length] = element
				gKeyNodes.push(element)
				if (gKeyNodes.length == 1) {
					var tt = element.getAttribute('exoprintfunction')
					if (tt)
						gKeyNodes[0].setAttribute('exoprintfunction', tt)
					var tt = element.getAttribute('exolistfunction')
					if (tt)
						gKeyNodes[0].setAttribute('exolistfunction', tt)
					element.accessKey = 'K'
				}

			}

			// SPAN host paint — classify once (exostyle); one width policy per class.
			var fieldStyle = form_field_exostyle(dictitem, element)
			if (element.tagName == 'SPAN' && typeof element.style.whiteSpace != 'undefined') {
				// code/number: nowrap; text: free-text fold
				var noFold = (fieldStyle === 'code' || fieldStyle === 'number')
				try {
					if (noFold) {
						element.style.whiteSpace = 'nowrap'
						element.style.overflowWrap = 'normal'
					} else {
						element.style.whiteSpace = 'pre-wrap'
						element.style.overflowWrap = 'break-word'
					}
				} catch (e) {
					try {
						//pre-wrap above errors before IEv8+ XP/Win2003
						element.style.whiteSpace = noFold ? 'nowrap' : 'pre'
						if (!noFold)
							element.style.wordWrap = 'break-word'
					}
					catch (e2) {
					}
				}
			}

			// code and number: same host width (floor 6ch, expand). length unused.
			// valign: leaf cell children use CSS top (.exodata > *).
			if (element.tagName == 'SPAN'
				&& (fieldStyle === 'number' || fieldStyle === 'code')) {
				element.style.display = 'inline-block'
				element.style.minWidth = '6ch'
				element.style.maxWidth = 'none'
				element.style.boxSizing = 'border-box'
			} else if (element.tagName == 'SPAN' && element.style.display != 'none'
				&& fieldStyle === 'text') {
				// Free-text fold (narrow default): fill cell, pre-wrap, maxWidth 100%.
				// Attribute exomaxwidth="30ch" is a WIDE-MODE soft max only — applied by
				// form_table_apply_freetext_wide_max when .exoform-wide. Do NOT set
				// style.maxWidth to 30ch here (would lock typing to ~30 chars always).
				var freeLen = parseInt(element.getAttribute('exolength'), 10)
				if (!(freeLen > 0))
					freeLen = 0
				if (!freeLen) {
					// HTM may preset exomaxwidth (e.g. schedules VEHICLE_NAME 20ch)
					if (!element.getAttribute('exomaxwidth'))
						element.setAttribute('exomaxwidth', '30ch')
				} else
					element.removeAttribute('exomaxwidth')
				var entryF = (element.getAttribute('exotype') == 'F'
					&& !element.getAttribute('exoreadonly'))
				if (entryF) {
					element.style.display = 'block'
					element.style.width = '100%'
					// Empty-host floor only. Not freeLen / not 30ch.
					element.style.minWidth = '6ch'
				} else {
					// display free-text / names — keep side-by-side with codes
					element.style.display = 'inline-block'
					element.style.minWidth = '0'
				}
				// Always 100% at paint; wide layout may raise soft max later
				element.style.maxWidth = '100%'
				element.style.boxSizing = 'border-box'
			}

			//allow for data entry in SPAN elements (unless hidden)
			if (element.getAttribute('exotype') == 'F' && element.tagName == 'SPAN' && element.style.display != 'none') {
				if (!(element.getAttribute('exoreadonly'))) {
					element.contentEditable = 'true'
					//element.contentEditable = true
					if (!(element.getAttribute('tabindex')))
						element.setAttribute('tabindex', exo_tabindex_default)
				}
			}

			// Constant default → CSS/native placeholder when empty (see form_apply_default_placeholder)
			form_apply_default_placeholder(element)

			// Type S display SPANs: not tabbable (empty chrome is CSS :empty::before only)
			if (element.tagName == 'SPAN' && element.getAttribute('exotype') == 'S')
				element.tabIndex = -1

			// F7/F6 chrome (dict di.popup / di.link → exoui_popup / exoui_link):
			//   non-empty string → real find/link icon + handler
			//   di.popup='' / di.link='' → pad that slot (with real other chrome, or alone)
			//   false/null → suppress (copydictitem: no attribute). Omit → nothing.
			//   never create a clickable icon for empty string.
			//   conversion SELECT free F7 only when popup slot is not pad (empty di.popup);
			//   otherwise pad-only would nest a second wrap and detach pads from the SELECT.
			var popupExpr = element.getAttribute('exopopup') || ''
			var linkExpr = element.getAttribute('exolink') || ''
			// pad only when dict set the property to empty (not when property omitted)
			var padPopup = element.hasAttribute('exopopup') && !popupExpr
			var padLink = element.hasAttribute('exolink') && !linkExpr
			// free F7 on SELECT (multivalue discoverability) — not when di.popup='' pad
			var freeSelectPopup = element.tagName == 'SELECT' && !padPopup
			var installedRealPopup = false
			var installedRealLink = false

			//add button before element with popups (and selects to make it clear to users that F7 is available - especially since useful when selecting multivalues)
			if (
				(
					freeSelectPopup
					||
					popupExpr
				)
				&&
				!element.getAttribute('exoreadonly')
				&&
				(
					element.type == 'text'
					||
					element.type == 'textarea'
					||
					element.isContentEditable
					||
					element.tagName == 'SELECT'
				)
			) {
				if (popupExpr || freeSelectPopup) {
					//conversion is a routine eg [await exofilepopup(filename,cols,coln,sortselect] [popup.clients]
					element = form_field_chrome_ensure_wrap(element, dictitem)
					installedRealPopup = true

					var element2 = exo_create_icon_element(
						fieldname.indexOf('DATE') >= 0 ? gcalendarimage : gfindimage
					)
					element2.id = fieldname + '_popup'
					element2.setAttribute('data-exo-host', fieldname)
					element2.style.flexShrink = '0'
					var strip = form_field_chrome_strip(element)
					if (strip)
						strip.appendChild(element2)
					// di.link='' → pad F6 slot (icon width only)
					if (padLink)
						form_field_chrome_pad(element, 'var(--exoui-icon-size)')

					element2.title = 'Find a' + ('aeioAEIO'.indexOf(element.getAttribute('exotitle').slice(0, 1)) != -1 ? 'n' : '') + ' ' + element.getAttribute('exotitle')
					element2.title += ' (F7)'
					element2.style.cursor = 'pointer'
					element2.setAttribute('isexopopup', '1')
				}
			}

			// F6 link icon in chrome strip (before host)
			if (linkExpr) {
				if (typeof element.getAttribute('exolink') != 'string') {
					systemerror('formfunction_onload', exoquote(fieldname) + ' link must be a string')
				}
				else {
					//conversion is a routine eg [await exofilepopup(filename,cols,coln,sortselect] [popup.clients]
					element = form_field_chrome_ensure_wrap(element, dictitem)
					installedRealLink = true

					var element2 = exo_create_icon_element(glinkimage)
					element2.setAttribute('data-exo-host', fieldname)
					element2.style.flexShrink = '0'
					// di.popup='' → pad F7 slot before link (e.g. DATELIST)
					if (padPopup)
						form_field_chrome_pad(element, 'var(--exoui-icon-size)')
					var strip = form_field_chrome_strip(element)
					if (strip)
						strip.appendChild(element2)

					element2.title = 'Open this ' + element.getAttribute('exotitle') + ' (F6)'
					element2.style.cursor = 'pointer'
					element2.setAttribute('isexolink', '1')
				}
			}

			// Pad-only (di.popup='' and/or di.link='' with no real F7/F6):
			//   Empty slots = icon width only. Skip if real F7/F6 already installed.
			if ((padPopup || padLink) && !installedRealPopup && !installedRealLink) {
				element = form_field_chrome_ensure_wrap(element, dictitem)
				var iconW = 'var(--exoui-icon-size)'
				if (padPopup)
					form_field_chrome_pad(element, iconW)
				if (padLink)
					form_field_chrome_pad(element, iconW)
			}

			//add image element and hide element
			if (element.getAttribute('exoimage')) {
				var element2 = document.createElement('img')
				element.parentNode.insertBefore(element2, element.nextSibling)
				element.style.display = 'none'
			}

			//use the data field name as the id and name of the element
			//NB the name appears to be lost on databinding table rows
			element.id = fieldname

			// Type S name after code with F7/F6: glue into prior .exofieldrow
			// unless a <br> (or other real content) deliberately separates them.
			// e.g. MARKET_CODE + MARKET_NAME; Brand uses <br> so stays stacked.
			if (element.tagName == 'SPAN' && element.getAttribute('exotype') == 'S')
				form_glue_name_to_prev_code_chrome(element)

			// conversion "color": text + swatch after id is set (swatch id = field_swatch)
			if (element.getAttribute('data-exo-color-pending') == '1') {
				element.removeAttribute('data-exo-color-pending')
				if (typeof colors_install_swatch == 'function')
					colors_install_swatch(element)
			}

			//NAME attribute cannot be set at run time on elements dynamically
			// created with the createElement method
			//element.name=element.name

			//group no
			//var groupno=parseInt(element.getAttribute('exogroupno'),10)
			var groupno = Number(element.getAttribute('exogroupno'))

			//align
			// right-align when dict left align empty and conversion is fixed-width numeric/date-ish
			// ([NUMBER…], [DATE…], [DATE_TIME…]). Not [TIME…] — helper keeps L for short times.
			if (
				!element.getAttribute('exoalign')
				&&
				typeof (element.getAttribute('exoconversion')) == 'string'
			) {
				var convU = element.getAttribute('exoconversion').toUpperCase()
				// Prefix match (same as form_field_exostyle) — not substring anywhere
				if (convU.indexOf('[NUMBER') === 0
					|| convU.indexOf('[DECIMAL') === 0
					|| convU.indexOf('[INTEGER') === 0
					|| convU.indexOf('[ROUND') === 0
					|| convU.indexOf('[DATE_TIME') >= 0
					|| /^\[DATE([,\]]|$)/.test(convU))
					element.setAttribute('exoalign', 'R')
			}
			//if (groupno>0&&element.getAttribute('exoalign')=='R'&&'THTD'.indexOf(element.parentNode.tagName)>=0)
			//if (element.getAttribute('exoalign')=='R'&&'THTD'.indexOf(element.parentNode.tagName)>=0)
			if (
				element.getAttribute('exoalign') == 'R'
				&&
				(
					groupno > 0
					||
					getancestor(element, 'TFOOT')
				)
			) {
				// Right-align field cell (tbody line or tfoot total).
				// style.textAlign beats TABLE.exoform { text-align:left }.
				var cellnode = getancestor(element, ' TD TH ')
				if (cellnode) {
					if (!cellnode.parentNode.align && !cellnode.align)
						cellnode.align = 'right'
					cellnode.style.textAlign = 'right'
					// Col title: tfoot = value cell only. Prefer id_title (same as sort).
					// Else Nth *visible* thead th for Nth *visible* body td — not cellIndex
					// (tbody-only hidden cols e.g. LINE_TAX_* shift cellIndex vs thead).
					var inFoot = !!getancestor(element, 'TFOOT')
					var titleelement = null
					if (!inFoot)
						titleelement = $$(element.id + '_title')
					if (!titleelement && !inFoot) {
						var bodyRow = cellnode.parentNode
						var gtable = getancestor(cellnode, 'TABLE')
						var gthead = gtable && gtable.tHead
						var titleRow = gthead && gthead.rows && gthead.rows.length
							? gthead.rows[gthead.rows.length - 1]
							: null
						if (bodyRow && bodyRow.cells && titleRow && titleRow.cells) {
							var visn = -1
							var found = false
							for (var bci = 0; bci < bodyRow.cells.length; bci++) {
								var bc = bodyRow.cells[bci]
								var bhid = bc.style.display == 'none'
									|| (bc.currentStyle && bc.currentStyle.display == 'none')
									|| (window.getComputedStyle && getComputedStyle(bc).display == 'none')
								if (bhid)
									continue
								visn++
								if (bc == cellnode) {
									found = true
									break
								}
							}
							if (found && visn >= 0) {
								var tvis = -1
								for (var tci = 0; tci < titleRow.cells.length; tci++) {
									var tc = titleRow.cells[tci]
									var thid = tc.style.display == 'none'
										|| (tc.currentStyle && tc.currentStyle.display == 'none')
										|| (window.getComputedStyle && getComputedStyle(tc).display == 'none')
									if (thid)
										continue
									tvis++
									if (tvis == visn) {
										titleelement = tc
										break
									}
								}
							}
						}
					}
					if (titleelement) {
						if (!titleelement.align)
							titleelement.align = 'right'
						titleelement.style.textAlign = 'right'
					}
				}
			}

			// align
			// Fix missing handing of right alignment for elements that no not
			// meet previous right alignment conditions e.g type URl or conversion
			// type is NUMBER. Done so that 'Other lang' Address fields can be right aligned
			// for Arabic text in E-invoicing enabled databases
			// Safety measure, dont interfere with existing assignments
			if (element.style.textAlign == '') {
				let tt = element.getAttribute('exoalign')

				if (tt.toUpperCase().startsWith('R'))
					element.style.textAlign = 'right'
				else if (tt.toUpperCase().startsWith('L') || tt.toUpperCase().startsWith('T'))
					element.style.textAlign = 'left'
			}

			//length and maxlength
			// INPUT paint width is set later (form_apply_input_field_width) after class/font.
			// Keep size = length only as a weak table hint — not +2, not the painted width.
			// length 0 = deliberate "no length" (legacy); treat like empty, not systemerror→10.
			if (element.tagName.match(gtexttagnames)) {
				if (element.size != 1 && element.getAttribute('exolength')) {
					var lenN = parseInt(element.getAttribute('exolength'), 10)
					if (lenN === 0) {
						element.setAttribute('exolength', '')
					} else if (!(lenN > 0)) {
						systemerror('formfunctions_onload()', element.id + '.getAttribute("exolength")=' + element.getAttribute('exolength') + ' is invalid. 10 used.')
						element.setAttribute('exolength', 10)
						element.size = 10
					} else {
						element.size = lenN
					}
				}
				if (element.tagName == 'TEXTAREA') {

					if (typeof CKEDITOR != 'undefined') {

						CKEDITOR.config.toolbarCanCollapse = true;
						CKEDITOR.config.toolbarStartupExpanded = false;
						CKEDITOR.config.enterMode = CKEDITOR.ENTER_BR;
						CKEDITOR.config.extraPlugins = 'autogrow';
						CKEDITOR.config.disableNativeSpellChecker = false;
						CKEDITOR.config.autoGrow_minHeight = 100;

						//CKEDITOR may not be ready until after first await opendoc2()
						CKEDITOR.on('instanceReady', function (event) {
							gCKEDITOR_EDITOR = event.editor
							gCKEDITOR_EDITOR.setReadOnly(!glocked)//also in await opendoc2() post read document_onfocus
							if (typeof exo_ckeditor_apply_theme == 'function')
								exo_ckeditor_apply_theme()
						})

						var verticalpercent = 100
						if (element.getAttribute('exorows'))
							verticalpercent *= element.getAttribute('exorows') / 10

						//var ockeditor = new CKEDITOR(element.id,'100%',verticalpercent+'%','EXODUS')

						//http://docs.cksource.com/Talk:CKEditor_3.x/Developers_Guide
						// contentsCss: DM override on first paint (no white flash before contentDom)
						var ckCfg = {
							extraPlugins: 'autogrow'
							, autoGrow_maxHeight: 800
						}
						if (typeof exo_ckeditor_contents_css == 'function')
							ckCfg.contentsCss = exo_ckeditor_contents_css()
						ockeditor = CKEDITOR.replace(element.id, ckCfg)

						// iframe body: re-theme after setData / focus (framework only)
						ockeditor.on('contentDom', function () {
							if (typeof exo_ckeditor_apply_theme == 'function')
								exo_ckeditor_apply_theme()
						})
						ockeditor.on('dataReady', function () {
							if (typeof exo_ckeditor_apply_theme == 'function')
								exo_ckeditor_apply_theme()
						})
						ockeditor.on('focus', function () {
							if (typeof exo_ckeditor_apply_theme == 'function')
								exo_ckeditor_apply_theme()
						})

						//element.id is passed as data on event to document_onfocus_sync
						//which calls document_onfocus to set/check gpreviouselement for changes
						ockeditor.on('focus', document_onfocus_sync, null, element.id)

						//doesnt seem to be needed (actually crashes out)
						//element.onfocus=onfocus
					}
					else {

						// Ordinary paint: fill cell like free-text entry SPAN;
						// length → min-width Nch floor (no length → 6ch anti-collapse).
						// Do not drive width with cols (that fixed preferred width and
						// held the form). cols=1 is a weak UA hint only.
						var taLen = parseInt(element.getAttribute('exolength'), 10)
						if (!(taLen > 0))
							taLen = 0
						element.cols = 1
						element.style.display = 'block'
						element.style.width = '100%'
						element.style.maxWidth = '100%'
						element.style.boxSizing = 'border-box'
						element.style.minWidth = taLen > 0 ? (taLen + 'ch') : '6ch'
						var exorows = element.getAttribute('exorows')
						if (exorows && exorows > 1)
							element.rows = exorows
					}
				}
				if (element.getAttribute('exomaxlength'))
					element.maxLength = parseInt(element.getAttribute('exomaxlength'), 10)
			}

			//lower case
			if (!(element.getAttribute('exolowercase'))) {
				if (element.tagName == 'SELECT'
					//exo_dict_text(di) now sets lowercase true but can be removed
					//to allow capitalised flowing text eg ratecard columns
					//|| element.getAttribute('exoalign') == 'T'
					|| element.getAttribute('exotype') == 'S') {
					element.setAttribute('exolowercase', true)
				}
				else {
					element.setAttribute('exolowercase', '')// not 'false' as user properties must be strings not boolean for row bound elements
				}
			}
			//force visual uppercase (actual conversion done in onbeforeupdate)
			if (
				!element.getAttribute('exolowercase')
				&&
				element.type != 'radio'
				&&
				element.type != 'checkbox'
			)
				element.style.textTransform = 'uppercase'

			//non calculated fields may be displayed as/converted to uppercase
			//if (element.getAttribute('exotype')=='F'&&element.tagName!='SPAN')
			if (element.getAttribute('exotype') == 'F') {

				// only focus elements that have tabindex
				//make them all the same and tab will work nicely
				//tabindex can also be hard coded in the form design
				//use <exo_tabindex_default to come before defaults and > to come after
				if (!element.tabIndex) {
					if (element.getAttribute('exoreadonly')) {
						element.tabIndex = -1
					}
					else {
						element.tabIndex = exo_tabindex_default
					}
				}

				await setfirstlastelement(element)

			}

			//calculated fields can display HTML
			else {
				try {
					element.dataFormatAs = 'HTML'
				}
				catch (e) {
					systemerror('formfunctions_onload()', element.name + ' should not be bound to an editable element ' + element.tagName)
				}
			}

			//clear any existing values (f5 refresh does not seem to clear databinding)
			/////but now we bind an empty record so this is not necessary
			// if (element.tagName.match(gdatatagnames))
			// {
			//  setvalue(element,'')
			// }

			//highlight required fields
			var elementclassname = ''
			//if (!element.className&&element.type!='radio'&&element.type!='checkbox')
			if (
				element.tagName.match(gdatatagnames)
				&&
				element.type != 'radio'
				&&
				element.type != 'checkbox'
			) {

				//spans are only input if .isContentEditable
				if (
					element.getAttribute('exotype') == 'F'
					&&
					!element.getAttribute('exoreadonly')
					&&
					(
						element.tagName != 'SPAN'
						||
						element.isContentEditable
					)
				) {
					elementclassname = 'clsNotRequired'
				}
				else {
					elementclassname = 'clsReadOnly'
				}

				if (
					(
						element.getAttribute('exorequired')
						||
						element.getAttribute('exofieldno') == '0'
					)
					//mark elements with default values as required even though probably no data entry if required
					//&&!element.getAttribute('exodefaultvalue')
					&& element.type != 'radio'
					&& element.type != 'checkbox'
				) {
					if (
						element.tagName != 'SELECT'
						||
						(
							element.tagName == 'SELECT'
							&&
							element.options[element.selectedIndex].value == ''
						)
					) {
						elementclassname = 'clsRequired'
					}
				}
			}
			//set the class
			if (elementclassname) {
				if (element.className)
					element.className = element.className + ' ' + elementclassname
				else
					element.className = elementclassname
			}

			// After clsRequired/clsReadOnly so measure uses final font.
			// Only INPUTs that remained INPUT (align T already converted to SPAN).
			form_apply_input_field_width(element)

			//handle groups

			//build a list of name elements
			var screenfn = gfields.length
			gfields[screenfn] = element

			//element.setAttribute('exo_screenfn',gfields.length-1)
			//why -1 ???
			element.setAttribute('exo_screenfn', screenfn)
			//gfields[screenfn].setAttribute('exo_screenfn',screenfn)

			var rowx = getancestor(element, 'TR')
			var tablex = getancestor(rowx, 'TABLE')

			// Leaf data cell: nearest td/th of a bound field (CSS .exodata = form body).
			// Skip structure hosts (exotable/group wrappers) — those stay base-tinted.
			var datacell = getancestor(element, ' TD TH ')
			if (datacell) {
				var dcn = ' ' + (datacell.className || '') + ' '
				if (dcn.indexOf(' exoembeddedtable ') < 0 && dcn.indexOf(' exodata ') < 0)
					datacell.className += (datacell.className ? ' ' : '') + 'exodata'
			}

			//set the datasrc of single valued elements
			if (groupno == 0) {

				//but allow it in THEAD/TFOOT elements!
				//prevent group 0 element in rows of a multivalued table
				//while (tablex)
				//{
				// if (Number(tablex.getAttribute('exogroupno'))) systemerror('formfunction_onload()',exoquote(element.name)+' is not multivalued and cannot be in a multivalued TABLE')
				// tablex=getancestor(tablex,'TABLE')
				//}

				//prevent group 0 element in rows of a multivalued table
				if (rowx && Number(rowx.getAttribute('exogroupno')))
					systemerror('formfunction_onload()', exoquote(element.name) + ' is not multivalued and cannot be in a multivalued TABLE')

				datasrcelements[datasrcelements.length] = element

				//elements with group no - set the data source of repeating lines
			}
			else {

				if (groupno > gmaxgroupno)
					gmaxgroupno = groupno

				//locate the table element in the parents
				if (tablex == null) {
					systemerror('formfunction_onload()', exoquote(element.name) + ' is multivalued and must exist inside a TABLE element')
				}

				//add sorting button to column title
				var titleelement
				if (1 && (titleelement = $$(element.id + '_title'))) {

					// Drop trailing text whitespace so the chevron sits on the label
					// (pretty-printed HTM often leaves "\n  " after the title words).
					var tlast = titleelement.lastChild
					if (tlast && tlast.nodeType == 3)
						tlast.nodeValue = String(tlast.nodeValue).replace(/\s+$/, '')

					var element2 = exo_create_icon_element(gsortimage)
					if (typeof exo_apply_sort_icon == 'function')
						element2 = exo_apply_sort_icon(element2, '') || element2
					titleelement.insertBefore(element2, null)

					element2.id = 'sortbutton_' + Number(element.getAttribute('exogroupno'))
					element2.title = 'Sort by ' + element.getAttribute('exotitle')
					//addeventlistener(element2, 'click', 'sorttable')
					element2.setAttribute('exo_onclick', 'await sorttable(event)')
					element2.sorttableelementid = element.id
					gsortimages[element.id] = element2

				}

				//any element can determine if emptyrows are allowed
				if (temp = element.getAttribute('exoallowemptyrows'))
					tablex.setAttribute('exoallowemptyrows', temp)

				//any element can determine if a row is required
				if (temp = element.getAttribute('exorowrequired'))
					tablex.setAttribute('exorowrequired', temp)

				//need to know the last data entry column
				if (!element.getAttribute('exoreadonly') && element.tagName.match(gdatatagnames)) {
					//spans are only input if .isContentEditable
					if (element.tagName != 'SPAN' || element.isContentEditable) {
						tablex.setAttribute('exo_lastinputcolscreenfn', element.getAttribute('exo_screenfn'))
						//element.setAttribute('exo_islastinputcolumn',true)
					}
				}
				//nb textarea.isContentEditable is false! so need to test for .type='textarea' or .tagName

				//need to know the first data entry column
				if (
					!tablex.getAttribute('exo_firstinputcolscreenfn')
					&& !element.getAttribute('exoreadonly')
					&& element.tagName.match(gdatatagnames)
				) {
					//spans are only input if .isContentEditable
					if (
						element.tagName != 'SPAN'
						|| element.isContentEditable
					) {
						tablex.setAttribute('exo_firstinputcolscreenfn', element.getAttribute('exo_screenfn'))
						element.setAttribute('exo_isfirstinputcolumn', true)
					}
				}

				//prevent use of separator characters unless multiword
				if (element.tagName != 'SELECT') {
					var wordsep = element.getAttribute('exowordsep')
					if (wordsep && element.getAttribute('exonwords') <= 1) {
						var invalidchars = element.getAttribute('exoinvalidcharacters')
						if (!invalidchars)
							invalidchars = ''
						element.setAttribute('exoinvalidcharacters', invalidchars + wordsep)
					}
				}

				//table already bound and used for group elements
				if (Number(tablex.getAttribute('exogroupno'))) {

					//check that all fields in same table have same group no
					//zzz should also check that the table has no other name in subtables
					if (Number(tablex.getAttribute('exogroupno')) != groupno) {
						systemerror('formfunctions_onload()', element.name + ' is in group ' + groupno + ' but the table has already been bound to group ' + Number(tablex.getAttribute('exogroupno')))
						return
					}

					//tablex.ncols++
					//element.coln=tablex.ncols
					gtables[groupno][gtables[groupno].length] = screenfn

					//allow finding table element via groupno
					gtables[groupno].tablex = tablex

					async function maybe_remove_rowbutton(insertdelete) {
						if (
							element.getAttribute('exono' + insertdelete + 'row')
							&& !tablex.getAttribute('no' + insertdelete + 'row')
						) {
							tablex.setAttribute('no' + insertdelete + 'row', true)
							exoremoveelementsbyid(insertdelete + 'rowbutton' + groupno)
						}
					}

					//maybe remove insertrow/deleterow buttons (in case first group dictionary element does not have the flag)
					//if (element.getAttribute('exonoinsertrow') && !tablex.getAttribute('noinsertrow')) {
					//if (element.getAttribute('exonodeleterow')&& !tablex.getAttribute('nodeleterow')) {
					await maybe_remove_rowbutton('insert')
					await maybe_remove_rowbutton('delete')
					// Both buttons gone: residual lead-in stays for clone/col align; hide via
					// table.exogroup_col0_hide (CSS). Tag cells if build path missed them.
					if (tablex.getAttribute('noinsertrow') && tablex.getAttribute('nodeleterow')
						&& !tablex.querySelector('[id^="insertrowbutton"], [id^="deleterowbutton"]')) {
						form_group_tag_col0(tablex, groupno)
						tablex.classList.add('exogroup_col0_hide')
					}

				}
				else {

					//first column is required
					//this should perhaps not be set since we have rowrequired and allowemptyrows
					//element.setAttribute('exorequired',true)

					//check this groupno not used on other tables
					if (gtables[groupno]) {
						systemerror('formfunctions_onload()', element.name + ' is in group ' + groupno + ' but that group is also used in another table by ' + gfields[gtables[groupno][0]].id)
						return
					}

					//tablex.ncols=0
					gtables[groupno] = []
					gtables[groupno][0] = screenfn
					gtables[groupno].tableelement = tablex
					tablex.setAttribute('exogroupno', groupno)
					tablex.setAttribute('name', 'group' + groupno)
					tablex.id = 'exogroup' + groupno
					datasrcelements[datasrcelements.length] = tablex
					tablex.setAttribute('exo_dependents', '')

					// Embedded group table inside an outer exoform cell — drop host row/cell inline borders
					var hostcell = tablex.parentNode
					if (hostcell && hostcell.tagName == 'TD') {
						hostcell.className += (hostcell.className ? ' ' : '') + 'exoembeddedtable'
						hostcell.style.removeProperty('border')
						var hostrow = hostcell.parentNode
						if (hostrow && hostrow.tagName == 'TR') {
							hostrow.style.removeProperty('border')
							hostrow.style.removeProperty('border-top')
							hostrow.style.removeProperty('border-bottom')
						}
					}

					//unfortunately this can only be set on the first element in the row at the moment
					//TODO process tables after processing all fields
					//add insert/delete buttons at the end instead of on the first table element discovered
					if (element.getAttribute('exonoinsertrow'))
						tablex.setAttribute('noinsertrow', 'noinsertrow')
					if (element.getAttribute('exonodeleterow'))
						tablex.setAttribute('nodeleterow', 'nodeleterow')

					//      if (!tablex.className) tablex.className='exoform'
					//      tablex.border=1

					//capture all double clicks for potential filtering
					//tablex.ondblclick=form_ondblclick
					addeventlistener(tablex, 'dblclick', 'form_ondblclick')
					//tablex.oncontextmenu=form_onrightclick

					//tablex.onreadystatechange=tablex_onreadystatechange
					addeventlistener(tablex, 'readystatechange', 'tablex_onreadystatechange')

					//mark group in row to allow check/prevent group0 in same row
					rowx.setAttribute('exogroupno', groupno)

					//add insert and delete row buttons at the first column in the tbody

					// duplicate keycodes in 3 places
					var t2 = '(Ctrl+I or Ctrl+Insert)'
					var t3 = '(Ctrl+D or Ctrl+Delete)'
					var hasIns = !(element.getAttribute('exonoinsertrow'))
					var hasDel = !(element.getAttribute('exonodeleterow'))
					// Lead-in col 0: ins/del, Show All, filter. Shared class for CSS hide.
					var col0class = 'exogroup_col0 exogroup' + groupno + '_col0'
					// Nest fill vs left-pack — form_group_needs_nest_fill; respect HTM width hardcode.
					// left-pack = size to content / not fill 100% of host (FORM-UI-WIDE-NARROW.md).
					var hardW = form_table_hardcoded_width(tablex)
					var nestfill = form_group_needs_nest_fill(groupno)
					// style width max-content|fit-content|min-content → force left-pack (no col0 1%)
					if (form_table_hardcode_is_hug(hardW))
						nestfill = false
					// Half 2: left-pack — only if author did not already set width
					if (!nestfill && !hardW)
						tablex.style.width = 'max-content'
					var t = ''
					t += '<span style="white-space: nowrap">'
					//if (!(exogetattribute(element,'exonoinsertrow')))
					if (hasIns) {
						t += exo_icon_html(ginsertrowimage, null,
							' id="insertrowbutton' + groupno + '"'
							+ ' title="Insert a new row here ' + t2 + '"'
							+ ' exo_onclick="await insertrow_onclick(event)"'
							+ ' style="cursor:pointer;vertical-align:top"')
					}
					//if (!(exogetattribute(element,'exonodeleterow')))
					if (hasDel) {
						t += exo_icon_html(gdeleterowimage, null,
							' id="deleterowbutton' + groupno + '"'
							+ ' title="Delete this row ' + t3 + '"'
							+ ' exo_onclick="await deleterow_onclick(event)"'
							+ ' style="cursor:pointer;vertical-align:top"')
					}
					t += '</span>'
					var insertdeletebuttons = document.createElement('td')
					insertdeletebuttons.className = col0class
					insertdeletebuttons.innerHTML = t
					insertdeletebuttons.style.borderRightWidth = '0px'
					// Half 1: col0 width=1% only when fill (free-text entry, not HTM left-pack hardcode).
					// Full theory: FORM-UI-WIDE-NARROW.md § Multivalue col0 / Nest left-pack override.
					if (nestfill)
						insertdeletebuttons.width = '1%'
					if (hasIns || hasDel)
						insertdeletebuttons.style.paddingRight = '3px'

					//locate the TR element in the parents
					var trx = getancestor(element, 'tr')
					if (trx == null)
						systemerror('formfunction_onload()', exoquote(element.name) + ' is multivalued and must exist inside a TABLE element')

					trx.insertBefore(insertdeletebuttons, trx.firstChild)

					//add page up/down buttons at the first column in the thead and tfoot
					var pgupdownbuttons = document.createElement('th')
					pgupdownbuttons.className = col0class
					if (nestfill)
						pgupdownbuttons.width = '1%'
					var t = ''
					t += '<button id=exogroup' + groupno + 'showall class=exobutton'
					t += ' style=display:none exo_onclick="await form_filter(\'unfilter\',' + groupno + ')"'
					t += '>Show All</button>'

					pgupdownbuttons.innerHTML = t

					//locate the THEAD element in the parents
					var thx = tablex.getElementsByTagName('thead')[0]
					if (thx) {

						//allow for multiple rows in thead
						//macie needs to be updated AFTER insertion of element
						//pgupdownbuttons.rowSpan=thx.children.length
						if (!thx.firstChild) {
							//ignore thead with no rows
							//systemerror('Missing table structure for multivalued field?')
							thx = null
						}
						else {
							var thxr = thx.getElementsByTagName('tr')[0]
							//var thxrd=thxr.getElementsByTagName('td')[0]
							var thxrd = thxr.firstChild
							thxr.insertBefore(pgupdownbuttons, thxrd)
							//pgupdownbuttons.rowSpan=thx.childNodes.length
							pgupdownbuttons.rowSpan = thx.rows.length
						}
					}

					//locate the TFOOT element in the parents
					// Spacer only (column align with thead Show All cell) — th like thead
					// lead-in (base bgcolor); was td to avoid th tint (obsolete with inversion).
					var tfx = tablex.getElementsByTagName('tfoot')[0]
					if (tfx) {
						var tfxr = tfx.getElementsByTagName('tr')[0]
						if (tfxr) {
							var footspacer = document.createElement('th')
							footspacer.className = col0class
							if (nestfill)
								footspacer.width = '1%'
							footspacer.innerHTML = ''
							tfxr.insertBefore(footspacer, tfxr.firstChild)
							footspacer.rowSpan = tfx.rows.length
						}
					}

					// Residual col0 (no ins/del): hide all col0 cells via CSS class on table
					if (!(hasIns || hasDel))
						tablex.classList.add('exogroup_col0_hide')

				}
			}
		} //name element
	}

	//set classname so that we can find multiple elements (screen fields) for the same database field name
	//(which occurs in the case of multivalues (rows), radio buttons and checkboxes)
	//using document.getElementsByClassName()
	id2classname()

	//zero all the dependents (nb exo_dependents is used in gds.js once)
	for (var ii = 0; ii < gfields.length; ii++)
		gfields[ii].setAttribute('exo_dependents', '')

	//for each calculated field add its field number
	// to all elements whose name (in double or single quotes)
	// appear in its functioncode
	for (var fieldn = 0; fieldn < gfields.length; fieldn++) {
		//with (gfields[fieldn])
		var field = gfields[fieldn]

		var deplist = ''
		//   alert(gfields[fieldn].outerHTML)
		if (field.getAttribute('exotype') == 'S') {
			var functioncode = field.getAttribute('exofunctioncode').toString()
			for (var fieldn2 = 0; fieldn2 < gfields.length; fieldn2++) {
				var field2 = gfields[fieldn2]
				if (functioncode.indexOf('"' + field2.getAttribute('exoname') + '"', 0) >= 0
					|| functioncode.indexOf("'" + field2.getAttribute('exoname') + "'", 0) >= 0) {
					var deps = field2.getAttribute('exo_dependents')
					if (deps)
						deps += ';'
					else
						deps = ''
					deps += fieldn
					field2.setAttribute('exo_dependents', deps)

					if (
						Number(field2.getAttribute('exogroupno'))
						&&
						Number(field.getAttribute('exogroupno')) != Number(field2.getAttribute('exogroupno'))
					) {
						var tablex = getancestor(field2, 'TABLE')
						var deps = tablex.getAttribute('exo_dependents')
						if (deps)
							deps += ';'
						else
							deps = ''
						deps += fieldn
						tablex.setAttribute('exo_dependents', deps)
					}

				}
			}
		}
	}

	//get copies of the gfields in case they are deleted from the document (eg table row 1 is deleted)
	for (var fieldn = 0; fieldn < gfields.length; fieldn++) {
		var id = gfields[fieldn].id
		gfields[fieldn] = gfields[fieldn].cloneNode(false)
		gfields[fieldn].id = id
	}

	if (!gfirstnonkeyelement)
		gfirstnonkeyelement = gfields[0]
	if (!gfirstnonkeyelement) {
		systemerror('formfunctions_onload()', 'There are no non-key elements in the form or no data elements')
		return
	}

	//do not do the following otherwise if a grouped checkbox is the firstnokeyelement
	//then there is a failure in getvalue because it cannot find the parentNode
	//name=gfirstnonkeyelement.name
	//gfirstnonkeyelement=gfirstnonkeyelement.cloneNode(false)
	//gfirstnonkeyelement.name=name

	//when user enters the key, get a record

	//add the open, save, close and delete buttons

	// Form chrome shortcuts use event.altKey (tooltip: Alt+…)
	var AltorCtrl = 'Alt'

	var buttonhtml = ''

	gformbuttonsplace = form_formbuttons_place()
	// Unbound (no keys): OK/Save + Close are real actions → raised .graphicbutton
	// (same face if relocated from bottom into the menubar).
	// Bound: New/Open/Save/nav… live as flat menubar tools → .menubutton
	// (Menu/Refresh/Logout are always flat menubutton too).
	var formActionAlign = gKeyNodes ? 'left' : 'center'

	//wrap form buttons in a span so they align the same as the menu, logout and refresh buttons
	function menubuttonhtml2(id, imagesrc, name, title, accesskey) {
		return '<span>' + menubuttonhtml(id, imagesrc, name, title, accesskey, formActionAlign) + '</span>'
	}

	//bound form buttons NEW/OPEN/EDIT-RELEASE
	if (gKeyNodes) {
		buttonhtml += menubuttonhtml2('newrecord', gnewimage, '<u>N</u>ew', 'Create a new document. ' + AltorCtrl + '+N', 'N')
		buttonhtml += menubuttonhtml2('openrecord', gopenimage, '<u>O</u>pen', 'Open an existing document. ' + AltorCtrl + '+O', 'O')
		buttonhtml += menubuttonhtml2('editreleaserecord', greleaseimage, 'R<u>e</u>lease', 'Edit/Release the current document. ' + AltorCtrl + '+E', 'E')
	}

	//bound and unbound form buttons have OK/SAVE and CLOSE buttons
	buttonhtml += menubuttonhtml2('saverecord', gsaveimage, '<u>S</u>ave', 'Save the current document. ' + AltorCtrl + '+S, Ctrl+Enter or F9', 'S')
	buttonhtml += menubuttonhtml2('closerecord', gcloseimage, '<u>C</u>lose', 'Close the current document. ' + AltorCtrl + '+C', 'C')

	//bound form buttons
	if (gKeyNodes) {

		//COPY
		if (typeof form_copyrecord == 'function')
			buttonhtml += menubuttonhtml2('copyrecord', gcopyimage, 'Copy', 'Copy the current document.', '')

		//DELETE
		//Alt+D not allowed in IE or FF since it goes to the address bar
		if (typeof gpreventdeletion == 'undefined' || gpreventdeletion != false) {
			//buttonhtml += menubuttonhtml2('deleterecord', gdeleteimage, '<u>D</u>elete', 'Delete the current document. ' + AltorCtrl + '+D', 'D')
			buttonhtml += menubuttonhtml2('deleterecord', gdeleteimage, 'Delete', 'Delete the current document.', '')
		}

		//PRINT
		if (gKeyNodes[0].getAttribute('exoprintfunction'))
			buttonhtml += menubuttonhtml2('printsendrecord', gprintsendimage, '<u>P</u>rint/Send', 'Print/Send this or these documents. ' + AltorCtrl + '+P', 'P')

		//LIST
		var tt2 = gKeyNodes[0].getAttribute('exolistfunction')
		if (tt2)
			buttonhtml += menubuttonhtml2('listrecord', glistimage, '<u>L</u>ist', 'List the current file. ' + AltorCtrl + '+L', 'L')

		//NAVIGATION multirecord — one group; CSS gap owns spacing (icon-only: empty label)
		buttonhtml += '<span class="exo_recordnav_group">'
		buttonhtml += menubuttonhtml2('firstrecord', gfirstimage, '', 'Open the first document. ' + AltorCtrl + '+{', '{')
		buttonhtml += menubuttonhtml2('previousrecord', gpreviousimage, '', 'Open the previous document. ' + AltorCtrl + '+[', '[')
		// Text only ("n of m"); CSS gap matches icon buttons — no spacer img
		buttonhtml += menubuttonhtml2('selectrecord', '', ' ', 'Select document. ' + AltorCtrl + '+^', '^')
		buttonhtml += menubuttonhtml2('nextrecord', gnextimage, '', 'Open the next document. ' + AltorCtrl + '+]', ']')
		buttonhtml += menubuttonhtml2('lastrecord', glastimage, '', 'Open the last document. ' + AltorCtrl + '+}', '}')
		buttonhtml += '</span>'

	}

	// Form action bar: top menubar, or under the form (see form_formbuttons_place)
	var formbuttons = document.createElement(gformbuttonsplace === 'top' ? 'SPAN' : 'DIV')
	formbuttons.id = 'formbuttonsdiv'
	formbuttons.innerHTML = buttonhtml

	if (gformbuttonsplace === 'top') {
		add_exo_menubar()
		// Form actions on the left; session + clear:left appended in form_place_menubar_session
		gexo_menubar.insertBefore(formbuttons, gexo_menubar.firstChild)
		if (typeof adjust_bodymargin == 'function')
			adjust_bodymargin()
	} else {
		formbuttons.className = 'exoformactions'
		document.body.insertBefore(formbuttons, null)
	}

	// Database name + username on menubar right (left of theme/logout). clear:left last.
	form_place_menubar_session()

	//make global variables to correspond to the buttons
	//to provide backward compatibility with IE code which can refer to document elements like global variables
	var buttonnames = ['new', 'open', 'editrelease', 'save', 'close', 'delete', 'copy', 'list', 'printsend', 'first', 'last', 'select', 'next', 'previous']
	for (var buttonn = 0; buttonn < buttonnames.length; ++buttonn) {
		buttonname = buttonnames[buttonn] + 'record'
		buttonelement = $$(buttonname + 'button')
		window[buttonname] = buttonelement
	}

	//program the various buttons to be visible when enabled
	exosetexpression(saverecord, 'style:display', 'saverecord.getAttribute("disabled")?"none":""')
	exosetexpression(closerecord, 'style:display', 'closerecord.getAttribute("disabled")?"none":""')

	//program printsend button to be invisible when disabled
	if (printsendrecord)
		exosetexpression(printsendrecord, 'style:display', 'printsendrecord.getAttribute("disabled")?"none":""')

	//program new/release/delete buttons to be invisible if disabled
	if (gKeyNodes) {
		exosetexpression(newrecord, 'style:display', 'newrecord.getAttribute("disabled")?"none":""')
		exosetexpression(editreleaserecord, 'style:display', 'editreleaserecord.getAttribute("disabled")?"none":""')
		if (deleterecord)
			exosetexpression(deleterecord, 'style:display', 'deleterecord.getAttribute("disabled")?"none":""')
	}

	if (firstrecord)
		await setgkeys([])

	if (newrecord && (greadonlymode || gupdateonlymode || gpreventcreation)) {
		setdisabledandhidden(newrecord, true)
		newrecord.title = 'Creating new records is not allowed'
	}
	if (deleterecord && (greadonlymode || gupdateonlymode || gpreventdeletion)) {
		setdisabledandhidden(deleterecord, true)
		deleterecord.title = 'Deleting records is not allowed'
	}

	var openfunction = ''
	var popupfunction = ''
	if (gKeyNodes) {
		var nvisiblekeys = 0
		for (var keyn = 0; keyn < gKeyNodes.length; keyn++) {
			//find the first openfunction (visible or not)
			if (openfunction = gKeyNodes[keyn].getAttribute('exoopenfunction'))
				break
			if (exoenabledandvisible(gKeyNodes[keyn])) {
				//count the number of visible keys
				nvisiblekeys++
				//remember the first visible non-empty popupfunction ("" is pad-only)
				if (!popupfunction) {
					var keypop = gKeyNodes[keyn].getAttribute('exopopup') || ''
					if (keypop)
						popupfunction = keypop
				}
			}
		}
		//if no openfunction and only one visible key with a popup function
		//then use the popup function as the open function
		if (
			!openfunction
			&& nvisiblekeys == 1
			&& popupfunction
		)
			openfunction = popupfunction
	}
	//hide the open button if no openfunction
	if (openfunction) {
		openrecord.setAttribute('exopopup', openfunction)
		openrecord.style.display = ''
	}
	else if (openrecord)
		openrecord.style.display = 'none'

	//remove record orientated buttons if no key fields
	if (!gKeyNodes) {

		// Unbound OK/Cancel: field-nav stop (DOM order; not historical 9998/99998)
		saverecord.tabIndex = 0
		closerecord.tabIndex = 0

		/*
		newrecord.style.display='none'
		openrecord.style.display='none'
		deleterecord.style.display='none'
		editreleaserecord.style.display='none'
		//firstrecord.style.display='none'
		//nextrecord.style.display='none'
		//previousrecord.style.display='none'
		//lastrecord.style.display='none'
		*/

		//saverecord.value='OK'
		//saverecord.value='OK'
		if (typeof gdisableok != 'undefined' && gdisableok) {
			setdisabledandhidden(saverecord, true)
			//saverecord.style.display='none'
		}
		else {
			setdisabledandhidden(saverecord, false)
			setgraphicbutton(saverecord, 'OK')
			saverecord.title = 'OK - Continue. ' + (gfields.length > 1 ? 'Ctrl+' : '') + 'Enter or F9'
			//saverecord.style.display=''
			saverecord.style.marginRight = '5px'
		}

		//change order of buttons ?
		//saverecord.swapNode(closerecord)
		//following does not work so comment out
		//var tempnode=saverecord.parentNode.replaceChild(closerecord,saverecord)
		//closerecord.parentNode.replaceChild(tempnode,closerecord)

		//no cancel button on main non-database forms
		//if (!gparameters.cancel&&typeof(form_write)=='function'||!window.dialogArguments)
		if (!gparameters.cancel && !window.dialogArguments) {
			setdisabledandhidden(closerecord, true)
		}
		else {
			// Same accesskey C as bound Close (menubuttonhtml2 … 'C'); show it on the tip.
			var AltorCtrl = 'Alt'
			setgraphicbutton(closerecord, '<u>C</u>ancel')
			closerecord.title = 'Cancel and exit. ' + AltorCtrl + '+C or Esc'
			setdisabledandhidden(closerecord, false)
		}

	}

	var temp = document.createElement('div')
	//temp.innerHTML='<p align=center style="margin-top:5px"><small>Software by <a tabindex="-1" href="http://www.neosys.com">EXODUS.COM</a></small></p>'
	document.body.insertBefore(temp, null)

	//start focused on the first key field
	if (gKeyNodes) {
		gstartelement = gKeyNodes[0]
		if (!(exoenabledandvisible(gstartelement)))
			gstartelement = gfirstnonkeyelement
	}
	else
		gstartelement = gfirstnonkeyelement

	//set the document titles from the parameters
	if (gparameters.documenttitle) {
		document.title = gparameters.documenttitle
		gdoctitle = document.title
	}

	//if form has a custom postinit routine
	if (typeof form_postinit == 'function') {
		//login('form_postinit before')
		var postinitok = await exoevaluate('await form_postinit()', 'form_functions()');
		//logout('form_postinit after')

		// Explicit false = abort (e.g. Cancel on a setup prompt). Do not set ginitok
		// or continue load — otherwise the form appears ready with empty/wrong data.
		if (postinitok === false) {
			if (window.dialogArguments || (typeof gisdialog != 'undefined' && gisdialog))
				exoui_windowclose()
			return false
		}

		//reverse the effect of any setvalue commands in postinit
		settouched(false)

	}

	//set the title hx element
	gdoctitle = document.title
	var temp = $$('documenttitle')
	if (temp && temp.tagName)
		temp.innerHTML = document.title

	//many routines will exit immediately if this is not set (to avoid further error messages)
	ginitok = true
	wstatus('')

	//yielding code cannot stop to ask questions in onbeforeunload
	window.onbeforeunload = window_onbeforeunload_sync

	//unstoppable - unlocks gkey if glocked
	//addeventlistener(window, 'unload', 'window_onunload')
	window.onunload = window_onunload_sync

	addeventlistener(document, 'keydown', 'document_onkeydown')
	// Alt+Up/Down pan; Alt+Left/Right pan only if wide (else browser history).
	form_ensure_scroll_viewport_capture()
	// Radio/checkbox: focus on mouseup only (not mousedown)
	form_ensure_radio_mouseup_focus()
	addeventlistener(document, 'click', 'document_onclick')

	addeventlistener(document, 'paste', 'document_onpaste')

	addeventlistener(document, 'copy', 'document_oncopy')

	// input: document-level so multivalue row cloneNode still gets form_oninput
	// (attributes copy; listeners do not). Install once per page.
	if (!gform_oninput_delegated) {
		gform_oninput_delegated = true
		addeventlistener(document, 'input', 'form_oninput')
		// document_onpaste already re-invokes form_oninput after single-line paste
	}

	//record based forms
	if (gKeyNodes) {
		if (gparameters.key) {

			//allow multiple keys to be loaded
			// keyn 0: gkey still empty here so setgkeys cannot locate — same as F7 setgkeys(reply, 0)
			if (typeof gparameters.key == 'object') {
				await setgkeys(gparameters.key, 0)
				gparameters.key = gparameters.key[0]
			}

			//necessary to initialise gds else any expressions relying on gds.data will
			//crash after this routine returns
			await cleardoc()

			//exosettimeout('await opendoc(' + exoquote(gparameters.key.replace(/\\/g, '\\\\')) + ')', 1)
			//we cant allow another event like focus to occur before this event is over
			//because there is only one geventhandler to rememeber which yielding function is pending resumption
			//therefore call opendoc immediately - seems to cause no problem
			await opendoc(gparameters.key.replace(/\\/g, '\\\\'))
		}
		else
			await cleardoc()
	}

	//non-record based forms can get parameters from URL or dialog arguments
	else {

		//initial form can come from a revstr
		if (gparameters.defaultrevstr)
			gro.defaultrevstr = gparameters.defaultrevstr

		// Framework open prefill (this flight only):
		// 1) cleardoc — empty load (full calcfields once), gparameters setx, form_postdisplay
		// 2) filldefaults — cell.text only + mark exo_dependents into gdependents
		// 3) limited calcfields(gdependents) then updatedisplay (not a second full calc)
		// That sequence is complete when the awaits below return. We do not detect
		// later custom work (e.g. leave-field validation that expands SCHEDULE_NO);
		// that is outside this open path. Same idea as settouched(false) after
		// form_postinit and after record cleardoc: machine-filled open is not a user edit.
		await cleardoc()

		await validateall('filldefaults')

		grecn = null
		// Only fields marked by setx (gparameters) or filldefaults dependents.
		// Full calc already ran in cleardoc via gds_onreadystatechange.
		await calcfields(gdependents)
		gdependents = []
		await updatedisplay()

		// Unbound save stays enabled (settouched: savebuttonactive || !gKeyNodes).
		// Forms that need touched after open call settouched(true) themselves
		// (e.g. draft keep after preview write).
		settouched(false)

		// filldefaults/validation can finish with no focus (e.g. passed DOCUMENT_NO
		// accepted without multi-hit decide). Land on start/first field.
		form_ensure_open_focus()

	}

	if (gparameters.savebuttonvalue) {
		//saverecord.value=gparameters.savebuttonvalue
		setgraphicbutton(saverecord, gparameters.savebuttonvalue)
	}

	// After form_postdisplay (and form_add_action_button): if unbound put the bar
	// under the form but it ended below the fold, keep it in the top menubar.
	form_keep_action_buttons_on_screen()

	// Page buttons with accesskey="1" etc. — map for Alt+digit (native accesskey is unreliable)
	form_register_accesskeys()

	//logout('formfunctions_onload')

}

////////////////// FORM DIGIT ACCESSKEYS (Alt+0…9) /////////////////////
//
// Historical IE: HTML accesskey on real buttons worked with Alt+digit.
//
// Dual handling: if content script sees Alt+digit, it is not an uncancellable
// chrome shortcut (those never reach the page). The second consumer is the
// browser’s *native* accesskey processing on the same control — so we both
// activate and the UA may still run accesskey / focus chrome.
//
// Fix: (1) register into gformdigitaccesskeys, (2) move accesskey →
// data-exo-accesskey so the UA no longer owns the key, (3) native capture
// keydown (outside Gate A) preventDefaults and activates via Gate A.
// Tooltips are not parsed. Letter shortcuts stay on the Alt+letter handlers.

// True if element is shown and not disabled (at press time or when registering).
function form_accesskey_usable(element) {

	if (!element || !element.getAttribute)
		return false

	if (element.disabled)
		return false
	// setdisabledandhidden sets attribute disabled; empty string still means disabled
	if (element.getAttribute('disabled') != null)
		return false

	if (element.style && element.style.display == 'none')
		return false

	// Hidden 1×1 / display:none capture stubs and truly collapsed nodes
	if (!element.offsetWidth || !element.offsetHeight)
		return false

	return true
}

// Element that should receive activation (exo_onclick host, or the control itself).
function form_accesskey_action_target(element) {

	var target = element
	while (target && target !== document && target !== document.body) {
		if (target.getAttribute && target.getAttribute('exo_onclick'))
			return target
		target = target.parentNode
	}

	var tag = element.tagName
	if (tag == 'BUTTON' || tag == 'A')
		return element
	if (tag == 'INPUT') {
		var typ = (element.type || '').toLowerCase()
		if (typ == 'button' || typ == 'submit' || typ == 'image' || typ == 'reset')
			return element
	}

	return null
}

// Prefer a usable visible control over a later duplicate accesskey (or hidden twin).
function form_accesskey_prefer(existing, candidate) {

	var exOk = form_accesskey_usable(existing) && form_accesskey_action_target(existing)
	var caOk = form_accesskey_usable(candidate) && form_accesskey_action_target(candidate)
	if (caOk && !exOk)
		return true
	return false
}

// Read digit from accesskey or from data-exo-accesskey after we disarm the UA.
function form_accesskey_digit_attr(element) {

	if (!element || !element.getAttribute)
		return ''
	var raw = element.getAttribute('data-exo-accesskey')
	if (raw == null || raw === '')
		raw = element.getAttribute('accesskey')
	if (raw == null)
		return ''
	raw = String(raw).replace(/^\s+|\s+$/g, '')
	if (raw.length != 1 || raw < '0' || raw > '9')
		return ''
	return raw
}

// Build gformdigitaccesskeys from digit accesskeys in the document.
// Disarm native accesskey (→ data-exo-accesskey) so only our handler fires.
// Call after form DOM is ready and after form_add_action_button.
function form_register_accesskeys() {

	var map = Object.create(null)
	// Include already-disarmed controls from a prior register pass
	var nodes = document.querySelectorAll('[accesskey], [data-exo-accesskey]')

	for (var i = 0; i < nodes.length; i++) {
		var el = nodes[i]
		var raw = form_accesskey_digit_attr(el)
		if (!raw)
			continue
		if (!form_accesskey_action_target(el))
			continue
		// Prefer usable candidates when choosing among duplicates.
		if (map[raw] && !form_accesskey_prefer(map[raw], el))
			continue
		map[raw] = el
	}

	// Disarm UA accesskey on winners (and digit accesskeys we skipped as losers
	// still keep accesskey — disarm all digit accesskeys we considered usable targets)
	for (var j = 0; j < nodes.length; j++) {
		var el2 = nodes[j]
		var dig = form_accesskey_digit_attr(el2)
		if (!dig)
			continue
		if (!form_accesskey_action_target(el2))
			continue
		// Always store canonical digit; remove HTML accesskey so browser native
		// accesskey path cannot also run when content handles the key.
		el2.setAttribute('data-exo-accesskey', dig)
		if (el2.getAttribute('accesskey') != null)
			el2.removeAttribute('accesskey')
	}

	gformdigitaccesskeys = map
	form_ensure_digit_accesskey_capture()
	return map
}

// Native capture keydown once — must NOT go through addeventlistener/Gate A.
function form_ensure_digit_accesskey_capture() {

	if (gformdigitaccesskey_capture_installed)
		return
	if (!document.addEventListener)
		return
	gformdigitaccesskey_capture_installed = true
	// Capture phase, sync. keyup too: some UAs still apply accesskey residual on keyup.
	document.addEventListener('keydown', form_digit_accesskey_capture_keydown, true)
	document.addEventListener('keyup', form_digit_accesskey_capture_keyup, true)
}

// keyCode/which → '0'…'9' for main keyboard or numpad; empty if not a digit.
function form_accesskey_digit_from_event(event) {

	var kc = event.keyCode ? event.keyCode : event.which
	if (kc >= 48 && kc <= 57)
		return String.fromCharCode(kc)
	if (kc >= 96 && kc <= 105)
		return String.fromCharCode(kc - 96 + 48)
	if (event.key && event.key.length == 1 && event.key >= '0' && event.key <= '9')
		return event.key
	return ''
}

// True if this form registered at least one digit accesskey (uses Alt+0…9 scheme).
function form_has_digit_accesskeys() {

	if (!gformdigitaccesskeys)
		return false
	for (var d in gformdigitaccesskeys)
		return true
	return false
}

// Sync lookup: first *usable* control for this digit (document order).
// Hidden/disabled siblings with the same accesskey are registered but ignored
// until they become usable — so hidden schedule buttons stay wired.
function form_lookup_digit_accesskey_element(event) {

	if (!gformdigitaccesskeys)
		return null

	var digit = form_accesskey_digit_from_event(event)
	if (!digit)
		return null

	// Fast path: preferred map entry if still usable
	var preferred = gformdigitaccesskeys[digit]
	if (preferred
		&& (!document.contains || document.contains(preferred))
		&& form_accesskey_usable(preferred)
		&& form_accesskey_action_target(preferred))
		return preferred

	// Scan all controls sharing this digit (data-exo-accesskey after disarm)
	var nodes = document.querySelectorAll(
		'[data-exo-accesskey="' + digit + '"], [accesskey="' + digit + '"]'
	)
	for (var i = 0; i < nodes.length; i++) {
		var el = nodes[i]
		if (!form_accesskey_action_target(el))
			continue
		if (!form_accesskey_usable(el))
			continue
		return el
	}

	// None usable (e.g. only a hidden wired button) — no activation
	return null
}

// Shared sync cancel for capture keydown/keyup.
function form_digit_accesskey_cancel_event(event) {

	if (event.preventDefault)
		event.preventDefault()
	event.returnValue = false
	if (event.stopPropagation)
		event.stopPropagation()
	event.cancelBubble = true
	if (event.stopImmediatePropagation)
		event.stopImmediatePropagation()
}

// Capture keyup: swallow Alt+digit when the form uses digit accesskeys.
function form_digit_accesskey_capture_keyup(event) {

	if (!ginitok || !form_has_digit_accesskeys())
		return
	if (!event.altKey || event.ctrlKey || event.metaKey)
		return
	if (!form_accesskey_digit_from_event(event))
		return
	form_digit_accesskey_cancel_event(event)
}

// Capture keydown: if the form uses any digit accesskey, claim *all* Alt+0…9
// (cancel even when the target is missing/hidden/disabled). Activate only when
// a usable control exists for that digit.
function form_digit_accesskey_capture_keydown(event) {

	if (!ginitok || !form_has_digit_accesskeys())
		return

	if (!event.altKey || event.ctrlKey || event.metaKey)
		return

	// Colour popup owns the keyboard (confirm-style); do not activate form accesskeys
	if (typeof colors_popup_is_open == 'function' && colors_popup_is_open()) {
		form_digit_accesskey_cancel_event(event)
		return
	}

	var digit = form_accesskey_digit_from_event(event)
	if (!digit)
		return

	// Always cancel — do not let the browser handle unused Alt+N on these forms
	form_digit_accesskey_cancel_event(event)

	var element = form_lookup_digit_accesskey_element(event)
	if (!element)
		return

	// Already in a Gate A flight (e.g. focus/validate) — event already cancelled;
	// do not start a concurrent flight or systemerror. User can press Alt+N again.
	if (typeof g_exo_flow != 'undefined' && g_exo_flow)
		return

	var targetel = element
	// Activation needs Gate A (exoevaluate / dbio).
	if (typeof exo_begin == 'function') {
		void exo_begin(async function form_digit_accesskey_activate(ev) {
			await form_activate_accesskey_control(ev, targetel)
		}, 'digit accesskey ' + digit, event)
	} else {
		void form_activate_accesskey_control(event, targetel)
	}
}

// Run the same action as a click on the accesskey control. Returns true if handled.
async function form_activate_accesskey_control(event, element) {

	if (!form_accesskey_usable(element))
		return false

	var target = form_accesskey_action_target(element)
	if (!target)
		return false

	var onclickexpression = target.getAttribute('exo_onclick')
	if (onclickexpression) {
		// Same path as document_onclick / form_activate_focused_action_button
		await exoevaluate(onclickexpression.replace(/\(\)$/, '(event)'), null, 'event', event)
		return true
	}

	try {
		if (typeof target.click == 'function') {
			target.click()
			return true
		}
	} catch (e) { }

	return false
}

async function setfirstlastelement(element) {

	//discover first non key input element
	if (element.getAttribute('exofieldno') > 0 && !element.getAttribute('exoreadonly')) {
		if (!gfirstnonkeyelement)
			gfirstnonkeyelement = element
		if (element.tabIndex < gfirstnonkeyelement.tabIndex)
			gfirstnonkeyelement = element
	}

	//discover last input element
	if (element.getAttribute('exofieldno') > 0 && !element.getAttribute('exoreadonly')) {
		if (!gfinalinputelement)
			gfinalinputelement = element
		if (element.tabIndex >= gfinalinputelement.tabIndex)
			gfinalinputelement = element
	}

	return
}

async function setfirstlastcolumn(groupno) {

	var tablex = $$('exogroup' + groupno)
	if (!tablex)
		await exoui_invalid('await setfirstlastcolumn() table' + groupno + ' is missing')

	for (var ii = 0; ii < gtables[groupno].length; ii++) {
		var screenfn = gtables[groupno][ii]
		//shouldnt this also be restricted to elements with exofieldno?
		if (
			!gfields[screenfn].getAttribute('exoreadonly')
			&& exoenabledandvisible(gfields[screenfn].id)
		)
			tablex.setAttribute('exo_lastinputcolscreenfn', screenfn)
	}

}

async function updatedisplay(elements) {

	//seems to be only implemented for non-bound forms

	//be careful not to update elements as it may be a global object

	//option to do all elements recursively
	if (typeof elements == 'undefined')
		return await updatedisplay(gfields)

	//do multiple elements recursively
	if (!elements.tagName) {
		for (var ii = 0; ii < elements.length; ii++)
			await updatedisplay(elements[ii])
		return
	}

	var element = elements

	//only do elements with a conditional display expression
	if (!(element.getAttribute('exodisplay')))
		return

	//login('updatedisplay ' + element.id)

	//determine required display
	var display = await gds.evaluate(element.getAttribute('exodisplay'))
	if (typeof display == 'undefined') {
		systemerror('await updatedisplay(' + element.id + ')', '.display() returned undefined')
	}
	display = display ? '' : 'none'

	//quit if display not changed
	//if (element.style.display == display)
	//    return false //logout('updatedisplay ' + element.id + ' still ' + display)

	//show/hide all elements with the same id
	var subelements = $$(element.id)
	if (subelements && subelements.tagName)
		subelements = [subelements]
	for (var subn = 0; subn < subelements.length; subn++) {

		var subelement = subelements[subn]

		//radio elements are surrounded by SPAN
		if ((subelement.type == 'radio' || subelement.type == 'checkbox') && subelement.parentNode && subelement.parentNode.tagName == 'SPAN')
			subelement = subelement.parentNode

		// Hide td/th if it only has this content (walk past icon wraps — parentNode
		// alone is the flex wrap SPAN for popup/link fields).
		var parent = getancestor(subelement, ' TD TH ')
		if (parent) {
			// TD enclosing a DIV can have a blank second child node
			var nchildnodes = 0
			for (var ii = 0; ii < parent.childNodes.length; ii++) if (parent.childNodes[ii].tagName) nchildnodes++
			if (nchildnodes == 1) subelement = parent
		}

		subelement.style.display = display

	}

	//show/hide associated elements (same id plus '_title')
	var elements = $$(element.id + '_title')
	if (elements) {
		if (elements.tagName)
			elements = [elements]
		for (var ii = 0; ii < elements.length; ii++)
			elements[ii].style.display = display
	}

	//logout('updatedisplay ' + element.id + ' became ' + display)

	return

}

async function element_exosetdropdown(element, request, noautoselection) {

	assertelement(element, 'element_setdropdown', 'element')

	if (!(element.getAttribute('exodropdown'))) {
		systemerror('await element_exosetdropdown()', element.id + ' has no dropdown')
		return
	}

	var dropdown = element.getAttribute('exodropdown').split(fm)
	var request = 'CACHE\r' + dropdown[0]
	var colarray = dropdown[1].split('\r')

	await exosetdropdown(element, request, colarray, '', noautoselection)

}

// Trace Esc undo baseline only when validateupdate commits (not every focus move).
// Console filter: gpreviousvalue. Off: gform_trace_gprevious = false
var gform_trace_gprevious = true

function form_trace_gprevious(why, el, oldVal, newVal) {
	if (!gform_trace_gprevious)
		return
	var ev = (typeof gevent != 'undefined' && gevent) ? gevent : null
	var ae = (typeof document != 'undefined') ? document.activeElement : null
	function lab(x) {
		if (!x)
			return ''
		return (x.id || '') + (x.tagName ? '/' + x.tagName : '')
			+ (x.type ? '[' + x.type + ']' : '')
	}
	console.log('gpreviousvalue', why, {
		el: lab(el),
		old: oldVal,
		'new': newVal,
		active: lab(ae),
		eventType: ev ? (ev.type || String(ev)) : '',
		eventTarget: ev && ev.target ? lab(ev.target) : '',
		stack: (new Error()).stack
	})
}

//be careful this is a sync function without _sync in name so only convert exodus3 to exodus2
//or add this function name to the list of functions that dont require yield if converting exodus2 to exodus3
function setgpreviouselement(element, value) {

	if (!element) {
		gpreviouselement = null
		gpreviousvalue = ''
		gprevious_sel = null
		// Clear radio group arrival (left form / no previous field)
		g_radio_arrival_anchor = null
		g_radio_arrival_value = ''
		return
	}

	if (element[0])
		gpreviouselement = element[0]
	else
		gpreviouselement = element

	//handle unusual case that if focused on OPTION instead of SELECT eg from newrecordfocus()
	if (gpreviouselement.tagName == 'OPTION')
		gpreviouselement = gpreviouselement.parentNode

	// Drop caret snap when baseline moves to a different field (successful leave)
	if (gprevious_sel && gprevious_sel.el != gpreviouselement)
		gprevious_sel = null

	//set gpreviousvalue as well
	if (typeof value == 'undefined') {
		//assumes grecn set if mv element
		gpreviousvalue = getvalue(gpreviouselement)
	} else
		gpreviousvalue = value
}

// Snapshot caret/selection before leave (focusout). Used when validate fails and
// focus returns — by then the browser has already dropped the old selection.
function form_capture_field_caret(el) {
	if (!el || !el.tagName)
		return null
	if (el.tagName == 'INPUT' || el.tagName == 'TEXTAREA') {
		if (el.type == 'checkbox' || el.type == 'radio' || el.type == 'button'
			|| el.type == 'submit' || el.type == 'image')
			return null
		try {
			if (typeof el.selectionStart != 'number')
				return null
			return {
				el: el,
				kind: 'input',
				start: el.selectionStart,
				end: el.selectionEnd
			}
		} catch (e) {
			return null
		}
	}
	// contenteditable SPAN (code/text/number hosts)
	if (!(el.isContentEditable || (el.tagName == 'SPAN' && el.getAttribute('contenteditable'))))
		return null
	if (!window.getSelection)
		return null
	try {
		var sel = window.getSelection()
		if (!sel || sel.rangeCount < 1)
			return null
		var range = sel.getRangeAt(0)
		var anc = range.commonAncestorContainer
		if (anc != el && !el.contains(anc))
			return null
		var pre = document.createRange()
		pre.selectNodeContents(el)
		pre.setEnd(range.startContainer, range.startOffset)
		var start = pre.toString().length
		pre.setEnd(range.endContainer, range.endOffset)
		var end = pre.toString().length
		return { el: el, kind: 'span', start: start, end: end }
	} catch (e) {
		return null
	}
}

function form_restore_field_caret(el, snap) {
	if (!el || !snap || snap.el != el)
		return false
	if (snap.kind == 'input' && (el.tagName == 'INPUT' || el.tagName == 'TEXTAREA')) {
		try {
			var len = (el.value || '').length
			var s = Math.max(0, Math.min(snap.start, len))
			var e = Math.max(0, Math.min(snap.end, len))
			el.setSelectionRange(s, e)
			return true
		} catch (e) {
			return false
		}
	}
	if (snap.kind == 'span' && window.getSelection && document.createRange) {
		try {
			function form_caret_point(root, offset) {
				var walked = 0
				var walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT, null)
				var node
				while ((node = walker.nextNode())) {
					var nlen = node.nodeValue.length
					if (walked + nlen >= offset)
						return { node: node, off: offset - walked }
					walked += nlen
				}
				// past end — place at end of last text node or root
				if (root.lastChild && root.lastChild.nodeType == 3)
					return { node: root.lastChild, off: root.lastChild.nodeValue.length }
				return { node: root, off: root.childNodes.length }
			}
			var sp = form_caret_point(el, snap.start)
			var ep = form_caret_point(el, snap.end)
			var range = document.createRange()
			if (sp.node.nodeType == 3)
				range.setStart(sp.node, sp.off)
			else
				range.setStart(el, Math.min(sp.off, el.childNodes.length))
			if (ep.node.nodeType == 3)
				range.setEnd(ep.node, ep.off)
			else
				range.setEnd(el, Math.min(ep.off, el.childNodes.length))
			var sel = window.getSelection()
			sel.removeAllRanges()
			sel.addRange(range)
			return true
		} catch (e) {
			return false
		}
	}
	return false
}

// focusout bubbles — last chance to record caret before next field steals it.
// Only snapshot the field we may bounce back to (gpreviouselement). When
// invalid leave focuses the next field then returns, focusout of that temporary
// target must not overwrite the real caret snap.
function form_onfocusout_capture(event) {
	var el = event && event.target
	if (!el || !el.getAttribute || !el.getAttribute('exotype'))
		return
	if (!el.tagName || !el.tagName.match(gtexttagnames))
		return
	if (el.type == 'checkbox' || el.type == 'radio' || el.type == 'button')
		return
	if (gpreviouselement && el != gpreviouselement)
		return
	gprevious_sel = form_capture_field_caret(el)
}

async function newrecordfocus() {
	//return
	//login('newrecordfocus')
	if (is(gfirstelement) && gloaded && !gds.isnewrecord) {
		gnextelement = $$(gfirstelement)
		if (gnextelement[0])
			gnextelement = gnextelement[0]
	}
	var element = gnextelement
	if (element)
		gnextelement = ''
	else {
		if (gloaded || !gKeyNodes) {
			//element=gfirstnonkeyelement
			//element=document.activeElement
			if (gevent)
				element = gevent.target
			if (!element || !element.getAttribute || !Number(element.getAttribute('exofieldno'))) {
				if (typeof gfirstnonkeyelement == 'string') {
					gfirstnonkeyelement = $$(gfirstnonkeyelement)
					if (gfirstnonkeyelement[0])
						gfirstnonkeyelement = gfirstnonkeyelement[0]
				}
				element = gfirstnonkeyelement
			}
			//   assertelement(element,'newrecordfocus','gfirstnonkeyelement')
		}
		else {
			// Blank keyed form (every new/clear, not only first postinit):
			// prefer form-set gstartelement when it is a key part (e.g. journals
			// JOURNAL_NO). Default remains first key node.
			element = gKeyNodes[0]
			if (gstartelement && gstartelement.id) {
				for (var si = 0; si < gKeyNodes.length; si++) {
					if (gKeyNodes[si].id == gstartelement.id) {
						element = gstartelement
						break
					}
				}
			}
			assertelement(element, 'newrecordfocus', 'gKeyNodes/gstartelement')
		}
	}

	//check no required fields are missing in prior data
	//fill in any defaults where possible
	//do this BEFORE setting gpreviouselement as setdefault will overwrite it
	//
	// Pre-open multipart key (!gloaded, !glocked), start on a later key part
	// (e.g. journals gstartelement=JOURNAL_NO): fill prior parts and commit into
	// gds (setdefault with validateupdate). DOM-only donotupdate broke Open
	// paths that gds.getx company/type while the screen already showed them.
	if (gKeyNodes && !glocked && !gloaded) {
		for (var kn = 0; kn < gKeyNodes.length; kn++) {
			var keyel = gKeyNodes[kn]
			if (keyel.id == element.id)
				break
			// Empty = DOM empty. Sync uses getvalue_internal (ICONV for any [ conversion:
			// NUMBER grouping, ACNO, DATE, …) so gds gets storage form not paint form.
			if (getvalue(keyel) == '')
				await setdefault(keyel)
			else {
				var keyival = getvalue_internal(keyel)
				if ((await gds.getx(keyel.id)) != keyival)
					// Already on screen (SELECT default etc.) but not yet in gds
					await gds.setx(keyel, null, keyival)
			}
		}
	}
	if (!gKeyNodes || glocked) {
		//check group 0 always
		// If a prior required is empty, checkrequired focuses it and must win —
		// do not fall through and schedule focus on `element` (e.g. VOUCHER_DATE
		// after Bank/Cash missing), which re-triggers the same required message.
		if (!(await checkrequired(gfields, element, 0)))
			return
	}

	//required so that if still focused on an element AFTER loading the record
	//then editing the field and moving onto another element causes a proper validateupdate
	//gpreviouselement = $$(element.id)
	//if (gpreviouselement[0])
	//    gpreviouselement = gpreviouselement[0]
	setgpreviouselement($$(element.id))

	//why false??? if (false&&gpreviouselement)
	//false results in repetitive batch number changing still asking discard? when no changes made
	//dont want to do this when in await cleardoc() otherwise always starts a batch
	if (gpreviouselement) {
		await setdefault(gpreviouselement)
		//gpreviouselement = $$(element.id)
		//if (gpreviouselement && gpreviouselement[0])
		//    gpreviouselement = gpreviouselement[0]
		//gpreviousvalue = getvalue(gpreviouselement)
		setgpreviouselement($$(element.id))
	}

	//prevent focussing on hidden keys
	if (!(exoenabledandvisible(element)))
		element = gstartelement

	window.scrollTo(0, 0)

	// element.focus()
	tt = element
	if (tt.id)
		tt = tt.id
	//increased to 100 to avoid "xxx is required" when key provided in dialog window
	//by allowing any initial opendoc to do its work first
	exosettimeout('focuson("' + tt + '")', 100)

	//logout('newrecordfocus')

}

// After open fill/validation: ensure some data field is focused.
// Unbound forms (scheduleprint etc.) run newrecordfocus early (cleardoc), then
// filldefaults/validation; multi-hit decide used to leave focus, quiet exact-key
// paths do not — land on gstartelement / first non-key if nothing is focused.
function form_ensure_open_focus() {
	var ae = document.activeElement
	if (ae && ae.getAttribute && ae.getAttribute('exofieldno')
		&& typeof exoenabledandvisible == 'function' && exoenabledandvisible(ae))
		return
	var el = gstartelement
	if (!el || (typeof exoenabledandvisible == 'function' && !exoenabledandvisible(el)))
		el = gfirstnonkeyelement
	if (!el)
		return
	if (typeof el == 'string') {
		el = $$(el)
		if (el && el[0])
			el = el[0]
	}
	if (!el)
		return
	if (el.id)
		focuson(el.id)
	else
		focuson(el)
}

async function gds_onreadystatechange() {

	if (gds.readystate != 'complete') return
	//login('gds_onreadystatechange')
	grecn = null
	await calcfields()
	await newrecordfocus()
	//logout('gds_onreadystatechange')
}

async function tablex_onreadystatechange(event) {

	event = getevent(event)

	//if need to focus on a table field then
	//do it when the table is complete

	if (event.target.readyState != 'complete') return

	//login('tablex_onreadystatechange ' + Number(event.target.getAttribute('exogroupno')))

	await newrecordfocus()

	//logout('tablex_onreadystatechange ' + Number(event.target.getAttribute('exogroupno')))

}

async function printsendrecord_onclick(event) {

	event = getevent(event)
	exocancelevent(event)

	//work out the print function else return
	var printfunction = gKeyNodes && gKeyNodes[0].getAttribute('exoprintfunction')
	if (!printfunction)
		return
	//printfunction=printfunction.replace(/%KEY%/g,gkeyexternal)
	//journal print does not work properly so ...
	//NB case INsensitive
	printfunction = printfunction.replace(/%KEY%/gi, gkey)
	//alert('DEBUG: saoc')
	await validateupdate()
	if (gtouched && !(await saveandorcleardoc('PRINT'))) {
		focusongpreviouselement()
		return
	}
	//alert('DEBUG: printfunction')
	await exoevaluate(printfunction, 'await printsendrecord_onclick()');

}

async function listrecord_onclick(event) {

	event = getevent(event)
	exocancelevent(event)

	//work out the print function else return
	var listfunction = gKeyNodes && gKeyNodes[0].getAttribute('exolistfunction')
	if (!listfunction)
		return

	await validateupdate()
	listfunction = listfunction.replace(/%KEY%/gi, gkey)

	if (gtouched && !(await saveandorcleardoc('PRINT'))) {
		focusongpreviouselement()
		return
	}
	await exoevaluate(listfunction, 'await listrecord_onclick()');

}

//'WINDOW ONBEFORE UNLOAD
//'''''''''''''''''''''''
function window_onbeforeunload_sync(event) {
	var result = window_onbeforeunload2_sync(event)
	console.log('onbeforeunload returning ' + result)
	return result
}

function window_onbeforeunload2_sync(event) {

	console.log('WINDOW_ONBEFOREUNLOAD')

	//no questions asked if not finished initialisation
	if (!ginitok)
		return undefined

	//persuade user to cancel unload if any request is pending
	if (db.requesting) {
		if (event.preventDefault)
			event.preventDefault()
		return '!!! WAITING FOR REQUEST TO COMPLETE !!!'

		//persuade user to cancel unload if any unsaved changes
	} else if (glocked && gtouched) {
		if (event.preventDefault)
			event.preventDefault()
		return '!!! YOU HAVE NOT SAVED YOUR DATA !!!'
	}

	//not possible to cancel from here on

	//makes the unlock synchronous which seems to be more reliable in onbeforeunload
	gonbeforeunload = true

	//ok to unload. dont ask user anything
	return undefined

}

//WINDOW_ONUNLOAD
/////////////////
//async function window_onunload() {
function window_onunload_sync() {

	//alert() is not available in onunload
	console.log('---------- WINDOW_ONUNLOAD SYNC ----------')

	//ensure any unlocking is synchronous - which seems to reach server, unlike async xmlhttp
	gonunload = true

	if (!ginitok)
		return

	//save gdataset in case we are refreshing and the parent window isnt there to get it from
	exosetcookie('', 'EXODUSlogincode', glogincode, 'logincode')

	//unlock any document before unloading (fire-and-forget async via exofireandforget)
	if (glocked) {

		//Save unlock request for following window to perform hopefully
		//in case following code doesnt successfully unlock its record in onbeforeunload
		var pending = ['UNLOCK', gro.filename, gkey, gro.sessionid].join('\r')
		exosetcookie(glogincode, 'EXODUSpending', pending)

		console.log('trying to unlock ' + gkey + ' immediately but async request doesnt seem to reach server reliably while unloading')
		exofireandforget(unlockdoc(), 'window_onunload_sync unlockdoc')

		//or cancel any pending request
	} else if (gxhttp && gxhttp.status != 200) {
		//        gxhttp.abort()
	}

}

////////////////// DOCUMENT EVENTS /////////////////////

async function document_onclick(event) {

	event = getevent(event)

	if (!event.target.getAttribute) {
		console.log('await document_onclick() missing event.target or event.target.getAttribute() ' + event)
		return
	}

	if (event.target && event.target.getAttribute)
		grecn = getrecn(event.target)

	///log('document_onclick ' + event.target.id)

	var result

	if (event.target.getAttribute('isexopopup'))
		result = await exoui_popup(event)

	else if (event.target.getAttribute('isexolink'))
		result = await exoui_link(event)

	//call the first exo_onclick expression found in element then parents
	var target = event.target
	do {
		var onclickexpression = target.getAttribute('exo_onclick')
		target = target.parentNode
	} while (!onclickexpression && target && target.parentNode && target.parentNode.getAttribute)
	if (onclickexpression) {
		//replace trailing "()" with "(event)" to pass event
		//simulating how document.onclick="funcx()" passes event into funcx()
		//exoevaluate3 will then be able to arrange that
		// the noclick function can refer to the event variable
		result = await exoevaluate(onclickexpression.replace(/\(\)$/, '(event)'), null, 'event', event);
	}

	//logout('document_onclick ' + event.target.id)

	return result
}

async function tabit2() {
	var element = gpreviouselement
	// createEvent/initMouseEvent+dispatchEvent path
	if (document.createEventxxx) {
		//mozilla http://developer.mozilla.org/en/docs/DOM:event.initMouseEvent
		var eventx = document.createEvent("KeyboardEvent");
		form_blockevents(false, 'tabit2')
		var modifiers = ''
		//if (event.shiftKey)
		//    modifiers+=' Shift'
		eventx.initKeyboardEvent('keydown', true, true, window, 9, 0, modifiers)//, repeat, locale);
		var cancelled = !element.dispatchEvent(eventx);
		form_blockevents(true, 'tabit2')
	}
	//newer standard in ff/chrome and win10 (new MouseEvent+despatchEvent)
	else if (typeof KeyboardEvent != 'undefined') {
		var eventx = new KeyboardEvent('keydown', {
			'view': window,
			'bubbles': true,
			'cancelable': true,
			'keyCode': 9
		});
		form_blockevents(false, 'tabit2')
		var cancelled = !element.dispatchEvent(eventx);
		form_blockevents(true, 'tabit2')
	} else
		focusdirection(direction, element, notgroupno)
}

/*
async function document_onkeypress(event) {
	console.log('document_onkeypress()')

	//event=getevent(event)
	//var keycode = event.keyCode ? event.keyCode : event.which
	//console.log('onkeypress '+keycode)
	////try to block ctrl+N from opening a new page but doesnt work
	//if (event.ctrlKey && keycode == 110)
	//    return exocancelevent(event)

	return false
}
*/
//DOCUMENT ON KEY DOWN
//////////////////////
// Gate A (exo_begin / gblockevents) already serializes keydown flights — no local mutex.
async function document_onkeydown(event) {

	//document_onkeydown also occurs in non-form windows not using dbform.js - like upload.htm etc

	// Popup isolation at document keydown (secondary to gblockevents + modal).
	// Same contract as starteventhandler helpers: null / true / false.
	// Confirm always available (client.js); colour only if colors.js loaded.
	// Calendar: Esc handled later via form_closepopups — no calendar helper yet.
	if (typeof exoconfirm_document_keydown == 'function') {
		var confKey = exoconfirm_document_keydown(event)
		if (confKey === true)
			return true
		if (confKey === false)
			return exocancelevent(event)
	}
	if (typeof colors_popup_document_keydown == 'function') {
		var colorKey = colors_popup_document_keydown(event)
		if (colorKey === true)
			return true
		if (colorKey === false)
			return exocancelevent(event)
	}

	return await document_onkeydown2(event)
}

// Enter/Space on a focused form-action control (menubutton/graphicbutton with exo_onclick).
// Not real <button>s — browser will not activate them. Only when already focused.
async function form_activate_focused_action_button(event, element) {

	if (!element || !element.getAttribute)
		return false

	var btn = element
	while (btn && btn !== document && btn !== document.body) {
		if (btn.classList
			&& (btn.classList.contains('graphicbutton') || btn.classList.contains('menubutton'))
			&& btn.getAttribute('exo_onclick'))
			break
		btn = btn.parentNode
	}
	if (!btn || !btn.getAttribute)
		return false
	if (!(btn.classList.contains('graphicbutton') || btn.classList.contains('menubutton')))
		return false

	if (btn.getAttribute('disabled') || btn.disabled)
		return false
	if (btn.style && btn.style.display == 'none')
		return false

	var onclickexpression = btn.getAttribute('exo_onclick')
	if (!onclickexpression)
		return false

	// Same path as document_onclick for exo_onclick controls
	await exoevaluate(onclickexpression.replace(/\(\)$/, '(event)'), null, 'event', event)
	return true
}

async function document_onkeydown2(event) {

	/*
	8   	Backspace
	9   	Tab
	13  	Enter
	16  	Shift
	17  	Ctrl
	18  	Alt
	19  	Pause, Break
	20  	CapsLock
	27  	Esc
	32  	Space
	33  	Page Up/PgUp
	34  	Page Down/PgDn
	35  	End
	36  	Home
	37  	Left arrow
	38  	Up arrow
	39  	Right arrow
	40  	Down arrow
	38  	Up arrow
	39  	Right arrow
	40  	Down arrow
	44  	PrntScrn
	45	    Insert
	46	    Delete
	48-57	0 to 9
	96-105	0-9 on Numpad
	65-90	A to Z
	91	    WIN Key (Start)
	93	    WIN Menu
	112-123	F1 to F12
	144	    NumLock
	145	    ScrollLock
	
	188	    , <
	190	    . >
	46	    . on Numpad
	191	    / ?
	192 	~
	219	    [ {
	220	    \ |
	221	    ] }
	222	    ' "
	
	Numpad
	45	    Ins
	35  	End
	40  	Down
	34  	PgDown
	37  	Left
	101/12	5
	39  	Right
	36  	Home
	38  	Up
	33  	PgUp
	106 	*
	107 	+
	109 	-
	110/46	. Del
	111     /
	*/

	event = getevent(event)
	var keycode = event.keyCode ? event.keyCode : event.which
	//var tt = 'onkeydown ' + keycode
	//if (event.ctrlKey)
	//    tt += ' + ctrl'
	//if (event.shiftKey)
	//    tt += ' + shift'
	//if (event.altKey)
	//    tt += ' + alt'
	//console.log(tt)

	///log('document_onkeydown ' + event.target.id + ' ' + keycode)

	//no processing if not initialised
	if (!ginitok)
		return

	////log(event.target.tagName+' '+event.keyCode)

	//must be before custom key handlers which may use gkey
	gkeycode = keycode
	if (gstepping)
		wstatus(gkeycode)

	// Enter on contenteditable (number/code/text SPAN hosts): same contract as INPUT
	// and as Enter→tab below (non-TEXTAREA = leave field). Must preventDefault before
	// any await — Gate A + form_onkeydown made the later Enter→tab cancel too late, so
	// the browser ran insertParagraph (EXCH_RATE became 1<br><br>). TEXTAREA keeps Enter.
	if (keycode == 13) {
		var cel = event.target
		if (cel && cel.tagName != 'TEXTAREA'
			&& (cel.isContentEditable
				|| (cel.tagName == 'SPAN' && cel.getAttribute
					&& cel.getAttribute('contenteditable')))) {
			if (event.preventDefault)
				event.preventDefault()
			event.returnValue = false
		}
	}

	// Miss-tinted field: no additional character entry (class is the marker)
	if (form_miss_tint_keydown(event) === false)
		return exocancelevent(event)

	// Find-as-you-type panel: Esc / arrows / Enter before form navigation
	var taKey = form_typeahead_keydown(event)
	if (taKey === false)
		return exocancelevent(event)

	// Alt+0…9: handled in form_digit_accesskey_capture_keydown (native capture,
	// outside Gate A). Not repeated here — preventDefault must not wait on async.

	//custom key handlers
	//must return false to prevent further action
	// (Alt+digit is handled in form_digit_accesskey_capture_keydown — not here.)
	if (typeof form_onkeydown == 'function') {
		if (!(await form_onkeydown(event))) {
			return exocancelevent(event)
		}
	}

	gwindowunloading = false
	element = event.target
	// if (typeof(closerecord)=='undefined') return

	try {
		if (element.tagName == 'TD') {
			if (element.firstChild.tagName == 'SPAN') {
				element = element.firstChild
			}
		}
	}
	catch (e) { }

	//ignore shift and ctrl by themselves
	if (keycode == 16 || keycode == 17)
		return true

	//alert(gkeycode)

	// F5 — leave to browser (all platforms)
	if (keycode == 116)
		return true


	//F6 is now link
	if (keycode == 117) {
		await exoui_link(event)
		return exocancelevent(event)
	}

	//F7 is now popup (used to be F2 in DOS) also replaces windows standard alt+down combination
	if (keycode == 118 || (event.altKey && keycode == 40 && element.tagName == 'SELECT')) {
		await exoui_popup(event)
		return exocancelevent(event)
	}

	// Alt+Up/Down pan always; Alt+Left/Right only when .exoform-wide (else history).

	//close (F8)
	if (keycode == 119) {
		await closedoc('CLEAR')
		return exocancelevent(event)
	}

	//alt+k or alt+home is goto top (but alt+home goes to home page and cannot be cancelled)
	if ((keycode == 75 && event.altKey) || (keycode == 36 && event.altKey)) {
		exosettimeout('focuson(gstartelement.id)', 1)
		return exocancelevent(event)
	}

	// menu bar Alt hot keys
	if (event.altKey && ! event.shiftKey && [71, 78, 76, 79, 83, 67, 82, 69, 77, 80, 88].exolocate(gkeycode)) {
		exocancelevent(event)
		var found = true
		//alt+m main menu
		if (gkeycode == 77) {
			window.scrollTo(0, 0)
			var menubutton = $$("menubutton");

			/* do it by directly calling menuonmouseover since clicking suffers conflict with onmouseover on mobile
			// createEvent/initMouseEvent+dispatchEvent path
			if (document.createEvent) {
				// initMouseEvent (legacy createEvent path)
				var mouseevent = document.createEvent("MouseEvents");
				form_blockevents(false,'onkeydown2 alt')
				mouseevent.initMouseEvent("click", true, true, window, 0, 0, 0, 0, 0, false, false, false, false, 0, null);
				var cancelled = !menubutton.dispatchEvent(mouseevent);
				form_blockevents(true,'onkeydown2 alt')
			}
			//newer standard in ff/chrome and win10 (new MouseEvent+despatchEvent)
			else if (typeof MouseEvent!='undefined') {
				var mouseevent = new MouseEvent('click', {
				'view': window,
				'bubbles': true,
				'cancelable': true
				});
				form_blockevents(false,'onkeydown2 not mouse')
				var cancelled = !menubutton.dispatchEvent(mouseevent);
				form_blockevents(true,'onkeydown2 not mouse')
			} else
			*/
			exosettimeout('menuonmouseover(null,$$("menubutton"),13)', 1)
			//menubutton.click()
		}
		//was done by accesskeys on hidden buttons but firefox requires shift+alt for access
		// unless configure http://kb.mozillazine.org/Ui.key.contentAccess
		// dont use timeout since allows the user interface to resume and send ANOTHER event in FF3
		//else if (gkeycode==76) exosettimeout('await exologout_onclick()',1)
		//Logout and List swapped to be G and L respectively
		else if (gkeycode == 71) await exologout_onclick()//g
		// Form-action bar Alt+letter
		else if (gkeycode == 78) await formbutton_op(newrecord_onclick)//n
		else if (gkeycode == 79) await formbutton_op(openrecord_onclick)//o
		else if (gkeycode == 83) await formbutton_op(saverecord_onclick)//s
		else if (gkeycode == 67) await formbutton_op(closerecord_onclick)//c
		else if (gkeycode == 69) await formbutton_op(editreleaserecord_onclick)//e
		//else if (gkeycode == 68) await formbutton_op(deleterecord_onclick)//d reserved
		else if (gkeycode == 76) await formbutton_op(listrecord_onclick)//l
		else if (gkeycode == 80) await formbutton_op(printsendrecord_onclick)//p
		else if (gkeycode == 82) await refreshcache_onclick()//r
		else
			found = false
		return exocancelevent(event)
	}

	//alt+{ is first record
	if (keycode == 219 && event.altKey && event.shiftKey) {
		await formbutton_op(firstrecord_onclick, event)
		return exocancelevent(event)
	}

	//alt+} is last record
	if (keycode == 221 && event.altKey && event.shiftKey) {
		await formbutton_op(lastrecord_onclick, event)
		return exocancelevent(event)
	}

	//alt+[ is previous record
	if (keycode == 219 && event.altKey) {
		await formbutton_op(previousrecord_onclick, event)
		return exocancelevent(event)
	}

	//alt+] is next record
	if (keycode == 221 && event.altKey) {
		await formbutton_op(nextrecord_onclick, event)
		return exocancelevent(event)
	}

	//alt+^ is select record
	if (keycode == 54 && event.altKey && event.shiftKey) {
		await formbutton_op(selectrecord_onclick, event)
		return exocancelevent(event)
	}

	//alt+end is goto bottom
	if (keycode == 35 && event.altKey) {
		exosettimeout('focuson(gfinalinputelement.id)', 1)
		return exocancelevent(event)
	}

	//F12 is exodus debug key
	//if (keycode==123/*&&gusername.indexOf('EXODUS')+1*/)
	if (keycode == 123 && gusername.indexOf('EXODUS') + 1) {

		//plain F12 - toggle stepping2
		if (!event.shiftKey && !event.ctrlKey && !event.altKey) {
			gstepping = !gstepping
			wstatus('stepping=' + gstepping)
			return exocancelevent(event)
		}

		//ctrl+shift+F12 - break
		//force error and into exobreak
		if (event.shiftKey && event.ctrlKey) {
			gstepping = true
			var _b = exobreak('', 'F12', '');
			if (exoisasyncfunction(_b))
				exo_begin_when_idle(_b, 'F12', { delay_ms: 0 })
			else if (_b && typeof _b.next === 'function')
				systemerror('document_onkeydown', 'function* break handler removed (stage 6)')
			return exocancelevent(event)
		}

		//alt+shift+F12 - show event log
		if (event.shiftKey && event.altKey) {
			glogging = true
			wstatus('glogging=true')
			if (geventlog) {
				windowx = await windowopen()
				if (windowx)
					windowx.document.body.innerHTML = geventlog
			}
			return exocancelevent(event)
		}

		temp = element

		//ctrl+F12 - element.parentNode.outerHTML
		var xxx
		if (xxx = (event.ctrlKey && !!temp.parentNode)) {
			temp = temp.parentNode
			//alt+F12 - element.parentNode.parentNode.outerHTML
			if (event.altKey && temp.parentNode)
				temp = temp.parentNode
		}

		if (temp && (temp.outerHTML || temp.innerHTML)) {
			var windowx = await windowopen()
			if (windowx)
				//windowx.document.body.innerText=decodehtmlcodes(temp.outerHTML?temp.outerHTML:temp.innerHTML)
				windowx.document.body.innerHTML = encodehtmlcodes(temp.outerHTML ? temp.outerHTML : temp.innerHTML)
		}

		return exocancelevent(event)

	}

	//F2 is now edit to be like office
	var textrange
	if (keycode == 113) {

		// legacy TextRange selection
		if (document.selection
			&& document.selection.createRange
			&& (textrange = document.selection.createRange()).text != '') {
			textrange.collapse(false)
			textrange.select()
			return exocancelevent(event)
		}

		//.selectionStart throws and error on ff/chrome/standard on radio/checkbox etc
		//works on INPUT text only
		if (element.type == 'text') {
			try {
				var selectionStart = element.selectionStart
			} catch (e) {
				var selectionStart = false
			}
			if (typeof selectionStart == 'number') {
				//select nothing if anything selected
				if (element.selectionStart != element.selectionEnd) {
					element.selectionStart = element.selectionEnd
					element.selectionEnd = document.activeElement.selectionStart
					//select all if nothing selected
				} else {
					element.selectionStart = 0
					element.selectionEnd = 999999999
				}
				return exocancelevent(event)
			}
		}

		//seems to be the most standards based way and works on spans too
		//http://stackoverflow.com/questions/11451353/how-to-select-the-text-of-a-span-on-click
		if (window.getSelection && document.createRange) {
			selection = window.getSelection();
			if (selection.isCollapsed) {
				//following selects all of text
				selection.removeAllRanges();
				range = document.createRange();
				range.selectNodeContents(element);
				selection.addRange(range);
			} else {
				//following removes any selection
				selection.collapseToEnd();
			}
			return exocancelevent(event)
		} else if (document.selection && document.body.createTextRange) {
			range = document.body.createTextRange();
			range.moveToElementText(element);
			range.select();
			return exocancelevent(event)
		}

	}

	//F9 is also save
	if (keycode == 120) {

		//first update the current field
		//otherwise the db is updated without the last entry!!!
		if (!(await validateupdate()))
			return exocancelevent(event)

		//prevent document save unless the save button is enabled
		if (!(saverecord.getAttribute('disabled'))) {
			//await savedoc()
			await formbutton_op(saverecord_onclick)
		}

		return exocancelevent(event)

	}

	//esc - when no changes means closerecord
	// if(keycode==27&&gloaded)
	if (keycode == 27) {

		//close any "modal" popups
		if (await form_closepopups()) {
			//prevent normal esc handling
			return exocancelevent(event)
		}

		// Text Esc undo (old): same field as gpreviouselement → restore gpreviousvalue.
		// Do not require element.name — free-text SPANs use id/exoname; .name is not
		// reliable field identity (INPUT-era gate; broke journals OTHER_DETAILS etc.).
		// Radio/checkbox: same group/id still counts as same field.
		var escSameField = (element == gpreviouselement)
			|| (element.type == 'radio' && gpreviouselement
				&& form_radio_same_group(element, gpreviouselement))
			|| (element.type == 'checkbox' && gpreviouselement
				&& gpreviouselement.type == 'checkbox' && element.id == gpreviouselement.id)
		// Esc: restore radio/checkbox to arrival value (not last click). Live click-validate unchanged.
		// (Same idea as text Esc + gpreviousvalue, but live validate advances gpreviousvalue.)
		var escArrival = g_radio_arrival_anchor && (
			(element.type == 'radio'
				&& form_radio_same_group(element, g_radio_arrival_anchor))
			|| (element.type == 'checkbox' && g_radio_arrival_anchor.type == 'checkbox'
				&& element.id == g_radio_arrival_anchor.id)
		)
		if (escArrival) {
			value = getvalue(element)
			if (value != g_radio_arrival_value) {
				// value = current (last live choice); after setvalue, getvalue is arrival
				setvalue(element, g_radio_arrival_value)
				// Do not setgpreviouselement — that would wipe gpreviousvalue / clear arrival.
				// Keep gpreviousvalue as last choice so validateupdate commits restore + layout.
				if (!gpreviouselement
					|| (element.type == 'radio'
						&& !form_radio_same_group(element, gpreviouselement))
					|| (element.type == 'checkbox'
						&& (gpreviouselement.type != 'checkbox'
							|| element.id != gpreviouselement.id)))
					gpreviouselement = element
				// Same as text Esc: clear touched only if this control "just" touched the form.
				// Capture before validateupdate (which nulls gelementthatjustcalledsettouched).
				var clearTouched = gelementthatjustcalledsettouched
					&& (element == gelementthatjustcalledsettouched
						|| (element.type == 'radio'
							&& form_radio_same_group(element, gelementthatjustcalledsettouched))
						|| (element.type == 'checkbox'
							&& gelementthatjustcalledsettouched.type == 'checkbox'
							&& element.id == gelementthatjustcalledsettouched.id))
				if (gpreviousvalue != g_radio_arrival_value) {
					if (!(await validateupdate())) {
						// Validation failed: put UI back to live choice (gds unchanged)
						setvalue(element, value)
						return exocancelevent(event)
					}
					// Like text Esc after uncommitted type — undo of that sole touch source
					if (clearTouched)
						settouched(false)
				}
				// Focus follows restored control (radio: form_radio_tab_target via focuson)
				focuson(element)
				form_typeahead_cancel()
				return exocancelevent(event)
			}
			// at arrival — fall through to closerecord
		// TEXTAREA: no field-level Esc undo (multi-line; do not wipe whole edit).
		} else if (escSameField && element.tagName != 'TEXTAREA') {

			value = getvalue(element)

			//old value — only evaluate default if field was empty on focus (e.g. revert to default)
			if (gpreviousvalue == '') {
				gdefaultvalue = await getdefault(element)
				if (gdefaultvalue)
					gpreviousvalue = gdefaultvalue
			}

			// Revert field edit when value differs OR this field was the touch source
			// (e.g. spaces-only: getvalue trims so value==gpreviousvalue, but form_oninput
			// already settouched — still restore display and clear gtouched).
			if (value != gpreviousvalue || element == gelementthatjustcalledsettouched) {

				//restore the original value
				//get from datasource
				setvalue(element, gpreviousvalue)
				// setvalue2 clears miss; cancel pending quiet search so it cannot re-tint
				form_typeahead_cancel()

				//if reverting element that settouched then revert that too
				if (element == gelementthatjustcalledsettouched)
					settouched(false)

				//prevent normal esc handling
				exocancelevent(event)

				// Full-select restored value (dates already did via INPUT.select;
				// free-text SPAN needs selectNodeContents). Similar: document_onfocus.
				try {
					if (element.select)
						element.select()
					else if (window.getSelection && document.createRange) {
						var escSel = window.getSelection()
						escSel.removeAllRanges()
						var escRange = document.createRange()
						escRange.selectNodeContents(element)
						escSel.addRange(escRange)
					} else if (document.selection && document.body.createTextRange) {
						var escTr = document.body.createTextRange()
						escTr.moveToElementText(element)
						escTr.select()
					}
				}
				catch (e) { }

				return exocancelevent(event)
			}
		}

		await formbutton_op(closerecord_onclick)

		return exocancelevent(event)

	}

	//get groupno otherwise return
	ggroupno = element.getAttribute('exogroupno')

	//the rest of the keys are only when located on a exodus data entry field
	if (typeof ggroupno == 'undefined' || ggroupno == null) {
		// Ctrl/Cmd+Enter: always Save/OK, never press the focused button (menubar List,
		// Reconcile, form Cancel, …). Same shortcut as Enter on a data field with Ctrl.
		if (keycode == 13 && (event.ctrlKey || event.metaKey) && !event.altKey) {
			if (saverecord && !saverecord.getAttribute('disabled'))
				await formbutton_op(saverecord_onclick)
			return exocancelevent(event)
		}
		// Form action focused: Shift+Enter = previous (like Up); bare Enter/Space activate.
		if (keycode == 13 && event.shiftKey && !event.ctrlKey && !event.metaKey && !event.altKey) {
			focusprevious(element)
			return exocancelevent(event)
		}
		if ((keycode == 13 || keycode == 32) && !event.ctrlKey && !event.metaKey && !event.altKey && !event.shiftKey) {
			if (await form_activate_focused_action_button(event, element))
				return exocancelevent(event)
		}
		if (keycode == 37 || keycode == 38) {
			focusprevious(element)
			return exocancelevent()
		}
		if (keycode == 39 || keycode == 40) {
			focusnext(element)
			return exocancelevent()
		}
		return true
	}

	ggroupno = Number(ggroupno)

	grecn = null
	grows = null
	if (ggroupno > 0) {

		//get the current table and ggroupno else exit
		var rowx = getancestor(element, 'TR')
		var tablex = getancestor(rowx, 'TABLE')
		if (!tablex)
			return
		grows = tablex.tBodies[0].getElementsByTagName('tr')

		//get the rown else exit
		//if (ggroupno > 0) {
		var rown = rowx.rowIndex
		if (tablex.tHead)
			rown -= tablex.tHead.rows.length
		//}

		//get the recordset
		var rs = gds.data['group' + ggroupno]
		if (!rs) {
			exocancelevent(event)
			return await exoui_invalid('Group number on non-group field')
		}
		//zzz if "paging" should be offset by subtracting record number of first row
		grecn = rown
		gnrecs = rs.length

		var nrows = grows.length
		var pagesize = tablex.dataPagesize ? tablex.dataPagesize : 999999
		var pagen = exoint(grecn / pagesize)
		var npages = Math.ceil((rs.length) / pagesize)

		var tablex_firstinputcolscreenfn = form_getfirstinputcolscreenfn(tablex)
		var tablex_lastinputcolscreenfn = form_getlastinputcolscreenfn(tablex)

	}

	//ctrl+G is goto line
	if (keycode == 71 & event.ctrlKey) {
		if (!(await validateupdate()))
			return false
		while (true) {

			var elements = ggroupno ? element : gfields[gtables[1][0]]
			elements = $$(elements.id)
			if (elements && elements.tagName)
				elements = [elements]

			var reply = window.prompt('Go to line number ? (1-' + elements.length + ')', grecn + 1)
			if (!reply)
				break

			reply = (+reply)
			if (reply < 1) reply = 1
			if (reply > elements.length)
				reply = elements.length

			focuson(elements[reply - 1])
			break

		}

		return exocancelevent(event)
	}

	//alt+Y is copy previous record/column
	if (keycode == 89 && event.altKey && element.type != 'button') {
		if (grecn > 0) {
			setvalue(element, await getpreviousrow(element.id, { skipblanks: true }))
			try { element.select() } catch (e) { }
			return exocancelevent(event)
		}
	}

	//in tables: same column, first/last visible row (not first/last col of that row)
	//ctrl+home 36 = current col, first row; ctrl+end 35 = current col, last row
	if (tablex && (keycode == 36 || keycode == 35) && event.ctrlKey && !event.shiftKey && !event.altKey) {
		var id = element.id
		if (keycode == 36) {
			var startrown = 0
			var increment = 1
		} else {
			var startrown = grows.length - 1
			var increment = -1
		}
		// forwards from first / backwards from last — same col if enabled+visible
		for (var ii = startrown; ii >= 0 && ii < grows.length; ii += increment) {
			var cellelement = grows[ii].exofields && grows[ii].exofields[id]
			if (cellelement && exoenabledandvisible(cellelement)) {
				focuson(cellelement)
				break
			}
		}
		return exocancelevent(event)
	}

	// Enter on TEXTAREA
	if (keycode == 13 && (element.tagName == 'TEXTAREA' && !event.ctrlKey && !event.shiftKey)) {
		if (gKeyNodes && !glocked) {
			keycode = 9
			event.keyCode = keycode
		} else if (element.value == '' || element.value.slice(-4) == '\r\n\r\n') {
			element.value = element.value.slice(0, -4)
			keycode = 9
			event.keyCode = keycode
		} else if (element.value == '' || element.value.slice(-2) == '\n\n') {
			element.value = element.value.slice(0, -2)
			keycode = 9
			event.keyCode = keycode
		} else {
			//.selectionStart throws and error on ff/chrome/standard on radio/checkbox etc
			try { var selectionStart = element.selectionStart } catch (e) { var selectionStart = false }
			if (typeof selectionStart == 'number' && element.selectionStart == 0 && element.selectionEnd == element.value.length) {
				keycode = 9
				event.keyCode = keycode
			}
		}
		if (event.keyCode != keycode) {
			var direction = event.shiftKey * -2 + 1
			focusdirection(direction, element)
			return exocancelevent(event)
		}
	}

	// Tab as data when conversion is [INDENTED] (e.g. chart ACCOUNT_NAME indent).
	// Shift+Tab still navigates. Before last-row empty first-col skip so blank rows indent.
	if (keycode == 9 && !event.ctrlKey && !event.altKey && !event.shiftKey
		&& form_try_insert_tab_char(element)) {
		return exocancelevent(event)
	}

	//tab or down or enter on first or last col of LAST row is special
	if ((ggroupno > 0) && grecn == (rs.length - 1)) {
		if ((keycode == 9 || keycode == 13 || keycode == 40) && !event.shiftKey && !event.ctrlKey && !tablex.getAttribute('noinsertrow')) {

			//down on select handled by browser, not EXODUS
			if (keycode == 40 && element.tagName == 'SELECT')
				return exocancelevent(event)

			//on last column - add a row
			//exo_firstinputcolscreenfn
			var firstcolelement = rowx.exofields[gfields[gtables[ggroupno][0]].id]
			if ((glocked || !gKeyNodes)
				//&& element.getAttribute('exo_screenfn') == tablex.getAttribute('exo_lastinputcolscreenfn')
				&& element.getAttribute('exo_screenfn') == tablex_lastinputcolscreenfn
				&& getvalue(firstcolelement) != ''
			) {
				if (glocked || !gKeyNodes) {
					//exoaddrow(ggroupno)
					if (!(await form_insertrow(event, true)))
						return exocancelevent(event)
					//continue on to focus on it
					//var nextelement=$$(gfields[tablex.getAttribute('exo_firstinputcolscreenfn')].id)[grecn+1]
					//focuson(nextelement)
					focusnext()
					return exocancelevent(event)
				}
			}
			//on first column, if empty - go to next field after current table
			if (element.getAttribute('exo_screenfn') == tablex_firstinputcolscreenfn && getvalue(element) == '') {
				focusdirection(1, element, ggroupno)
				return exocancelevent(event)
			}
		}
	}

	//enter key becomes tab (or ok if only one entry field)
	if (keycode == 13 && (element.tagName != 'TEXTAREA' || event.ctrlKey || event.shiftKey)) {

		//ctrl+enter or enter with single field is ok/save record
		//if (event.ctrlKey||(gfields.length==1&&Number(gfields[0].getAttribute('exogroupno'))))
		if (event.ctrlKey
			|| (
				!Number(element.getAttribute('exogroupno'))
				&& gfinalinputelement
				&& gfinalinputelement.id == gstartelement.id
			)
		) {
			if (!(saverecord.getAttribute('disabled'))) {
				//await savedoc()
				await formbutton_op(saverecord_onclick)
			}
			return exocancelevent(event)
		}

		// Enter on last field wraps to first (not form OK/Cancel — those are Tab/arrows only)
		if (!event.shiftKey && element == gfinalinputelement && (ggroupno == 0 || (ggroupno > 0 && grecn == gnrecs))) {
			focuson(gstartelement)
			return exocancelevent(event)
		}

		//shift+enter on 1st field → end (or OK via focusdirection wrap / DOM order)
		if (event.shiftKey && element == gstartelement) {
			focusdirection(-1, element)
			return exocancelevent(event)
		}
		/*
				var keyEvt = document.createEvent("KeyboardEvent");
				if (keyEvt.initKeyEvent){
					//mozilla deprecated
					keyEvt.initKeyEvent(
						"keydown",   //event type
						true,       //bubbled
						true,       //cancelable
						null,       //viewArg
						event.ctrlKey,
						event.altKey,
						event.shiftKey,
						false,      //metaKeyArg
						keycode,    //virtual keycode of depressed key else 0
						0           //unicode of depressed key if any else 0
						);
					gonkeydown=false
					element.dispatchEvent(keyEvt);
					keyEvt.stopPropagation;
				} else if (keyEvt.initKeyboardEvent){
							  // Safari?
				}
		*/
		keycode = 9
		event.keyCode = keycode

		// if assigning keyCode was ignored, synthesise tab navigation
		if (event.keyCode != keycode) {
			var notgroupno
			if (event.shiftKey)
				direction = -1
			else {
				direction = 1
				//enter on first column if required and no default skips (TODO and not rowrequired) first field after the table)
				if (ggroupno
					&& element.getAttribute('exorequired')
					&& element.getAttribute('exo_screenfn') == tablex_firstinputcolscreenfn
					&& !getvalue(element)
				) {
					notgroupno = ggroupno
				}
			}

			focusdirection(direction, element, notgroupno)
			//window.setTimeout('await tabit2()',1)

			return exocancelevent(event)
		}

		//dont return because enter key has some special processing on rows below
		//return

	}

	// Tab: same focusdirection path as Enter-as-tab (form actions are stops when tabIndex >= 0).
	if (keycode == 9 && !event.ctrlKey && !event.altKey) {
		focusdirection(event.shiftKey ? -1 : 1, element)
		return exocancelevent(event)
	}

	// Horizontal radio: Up/Down = field leave (same focusdirection as Enter/Tab).
	// Same-group skip still applies so any option leaves the group as one stop.
	// Left/Right stay browser option change.
	if (element.type == 'radio' && element.getAttribute('exohorizontal')
		&& (keycode == 38 || keycode == 40)
		&& !event.ctrlKey && !event.shiftKey && !event.altKey) {
		// keep gkeycode 38/40 so readonly-skip knows back vs forward
		focusdirection(keycode == 38 ? -1 : 1, element)
		return exocancelevent(event)
	}

	//all remaining key events are related to loaded records
	if (gKeyNodes && closerecord.getAttribute('disabled'))
		return true

	// Arrow keys on radio/SELECT: leave to browser (horizontal Up/Down already handled)
	if (keycode == 37 || keycode == 38 || keycode == 39 || keycode == 40) {
		if (element.type && element.type == 'radio')
			return true
		if (element.tagName == 'SELECT')
			return true
	}

	//up and down keys not on radio or select tags
	if ((keycode == 38 || keycode == 40) && !event.ctrlKey && !event.shiftKey && !event.altKey) {

		//up/down on select and textarea leave system to process it normally
		if (element.tagName == 'TEXTAREA' && getvalue(element) && !form_field_all_selected(element))
			return true
	}

	//left arrow or right arrow and whole field is selected else return
	if ((keycode == 37 || keycode == 39) && !event.ctrlKey && !event.shiftKey && !event.altKey) {

		//handle left/right normally if not all text selected
		if (element.maxLength != 1 && element.tagName.match(gtexttagnames) && element.type != 'button' && element.type != 'checkbox') {
			if (!form_field_all_selected(element))
				return

		}

		//left or right not in tables
		if (ggroupno == 0 && !event.ctrlKey && !event.shiftKey && !event.altKey) {
			// Island L/R: Left → ti-step if present; else walk +step while present (last wins);
			// else normal. Right: ti+step; else walk -step while present; else normal.
			var curTi = element.tabIndex
			if (curTi > 0 && curTi != exo_tabindex_default) {
				var step = exo_tabindex_col_step
				var first = keycode == 37 ? -step : step
				var walk = -first
				var sideEl = focusdirection_find_tabindex(curTi + first, element, '')
				if (!sideEl) {
					var t = curTi + walk
					while (true) {
						var el = focusdirection_find_tabindex(t, element, '')
						if (!el)
							break
						sideEl = el
						t += walk
					}
				}
				if (sideEl) {
					focuson(sideEl)
					return exocancelevent(event)
				}
			}
			if (keycode == 37)
				focusprevious()
			else
				focusnext()
			return exocancelevent(event)
		}

	}

	//up down in multiline text treated normally - up and down in the text
	if ((keycode == 38 || keycode == 40) && !event.ctrlKey && !event.shiftKey && !event.altKey) {
		if (!form_field_all_selected(element) && getvalue(element).indexOf('\n') >= 0)
			return
	}

	//up down keys outside a table are like left/right
	//if((keycode==38||keycode == 40)&&ggroupno==0&&element.tagName!='SELECT'&&element.type!='radio')
	if ((keycode == 38 || keycode == 40) && ggroupno == 0 && !event.ctrlKey && !event.shiftKey && !event.altKey) {

		//option to revert to windows standard (eg to show rapid effect of different values)
		if (element.tagName == 'SELECT' && element.getAttribute('exoallowcursor'))
			return true

		if (keycode == 38)
			focusprevious()
		else
			focusnext()

		//prevent the key from being processed as a cursor movement
		return exocancelevent(event)

	}

	//left arrow and right arrow in tables, except on SELECT since they change it
	if (ggroupno != 0 && rown >= 0 && (keycode == 37 || keycode == 39) && !event.ctrlKey && !event.shiftKey && !event.altKey && element.tagName != 'SELECT') {
		//if not on the sole input element in the row (isfirst and islast)
		//if (!element.getAttribute('exo_isfirstinputcolumn') || !element.getAttribute('exo_islastinputcolumn')) {

		//if (!element.getAttribute('exo_isfirstinputcolumn') || element.getAttribute('exo_screenfn') != tablex.getAttribute('exo_lastinputcolscreenfn')) {
		var screenfn = element.getAttribute('exo_screenfn')
		if (screenfn != tablex_firstinputcolscreenfn || screenfn != tablex_lastinputcolscreenfn) {

			var scope = grows[grecn].getElementsByTagName('*')
			if (keycode == 37)
				focusprevious(element, scope)
			else
				focusnext(element, scope)
		}
		return exocancelevent(event)
	}

	//PGUP/PGDN/UP/DOWN/LEFT/RIGHT
	if (keycode == 34 || keycode == 33 || keycode == 40 || keycode == 38 || keycode == 37 || keycode == 39) {

		if (ggroupno == 0)
			return

		if (rown < 0)
			return//must be in a thead row

		//if down arrow on last row
		if (keycode == 40 && grecn == (grows.length - 1)) {

			//not on first column, focus next column
			if (element.getAttribute('exo_screenfn') != tablex_firstinputcolscreenfn) {
				focusnext(element)
				return exocancelevent(event)
			}

			//on first column, add a new row
			if (glocked || !gKeyNodes) {
				//exoaddrow(ggroupno)
				await form_insertrow(event, true)
				//and continue to focus on it
			}
		}

		var nrows = grows.length
		var id = element.id
		if (!id)
			return//exodus like button with no id?

		var pagesize = tablex.dataPagesize ? tablex.dataPagesize : 999999
		var pagen = exoint(grecn / pagesize)
		var npages = Math.ceil((rs.length) / pagesize)

		//pgdn or down arrow
		if (keycode == 34 || (keycode == 40 && !event.ctrlKey && !event.shiftKey && !event.altKey)) {

			//ctrl+pgdn sadly not supported since reserved by firefox to change tabs
			if (rown < nrows - 1) {
				if (keycode == 40) {
					if (!grows[rown + 1].exofields[id]) {
						systemerror('await document_onkeydown()', 'Cannot locate id ' + id + ' in rown: ' + rown + ' nrows:' + grows.length + ' tagname:' + grows[rown + 1].exofields[id])
						return false
					}
					focuson(grows[rown + 1].exofields[id])
				}
				else {
					var newrown = grecn + gpagenrows
					if (newrown > (nrows - 1))
						newrown = nrows - 1
					focuson(grows[newrown].exofields[id])
				}
			}

			//if on last row go to first row of next page if any, otherwise stay put
			else {

				if (pagen < npages - 1) {
					//tablex.nextPage()
					focuson(grows[0].exofields[id])
				}
			}

		}

		//pgup or up arrow
		if (keycode == 33 || (keycode == 38 && !event.ctrlKey && !event.shiftKey && !event.altKey)) {

			//ctrl+pgup or up (sadly not since reserved by firefox to change tabs
			//goes to first line of first page
			if (event.ctrlKey) {
				//tablex.firstPage()
				focuson(grows[0].exofields[id])
			}
			else {

				//if not on first row of page go up one row (up arrow) or to first line (pgup)
				if (rown > 0) {
					//up
					if (keycode == 38) {
						var newrown = rown - 1
						while (newrown > 0 && !exoenabledandvisible(grows[newrown])) {
							newrown--
						}
						focuson(grows[newrown].exofields[id])
					}
					//page up
					else {
						var newrown = grecn - gpagenrows
						if (newrown < 0)
							newrown = 0
						//focuson(grows[newrown].exofields[id])
						if (document.getElementsByClassName)
							focuson(grows[newrown].exofields[id])
						else
							focuson(grows[newrown].all[id])
					}
				}

				//if on first line of page go to last line of previous page if any, or stay put
				//(same for pgup and up)
				else {
					if (grecn == 0) {
						focusprevious(element)
					}
					else {
						//tablex.previousPage()
						focuson(grows[nrows - 1].exofields[id])
					}
				}
			}
		}

		//allow various control and shift key combinations
		if (!event.ctrlKey && !event.shiftKey && !event.altKey)
			return exocancelevent(event)

	}

	//following are all keycodes when within tabular rows section
	//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	if (event.altKey && event.ctrlKey && keycode == 18)
		return true

	//Ctrl+I or Ctrl+Insert is insert row, but only in rows.
	//with or without shift
	if (event.ctrlKey && (keycode == 73 || keycode == 45) && rowx) {
		var button = rowx.exofields['insertrowbutton' + ggroupno]
		if (!event.repeat && button && button.style && button.style.display != 'none') {
			exocancelevent(event)
			await form_insertrow(event)
		}
		return false
	}

	//Ctrl+D or Ctrl+Delete is delete row, but only in rows.
	//with or without shift
	if (event.ctrlKey && (keycode == 68 || keycode == 46) && rowx) {
		var button = rowx.exofields['deleterowbutton' + ggroupno]
		if (!event.repeat && button && button.style && button.style.display != 'none') {
			exocancelevent(event)
			await form_deleterow(event, event.target)
		}
		return false
	}

	//protect readonly fields
	if (!(event.altKey || event.ctrlKey)
		&& (element.getAttribute('exoreadonly')
			|| (gKeyNodes
				&& !glocked
				&& element
				&& element.getAttribute('exofieldno')
				&& element.getAttribute('exofieldno') != 0
			))) {
		if (![9, 16, 17, 18, 20, 35, 36, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123].exolocate(keycode)) {
			exocancelevent(event)
			if (gKeyNodes && !glocked) {
				return await readonlydocmsg()
			}
			else if (element.getAttribute('exoreadonly') != 'true') {
				await exoui_invalid(element.getAttribute('exoreadonly'))
			}
			return false
		}
	}

}//document_onkeydown2

//return the first or last enabled and visible input exo_screenfn given a table element
//or the precalculated ones if none
function form_getlastinputcolscreenfn(tablex) {
	return form_getfirstinputcolscreenfn(tablex, true)
}
function form_getfirstinputcolscreenfn(tablex, last) {
	var groupno = tablex.getAttribute('exogroupno')
	var sfns = gtables[groupno]
	var sfn
	for (var ii = 0; ii < sfns.length; ++ii) {
		var element = gfields[sfns[ii]]

		//skip non-input elements
		if (!element.getAttribute('exofieldno') || element.getAttribute('exoreadonly') || !element.tagName.match(gdatatagnames))
			continue

		//capture only enabled and visible elements
		if (exoenabledandvisible(element)) {
			sfn = sfns[ii]
			//if first then skip out as soon as found one
			if (!last)
				return sfn
		}
	}

	//if last then skip out with last found sfn
	if (sfn)
		return sfn

	//otherwise return the precalculated defaults
	return tablex.getAttribute(last ? 'exo_lastinputcolscreenfn' : 'exo_firstinputcolscreenfn')
}

//checks and blocks if document is readonly
function document_onchange_sync(event) {
	if (ockeditor)
		ockeditor.setReadOnly(gkeys.length && !glocked)
}

function focusprevious(element, scope) {
	focusdirection(-1, element, '', scope)
}

function focusnext(element, scope) {
	focusdirection(1, element, '', scope)
}

// Radios of one field: Tab lands once (checked, else first). Arrows still change value (browser).
function form_radio_group_members(radio) {
	if (!radio || radio.type != 'radio')
		return null
	try {
		if (typeof getradiocheckboxelements == 'function')
			return getradiocheckboxelements(radio)
	} catch (e) { }
	if (radio.name) {
		var byname = document.getElementsByName(radio.name)
		if (byname && byname.length)
			return byname
	}
	return [radio]
}

function form_radio_same_group(a, b) {
	if (!a || !b || a.type != 'radio' || b.type != 'radio')
		return false
	var members = form_radio_group_members(a)
	if (!members)
		return !!(a.name && b.name && a.name == b.name)
	for (var i = 0; i < members.length; i++) {
		if (members[i] == b)
			return true
	}
	return false
}

// When Tab/focus lands on a radio group: checked option, else first option.
function form_radio_tab_target(radio) {
	if (!radio || radio.type != 'radio')
		return radio
	var members = form_radio_group_members(radio)
	if (!members || !members.length)
		return radio
	var first = null
	for (var i = 0; i < members.length; i++) {
		var el = members[i]
		if (!el || el.type != 'radio')
			continue
		if (!first)
			first = el
		if (el.checked)
			return el
	}
	return first || radio
}

// Form action face SPANs (OK/Cancel): tabIndex 0 = stop; -1 = skip. Enter never.
function focusdirection_form_action_tab_stop(el) {
	return !!(el && el.classList
		&& (el.classList.contains('graphicbutton') || el.classList.contains('menubutton'))
		&& el.getAttribute('exo_onclick')
		&& el.tabIndex >= 0
		&& gkeycode != 13)
}

// Shared “would classic focusdirection accept this as next stop?”
// Includes form-action faces (OK/Cancel). Tabindex inequality is NOT here.
function focusdirection_is_stop(el, fromEl, notgroupno) {
	if (!el || el == fromEl)
		return false

	var formActionTabStop = focusdirection_form_action_tab_stop(el)

	if (!formActionTabStop) {
		if (!el.id || !el.tagName.match(gdatatagnames))
			return false
	}

	if (el.tagName != 'INPUT' && !el.isContentEditable && el.tagName != 'SELECT' && el.tagName != 'TEXTAREA') {
		if (!formActionTabStop)
			return false
	}

	if (notgroupno !== '' && Number(el.getAttribute('exogroupno')) == notgroupno)
		return false

	if (el.getAttribute('exoreadonly'))
		return false

	if (el.disabled)
		return false

	if (!el.offsetWidth)
		return false

	if (!formActionTabStop && el.tabIndex == -1)
		return false

	if (!exoenabledandvisible(el))
		return false

	if ((gkeycode == 37 || gkeycode == 39 || gkeycode == 38 || gkeycode == 40)
		&& (el.tagName == 'SELECT' || el.type == 'radio')) {
		if (!(el.type == 'radio'
			&& el.getAttribute('exohorizontal')
			&& (gkeycode == 38 || gkeycode == 40)))
			return false
	}

	if (gkeycode == 13 && el.type == 'button')
		return false

	if (fromEl && fromEl.type == 'radio' && el.type == 'radio'
		&& form_radio_same_group(fromEl, el))
		return false

	return true
}

// Exact tabIndex stop (island L/R). Ignores 0 / default / negative — not column peers.
function focusdirection_find_tabindex(wantTi, fromEl, notgroupno) {
	if (wantTi == null || wantTi === '' || wantTi <= 0 || wantTi == exo_tabindex_default)
		return null
	var scope = document.getElementsByTagName('*')
	for (var i = 0; i < scope.length; i++) {
		var el = scope[i]
		if (el.tabIndex != wantTi)
			continue
		if (focusdirection_is_stop(el, fromEl, notgroupno))
			return el
	}
	return null
}

// Nav when start field has a design tabindex (not 0, not exo_tabindex_default).
// Forward: next higher island peer; else next DOM stop; else wrap from start.
// Backward: prev lower island peer; else prev DOM stop; else wrap from end.
// Peers ignore ti 0 and default and form-action faces. Returns element or null.
function focusdirection_tabindexed(direction, startEl, fromTi, notgroupno, scope, scopeindex, scopex) {
	var best = null
	var bestTi = direction > 0 ? Infinity : -Infinity
	var bestIdx = direction > 0 ? Infinity : -Infinity
	var i
	var el
	var ti
	for (i = 0; i < scope.length; i++) {
		el = scope[i]
		if (!focusdirection_is_stop(el, startEl, notgroupno))
			continue
		if (focusdirection_form_action_tab_stop(el))
			continue
		ti = el.tabIndex
		if (ti == 0 || ti == exo_tabindex_default || ti < 0)
			continue
		if (direction > 0) {
			if (ti <= fromTi)
				continue
			if (ti < bestTi || (ti == bestTi && i < bestIdx)) {
				best = el
				bestTi = ti
				bestIdx = i
			}
		} else {
			if (ti >= fromTi)
				continue
			if (ti > bestTi || (ti == bestTi && i > bestIdx)) {
				best = el
				bestTi = ti
				bestIdx = i
			}
		}
	}
	if (best)
		return best

	var passzero = scopeindex
	while (true) {
		scopeindex = scopeindex + direction
		if (passzero == scopeindex)
			return null
		if (scopeindex < 0) {
			if (typeof scopex == 'undefined')
				window.scrollBy(0, 999999)
			scopeindex = scope.length - 1
		} else if (scopeindex > scope.length - 1) {
			if (typeof scopex == 'undefined')
				window.scrollBy(0, -999999)
			scopeindex = 0
		}
		el = scope[scopeindex]
		if (focusdirection_is_stop(el, startEl, notgroupno))
			return el
	}
}

function focusdirection(direction, element, notgroupno, scopex) {
	if (typeof notgroupno == 'undefined')
		notgroupno = ''

	//find next input field
	var nextelement = element
	if (!nextelement) {
		//if (!window.event) return
		if (gevent)
			nextelement = gevent.target
		if (!nextelement)
			return
	}
	// if (!element) return
	var startEl = nextelement
	var elementtabindex = startEl ? startEl.tabIndex : 999999999999

	var origgroupno = startEl.getAttribute('exogroupno')

	var scope = scopex
	if (!scope)
		scope = document.all
	if (!scope)
		scope = document.getElementsByTagName('*')

	// work out index into scope of elements (all or one row usually)
	var scopeindex
	//if (typeof nextelement.sourceIndex != 'undefined') {
	//    var sourceindex = nextelement.sourceIndex
	//    scopeindex = sourceindex - scope[0].sourceIndex
	//} else
	{
		//find nextelement in scope
		if (scope.indexOf) {
			//probably never occurs since scope is a collection not an array
			scopeindex = scope.indexOf(nextelement)
		} else {
			for (scopeindex = 0; scopeindex <= scope.length; ++scopeindex) {
				if (scope[scopeindex] == nextelement)
					break;
			}
		}
		//fail if cannot locate nextelement (eg element is no longer linked into document due to  something like deleteallrows()?)
		if (scopeindex >= 0 && scope[scopeindex] != nextelement) {
			//focuson(nextelement)
			return
		}
	}

	//fail if cannot index into scope
	if (scope[scopeindex] != nextelement) {
		//wrongscopeindex()
		//focuson(nextelement)
		return
	}

	// Island = design tabindex only. 0 = browser default input; exo_tabindex_default = SPAN default;
	// -1 / unset-as--1 = not a tab stop for this ladder (classic path or skip elsewhere).
	if (elementtabindex > 0 && elementtabindex != exo_tabindex_default) {
		nextelement = focusdirection_tabindexed(direction, startEl, elementtabindex, notgroupno, scope, scopeindex, scopex)
		if (!nextelement)
			return
	} else {
		// Ordinary path (ti 0/default): next focusable in DOM order.
		var passzero = scopeindex

		while (true) {
			scopeindex = scopeindex + direction

			if (passzero == scopeindex)
				return

			if (scopeindex < 0) {
				if (typeof scopex == 'undefined')
					window.scrollBy(0, 999999)
				scopeindex = scope.length - 1
			} else if (scopeindex > scope.length - 1) {
				if (typeof scopex == 'undefined')
					window.scrollBy(0, -999999)
				scopeindex = 0
			}

			nextelement = scope[scopeindex]
			if (!focusdirection_is_stop(nextelement, startEl, notgroupno))
				continue

			break
		}
	}

	//if arrived on last column and row of a table using up then focus on the first column last row
	if (direction < 0) {
		var nextgroupno = nextelement.getAttribute('exogroupno')
		if (nextgroupno != origgroupno && nextgroupno != 0) {
			if (gtables[nextgroupno]) {
				var tablex = gtables[nextgroupno].tablex
				if (tablex) {
					var tablex_lastinputcolscreenfn = form_getlastinputcolscreenfn(tablex)
					if (nextelement.getAttribute('exo_screenfn') == tablex_lastinputcolscreenfn) {
						var tablex_firstinputcolscreenfn = form_getfirstinputcolscreenfn(tablex)
						var firstinputcolid = gfields[tablex_firstinputcolscreenfn].id
						var row = tablex.tBodies[0].getElementsByTagName('tr')[getrecn(nextelement)]
						setupnewrow(row)//create .exofields
						var firstcolelement = row.exofields[firstinputcolid]
						if (firstcolelement)
							nextelement = firstcolelement
					}
				}
			}
		}
	}

	// Arriving on a radio group: checked option, else first (not whichever DOM order hit first)
	if (nextelement && nextelement.type == 'radio')
		nextelement = form_radio_tab_target(nextelement)

	//found it. focus on it
	//console.log('focusdirection ' + nextelement.tagName + ' ' + nextelement.id)
	form_scroll_log_msg('focusdirection', direction > 0 ? '+1' : '-1',
		'from', form_scroll_el_label(startEl),
		'to', form_scroll_el_label(nextelement),
		'gkeycode=', gkeycode)
	focuson(nextelement)

}

// Programmatic focus without browser mid-viewport jump (scrollintoview owns both axes).
function form_focus_noscroll(el) {
	if (!el)
		return
	try {
		el.focus({ preventScroll: true })
	} catch (e) {
		try { el.focus() } catch (e2) { }
	}
}

// Form radio/checkbox: no focus on mousedown — only mouseup on the same control.
// Mousedown focus runs document_onfocus before mouseup; scroll/layout then moves
// the page so only focus changes and the click does not commit the value.
function form_ensure_radio_mouseup_focus() {
	if (gform_radio_mouseup_focus_installed)
		return
	if (typeof document == 'undefined' || !document.addEventListener)
		return
	gform_radio_mouseup_focus_installed = true
	document.addEventListener('mousedown', form_radio_mousedown_nofocus, true)
	document.addEventListener('mouseup', form_radio_mouseup_focus, true)
	// Space synthesizes click without mousedown — same was_checked snapshot
	document.addEventListener('keydown', form_radio_keydown_space_snapshot, true)
}

function form_radio_is_exo_toggle(el) {
	if (!el || (el.type != 'radio' && el.type != 'checkbox') || el.disabled)
		return false
	if (el.getAttribute && el.getAttribute('exotype'))
		return true
	var oc = el.getAttribute && el.getAttribute('exo_onclick')
	return !!(oc && oc.indexOf('onclickradiocheckbox') >= 0)
}

function form_radio_mousedown_nofocus(event) {
	if (event.button != 0)
		return
	var t = event.target
	if (!form_radio_is_exo_toggle(t)) {
		gform_radio_md_target = null
		gform_radio_was_checked = false
		return
	}
	gform_radio_md_target = t
	// Snapshot before click — optional radios uncheck on re-activate of the same option
	gform_radio_was_checked = (t.type == 'radio' && !!t.checked)
	if (event.preventDefault)
		event.preventDefault()
}

function form_radio_keydown_space_snapshot(event) {
	if (event.keyCode != 32 || event.ctrlKey || event.metaKey || event.altKey || event.shiftKey)
		return
	var t = event.target
	if (!form_radio_is_exo_toggle(t) || t.type != 'radio')
		return
	gform_radio_was_checked = !!t.checked
}

function form_radio_mouseup_focus(event) {
	if (event.button != 0)
		return
	var t = event.target
	var md = gform_radio_md_target
	gform_radio_md_target = null
	// Abort (drag away / different target): drop was_checked so a later gesture cannot clear
	if (!md || t != md || !form_radio_is_exo_toggle(t)) {
		gform_radio_was_checked = false
		return
	}
	if (document.activeElement != t)
		form_focus_noscroll(t)
}

// True if any main form is extreme-wide (.exoform-wide) — horizontal pan useful.
function form_any_wide_layout() {
	var tables = document.querySelectorAll
		? document.querySelectorAll('TABLE.exoform')
		: []
	for (var i = 0; i < tables.length; i++) {
		if (typeof form_table_is_wide == 'function') {
			if (form_table_is_wide(tables[i]))
				return true
		} else if (tables[i].classList && tables[i].classList.contains('exoform-wide'))
			return true
	}
	return false
}

/*
 * Alt+arrows: pan the window by almost one viewport (overlap for context).
 * Alt+Up/Down always; Alt+Left/Right only when form_any_wide_layout() (else browser history).
 * Focus stays put; does not move fields.
 * Called only from form_scroll_viewport_capture_keydown (sync, outside Gate A).
 */
function form_scroll_viewport(keycode) {
	var rem = 16
	try {
		rem = parseFloat(window.getComputedStyle(document.documentElement).fontSize) || 16
	} catch (e) { }
	var vw = window.innerWidth || document.documentElement.clientWidth || 0
	var vh = window.innerHeight || document.documentElement.clientHeight || 0
	var topCover = 0
	try {
		topCover = parseFloat(
			window.getComputedStyle(document.documentElement)
				.getPropertyValue('--exosticky-top')
		) || 0
	} catch (e) { }

	// Leave ~3rem of previous content visible so you don't lose place
	var stepY = Math.max(vh - topCover - 3 * rem, rem)
	var stepX = Math.max(vw - 3 * rem, rem)
	var dx = 0
	var dy = 0
	if (keycode == 38)
		dy = -stepY
	else if (keycode == 40)
		dy = stepY
	else if (keycode == 37)
		dx = -stepX
	else if (keycode == 39)
		dx = stepX

	if (!(dx || dy))
		return

	// window.scrollBy is enough when overflow is not locked (Gate A modal block).
	window.scrollBy(dx, dy)
}

// Sync capture — same reason as digit accesskeys: preventDefault must not wait on
// async Gate A, and blockmodalui overflow:hidden + scroll restore undoes scrollBy.
function form_ensure_scroll_viewport_capture() {
	if (gform_scroll_viewport_capture_installed)
		return
	if (!document.addEventListener)
		return
	gform_scroll_viewport_capture_installed = true
	document.addEventListener('keydown', form_scroll_viewport_capture_keydown, true)
}

function form_scroll_viewport_capture_keydown(event) {
	if (!ginitok)
		return
	if (!event.altKey || event.ctrlKey || event.metaKey || event.shiftKey)
		return

	var keycode = event.keyCode ? event.keyCode : event.which
	if (keycode != 37 && keycode != 38 && keycode != 39 && keycode != 40)
		return
	// Horizontal pan only when form is wide; otherwise Alt+Left/Right → browser history
	if ((keycode == 37 || keycode == 39) && !form_any_wide_layout())
		return

	// Alt+Down on SELECT opens popup (F7 path in async keydown) — do not pan
	var t = event.target
	if (keycode == 40 && t && t.tagName == 'SELECT')
		return

	// Confirm / colour popup own the keyboard
	if (typeof $$ == 'function' && $$('exoconfirmdiv'))
		return
	if (typeof colors_popup_is_open == 'function' && colors_popup_is_open())
		return

	form_scroll_viewport(keycode)

	if (event.preventDefault)
		event.preventDefault()
	event.returnValue = false
	if (event.stopPropagation)
		event.stopPropagation()
	event.cancelBubble = true
	if (event.stopImmediatePropagation)
		event.stopImmediatePropagation()
}

/*
 * Safe viewport band — geometry only (no elementFromPoint).
 * top = menubar (--exosticky-top) + pad, then raised by any stuck
 * multirow thead that contains the field (clients brands, journals lines, …).
 */
function scrollintoview_viewport(element) {
	var rem = 16
	try {
		rem = parseFloat(window.getComputedStyle(document.documentElement).fontSize) || 16
	} catch (e) { }
	var pad = 0.75 * rem
	var vw = window.innerWidth || document.documentElement.clientWidth || 0
	var vh = window.innerHeight || document.documentElement.clientHeight || 0
	var stickyTop = 0
	try {
		stickyTop = parseFloat(
			window.getComputedStyle(document.documentElement)
				.getPropertyValue('--exosticky-top')
		) || 0
	} catch (e) { }
	var top = stickyTop + pad

	// Multirow TABLE[exogroupno] > thead { position:sticky; top: menubar }.
	// When stuck, free band starts below that thead — otherwise Up leaves the
	// focused row under the column headings.
	if (element && typeof getancestor === 'function') {
		try {
			var table = getancestor(element, 'TABLE')
			while (table) {
				if (table.tHead) {
					var thSt = window.getComputedStyle(table.tHead)
					if (thSt && (thSt.position == 'sticky' || thSt.position == 'fixed')) {
						var thr = table.tHead.getBoundingClientRect()
						var stickAt = 0
						try {
							stickAt = parseFloat(thSt.top)
							if (isNaN(stickAt))
								stickAt = stickyTop
						} catch (e2) {
							stickAt = stickyTop
						}
						// Stuck (or nearly): top sits at CSS sticky offset
						if (thr.bottom > 0 && thr.top <= stickAt + 2)
							top = Math.max(top, thr.bottom + 0.25 * rem)
					}
				}
				table = table.parentNode
					? getancestor(table.parentNode, 'TABLE')
					: null
			}
		} catch (e) { }
	}

	return {
		left: pad,
		right: vw - pad,
		top: top,
		bottom: vh - pad,
		vw: vw,
		vh: vh
	}
}

/*
 * Horizontal target: field's TD/TH + consecutive preceding TH prompts in the row.
 * Multirow body TDs are not expanded (previous siblings are TDs → pure column).
 * First focussable column of a multirow: also include the row's first TD
 * (ins/del buttons) so those stay in the free band — not document left; works
 * when the table is not at the left of the form.
 */
function scrollintoview_hrect(element, cell) {
	var elR = element.getBoundingClientRect()
	if (!cell || !cell.getBoundingClientRect)
		return elR
	var r = cell.getBoundingClientRect()
	var left = r.left
	var right = r.right
	var p = cell.previousElementSibling
	while (p && p.tagName == 'TH') {
		try {
			var pr = p.getBoundingClientRect()
			left = Math.min(left, pr.left)
			right = Math.max(right, pr.right)
		} catch (e) { }
		p = p.previousElementSibling
	}
	// First focussable field of a multirow → include leading ins/del TD.
	var groupno = Number(element.getAttribute && element.getAttribute('exogroupno'))
	if (groupno > 0 && typeof gtables != 'undefined' && gtables[groupno]) {
		var firstSfn = null
		try {
			var tablex = gtables[groupno].tablex || gtables[groupno].tableelement
			if (tablex && typeof form_getfirstinputcolscreenfn == 'function')
				firstSfn = form_getfirstinputcolscreenfn(tablex)
		} catch (e) { }
		if (firstSfn == null || firstSfn === '' || typeof firstSfn == 'undefined')
			firstSfn = gtables[groupno][0]
		var isFirst = element.getAttribute('exo_isfirstinputcolumn')
			|| (gfields[firstSfn]
				&& (element.id == gfields[firstSfn].id
					|| String(element.getAttribute('exo_screenfn')) == String(firstSfn)))
		if (isFirst) {
			var tr = null
			try {
				tr = typeof getancestor === 'function' ? getancestor(element, 'tr') : null
			} catch (e) { }
			var btnTd = tr && tr.firstElementChild
			if (btnTd && btnTd.tagName == 'TD' && btnTd != cell) {
				try {
					var br = btnTd.getBoundingClientRect()
					left = Math.min(left, br.left)
					right = Math.max(right, br.right)
				} catch (e) { }
			}
		}
	}
	return { left: left, right: right, top: r.top, bottom: r.bottom }
}

/*
 * Horizontal dx for column rect (TD/TH + preceding prompt TH).
 *  - Fits in free band → must be fully inside; min-fit if any edge clips.
 *  - Wider than free band → only require leading edge in band (may clip right).
 */
function scrollintoview_dx(r, vp) {
	if (!r || !vp || !vp.vw)
		return 0
	var w = r.right - r.left
	var avail = vp.right - vp.left
	if (avail <= 0)
		return 0
	if (w <= avail + 0.5) {
		if (r.left >= vp.left - 0.5 && r.right <= vp.right + 0.5)
			return 0
		if (r.left < vp.left)
			return r.left - vp.left
		if (r.right > vp.right)
			return r.right - vp.right
		return 0
	}
	// Oversized: satisfied if start is in the free band
	if (r.left >= vp.left - 0.5 && r.left < vp.right - 0.5)
		return 0
	return r.left - vp.left
}

/*
 * Vertical dy for the focused control. Same split as horizontal.
 */
function scrollintoview_dy(r, vp) {
	if (!r || !vp || !vp.vh)
		return 0
	var h = r.bottom - r.top
	var avail = vp.bottom - vp.top
	if (avail <= 0)
		return 0
	if (h <= avail + 0.5) {
		if (r.top >= vp.top - 0.5 && r.bottom <= vp.bottom + 0.5)
			return 0
		if (r.top < vp.top)
			return r.top - vp.top
		if (r.bottom > vp.bottom)
			return r.bottom - vp.bottom
		return 0
	}
	if (r.top >= vp.top - 0.5 && r.top < vp.bottom - 0.5)
		return 0
	return r.top - vp.top
}

/*
 * Scroll only when geometry says the target is not ok.
 * Horizontal: column TD/TH (+ prompt TH). Vertical: control.
 */
function scrollintoview(element) {
	if (!element || !element.getBoundingClientRect) {
		form_scroll_log_msg('scrollintoview skip: no element/rect')
		return
	}
	if (!element.tagName || !element.tagName.match(gdatatagnames)) {
		form_scroll_log_msg('scrollintoview skip: not data tag', form_scroll_el_label(element))
		return
	}

	var cell = null
	try {
		cell = typeof getancestor === 'function' ? getancestor(element, ' TD TH ') : null
	} catch (e) { }

	var vp = scrollintoview_viewport(element)
	if (!vp.vw || !vp.vh)
		return

	var rH = scrollintoview_hrect(element, cell)
	var rV = element.getBoundingClientRect()
	var dx = scrollintoview_dx(rH, vp)
	var dy = scrollintoview_dy(rV, vp)

	form_scroll_log_msg('scrollintoview', form_scroll_el_label(element),
		'hL/R=', Math.round(rH.left) + '/' + Math.round(rH.right),
		'vT/B=', Math.round(rV.top) + '/' + Math.round(rV.bottom),
		'vpL/R=', Math.round(vp.left) + '/' + Math.round(vp.right),
		'vpT/B=', Math.round(vp.top) + '/' + Math.round(vp.bottom),
		'dx/dy=', Math.round(dx) + '/' + Math.round(dy))

	if (dx || dy)
		window.scrollBy(dx, dy)
}

///////////////////// BUTTON EVENTS /////////////////////////

//NEW button
////////////
async function newrecord_onclick() {

	//save and close existing document otherwise cancel
	if (gkey && !(await closedoc('NEW')))
		return false

	//suppress defaulting if is a fixed default (eg current market in market file)
	//only look at single part keys
	if (gKeyNodes.length == 1) {
		var keyelement = gKeyNodes[0]
		var defaultvalue = keyelement.getAttribute('exodefaultvalue')
		//if no default and key field not required, that means that key will be generated by server on request
		//if default and default starts with " character then considered to be a fixed default and not required if user clicks "New" button
		if (
			(!defaultvalue && keyelement.getAttribute('exorequired'))
			||
			(defaultvalue && defaultvalue.slice(0, 1) == '"')
		) {
			//if already defaulted with fixed string then remove default value and prevent defaulting
			//if they enter something then click New it will be like they clicked Tab to the first non-key element
			var keyid = keyelement.id
			var currentvalue = await gds.getx(keyid)
			if (currentvalue && currentvalue == eval(defaultvalue)) {
				await gds.setx(keyid, '', '')
				gds.setdefaulted(keyid, null, true)
			}
			//focus on the key element
			//exosettimeout('focuson("' + gKeyNodes[0].id + '")', 200)
			//return true
			//skip the above to allow focussing on first non-key element because prompts them with a message
		}
	}

	// element.focus()
	var tt = gfirstnonkeyelement
	if (tt.id)
		tt = tt.id

	//if new clicked while on gfirstnonkeyelement ensure that focussing on it triggers read record
	//gpreviouselement=null
	if (gstartelement && gstartelement.id)
		exosettimeout('focuson("' + gstartelement.id + '")', 100)

	exosettimeout('focuson("' + tt + '")', 200)

	//await newrecordfocus()
	return true

}

//RELEASE BUTTON
////////////////
async function editreleaserecord_onclick() {

	//not locked and there is a key
	if (!glocked) {
		await opendoc(gkey)
		return true
	}

	//if (!(await saveandunlockdoc())) return false
	if (!(await saveandorcleardoc('RELEASE')))
		return false

	//editreleaserecord.value=' Edit'
	setgraphicbutton(editreleaserecord, '<u>E</u>dit', geditimage)
	setdisabledandhidden(editreleaserecord, false)

	return true

}

//SAVE BUTTON
/////////////
async function saverecord_onclick() {

	//first update the current field
	//otherwise the db is updated without the last entry!!!
	if (!(await validateupdate()))
		return false

	//check for missing required
	if (!(await validateall()))
		return false

	// saverecord.focus()

	//if databound form then save it
	if (gKeyNodes) {

		//save the record
		if (!(await savedoc()))
			return false

		//prune the cache to reselect any edited record
		prunecache('SELECT\r' + gdatafilename + '\r')
		prunecache('READ\r' + gdatafilename + '\r' + gkey)
		prunecache('GETINDEX\r' + gdatafilename + '\r')

		//custom postwrite function
		if (typeof form_postwrite == 'function') {
			if (!(await exoevaluateall('await form_postwrite()')))
				return false
		}
		//otherwise automatic option to print if available
		else {
			//TODO put this back as a form level opt in
			//   if (printsendrecord&&!printsendrecord.getAttribute('disabled')&&(await exoui_yesno('Print/Send '+gkeyexternal+' ?',2))) await printsendrecord_onclick()
		}

		return true

	}

	//from here on is an unbound form and not a databound form
	//////////////////////////////////////////////////////////

	//custom prewrite function
	if (typeof (form_prewrite) == 'function') {
		if (!(await exoevaluateall('await form_prewrite()', 'await saverecord_onclick()')))
			return false
	}

	//option to confirm
	if (gparameters.confirm || gparameters.savemode && gparameters.savemode.indexOf('CONFIRM') >= 0) {
		if (!(await exoui_yesno('OK to continue?', 2)))
			return false
	}

	///convert screen data into a rev string
	gro.data = gds.data
	if (!(gro.obj2rev()))
		return false

	//alert(gro.revstr.split(fm))

	//custom write and postwrite routine
	if (typeof form_write == 'function') {

		if (!(await exoevaluateall('await form_write()', 'await saverecord_onclick()')))
			return false
		settouched(false)

		if (typeof (form_postwrite) == 'function') {
			await exoevaluateall('await form_postwrite(db)', 'await saverecord_onclick()');
		}

		return true
	}

	//option to return the form data to a calling program
	//instead of sending to the database
	if (window.dialogArguments && (!gparameters.savemode || !gparameters.savemode.exoswap('CONFIRM', ''))) {

		//window.returnValue = gro.revstr
		//window.close()
		//return false
		return exoui_windowclose(gro.revstr)
	}

	//standard unbound write routine
	if (!(await unbound_form_write()))
		return false

	//if postwrite routine
	if (typeof (form_postwrite) == 'function') {
		await exoevaluateall('await form_postwrite(db)', 'await saverecord_onclick()');
	}

	//otherwise, if no postwrite function then assume that
	//returned data is a url and open it (in a new tab by preference or a new window if cannot)
	else if (db.data) {
		//dont open in tab if dialog window and not closing
		//because user cant focus on underlying window tabs while in (pseudo) modal dialog window
		if (gisdialog && !closeafter)
			await windowopen(db.data)//new window
		else
			await windowopen(db.data, '', 'tab')//tab
	}

	//option to close window
	var closeafter = gparameters.savemode && gparameters.savemode.indexOf('CLOSE') >= 0
	if (closeafter) {
		//window.returnValue=true
		//window.close()
		exoui_windowclose(true)
	}

	return true

}

async function unbound_form_write() {

	//send the instructions for processing and open the report
	db.request = 'EXECUTE\r' + gmodule + '\r' + gdatafilename
	if (!(await db.send(gro.revstr))) {
		await exoui_invalid(db.response)
		return false
	}
	settouched(false)
	if (db.response.slice(0, 3) == 'OK ') {
		await exoui_note(db.response.slice(3))
	}

	return true

}

//CLOSE BUTTON
//////////////
async function closerecord_onclick() {

	//cancel on dialoged window closes window returns false
	if (window.dialogArguments || !gKeyNodes && gparameters.savemode && gparameters.savemode.indexOf('CONFIRM') >= 0)
	//if (window.dialogArguments)
	{

		// Dirty leave: Discard / Cancel by default. Light modals set gparameters.discardable
		// to skip (search, agencyfilter, settings, upload, schedulefind/print, consolidation).
		if (gtouched) {
			if (!gparameters.discardable) {
				var response = await exoui_confirm('Discard data or instructions entered ?', 1, '', 'D<u>i</u>scard', '<u>C</u>ancel')
				if (response != 2) return false
			}
			settouched(false)// discard chosen / discardable — avoid a second Q in closedoc
		}

		var returnvalue = ''

		//clear the document so user is not asked again in onunload
		if (gKeyNodes) {
			gwindowunloading = true
			if (!gtouched)
				returnvalue = 'ACCESS ' + gkey
			await cleardoc()
		}

		if (window.dialogArguments) {

			//close the window
			//window.returnValue = returnvalue
			//return window.close() && false
			return exoui_windowclose(returnvalue)
		}

	}

	return await closedoc('CLOSE')

}

//DELETE BUTTON
///////////////
async function deleterecord_onclick(event) {

	event = getevent(event)

	if (!(await deletedoc()))
		return exocancelevent(event)

	//prune the cache to reselect any deleted record
	prunecache('SELECT\r' + gdatafilename + '\r')
	prunecache('GETINDEX\r' + gdatafilename + '\r')

	//custom postdelete function
	if (typeof form_postdelete == 'function') {
		if (!(await exoevaluateall('await form_postdelete()')))
			return false
	}

	return true

}

/*
//CHANGE PAGE BUTTONS
/////////////////////
async function changepage(pagen) {
	if (gKeyNodes && closerecord.getAttribute('disabled')) return false

	var tablex = getancestor(window.event.target, 'TABLE')
	//if (tablex!=null&&typeof(tablex.getAttribute('exogroupno'))!='undefined')
	if (tablex != null && Number(tablex.getAttribute('exogroupno'))) {
		switch (pagen) {
			case 0: { tablex.firstPage(); break }
			case -1: { tablex.previousPage(); break }
			case 1: { tablex.nextPage(); break }
			default: { tablex.lastPage(); break }
		}
	}

}*/

/////////////////////// DOCUMENT FUNCTIONS //////////////////


//'''''''''
//'OPENDOC
//'''''''''

// True while opendoc body runs — focus handlers skip validation races during load.
// Not a Gate A mutex (Gate A already serializes); product guard only.
var gopening = false

async function opendoc(newkey) {

	//login('opendoc')
	var result = await opendoc_body(newkey)
	//logout('opendoc '+result)

	return result
}

async function opendoc_body(newkey) {
	//await debug('opendoc:'+newkey)
	//how to avoid double calls from multiple timeouts
	if (newkey == gkey && glocked)
		return true

	if (newkey == '%NEW%') newkey = ''

	//check potential key has as many parts as there are key fields
	if (newkey && gKeyNodes.length > 1) {
		if (typeof newkey == 'string')
			newkey1 = newkey
		else
			newkey1 = newkey[0]
		// dict attrs via Attribute API (not expandos — cloneNode keeps attributes only)
		var sepchar = gKeyNodes[0].getAttribute('exokeypart') ? '*' : gKeyNodes[0].getAttribute('exowordsep')
		if (sepchar && newkey1.split(sepchar).length != gKeyNodes.length) {
			systemerror('opendoc', newkey1.exoquote() + ' does not have the correct number of key parts (' + gKeyNodes.length + ')')
			return false
		}
	}

	gopening = true
	var opened = false
	try {
		opened = await opendoc2(newkey)
	} finally {
		gopening = false
	}

	//add key into gkeys
	if (opened && gKeyNodes) {
		if (!gkeys.exolocate(gkey)) {
			gkeys = gkeys.exoinsert(gkeyn + 1 + 1, gkey)
			await setgkeys(gkeys)
			if (gkeys.length > 1)
				gkeyn++
		}
	}

	//focus on key (last if more than one) element if opendoc2 fails
	if (!opened) {
		exosettimeout('try{gKeyNodes[gKeyNodes.length-1].focus()}catch(e){}', 10)
		return false
	}

	return true

}

function getlockholder(response) {
	var lockholder = 'somebody else'
	var tt = db.response.indexOf('LOCKHOLDER:')
	if (tt >= 0) lockholder = db.response.slice(tt + 11).split('"')[1]
	return lockholder
}

async function opendoc2(newkey0) {
	//await debug('opendoc2:'+newkey0)

	//login('opendoc2')

	//detect if a new key is present and quit if not
	var newkey = ''
	if (newkey0) {
		if (typeof newkey0 == 'string')
			newkey = newkey0
		else if (newkey0 && typeof newkey0.join == 'function')
			newkey = newkey0.join('*')
		else
			newkey = String(newkey0)
	}
	else {
		if (gKeyNodes) {
			newkey = await getkey()
			if (newkey == gkey && gloaded) {
				//newkey=''
				return false //logout('opendoc2 - no new key')
			}
		}
		if (!newkey) {
			//qqq allow read without key to return next sequential key
			//   return false //logout('opendoc2 - no new key')
		}
	}

	//unload previous doc first
	///////////////////////////
	//have to clear even if not loaded othewise setkeyvalues fails because gds.data not yet available when key=xxx in the url
	if (!(await closedoc('OPEN'))) {
		//qqq should try and restore the previous key data?
		return false //logout('opendoc2 - did not close currently open doc')
	}

	await setkeyvalues(newkey)//closedoc zaps them.
	gkey = newkey
	gkeyexternal = await getkeyexternal()

	//optional preread function (returns false to abort)
	if (typeof (form_preread) == 'function') {

		///log('preread external key=' + gkeyexternal + ' internalkey=' + gkey)
		if (!(await exoevaluateall('await form_preread()', 'await opendoc2()')))
			return false //logout('opendoc2 - preread false')

	}

	wstatus('Requesting data, please wait ...')

	//read record (with or without a lock)
	/////////////

	//gkey can be blank in which case a new key will be
	//returned in gro.key

	gro.key = gkey
	if (gkey || !gupdateonlymode)
	//read with key='' means get the next sequential number
		/**/ await gro.read(!greadonlymode && !gparameters.openreadonly)

	//switch off one-time option
	gparameters.openreadonly = false

	await loadnewkey()

	glocked = gro.sessionid != ''
	//ensure lock is automatically re-locked every x minutes while record is active on-screen
	if (glocked)
		startrelocker()

	//fail if no data (failure to reach backend database)
	//listen always provides data even if only an empty dataset (eg new record)

	if (!gro.data) {
		if (glocked)
			await unlockdoc()//fail safe

		//logout('opendoc2 - no data')
		return await exoui_invalid(gro.response)
	}

	var lockholder = getlockholder(db.response)

	if (db.response.toUpperCase().slice(0, 16) == 'ERROR: NO RECORD') {
		if (gupdateonlymode) {
			//await exoui_invalid(exoquote(gkeyexternal)+' does not exist.')
			await exoui_invalid(exoquote(await getkeyexternal()) + ' does not exist.')
			//('opendoc2 - cannot create new record because gupdateonlymode is true')
			return false
		}

		if (!glocked) {

			//cannot create records if cannot lock them
			if (db.response.toUpperCase().indexOf('LOCK NOT AUTHORISED') >= 0) {
				await exoui_invalid('Sorry, you are not authorised to create new records in this file.')
				return false //logout('opendoc2 - cannot create new record because lock not authorised')
			}

			//may not be authorised to read a record
			if (db.response.toUpperCase().indexOf('NOT AUTHORISED') >= 0) {
				await exoui_invalid(db.response)
				return false //logout('opendoc2 - cannot access record because not authorised')
			}

			//cannot create a record in read only mode or update only mode
			if (greadonlymode || gupdateonlymode || gpreventcreation) {
				//await exoui_invalid('Sorry, '+gkeyexternal+' does not exist and\nyou are not authorised to create new records in this file.')
				await exoui_invalid('Sorry, ' + (await getkeyexternal()) + ' does not exist and\nyou are not authorised to create new records in this file.')
				return false //logout('opendoc2 - cannot create new record in read only mode')
			}

			//cannot create a record without a lock
			//if possible get another default key
			if (gKeyNodes.length == 1 && gKeyNodes[0].getAttribute('exodefaultvalue')) {
				var newkey = await getdefault(gKeyNodes[0])
				if (newkey && newkey != gkey) {
					//logout('opendoc2 - cannot create new record because ' + lockholder + ' is creating it.')
					return await opendoc2(newkey)
				}
			}

			await exoui_invalid(exoquote(gkeyexternal) + ' is being created by ' + lockholder + '.\r\n\r\nYou cannot view or update it until they have finished or cancel.')
			//logout('opendoc2 - cannot create new record because ' + lockholder + ' is creating it.')
			return false
		}

		/*
		//optionally cancel if (record does not exist
		if (!(await exoui_okcancel('Document '+exoquote(getkeyexternal)+' does not exist. Create a new document ?',2))) {

		if (glocked)
		await unlockdoc()
		exocancelevent(event)
		return false //logout('opendoc - user chose not to create new record')
		}
		*/

		gds.isnewrecord = true

	}
	else {

		if (!glocked) {

			//cannot create records if cannot lock them
			if (db.response.toUpperCase().indexOf('LOCK NOT AUTHORISED') >= 0) {
				//alert('Sorry, you are not authorised to update this file.')
			}
			else {

				if (greadonlymode) {
					//open in read only mode
				}
				else {
					if (db.response.toUpperCase().indexOf('CANNOT LOCK RECORD') >= 0) {
						if ((await exoui_confirm(exoquote(gkeyexternal) + ' is being updated by ' + lockholder + '.\nOpen for viewing only?', 1, 'Yes', '', 'Cancel')) != 1) {
							return false //logout('opendoc2 - because it is being updated by ' + lockholder + ' and the user chose not to open it in read only mode')
						}
						//editreleaserecord.value='Edit'
						setgraphicbutton(editreleaserecord, '<u>E</u>dit', geditimage)
						setdisabledandhidden(editreleaserecord, false)
					}
					else {
						if (db.response.toString().slice(0, 2) != 'OK')
							return await exoui_invalid(db.response)
						else
							await exoui_note(db.response.toString().slice(2))
					}
				}
			}
		}

		gds.isnewrecord = false

	}

	//make sure gds is available in postread
	gds.data = gro.data
	gds.dict = gro.dict
	gds.dictitem = gro.dictitem

	//if record created/updated in form_postread
	//form_postread should set settouched(true) to allow save without user edits
	settouched(false)
	//gpreviouselement = null
	//gpreviousvalue = ''
	setgpreviouselement(null)

	//postread
	if (typeof form_postread == 'function') {
		grecn = null
		if (!(await exoevaluateall('await form_postread()', 'await opendoc2()')))
		//if (!(await form_postread()))
		{
			if (glocked)
				await unlockdoc()//fail safe
			await cleardoc()
			return false //logout('opendoc2 - postread failed')
		}

	}

	//load the record
	if (gds.isnewrecord)
		wstatus('New record')
	else
		wstatus('Formatting, please wait ...')
	gloaded = true
	await gds.load(gro)

	if (!gds.isnewrecord)
		wstatus('Existing record')

	document.title = gdoctitle + ' ' + gkeyexternal.exoconvert('*', ' ')

	//enable the delete and save buttons if locked
	//according to need
	if (glocked) {
		setdisabledandhidden(saverecord, false)
		setdisabledandhidden(editreleaserecord, false)
		//editreleaserecord.value='Release'
		setgraphicbutton(editreleaserecord, 'R<u>e</u>lease', greleaseimage)
		if (deleterecord && !gds.isnewrecord && !greadonlymode && !gupdateonlymode && !gpreventdeletion)
			setdisabledandhidden(deleterecord, false)
	}
	else if (!greadonlymode) {
		setdisabledandhidden(editreleaserecord, false)
		setgraphicbutton(editreleaserecord, '<u>E</u>dit', geditimage)
	}

	//set editor textarea readonly or not
	//gCKEDITOR_EDITOR may not be ready yet when first opening the page
	//so we also do the same in its instanceReady event.
	// setReadOnly can throw "permission denied" if editor not ready — skip on error
	try {
		if (typeof gCKEDITOR_EDITOR != 'undefined')
			gCKEDITOR_EDITOR.setReadOnly(!glocked)
	} catch (e) { }

	//enable the close and print buttons
	setdisabledandhidden(closerecord, false)
	if (printsendrecord)
		setdisabledandhidden(printsendrecord, false)

	//hide the enter button
	//setdisabledandhidden(enter,true)

	//postdisplay (in opendoc2 and cleardoc) — after gds.load; use for bound-DOM / per-row work
	if (typeof form_postdisplay == 'function') {
		grecn = null
		if (!(await exoevaluateall('await form_postdisplay()', 'await opendoc2()'))) {
			if (glocked)
				await unlockdoc()//fail safe
			await cleardoc()
			return false //logout('opendoc2 - postdisplay failed')
		}

	}

	// Crush decide with record bound (cleardoc used skeleton on empty).
	if (typeof form_update_wide_layout == 'function')
		form_update_wide_layout()

	//logout('opendoc2')

	return true

}

async function loadnewkey() {

	//load the new key
	if (gkey != gro.key) {
		await setkeyvalues(gro.key)
		gkey = await getkey()
		//form key  x*y*z* could be a little different than the db key x*y*z
		gkeyexternal = await getkeyexternal()
	}

}

///////////
//CLOSE DOC
///////////
async function closedoc(mode) {

	//WINDOWUNLOAD,CLOSE,NEW,OPEN
	if (!mode || mode.type)
		mode = 'CLOSE'

	//first update the current field
	//otherwise the db is updated without the last entry!!!
	//if (save&&!(await validateupdate()))
	// return false
	//should not do validateupdate since maybe called from validateupdate to close/ timeout to open a new record

	//return await saveandorcleardoc(mode,save,clear=mode!='OPEN')
	//return await saveandorcleardoc(mode,save=mode!='CLEAR',clear=mode!='OPEN')
	var result = await saveandorcleardoc(mode)

	return result
}

//makedoc readonly
async function saveandunlockdoc() {
	if (!(await validateupdate()))
		return false
	if (gtouched && !(await savedoc()))
		return false
	if (!(await unlockdoc()))
		return false
	return true
}

//'''''''''''
//'SAVE DOC
//'''''''''''
var gform_in_savedoc = false
async function savedoc(mode) {

	//prevent reentry in async environment
	if (gform_in_savedoc)
		return false

	gform_in_savedoc = true
	var result = await savedoc_body(mode)
	gform_in_savedoc = false

	return result
}

async function savedoc_body(mode) {

	//called from await saverecord_onclick()
	if (!mode)
		mode = 'SAVE'

	//first update the current field
	//otherwise the db is updated without the last entry!!!
	if (!(await validateupdate()))
		return false

	if (gKeyNodes && (!gtouched || !glocked) && !gallowsavewithoutchanges) {
		await exoui_invalid('Nothing to be saved.\n\nPlease enter or change some data first or just click Close')
		return false
	}

	//return await saveandorcleardoc(mode,save=true,clear=false)
	return await saveandorcleardoc(mode)

}

//'''''''''''
//'SAVE DOC
//'''''''''''
var gform_in_saveandorcleardoc = false
async function saveandorcleardoc(mode) {

	//prevent reentry in async environment
	if (gform_in_saveandorcleardoc)
		return false

	gform_in_saveandorcleardoc = true

	var result = await saveandorcleardoc_body(mode)

	gform_in_saveandorcleardoc = false

	return result
}

async function saveandorcleardoc_body(mode) {

	// await debug('saveandorcleardoc:'+mode)

	//mode CLOSE, SAVE, CLEAR, CHANGEKEY, PRINT
	var save = (mode != 'CLOSE')
	//var clear=(mode!='SAVE'&&mode!='OPEN'&&mode!='CLOSE')
	var clear = (mode == 'CLOSE' || mode == 'NEW' || mode == 'CLEAR')
	var unlock = (mode != 'SAVE' && mode != 'PRINT' && mode != 'LIST' && mode != 'NEW')
	//await debug('saveandorclear '+save+' '+clear+' '+unlock)
	//called from
	//should not do validateupdate since maybe called from validateupdate to close/ timeout to open a new record

	//async function savedoc(mode)
	//('SAVE',true,false)
	//mode can be SAVE

	//async function printsendrecord_onclick()
	//('PRINT',true,false)

	//async function closedoc(mode)

	//(mode,save,true)
	//mode can be WINDOWUNLOAD,CLOSE,NEW,OPEN

	//login('saveandorcleardoc ' + save + ' ' + clear)

	//first update the current field
	//otherwise the db is updated without the last entry!!!
	//if (save&&!(await validateupdate()))
	// return false //logout('saveandorcleardoc - invalidateupdate failed')

	// Unbound / unlocked touched clear (F8, Esc→CLOSE on parameter forms): no lock,
	// so the glocked Save/Discard path never runs — still ask before wipe unless
	// gparameters.discardable (light criteria/settings modals).
	if (!glocked && gtouched && clear && !gparameters.discardable) {
		var response = await exoui_confirm('Discard data or instructions entered ?', 1, '', 'D<u>i</u>scard', '<u>C</u>ancel')
		if (response != 2) {
			focusongpreviouselement()
			return false
		}
	}

	//if anything updated then option to save
	if (glocked
		&& (gtouched
			|| (gallowsavewithoutchanges
				&& !clear && mode != 'OPEN' && mode != 'RELEASE'
			)
			|| (gpreviouselement
				&& gpreviouselement.getAttribute('exofieldno') > 0
				&& getvalue(gpreviouselement) != gpreviousvalue
			)
		)
	) {

		//check last data entry is valid unless closing in which case discard will not validate
		if (mode != 'CHANGEKEY' && mode != 'CLOSE' && !(await validateupdate())) {
			focusongpreviouselement()
			return false //logout('saveandorcleardoc - invalid input')
		}

		//opportunity to save or cancel
		if (typeof form_prewrite != 'undefined' && mode == 'SAVE') {

			//confirm specific save
			if (gparameters.savemode == 'CONFIRM') {
				if (!(await exoui_okcancel('OK to save ' + exoquote(gkeyexternal.exoconvert('*', ' ')) + ' ?', 1))) {
					//return false
					return false //logout('saveandorcleardoc - user cancelled')
				}
			}

			var response = 1
		}
		else {

			//save or discard?
			var savetitle = ''
			var discardtitle = ''
			var action = ''
			if (true || save)
				savetitle = '<u>S</u>ave'//Save
			action = savetitle
			if (true) {
				//discardtitle = 'Do<u>n</u>\'t Save'
				discardtitle = 'D<u>i</u>scard'//Discard
				if (true || save)
					action += ' or '
				action += discardtitle
			}
			var canceltitle = '<u>C</u>ancel'//Cancel
			var response = await exoui_confirm(action + ' ' + exoquote(gkeyexternal.exoconvert('*', ' ')) + ' ?', 1, savetitle, discardtitle, canceltitle)
		}

		//user cancels
		if (response == 0) {
			focusongpreviouselement()
			return false //logout('saveandorcleardoc - user cancelled')
		}

		//prepare to close modal window
		window.returnValue = ''

		//user chooses to save
		if (response == 1) {

			//first update the current field
			//otherwise the db is updated without the last entry!!!
			//if (mode!='CHANGEKEY'&&!(await validateupdate()))
			if (mode == 'CLOSE' && !(await validateupdate())) {
				focusongpreviouselement()
				return false //logout('saveandorcleardoc - user cancelled')
			}

			var savekey = gkey
			var wasnewrecord = gds.isnewrecord

			//WRITEDOC
			if (!(await writedoc(unlock)))
				return false //logout('saveandorcleardoc - writedoc failed')

			//prepare to close modal window
			window.returnValue = (wasnewrecord ? 'NEW ' : 'UPDATE ') + savekey

		}

		//close window if modal also in deletedoc
		if (window.dialogArguments && (mode == 'SAVE' || mode == 'DELETE' || mode == 'CLOSE')) {
			gwindowunloading = true
			await cleardoc()

			//close the window
			//window.returnValue=''
			//return window.close() && false
			return exoui_windowclose(window.returnValue)

		}

		//enable deleterecord
		if (!greadonlymode && !gupdateonlymode && !gpreventdeletion)
			setdisabledandhidden(deleterecord, false)

	}

	//clear
	if (clear || response == 2) {
		await cleardoc()
	}

	//unlock
	else if (unlock && glocked) {
		if (!(await unlockdoc()))
			return false //logout('saveandorcleardoc - could not unlock')
	}

	//reset
	settouched(false)

	//logout('saveandorcleardoc ok')

	return true

}

async function cleardoc() {
	//no processing if not initialised
	if (!ginitok) return

	//unconditionally closes the document and prepares a new blank record
	//but returns false if cannot unlock record
	//1. unlock
	//2. disable buttons
	//3. clear keys and record

	//called by:
	//unloaddoc if no changes or user does not cancel
	//deletedoc if users confirms

	//login('cleardoc')

	// Drop quiet typeahead (panel + miss tint) before fields are wiped
	form_typeahead_reset()

	if (gKeyNodes && !(await unlockdoc()))
		return false //logout('cleardoc - unlockdoc failed')

	//disable the buttons
	settouched(false)
	if (gKeyNodes) {
		setdisabledandhidden(saverecord, true)
		setdisabledandhidden(editreleaserecord, true)
		//editreleaserecord.value='Release'
		setgraphicbutton(editreleaserecord, 'R<u>e</u>lease', greleaseimage)
		if (deleterecord)
			setdisabledandhidden(deleterecord, true)
		setdisabledandhidden(closerecord, true)
		//if (printsendrecord)
		//    setdisabledandhidden(printsendrecord, true)
	}

	//set this before clear because .load will call await newrecordfocus()
	//gpreviouselement = null
	//gpreviousvalue = ''
	setgpreviouselement(null)

	//close it
	if (!gwindowunloading) {

		//gkey=''
		//gkeyexternal=''
		gloaded = false
		settouched(false)
		document.title = gdoctitle

		//get an empty record and load it
		wstatus('Clearing, please wait ...')
		//  if (!glogging)
		//  {
		gro.key = ''
		gro.revstr = gro.defaultrevstr
		gro.rev2obj()
		await gds.load(gro)

		//any parameters named like form by matching name
		for (var paramid in gparameters) {
			var paramvalue = gparameters[paramid]
			var dictitem = gro.dictitem(paramid)
			if (!dictitem) continue

			//update multiple values (only works for single field groups)
			if (typeof paramvalue == 'string' && dictitem.groupno > 0 && paramvalue.indexOf(vm) >= 0) {
				var element = $$(paramid)
				if (element && element[0])
					element = element[0]
				await insertallrows(element, paramvalue.split(vm))
			}

			//update single values
			else {
				await gds.setx(paramid, null, paramvalue)
			}

		}

		//  }
		wstatus('')

		await resetsortimages()

	}

	//if key is defaulted then clearing may have loaded a new record already
	if (!glocked && gkey) {
		glastkey = gkey
		gkey = ''
	}

	//not if clearing
	if (!gwindowunloading && gkey) {

		//setfocus on first non-defaulted key node
		var focussed = false
		if (gKeyNodes && gKeyNodes.length > 1) {
			for (var ii = 0; ii < gKeyNodes.length; ii++) {
				await setdefault(gKeyNodes[ii])
				if (!(getvalue(gKeyNodes[ii]))) {
					focuson(gKeyNodes[ii])
					focussed = true
					break
				}
			}
		}

		if (!focussed) {
			await setdefault(gstartelement)
			focuson(gstartelement)
			//if (gstartelement&&gstartelement.tagName!='TEXTAREA')
			if (gstartelement.select)
				try {
					gstartelement.select()
				}
				catch (e) {
					stoprighthere
				}
		}

		settouched(false)

	}

	//force any updates to be validated
	await validateupdate()

	//postdisplay in cleardoc and postinit — after gds.load; use for bound-DOM / per-row work
	if (typeof form_postdisplay == 'function') {
		grecn = null
		await exoevaluateall('await form_postdisplay()', 'await formfunctions_onload()');
	}

	// Wide decide for bound and unbound (skeleton on empty). Journals can be
	// .exoform-wide before first record paints — avoids crushed→wide flash.
	if (typeof form_update_wide_layout == 'function')
		form_update_wide_layout()

	//logout('cleardoc')

}

async function resetsortimages(groupno) {
	//reset the sort buttons
	if (!groupno) {
		for (groupno = 1; groupno < 99; groupno++) {
			await resetsortimages(groupno)
		}
		return
	}

	// Scope to this multivalue table only (TABLE#exogroupN). Column headers
	// and sort icons live in that thead. Do not touch other groups/forms.
	// All columns in the group share id sortbutton_N — query all twins in-scope.
	var tablex = document.getElementById('exogroup' + groupno)
	if (!tablex)
		return
	var elements = tablex.querySelectorAll('[id="sortbutton_' + groupno + '"]')
	for (var elementn = 0; elementn < elements.length; elementn++) {
		if (typeof exo_apply_sort_icon == 'function')
			exo_apply_sort_icon(elements[elementn], '')
		else
			exo_set_icon_element(elements[elementn], gsortimage)
	}

}

//not used
function copychildnodes(fromcell, tocell) {
	for (var childn = 0; childn < fromcell.childNodes.length; childn++) {
		var fromchildcell = fromcell.childNodes[childn]
		var tochildcell = tocell.insertBefore(fromchildcell.cloneNode(false), null)
		if (tochildcell.tagName == 'TD') tochildcell.innerText = ''
		if (fromchildcell.childNodes.length > 0) copychildnodes(fromchildcell, tochildcell)
	}
}

async function deletedoc() {

	//prevent delete if not locked
	if (!glocked)
		return await exoui_invalid(await readonlydocmsg())

	//prevent delete if new record
	if (gds.isnewrecord)
		return await exoui_invalid('You cannot delete this document because it hasnt been saved')

	//login('deletedoc')

	var question = exoquote(gkeyexternal) + '\nWarning! Are you SURE that you want to delete this document?'
	if ((await exoui_yesno(question, 2)) != 1) {
		await exoui_invalid('The document has NOT been deleted\nbecause you did not confirm.')
		return false //logout('deletedoc - user cancelled')
	}

	//stop relocking while trying to delete
	//not really necessary as DELETE is synchronous
	stoprelocker()
	var savekey = gkey

	//delete it
	db.request = 'DELETE\r' + gdatafilename + '\r' + gkey + '\r\r' + gro.sessionid
	if (!(await db.send())) {

		//await exoui_note(db.response)
		await exoui_invalid(db.response)

		//start the relocker again
		startrelocker()

		return false //logout('deletedoc - db returns false')

	}

	//any warnings are appended after response like 'OK xxx'
	if (db.response != 'OK' && typeof form_postdelete == 'undefined')
		await exoui_warning(db.response.slice(2))

	//deleting a record automatically unlocks it
	glocked = false

	await exoui_warning(exoquote(gkeyexternal) + ' has been deleted.')

	//close window if modal also in writedoc
	if (window.dialogArguments) {
		gwindowunloading = true
		await cleardoc()

		//close the window
		//window.returnValue = 'DELETE ' + savekey
		//return window.close()
		return exoui_windowclose('DELETE ' + savekey)

	}

	await cleardoc()

	//logout('deletedoc')

	return true

}

// Debounced dict.onchange (find-as-you-type). Not full validate — side-effects stay on blur.
var gform_onchange_timer = null
var gform_onchange_seq = 0
var gform_onchange_element = null
var gform_oninput_delegated = false
// Private link for quiet typeahead I/O only — leave-field validate keeps main db.
var gform_typeahead_db = null

function form_typeahead_dblink() {
	if (!gform_typeahead_db && typeof exodblink == 'function') {
		gform_typeahead_db = new exodblink()
		// Quiet: no blockmodalui on send (avoids scroll-to-top every key)
		gform_typeahead_db.quiet = true
		// Typeahead I/O is client-cacheable by full request string (gcache: Alt+R,
		// or reload attach then clearcache()). exo_typeahead prefixes CACHE\r.
		var _ta_send = gform_typeahead_db.send
		gform_typeahead_db.send = async function form_typeahead_send(data) {
			if (this.request && String(this.request).slice(0, 6) != 'CACHE\r')
				this.request = 'CACHE\r' + this.request
			return await _ta_send.call(this, data)
		}
	}
	return gform_typeahead_db || db
}

// Drop an in-flight typeahead send so a newer query can use the private link.
// abort() is OK — same contract as Wait/Cancel (onabort → 'Cancelled', not an error).
// See client.js xhttp.onabort.
function form_typeahead_dblink_reset() {
	var tdb = gform_typeahead_db
	if (!tdb)
		return
	try {
		if (tdb.requesting && tdb.XMLHTTP)
			tdb.XMLHTTP.abort()
	} catch (e) { }
	tdb.requesting = false
	tdb.request = ''
}

async function form_oninput(event) {
	event = getevent(event)

	var element = event.target
	if (!element || !element.getAttribute)
		return true
	// Document-delegated: only bound form fields (attrs survive row cloneNode)
	if (element.getAttribute('exofieldno') == null && !element.getAttribute('exoonchange'))
		return true
	if (element.tagName == 'SELECT')
		return true

	//changing key fields does not cause gtouched
	var fn = Number(element.getAttribute('exofieldno'))
	if (fn && !gtouched) {
		//remember this element so pressing escape can cancel gtouched
		//removed in onfocus
		gelementthatjustcalledsettouched = element
		settouched(true)
	}

	// optional live onchange (e.g. brand_code_onchange) — debounced typeahead
	var onchangexpr = element.getAttribute('exoonchange')
	if (onchangexpr) {
		gform_onchange_element = element
		// Supersede in-flight typeahead for older text. Without this, a late
		// empty response for "A" can paint red miss after the user has already
		// typed "AB" and before the new search opens the list.
		gform_onchange_seq++
		form_typeahead_set_miss(element, false)
		if (typeof form_typeahead_hide == 'function')
			form_typeahead_hide()
		form_typeahead_dblink_reset()
		if (gform_onchange_timer)
			window.clearTimeout(gform_onchange_timer)
		gform_onchange_timer = window.setTimeout(function () {
			gform_onchange_timer = null
			form_run_onchange(element, onchangexpr)
		}, 200)
	}

	return true
}

async function form_run_onchange(element, onchangexpr) {

	if (!element || !onchangexpr)
		return
	// stale timer after focus moved
	if (gform_onchange_element !== element)
		return
	if (document.activeElement !== element)
		return

	var seq = ++gform_onchange_seq
	var text = getvalue(element)
	if (text == null)
		text = ''
	text = text.toString()
	if (!element.getAttribute('exolowercase'))
		text = text.toUpperCase()
	// keep field display in sync for search (matches validate uppercase)
	if (getvalue(element) != text)
		setvalue(element, text)

	if (!String(text).replace(/^\s+|\s+$/g, '')) {
		form_typeahead_set_miss(element, false)
		form_typeahead_hide()
		return
	}

	// green allownew: stay tinted, no re-search (red uses form_is_miss_tinted to block keys)
	if (element.classList && element.classList.contains('exotypeahead_new'))
		return

	gvalue = text

	// Clear prior tint before I/O
	form_typeahead_set_miss(element, false)

	// do not set gpreviouselement here — leave-field validate owns that
	// one quiet request at a time on the private link
	form_typeahead_dblink_reset()
	var ok
	try {
		ok = await exoevaluate(onchangexpr, 'form_run_onchange ' + element.id)
	} catch (e) {
		console.log('form_run_onchange', element.id, e)
		return
	}
	if (seq != gform_onchange_seq)
		return
	return ok
}

// ---------------------------------------------------------------------------
// Find-as-you-type suggest panel (non-modal; not exoui_decide)
// Panel is fixed in the viewport but re-anchored on scroll/resize so it stays
// glued under the field (window or nested overflow scroll).
// ---------------------------------------------------------------------------
var gform_typeahead_div = null
var gform_typeahead_element = null
var gform_typeahead_returncoln = 0
var gform_typeahead_rows = []
var gform_typeahead_focusn = -1
var gform_typeahead_mousedown = false
var gform_typeahead_scroll_listening = false
// mouseover ignored while locked (open + keyboard); mousemove unlocks.
var gform_typeahead_hover_locked = false

function form_typeahead_ensure() {

	if (gform_typeahead_div)
		return gform_typeahead_div
	var div = document.createElement('div')
	div.id = 'exo_typeahead'
	div.className = 'exo_typeahead'
	div.style.display = 'none'
	div.onmousedown = function () {
		gform_typeahead_mousedown = true
	}
	div.onmouseup = function () {
		gform_typeahead_mousedown = false
	}
	// Real pointer move only — not list open under a stationary mouse
	div.onmousemove = function () {
		gform_typeahead_hover_locked = false
	}
	// Wheel = normal overflow scroll only (CSS overscroll-behavior: contain).
	// No JS option-stepping (decide does that). No custom scrollTop.
	document.body.appendChild(div)
	gform_typeahead_div = div
	return div
}

function form_typeahead_scroll_sync() {

	if (!gform_typeahead_div || gform_typeahead_div.style.display == 'none')
		return
	if (!gform_typeahead_element)
		return
	// Field scrolled out of view → hide rather than float orphaned
	var r = gform_typeahead_element.getBoundingClientRect()
	if (r.bottom < 0 || r.top > window.innerHeight || r.right < 0 || r.left > window.innerWidth) {
		form_typeahead_hide()
		return
	}
	form_typeahead_place(gform_typeahead_element, gform_typeahead_div)
}

function form_typeahead_listen_scroll(on) {

	if (on && !gform_typeahead_scroll_listening) {
		// capture: nested overflow panes scroll without bubbling
		window.addEventListener('scroll', form_typeahead_scroll_sync, true)
		window.addEventListener('resize', form_typeahead_scroll_sync, true)
		gform_typeahead_scroll_listening = true
	}
	else if (!on && gform_typeahead_scroll_listening) {
		window.removeEventListener('scroll', form_typeahead_scroll_sync, true)
		window.removeEventListener('resize', form_typeahead_scroll_sync, true)
		gform_typeahead_scroll_listening = false
	}
}

// Detect create-style allownew for a bound field (for typeahead etc.).
//  - Form forbids new records: gupdateonlymode / gpreventcreation / greadonlymode
//    (same as New button hide — e.g. users.htm gupdateonlymode=true)
//  - EXECUTIVE_CODE / BRAND_EXECUTIVE_CODE: always true (for now)
//  - else true only if fieldno 0 and single-part key (gKeyNodes length 1)
function form_field_is_allownew(el) {
	if (!el || !el.getAttribute)
		return false
	// No new records on this form → typeahead miss is red, not green allownew
	if ((typeof gupdateonlymode != 'undefined' && gupdateonlymode)
		|| (typeof gpreventcreation != 'undefined' && gpreventcreation)
		|| (typeof greadonlymode != 'undefined' && greadonlymode))
		return false
	var id = el.id || el.getAttribute('exoname') || ''
	if (id == 'EXECUTIVE_CODE' || id == 'BRAND_EXECUTIVE_CODE')
		return true
	if (String(el.getAttribute('exofieldno')) !== '0')
		return false
	// multipart key (e.g. ratecards vehicle+date): not allownew for key parts alone
	if (typeof gKeyNodes == 'undefined' || !gKeyNodes || gKeyNodes.length !== 1)
		return false
	return true
}

// Typeahead no-hit tints (class only; typeahead is the producer):
//   .exotypeahead_miss — red/Highlight: not found, not allownew (form_is_miss_tinted blocks keys)
//   .exotypeahead_new  — green allownew (form_run_onchange skips re-search)
function form_is_miss_tinted(el) {
	return !!(el && el.classList && el.classList.contains('exotypeahead_miss'))
}

function form_typeahead_clear_miss() {
	var lists = ['exotypeahead_miss', 'exotypeahead_new']
	for (var c = 0; c < lists.length; c++) {
		var list = document.getElementsByClassName(lists[c])
		for (var i = list.length - 1; i >= 0; i--)
			list[i].classList.remove(lists[c])
	}
}

// miss true → red; miss 'new' → green allownew; miss false → clear
function form_typeahead_set_miss(el, miss) {
	if (miss === 'new' || miss === true) {
		if (!el || !el.classList)
			return
		form_typeahead_clear_miss()
		el.classList.add(miss === 'new' ? 'exotypeahead_new' : 'exotypeahead_miss')
		return
	}
	if (!el) {
		form_typeahead_clear_miss()
		return
	}
	if (el.classList) {
		el.classList.remove('exotypeahead_miss')
		el.classList.remove('exotypeahead_new')
	}
}

// While miss-tinted: block insert keys; BS/Delete/nav free.
// Returns false to cancel the key, null to leave alone.
function form_miss_tint_keydown(event) {
	var el = event && event.target
	if (!form_is_miss_tinted(el))
		return null
	if (event.ctrlKey || event.metaKey || event.altKey)
		return null
	var key = event.key
	if (!key)
		return null
	if (key === 'Backspace' || key === 'Delete' || key === 'Tab' || key === 'Escape'
		|| key === 'Enter' || key === 'Home' || key === 'End'
		|| key === 'ArrowLeft' || key === 'ArrowRight'
		|| key === 'ArrowUp' || key === 'ArrowDown'
		|| key === 'PageUp' || key === 'PageDown')
		return null
	// Non-character keys (F1…, dead keys as multi-char names, …)
	if (key.length !== 1)
		return null
	// Replace a selection — not "additional" entry
	try {
		if (typeof el.selectionStart === 'number' && el.selectionEnd > el.selectionStart)
			return null
	} catch (e) { }
	return false
}

// Stop pending quiet search and drop the panel (does not touch miss tint).
function form_typeahead_cancel() {
	if (gform_onchange_timer) {
		window.clearTimeout(gform_onchange_timer)
		gform_onchange_timer = null
	}
	gform_onchange_seq++
	form_typeahead_dblink_reset()
	form_typeahead_hide()
}

// Record boundary: drop quiet search, panel, and any miss tint.
function form_typeahead_reset() {
	form_typeahead_cancel()
	form_typeahead_clear_miss()
}

function form_typeahead_hide() {

	form_typeahead_listen_scroll(false)
	if (gform_typeahead_div) {
		gform_typeahead_div.style.display = 'none'
		gform_typeahead_div.scrollTop = 0
		gform_typeahead_div.scrollLeft = 0
	}
	gform_typeahead_element = null
	gform_typeahead_rows = []
	gform_typeahead_focusn = -1
	gform_typeahead_hover_locked = false
	gform_typeahead_select_all = false
	gform_typeahead_ctx_menu = false
	gform_typeahead_prefix = ''
	if (gform_typeahead_ctx_timer) {
		window.clearTimeout(gform_typeahead_ctx_timer)
		gform_typeahead_ctx_timer = null
	}
}

// Free viewport band for typeahead (below sticky menubar).
function form_typeahead_free_band() {

	var vw = window.innerWidth || document.documentElement.clientWidth || 0
	var vh = window.innerHeight || document.documentElement.clientHeight || 0
	var stickyTop = 0
	try {
		stickyTop = parseFloat(
			window.getComputedStyle(document.documentElement)
				.getPropertyValue('--exosticky-top')
		) || 0
	} catch (e) { }
	var pad = 4
	return {
		left: pad,
		right: vw - pad,
		top: stickyTop + pad,
		bottom: vh - pad,
		vw: vw,
		vh: vh
	}
}

/*
 * Always under the field (never flip above). fixed + live scroll sync.
 * Horizontal: align under field left; if past free-band right, shift left by
 * the overflow (not past free-band left).
 */
function form_typeahead_place(element, div) {

	if (!element || !div)
		return
	var r = element.getBoundingClientRect()
	var margin = 8
	var band = form_typeahead_free_band()
	var left0 = Math.max(band.left, r.left)
	div.style.position = 'fixed'
	div.style.left = left0 + 'px'
	div.style.top = (r.bottom + 2) + 'px'
	// Full free-band width available after a left shift (not only space right of field)
	var maxW = Math.max(120, band.right - band.left)
	var minW = Math.max(r.width, 280)
	if (minW > maxW)
		minW = maxW
	div.style.minWidth = minW + 'px'
	div.style.maxWidth = maxW + 'px'
	var maxH = Math.max(120, (band.bottom || window.innerHeight) - (r.bottom + 2) - margin)
	div.style.maxHeight = maxH + 'px'
	div.style.zIndex = 10050
	// Layout then clamp: move left by overflow right, not past band.left
	void div.offsetWidth
	var pr = div.getBoundingClientRect()
	if (pr.right > band.right) {
		var left = pr.left - (pr.right - band.right)
		if (left < band.left)
			left = band.left
		div.style.left = left + 'px'
	}
}

/*
 * Scroll the *main window only* so the panel (always under the field) can show
 * ≥ half its preferred size on-screen. Does not reposition the typeahead
 * relative to the field — only window.scrollBy, then re-place under field.
 */
function form_typeahead_scroll_main_into_view(element, div) {

	if (!element || !div || !div.getBoundingClientRect)
		return

	var margin = 8
	var gap = 2
	var minW = 280
	var preferCap = 28 * 16
	try {
		preferCap = 28 * (parseFloat(window.getComputedStyle(div).fontSize) || 16)
	} catch (e) { }

	var pass
	for (pass = 0; pass < 3; pass++) {
		form_typeahead_place(element, div)
		void div.offsetHeight

		var band = form_typeahead_free_band()
		if (!(band.vw > 0) || !(band.vh > 0) || band.right <= band.left || band.bottom <= band.top)
			return

		var fr = element.getBoundingClientRect()
		var pr = div.getBoundingClientRect()
		var pw = pr.right - pr.left
		var ph = pr.bottom - pr.top
		if (!(pw > 0) || !(ph > 0))
			return

		// Preferred size from content (not the cramped maxHeight when field is low)
		var naturalH = Math.min(div.scrollHeight || ph, preferCap)
		var naturalW = Math.min(Math.max(div.scrollWidth || pw, minW), preferCap * 2)
		var halfH = Math.max(60, naturalH * 0.5)
		var halfW = Math.max(minW * 0.5, naturalW * 0.5)

		// Room below field for panel (always opens under field)
		var spaceBelow = band.bottom - (fr.bottom + gap)
		var spaceRight = band.right - fr.left - margin

		var dx = 0
		var dy = 0
		// scrollBy(+dy): field moves up on screen → more room below for panel
		if (spaceBelow < halfH - 0.5) {
			dy = halfH - spaceBelow
			var maxDy = Math.max(0, fr.top - band.top - 8)
			if (dy > maxDy)
				dy = maxDy
		}
		if (spaceRight < halfW - 0.5) {
			dx = halfW - spaceRight
			var maxDx = Math.max(0, fr.left - band.left - 8)
			if (dx > maxDx)
				dx = maxDx
		}

		// Residual: painted panel still under half in free band
		var visL = Math.max(pr.left, band.left)
		var visR = Math.min(pr.right, band.right)
		var visT = Math.max(pr.top, band.top)
		var visB = Math.min(pr.bottom, band.bottom)
		var visW = Math.max(0, visR - visL)
		var visH = Math.max(0, visB - visT)
		if (visH < ph * 0.5 - 0.5 && !dy) {
			if (pr.bottom > band.bottom)
				dy = pr.bottom - band.bottom
			else if (pr.top < band.top)
				dy = pr.top - band.top
		}
		if (visW < pw * 0.5 - 0.5 && !dx) {
			if (pr.right > band.right)
				dx = pr.right - band.right
			else if (pr.left < band.left)
				dx = pr.left - band.left
		}

		if (!(dx || dy))
			break
		window.scrollBy(dx, dy)
	}
	form_typeahead_place(element, div)
}

// Default highlight for open list: exact match on returnable col → that row;
// else first row when not allownew. allownew + no match → -1 (Enter leaves typed as new).
// Multi-code fields: match last segment only (gform_typeahead_prefix restored on pick).
function form_typeahead_auto_focusn(element, rows, returncoln) {
	if (!rows || !rows.length)
		return -1
	var typed = ''
	if (element) {
		typed = (typeof getvalue == 'function') ? getvalue(element) : (element.value || '')
		typed = String(typed == null ? '' : typed)
		if (typeof gform_typeahead_prefix == 'string' && gform_typeahead_prefix
			&& typed.indexOf(gform_typeahead_prefix) === 0)
			typed = typed.slice(gform_typeahead_prefix.length)
		typed = typed.replace(/^\s+|\s+$/g, '')
		if (!(element.getAttribute && element.getAttribute('exolowercase')))
			typed = typed.toUpperCase()
	}
	if (typed) {
		for (var r = 0; r < rows.length; r++) {
			var cell = rows[r] ? rows[r][returncoln] : ''
			if (cell == null)
				cell = ''
			cell = String(cell).replace(/^\s+|\s+$/g, '')
			if (!(element && element.getAttribute && element.getAttribute('exolowercase')))
				cell = cell.toUpperCase()
			if (cell === typed)
				return r
		}
	}
	if (typeof form_field_is_allownew == 'function' && form_field_is_allownew(element))
		return -1
	return 0
}

// cols: [[id,title],…] or [id,…]; rows: [[cell,…],…]; returncoln: 0-based col to write on pick
function form_typeahead_show(element, cols, rows, returncoln) {

	if (!element || !rows || !rows.length) {
		form_typeahead_hide()
		return
	}
	if (typeof returncoln == 'undefined' || returncoln == null || returncoln === '')
		returncoln = 0
	returncoln = Number(returncoln)
	if (isNaN(returncoln))
		returncoln = 0

	// Cap display only — do not change backend tools (FINDACCOUNT/VAL/GETACC).
	// SELECT typeahead requests pass the same typeahead_limitn as maxnrecs.
	// rg typeahead_limitn for mass-update. Exact-N hit may mean server truncated.
	var typeahead_limitn = 100
	var truncated = rows.length >= typeahead_limitn
	if (rows.length > typeahead_limitn)
		rows = rows.slice(0, typeahead_limitn)

	var div = form_typeahead_ensure()
	gform_typeahead_element = element
	gform_typeahead_returncoln = returncoln
	gform_typeahead_rows = rows
	gform_typeahead_focusn = -1
	var autoFocus = form_typeahead_auto_focusn(element, rows, returncoln)

	// col[0] may be a numeric field index into the row (same as exoui_decide / ACCOUNTLIST).
	// col[1] is the title when col is [id, title, …].
	var html = '<table class="exo_typeahead_table" cellspacing="0" cellpadding="0">'
	html += '<thead><tr class="exo_typeahead_head">'
	for (var c0 = 0; c0 < cols.length; c0++) {
		var coldef0 = cols[c0]
		var title = ''
		if (typeof coldef0 == 'object' && coldef0 != null && coldef0[1] != null && coldef0[1] !== '')
			title = coldef0[1]
		else if (typeof coldef0 == 'string')
			title = coldef0
		else if (typeof coldef0 == 'object' && coldef0 != null && coldef0[0] != null)
			title = coldef0[0]
		html += '<th>' + HTMLEncode(String(title)) + '</th>'
	}
	html += '</tr></thead><tbody>'
	for (var r = 0; r < rows.length; r++) {
		html += '<tr data-ta-row="' + r + '">'
		var row = rows[r]
		for (var c2 = 0; c2 < cols.length; c2++) {
			var coldef = cols[c2]
			var cell = ''
			if (row) {
				if (typeof coldef == 'object' && coldef != null
					&& (typeof coldef[0] == 'number'
						|| (typeof coldef[0] == 'string' && coldef[0] !== ''
							&& String(Number(coldef[0])) === String(coldef[0]))))
					cell = row[Number(coldef[0])]
				else
					cell = row[c2]
			}
			if (cell == null)
				cell = ''
			html += '<td>' + HTMLEncode(String(cell)) + '</td>'
		}
		html += '</tr>'
	}
	html += '</tbody></table>'
	if (truncated)
		html += '<div class="exo_typeahead_truncated">Showing first ' + typeahead_limitn + ' — type more to narrow</div>'
	div.innerHTML = html
	if (truncated)
		div.classList.add('exo_typeahead_is_truncated')
	else
		div.classList.remove('exo_typeahead_is_truncated')
	form_typeahead_place(element, div)
	div.style.display = ''
	// Must reset AFTER display is visible — scrollTop while display:none is ignored
	// (reused panel kept PageDown offset across Esc → retype same key).
	div.scrollTop = 0
	div.scrollLeft = 0
	// Scroll main page so ≥ half preferred panel size is usable (v+h)
	form_typeahead_scroll_main_into_view(element, div)
	div.scrollTop = 0
	div.scrollLeft = 0
	form_typeahead_listen_scroll(true)

	// Only data rows (data-ta-row); header has no pick/hover handlers.
	// click (not mousedown) to pick so drag-select + Ctrl+C / right-click Copy work.
	var trs = div.querySelectorAll('tr[data-ta-row]')
	for (var i = 0; i < trs.length; i++) {
		trs[i].onmouseover = form_typeahead_row_hover
		trs[i].onclick = form_typeahead_row_pick
	}
	// Arrival: auto-highlight for Enter; mouseover ignored until mousemove unlocks
	gform_typeahead_hover_locked = true
	// Highlight now so Enter before rAF still applies the match / first row.
	if (autoFocus >= 0)
		form_typeahead_set_focus(autoFocus)
	// After paint: main into-view; re-apply focus scroll for mid-list exact match.
	if (typeof requestAnimationFrame == 'function') {
		requestAnimationFrame(function () {
			if (gform_typeahead_div !== div || div.style.display == 'none')
				return
			form_typeahead_scroll_main_into_view(element, div)
			if (autoFocus >= 0)
				form_typeahead_set_focus(autoFocus)
			else {
				div.scrollTop = 0
				div.scrollLeft = 0
			}
		})
	}
}

function form_typeahead_row_hover(event) {

	if (gform_typeahead_hover_locked)
		return
	event = getevent(event)
	var tr = event.target
	while (tr && tr.tagName != 'TR')
		tr = tr.parentNode
	if (!tr || tr.getAttribute('data-ta-row') == null)
		return
	form_typeahead_set_focus(Number(tr.getAttribute('data-ta-row')))
}

// True if the user has a non-empty text selection inside the open typeahead list.
function form_typeahead_selection_in_list() {
	var div = gform_typeahead_div
	if (!div || div.style.display == 'none')
		return false
	var sel = window.getSelection && window.getSelection()
	if (!sel || sel.isCollapsed || !sel.rangeCount)
		return false
	return !!(div.contains(sel.anchorNode) || div.contains(sel.focusNode))
}

// Ctrl+A / context-menu Select All: select whole list (incl. scrolled). Copy uses gform_typeahead_rows.
var gform_typeahead_select_all = false
// After right-click on list/field, watch selectionchange so menu "Select All" re-scopes to the list.
var gform_typeahead_ctx_menu = false
var gform_typeahead_ctx_timer = null

function form_typeahead_select_all_list() {
	var div = gform_typeahead_div
	if (!div || div.style.display == 'none')
		return false
	var table = div.querySelector('.exo_typeahead_table')
	if (!table)
		return false
	if (window.getSelection && document.createRange) {
		var sel = window.getSelection()
		var range = document.createRange()
		range.selectNodeContents(table)
		sel.removeAllRanges()
		sel.addRange(range)
	}
	gform_typeahead_select_all = true
	return true
}

// Context-menu Select All usually selects the field or the whole page (focus stays on the field).
// After a right-click on the open list or its field, clamp any non-list selection to the full list.
function form_typeahead_contextmenu_watch(event) {
	if (!gform_typeahead_div || gform_typeahead_div.style.display == 'none')
		return
	event = getevent(event)
	var t = event.target
	if (!t)
		return
	var onList = gform_typeahead_div.contains(t)
	var onField = gform_typeahead_element
		&& (t === gform_typeahead_element || (gform_typeahead_element.contains && gform_typeahead_element.contains(t)))
	if (!onList && !onField)
		return
	gform_typeahead_ctx_menu = true
	if (gform_typeahead_ctx_timer)
		window.clearTimeout(gform_typeahead_ctx_timer)
	gform_typeahead_ctx_timer = window.setTimeout(function () {
		gform_typeahead_ctx_menu = false
		gform_typeahead_ctx_timer = null
	}, 5000)
}

function form_typeahead_on_selectionchange() {
	if (!gform_typeahead_ctx_menu)
		return
	if (!gform_typeahead_div || gform_typeahead_div.style.display == 'none')
		return
	var sel = window.getSelection && window.getSelection()
	if (!sel || !sel.rangeCount || sel.isCollapsed)
		return
	// Already confined to the list (drag-select) — leave alone unless select-all flag needed
	if (form_typeahead_selection_in_list())
		return
	// Select All on field/page → same as Ctrl+A on the list
	form_typeahead_select_all_list()
}

// Full list as TSV (headers + every data row in gform_typeahead_rows — not just viewport).
function form_typeahead_list_tsv() {
	var lines = []
	var div = gform_typeahead_div
	if (div) {
		var ths = div.querySelectorAll('thead th')
		if (ths.length) {
			var h = []
			for (var i = 0; i < ths.length; i++)
				h.push(String(ths[i].textContent || '').replace(/\t/g, ' ').replace(/\r?\n/g, ' '))
			lines.push(h.join('\t'))
		}
	}
	var rows = gform_typeahead_rows || []
	for (var r = 0; r < rows.length; r++) {
		var row = rows[r]
		var cells = []
		if (row) {
			var n = (typeof row.length == 'number') ? row.length : 0
			for (var c = 0; c < n; c++)
				cells.push(String(row[c] == null ? '' : row[c]).replace(/\t/g, ' ').replace(/\r?\n/g, ' '))
		}
		lines.push(cells.join('\t'))
	}
	return lines.join('\n')
}

// Put plain text on clipboard during copy event (same constraints as form_copy_text_field_sync).
function form_typeahead_copy_text_sync(event, text) {
	if (text == null || text === '')
		return false
	var clip = event.clipboardData || window.clipboardData
	if (!clip || !clip.setData)
		return false
	try {
		if (window.clipboardData && clip === window.clipboardData)
			clip.setData('Text', text)
		else {
			clip.setData('text/plain', text)
			clip.setData('text/html', String(text)
				.replace(/&/g, '&amp;')
				.replace(/</g, '&lt;')
				.replace(/>/g, '&gt;')
				.replace(/\n/g, '<br>\n'))
		}
	} catch (e) {
		return false
	}
	return exocancelevent(event) || true
}

function form_typeahead_row_pick(event) {

	event = getevent(event)
	// Right/middle button: no pick (context menu / selection). Left click only.
	if (event.button != null && event.button !== 0)
		return
	// Drag-selected list text → leave selection for Ctrl+C / right-click Copy
	if (form_typeahead_selection_in_list())
		return
	gform_typeahead_select_all = false
	var tr = event.target
	while (tr && tr.tagName != 'TR')
		tr = tr.parentNode
	if (!tr || tr.getAttribute('data-ta-row') == null)
		return false
	form_typeahead_apply(Number(tr.getAttribute('data-ta-row')))
	return false
}

// fromkeys: arrows/Pg/Home/End steal highlight and lock until mousemove.
function form_typeahead_set_focus(n, fromkeys) {

	var div = gform_typeahead_div
	if (!div)
		return
	var trs = div.querySelectorAll('tr[data-ta-row]')
	if (n < 0)
		n = 0
	if (n >= trs.length)
		n = trs.length - 1
	gform_typeahead_focusn = n
	for (var i = 0; i < trs.length; i++) {
		if (i == n)
			trs[i].classList.add('exo_typeahead_focus')
		else
			trs[i].classList.remove('exo_typeahead_focus')
	}
	if (fromkeys)
		gform_typeahead_hover_locked = true
	// Keep highlight below sticky thead (same helper as decide)
	if (trs[n]) {
		if (typeof exo_scroll_row_below_sticky_thead == 'function')
			exo_scroll_row_below_sticky_thead(trs[n], gform_typeahead_div)
		else if (trs[n].scrollIntoView)
			trs[n].scrollIntoView({ block: 'nearest' })
		// Following row (if any) — or truncation footer after the last data row
		var follow = trs[n + 1] || div.querySelector('.exo_typeahead_truncated')
		if (follow) {
			var paneRect = div.getBoundingClientRect()
			var followRect = follow.getBoundingClientRect()
			if (followRect.bottom > paneRect.bottom)
				div.scrollTop += (followRect.bottom - paneRect.bottom)
		}
	}
}

// Mouse/keyboard pick: set value and focusnext only.
// Validation is normal leave-field via gpreviouselement (same as Tab/Enter).
// gform_typeahead_prefix: optional multi-value prefix (e.g. "AAA,") restored on pick.
var gform_typeahead_prefix = ''

function form_typeahead_apply(n) {

	var element = gform_typeahead_element
	var rows = gform_typeahead_rows
	if (!element || !rows || !rows[n])
		return
	var coln = gform_typeahead_returncoln
	var val = rows[n][coln]
	if (val == null)
		val = ''
	if (typeof gform_typeahead_prefix == 'string' && gform_typeahead_prefix)
		val = gform_typeahead_prefix + val
	gform_typeahead_prefix = ''
	if (gform_onchange_timer) {
		window.clearTimeout(gform_onchange_timer)
		gform_onchange_timer = null
	}
	gform_onchange_seq++
	setvalue(element, val)
	form_typeahead_set_miss(element, false)
	form_typeahead_hide()
	focusnext(element)
}

// Esc dismiss; arrows / PgUp/PgDn / Home / End move highlight;
// Enter applies current highlight (auto: exact returnable match, or first if not allownew).
// No highlight (allownew + no match): hide list and let normal Enter / focusnext run.
// Up/Down wrap (top↔bottom), same as decide list arrows. PgUp/PgDn clamp.
// PgUp/PgDn step by a fixed page of rows (viewport auto-count is unreliable with wrapping cells).
var gform_typeahead_pagesize = 10

function form_typeahead_keydown(event) {

	if (!gform_typeahead_div || gform_typeahead_div.style.display == 'none')
		return null
	var keycode = event.keyCode ? event.keyCode : event.which
	// Ctrl/Cmd+A: select whole typeahead list (incl. scrolled rows), not the field
	if ((event.ctrlKey || event.metaKey) && !event.altKey && keycode == 65) {
		if (form_typeahead_select_all_list())
			return false
		return null
	}
	// Other modifiers: leave to form/browser (Ctrl+C, ctrl+home, …)
	if (event.ctrlKey || event.altKey || event.metaKey)
		return null
	if (keycode == 27) {
		form_typeahead_hide()
		return false
	}
	var nrows = gform_typeahead_rows ? gform_typeahead_rows.length : 0
	// Down / Up — wrap at ends. No highlight yet: Down → first, Up → last.
	if (keycode == 40 || keycode == 38) {
		if (!nrows)
			return false
		var n = gform_typeahead_focusn
		if (n < 0)
			n = (keycode == 38) ? nrows - 1 : 0
		else if (keycode == 40)
			n = (n + 1) % nrows
		else
			n = (n - 1 + nrows) % nrows
		form_typeahead_set_focus(n, true)
		return false
	}
	// Page Down / Page Up — fixed step (gform_typeahead_pagesize); clamp at ends
	if (keycode == 34) {
		form_typeahead_set_focus(gform_typeahead_focusn < 0 ? 0 : gform_typeahead_focusn + gform_typeahead_pagesize, true)
		return false
	}
	if (keycode == 33) {
		form_typeahead_set_focus(gform_typeahead_focusn < 0 ? 0 : gform_typeahead_focusn - gform_typeahead_pagesize, true)
		return false
	}
	// Home → first row; End → last (form_typeahead_set_focus clamps)
	if (keycode == 36) {
		form_typeahead_set_focus(0, true)
		return false
	}
	if (keycode == 35) {
		form_typeahead_set_focus(999999, true)
		return false
	}
	if (keycode == 13) {
		if (gform_typeahead_focusn >= 0) {
			form_typeahead_apply(gform_typeahead_focusn)
			return false
		}
		form_typeahead_hide()
		return null
	}
	return null
}

// Hide when focus leaves the field (unless mousedown on list).
// Re-focus also drops the panel so a stale list (old INC after INCO) is not shown again.
// Context-menu Select All: watch selectionchange after right-click on list/field.
;(function form_typeahead_blur_install() {
	if (typeof document == 'undefined' || !document.addEventListener)
		return
	document.addEventListener('focusin', function (event) {
		if (!gform_typeahead_element)
			return
		if (gform_typeahead_mousedown)
			return
		if (gform_typeahead_div && gform_typeahead_div.contains(event.target))
			return
		// leave field or return to it — drop panel; typing re-arms search
		form_typeahead_hide()
	}, true)
	document.addEventListener('contextmenu', form_typeahead_contextmenu_watch, true)
	document.addEventListener('selectionchange', form_typeahead_on_selectionchange)
})()

async function form_onchangeselect(event) {

	event = getevent(event)

	if (!gpreviouselement) {
		//gpreviouselement = event.target
		//gpreviousvalue=null
		setgpreviouselement(event.target)
	}

	if (!(await validateupdate(event)))
		return await exoui_invalid()

	//encourage changing key or key part in a SELECT to change record
	if (gpreviouselement.getAttribute('exofieldno') == '0')
		focusnext()

	return true
}

async function validateall(mode) {

	//login('validateall ' + mode)

	//check all required fields are present
	//if mode is 'filldefaults' use default if possible
	for (groupno = 0; groupno == 0 || groupno <= gmaxgroupno; groupno++) {

		//skip missing groups
		if (groupno > 0 && !gtables[groupno])
			continue

		//for groups, we need to know the first input column name
		//if (groupno>0) var firstcolumnname=gfields[gtables[groupno][0]].name
		if (groupno > 0)
			var firstcolumnname = gfields[gtables[groupno][0]].id

		//get header row or single row of group
		var rows = (groupno == 0) ? [gds.data] : gds.data['group' + groupno]

		var tablex = gtables[groupno]
		var allowemptyrows = (groupno == 0) ? false : tablex.tableelement.getAttribute('exoallowemptyrows')
		var rowrequired = (groupno == 0) ? false : tablex.tableelement.getAttribute('exorowrequired')
		var nrowsfound = 0

		for (var rown = 0; rown < rows.length; rown++) {
			var row = rows[rown]

			////only validate rows if the first input column is entered
			////if (groupno==0||row[firstcolumnname].element.text!='')
			//if (groupno==0||row[firstcolumnname].text!='')

			var anydata = false
			var missingelement = ''
			var propname
			for (propname in row) {
				var cell = row[propname]

				//skip unbound, calculated and not required cells
				var element = cell.element
				if (!element || element.getAttribute('exotype') != 'F')
					continue

				//skip cells with data
				if (cell.text && (typeof cell.text != 'string' || cell.text.replace(/ *$/, ''))) {

					//SELECT elements
					//if (element.tagName=='SELECT'&&element.options[0].selected)

					anydata = true
					continue
				}

				//only interested in required fields from here on
				if (!(element.getAttribute('exorequired')))
					continue

				//skip cells that can be defaulted
				//do not default in rows to avoid problem of blank line and first/line line
				//filling in unwanted data (perhaps select tags also have similar problem)
				if (!groupno) {
					gdefault = await getdefault(element)
					if (gdefault != null && gdefault != '') {
						cell.text = gdefault
						// Mark dependents only (no setx/paint) — same list as setx2/validateupdate.
						// Unbound open uses calcfields(gdependents) instead of a second full calc.
						var deps = element.getAttribute('exo_dependents')
						if (deps) {
							deps = deps.split(';')
							for (var depn = 0; depn < deps.length; depn++) {
								if (!gdependents.exolocate(deps[depn]))
									gdependents[gdependents.length] = deps[depn]
							}
						}
						anydata = true
						continue
					}
				}

				//flag first element that is required, enabled and visible and missing
				if (mode != 'filldefaults'
					&& !missingelement
					//setting .disabled to true seems to have the effect of
					// setting attribute disabled to "" in modern browsers!
					//&& !element.getAttribute('disabled')
					&& !element.disabled
					&& exoenabledandvisible(element)) {
					missingelement = element
					if (groupno == 0)
						break
				}

			} //propname in row

			// Empty last group row is always ok (template after real rows, or sole
			// empty optional group e.g. schedule account share). Required cols apply
			// only when the row has other data. Sole empty *required* groups use
			// rowrequired below ("At least one …"), not field-level required.
			if (groupno > 0 && !anydata && rown == rows.length - 1)
				missingelement = false

			//fail if any missing data
			if (missingelement && (!allowemptyrows || anydata)) {
				await exoui_invalid(missingelement.getAttribute('exotitle') + ' is required.')
				focuson(missingelement)
				return false //logout('validateall ' + mode)
			}

			//prevent blank rows (except last one) unless allowed
			if (mode != 'filldefaults'
				&& !anydata
				&& groupno != 0
				&& rown < (rows.length - 1)) {
				if (!allowemptyrows) {
					var missingelement = rows[0][firstcolumnname].element
					await exoui_invalid('Empty rows are not allowed for ' + missingelement.getAttribute('exotitle'))
					focuson(missingelement)
					return false //logout('validateall ' + mode + ' empty row')
				}
			}

			//count the number of rows with data
			if (anydata) nrowsfound++

		} //rows

		//error if not enough rows (sole empty template: field required cleared above)
		if
			(
			groupno > 0
			&& mode != 'filldefaults'
			&& !nrowsfound
			&& rowrequired
			&& exoenabledandvisible($$('exogroup' + groupno))
		) {
			var missingelement = rows[0][firstcolumnname].element
			await exoui_invalid('At least one ' + missingelement.getAttribute('exotitle') + ' is required.')
			focuson(missingelement)
			return false //logout('validateall ' + mode + ' no rows')
		}

	}

	//logout('validateall ' + mode)
	return true

}

async function writedoc(unlock) {

	//login('writedoc')

	//check all required fields are present
	if (!(await validateall()))
		return false //logout('writedoc - validate all failed')

	//custom prewrite routine
	goldvalue = ''
	gvalue = ''
	if (typeof (form_prewrite) == 'function') {
		if (!(await exoevaluateall('await form_prewrite()', 'await writedoc()'))) return false
		///log('form_prewrite - after')
	}

	//stop the relocker while saving the document
	stoprelocker()

	//ok save it

	gro.request = unlock ? 'WRITEU' : 'WRITE' + '\r' + gdatafilename + '\r' + gkey
	gro.data = gds.data
	if (!(/**/ await gro.writex(unlock))) {

		//await exoui_note('Cannot save '+exoquote(gkeyexternal)+' because: \r\r'+gro.response)
		await exoui_invalid('Cannot save ' + exoquote(gkeyexternal) + ' because: \n\n' + gro.response)

		return false //logout('writedoc - write failed')

	}

	//any warnings are appended after response like 'OK xxx'
	if (gro.response != 'OK' && typeof form_postwrite == 'undefined') {
		await exoui_warning(gro.response.slice(2))
	}

	//if a cached is written then remove it from the cache (could update it instead?)
	deletecacherecord(gdatafilename, gkey)

	//option to unlock after saving (WRITEU)
	if (unlock) {
		glocked = false
		setdisabledandhidden(deleterecord, true)
		setdisabledandhidden(saverecord, true)
		setgraphicbutton(editreleaserecord, '<u>E</u>dit', geditimage)
	} else
		//restart the relocker if failed to save
		startrelocker()

	await loadnewkey()

	//merge new data back into the screen
	if (gro.data) {
		wstatus('Formatting, please wait ...')

		//existing gds' elements are pointing to screen elements so do not copy
		//gds.data=gro.data

		/**/ await gds.rebind(gro.data)

		var recn = grecn
		grecn = null
		await calcfields()
		grecn = recn

		wstatus('')

		gds.isnewrecord = false
		if (!gupdateonlymode && !greadonlymode && !gpreventdeletion)
			setdisabledandhidden(deleterecord, false)

		//update in case changed (eg rows resorted)
		if (gpreviouselement) {
			var temp = getvalue(gpreviouselement)
			if (gpreviousvalue != temp) {
				gpreviousvalue = temp
				try { gpreviouselement.select() } catch (e) { }
			}
		}

	}

	//logout('writedoc ok')

	return true

}

function startrelocker() {
	//2.2 ie try at least two relocks within the locktimeout period
	// Optional: skip if Gate A busy — never queue (form/lock state may have changed).
	grelocker = window.setInterval(function () {
		void exo_begin_if_idle(relockdoc, 'relockdoc')
	}, glocktimeoutinmins / 2.2 * 60 * 1000)
}

function stoprelocker() {
	window.clearInterval(grelocker)
}

async function relockdoc() {

	// if (!gloaded) return false

	//login('relockdoc')

	//dont relock if (already in the process of relocking or something else)
	if (grelockingdoc)
		return false //logout('relockdoc')
	if (db.requesting)
		return false //logout('relockdoc')

	grelockingdoc = true

	var relockdb = new exodblink()
	relockdb.request = 'RELOCK\r' + gdatafilename + '\r' + gkey + '\r' + gro.sessionid + '\r' + glocktimeoutinmins
	//document.bgcolor='green'
	var result
	if (await relockdb.send()) {
		//document.bgcolor='white'
		result = true
	}
	else {
		response = relockdb.response
		result = false
		//document.bgColor='RED'

		//lost control so prevent further editing of document
		if (response.toUpperCase().indexOf('EXPIRED') >= 0) {
			stoprelocker()
			glocked = false
			settouched(false)
			setdisabledandhidden(saverecord, true)
			setgraphicbutton(editreleaserecord, '<u>E</u>dit', geditimage)
			setdisabledandhidden(deleterecord, true)
			await exoui_warning(response)
		}
		else {
			await exoui_note(response)
		}

	}

	grelockingdoc = false

	//logout('relockdoc')

	return result

}

//'''''''
//'UNLOCK
//'''''''
async function unlockdoc() {

	if (!gKeyNodes)
		return

	//login('unlockdoc')

	//always return true even if cannot unlock

	//stop the relocker while trying to unlock
	stoprelocker()

	if (glocked) {
		while (true) {
			db.request = 'UNLOCK\r' + gdatafilename + '\r' + gkey + '\r' + gro.sessionid
			if (await db.send())
				break
			if (!(await exoui_yesno('Cannot release document - try again?\n\n' + db.response))) break
		}
	}

	glocked = false
	setdisabledandhidden(deleterecord, true)
	setdisabledandhidden(saverecord, true)
	//setdisabledandhidden(editreleaserecord,true)
	setgraphicbutton(editreleaserecord, '<u>E</u>dit', geditimage)

	//logout('unlockdoc')

	return true

}

////////////////////// FIELD FUNCTIONS //////////////////////

// Sync DOM focus schedule only (not async — must not start a floating Promise).
// Live path defers to focuson2 via timeout; does not do main-line dbio.
function focuson(element) {

	//login('focuson')

	if (!element)
		return false //logout('focuson - no element')

	//convert element name into element
	//if (typeof(element)=='string'||!element.parentNode)
	if (typeof (element) == 'string') {
		//element=$$(element)
		//var element2=$$(element)

		var element2 = $$(element)
		if (element2 && element2.tagName)
			element2 = [element2]

		if (grecn && element2[grecn])
			element2 = element2[grecn]
		else
			element2 = element2[0]

		if (!element2)
			return false //logout('focuson - cannot getElementById ' + exoquote(element))

		element = element2
	}

	//use the first if more than one
	if (typeof element == 'object' && !element.tagName && element.length > 0)
		element = element[0]

	// Radio group: focus checked (else first), not always [0]
	if (element && element.type == 'radio')
		element = form_radio_tab_target(element)

	///log(element.id + ' ' + element.outerHTML)

	//needed because delete line leaves grecn>nrecs
	grecn = getrecn(element)

	//taken out otherwise F7 on job number goes to market code
	// and validateupdate fails because record has not been loaded
	// (setdefault is async — must be awaited from a Gate A flight if re-enabled)

	gfocusonelement = element
	exosettimeout('focuson2()', 10)

	//logout('focuson ' + element.id)

}

function focuson2() {

	if (!gfocusonelement)
		return

	var focusonelement = gfocusonelement
	gfocusonelement = null

	//console.log('focuson2 ' + focusonelement.tagName + ' ' + focusonelement.id)

	//allowreadonly=true
	if (!(exoenabledandvisible(focusonelement, true)))
		return focusnext(focusonelement)

	try {
		// Never blur() to "force" focus — that closes a native <select> opened on click.
		// preventScroll: native focus scroll jumps mid-viewport; we scroll after focus.
		var needFocus = document.activeElement != focusonelement
		if (needFocus)
			form_focus_noscroll(focusonelement)
		form_scroll_log_msg('focuson2', form_scroll_el_label(focusonelement),
			'needFocus=', needFocus,
			'activeNow=', form_scroll_el_label(document.activeElement))

		// Text selection only — not SELECT (no .select() listbox contract).
		if (focusonelement.tagName != 'SELECT'
			&& focusonelement.tagName != 'OPTION'
			&& focusonelement.tagName != 'TEXTAREA'
			&& focusonelement.select)
			focusonelement.select()

		// Scroll after every programmatic land — not only after document_onfocus
		// finishes (that path early-exits often and skipped scroll). Same rule as
		// document_onfocus: key fields home (0,0); others scrollintoview.
		if (focusonelement.tagName
			&& focusonelement.tagName.match(gdatatagnames)
			&& focusonelement.getAttribute
			&& focusonelement.getAttribute('exotype')) {
			if (focusonelement == gstartelement
				|| focusonelement.getAttribute('exofieldno') === '0') {
				form_scroll_log_msg('focuson2 scroll home key field',
					form_scroll_el_label(focusonelement))
				window.scrollTo(0, 0)
				if (typeof modalblock_note_scroll_home == 'function')
					modalblock_note_scroll_home()
			} else if (typeof scrollintoview == 'function') {
				scrollintoview(focusonelement)
			}
		}

	}
	catch (e) {
		form_scroll_log_msg('focuson2 error', e && (e.message || e))
	}

}

//'''''''''
//'ON FOCUS
//'''''''''
function document_onfocus_sync(event) {
	try {
		var t = event && (event.target || event.srcElement)
		form_scroll_log_msg('document_onfocus_sync', form_scroll_el_label(t),
			'gblockevents=', typeof gblockevents != 'undefined' ? gblockevents : '?')
	} catch (e) { }
	var eventhandlerx = starteventhandler('exocode', document_onfocus)
	return eventhandlerx(event)
}

async function document_onfocus(event) {

	event = getevent(event)
	//window.status='onfocus '+new Date

	//ckeditor configured to pass element.id in event
	if (event.target && event.target.listenerData) {
		event.target = $$(event.target.listenerData)
		event.target.isckeditor = true
		if (gCKEDITOR_EDITOR)
			gCKEDITOR_EDITOR.setReadOnly(!glocked)//also in await opendoc2() post read document_onfocus
	}

	if (!event.target)
		return

	//do nothing if not initialised
	if (!ginitok)
		return

	exosetcookie('', 'EXODUSlogincode', glogincode, 'logincode')
	//window.status=new Date()+' '+glogincode

	//var text = 'document_onfocus' + ' tag:' + event.target.tagName + ' id:' + event.target.id + (gpreviouselement ? ' gpreviouselement:' + gpreviouselement.id : '')
	//console.log(text)
	///log(text)
	//window.status=text

	//window.status='document_onfocus'+' tag:'+event.target.tagName+' id:'+event.target.id+(gpreviouselement?' gpreviouselement:'+gpreviouselement.id:'')

	///log('get element and elementid')
	var element = event.target

	///log('quit if no element.id')
	if (!element.id)
		return false //logout('document_onfocus')

	///log('quit calendar focus')
	if (element.className == 'calendar')
		return false //logout('document_onfocus')

	///log('get elementid')
	var elementid = ''
	try {
		elementid = element.id.toString()
		///log('handle focus on ckeditor')
		if (elementid.indexOf('___Frame') > 0) {
			///log('to prevent endless loop after cancelling postread (due to ckeditor refocussing on itself?')
			///log('minor problem is that this prevents click on ckeditor from triggering a record read')
			if (gKeyNodes && !gkey) {
				///log('timeout to focus on last key element')
				exosettimeout('try{gKeyNodes[gKeyNodes.length-1].focus()}catch(e){}', 10)
				return false //logout('document_onfocus')
			}
			elementid = elementid.slice(0, -8)
			var ckeditor = element
			element = window.$$(elementid)
			element.isckeditor = ckeditor
		}
	}
	catch (e) {
		elementid = ''
	}

	//dont add twice otherwise event occurs twice!
	////log('ensure checkboxes in rows have onclick set')
	//if (!element.onclick&&element.type=='checkbox'||element.type=='radio')
	// //element.onclick=onclickradiocheckbox
	// addeventlistener(element,'click','onclickradiocheckbox')

	///log('no validation/update except changing exodus elements:' + element.getAttribute('exotype'))
	form_scroll_log_msg('document_onfocus enter', form_scroll_el_label(element),
		'exotype=', element.getAttribute('exotype'),
		'gkeycode=', gkeycode,
		'prev=', form_scroll_el_label(gpreviouselement))
	if (!(element.getAttribute('exotype'))) {
		form_scroll_log_msg('document_onfocus EXIT no exotype', form_scroll_el_label(element))
		//logout('document_onfocus')
		return
	}

	///log('quit if opening')
	if (gopening) {
		console.log('gopening still!')
		form_scroll_log_msg('document_onfocus EXIT gopening')
		//ignore this until fix resuming after ok/cancel
		//logout('document_onfocus')
		return exocancelevent(event)
	}

	///log('drop down any "modal" popup divs')
	await form_closepopups()

	///log('quit if refocussing')
	if (element == gonfocuselement) {
		form_scroll_log_msg('document_onfocus EXIT re-focus same gonfocuselement',
			form_scroll_el_label(element))
		//  if (await setdefault(element))  if (element.tagName!='TEXTAREA') element.select()
		return false //logout('document_onfocus')
	}
	gonfocuselement = element

	///log('quit if refocussing on gpreviouselement')
	if (gpreviouselement && element == gpreviouselement) {
		form_scroll_log_msg('document_onfocus EXIT same gpreviouselement',
			form_scroll_el_label(element))
		return false //logout('document_onfocus')
	}

	//dont validate update if clicked popup
	//if (element&&element.id==(gpreviouselement.id+'_popup'))
	// return false //logout('document_onfocus')

	///log('check if changed element')
	if (element != gpreviouselement) {
		///log('validate/update previous data entry ' + (gpreviouselement ? gpreviouselement.id : ''))
		if (!(await validateupdate())) {
			form_scroll_log_msg('document_onfocus EXIT validateupdate failed',
				'prev=', form_scroll_el_label(gpreviouselement),
				'active=', form_scroll_el_label(document.activeElement))
			return false //logout('document_onfocus')
		}
	}

	///log('quit if we are not focused on a proper element')
	if (!elementid) {
		form_scroll_log_msg('document_onfocus EXIT no elementid')
		return false //logout('document_onfocus')
	}

	///log('OK. Now previous element is valid')

	///log('find what row we are on')
	grecn = getrecn(element)

	///log('quit if not exodus data entry field')
	if (!element.tagName.match(gdatatagnames)) {
		form_scroll_log_msg('document_onfocus EXIT not data tag', element.tagName)
		return false //logout('document_onfocus')
	}

	///log('check key fields')
	if (gKeyNodes && !glocked) {
		if (!(await checkrequired(gKeyNodes, element, 0))) {
			form_scroll_log_msg('document_onfocus EXIT checkrequired keys')
			return false //logout('document_onfocus' + ' ' + elementid + ' is required but is blank (0)')
		}
	}

	///log('check no required fields are missing in prior data')
	///log(' fill in any defaults where possible')
	if (!gKeyNodes || glocked)//check keynodes as well now
	{

		///log('check no missing data in group 0 always')
		if (!(await checkrequired(gfields, element, 0))) {
			form_scroll_log_msg('document_onfocus EXIT checkrequired group0')
			return false //logout('document_onfocus' + ' ' + elementid + ' a prior element is visible and required but is blank (1)')
		}

		//check specific group if >0
		//var elements
		//if (Number(element.getAttribute('exogroupno')))
		//{
		// elements=getancestor(element,'TR').all
		// if (!(await checkrequired(elements,element,Number(element.getAttribute('exogroupno')))))
		//  return false //logout('document_onfocus'+' '+elementid+' a prior element is visible and required but is blank (2)')
		//}

	}

	///log('all previous fields are valid now')
	//wwww
	//triggered by timeout from validateupdate now
	//but do it ALSO here in case triggered by defaulted key field
	///log('if the key is complete and not on a key field, get the record')
	//if (!gloaded&&gKeyNodes&&element.getAttribute('exofieldno')!=0)
	//dont check if element not changed to avoid endless loop if opendoc errors
	if (gKeyNodes && element != gpreviouselement) {
		var nextkey = await getkey()
		//if (nextkey.toUpperCase() != nextkey)
		//    xxx = 1
		if (nextkey && (gloaded && nextkey != gkey) || (!gloaded && element.getAttribute('exofieldno') != 0))
		//if (key&&(gloaded&&key!=gkey)||(!gloaded))
		{
			//exosettimeout('await opendoc()',100)
			form_scroll_log_msg('document_onfocus EXIT opendoc', nextkey)
			await opendoc(nextkey)
			return false //logout('document_onfocus' + ' ' + exoquote(elementid) + ' new record')
		}
	}

	// Tab/Enter/arrows landed on a readonly field: skip to next/previous editable.
	// Must not run before prior-required + new-record checks above.
	// Horizontal radio Up keeps gkeycode 38 so back-nav works without a dir flag.
	if (gkeycode == 9 || gkeycode == 13 || gkeycode == 38 || gkeycode == 40) {
		if (element.getAttribute('exoreadonly')
			&& (element.tabIndex == exo_tabindex_default || element.tabIndex == -1
				|| element.getAttribute('oldtabindex'))) {
			form_scroll_log_msg('document_onfocus EXIT readonly skip to next',
				form_scroll_el_label(element))
			if (event.shiftKey || gkeycode == 38)
				focusprevious(element)
			else
				focusnext(element)
			return false //logout('document_onfocus')
		}
	}
	///log('there is no new record so setup current element')

	///log('scroll to top left if the key field')
	// Strict === '0': loose == 0 also matches missing attribute (null).
	// modalblock_note_scroll_home: unpin must not restore pre-home scroll.
	// focuson2 applies the same key-home / scrollintoview split for programmatic
	// focus when this handler early-exits; still run here for click/tab.
	if (element == gstartelement || element.getAttribute('exofieldno') === '0') {
		form_scroll_log_msg('document_onfocus scroll home key field', form_scroll_el_label(element))
		window.scrollTo(0, 0)
		if (typeof modalblock_note_scroll_home == 'function')
			modalblock_note_scroll_home()
	} else {
		form_scroll_log_msg('document_onfocus → scrollintoview', form_scroll_el_label(element))
		scrollintoview(element)
	}

	///log('remove blanks used to force formatting of spans')
	if (element.tagName == 'SPAN' && element.innerText == ' ')
		element.innerText = ''

	///log('change gpreviouselement/value to current element/value')
	//gpreviouselement = element
	//gpreviousvalue = getvalue(gpreviouselement)
	setgpreviouselement(element)

	// Radio/checkbox arrival snapshot — getvalue when focus first enters field/group;
	// Esc restores it. Leave field → clear (not a twin of gpreviousvalue; click-validate
	// may advance that).
	if (element.type == 'radio') {
		if (!g_radio_arrival_anchor || !form_radio_same_group(element, g_radio_arrival_anchor)) {
			g_radio_arrival_anchor = element
			g_radio_arrival_value = getvalue(element)
		}
	} else if (element.type == 'checkbox') {
		if (!g_radio_arrival_anchor || g_radio_arrival_anchor.type != 'checkbox'
			|| g_radio_arrival_anchor.id != element.id) {
			g_radio_arrival_anchor = element
			g_radio_arrival_value = getvalue(element)
		}
	} else if (g_radio_arrival_anchor) {
		// Clear arrival (left radio/checkbox field)
		g_radio_arrival_anchor = null
		g_radio_arrival_value = ''
	}

	///log('set the default of the current element')
	await setdefault(element, donotvalidateupdate = true)

	///log('deselect previous (this does not work unless .select() is used')
	///log(' but causes a problem in focussing on the previous element and rescrolling')
	//if (element.tagName == 'SELECT' && document.selection && document.selection.createRange) {
	//    var textrange = document.selection.createRange()
	//    textrange.collapse()
	//    //  textrange.select()
	//}

	// Field arrival selection.
	// Always clear document selection first — leaving a contenteditable SPAN
	// (code/text/number) keeps a Range selected even after focus moved (Enter/
	// Tab to INPUT, SELECT, radio, button, etc.). Only radio/button used to
	// clear; INPUT.select() does not drop a prior SPAN range → old field stays
	// highlighted while the next control shows focus chrome.
	try {
		if (window.getSelection)
			window.getSelection().removeAllRanges()
	} catch (e) { }
	// Full-select this field when appropriate (not TEXTAREA/SELECT/OPTION;
	// not radio/checkbox/button — clear alone is enough).
	if (element.tagName != 'TEXTAREA' && element.tagName != 'OPTION' && element.tagName != 'SELECT')
		try {
			var isRadioCheckBtn = element.type == 'checkbox' || element.type == 'radio'
				|| element.type == 'button' || element.type == 'submit'
			if (isRadioCheckBtn) {
				// selection already cleared
			} else if (element.select && !element.isContentEditable) {
				element.select()
			} else if (window.getSelection && document.createRange) {
				selection = window.getSelection()
				range = document.createRange()
				range.selectNodeContents(element)
				selection.addRange(range)
			} else if (document.selection && document.body.createTextRange) {
				range = document.body.createTextRange()
				range.moveToElementText(element)
				range.select()
			}
		} catch (e) { }

	// Re-assert focus only if the browser is no longer on this control (e.g.
	// prior validation removed a node). If we already have focus — the normal
	// click/tab path — do not schedule focuson: that deferred path used to
	// blur+focus and closed native <select> lists after document_onfocus.
	// Text .select() for INPUT already ran above when applicable.
	if (!element.isckeditor && document.activeElement != element) {
		try {
			focuson(element)
		}
		catch (e) {
			focusnext(element)
		}
	}

	//logout('document_onfocus' + ' ' + elementid)

}

/*
async function onclickradiocheckbox(event) {

event=getevent(event)

//wstatus(getvalue(event.target.id)+' '+gpreviousvalue)
//space or arrow keys also simulate a click event

//update immediately
//if (element.type=='radio') element.checked=true
//if (element.type=='checkbox') element.checked=!element.checked
//alert(event.target.getAttribute)
//alert('onclickradiocheckbox')
//validate or return to original
if (!(await validateupdate())) {

setvalue(gpreviouselement,gpreviousvalue)
return
}
gpreviousvalue=getvalue(gpreviouselement)

}
*/

async function onclickradiocheckbox(event) {

	event = getevent(event)

	//wstatus(getvalue(event.target.id)+' '+gpreviousvalue)
	//space or arrow keys also simulate a click event

	// Optional radio: re-activate already-checked option (click or Space) → clear
	if (event.target.type == 'radio'
		&& gform_radio_was_checked
		&& !(event.target.getAttribute('exorequired')
			&& event.target.getAttribute('exorequired') != 'false'))
		event.target.checked = false
	gform_radio_was_checked = false

	//handle case where onfocus has not been called before onclick
	if (gpreviouselement != event.target) {

		//get the CURRENT value BEFORE it was clicked
		//NB converted to grecn/gpreviousvalue IF AND AFTER previous element validates below
		var recn = getrecn(event.target)
		var previousvalue = await gds.getx(event.target, recn)
		//checkbox and radio buttons are currently not supported in multivalues
		//so the following is probably never executed
		if (typeof recn == 'number')
			previousvalue = previousvalue[0]

		//validate the PREVIOUS field
		if (!(await validateupdate())) {
			//if returning to previous field then reset the CURRENT element
			// back to what it was prior to clicking it
			setvalue(event.target, previousvalue)
			return
		}

		//setup as if onfocus had been called prior to onclick
		grecn = recn
		//gpreviouselement = event.target
		//gpreviousvalue = previousvalue
		setgpreviouselement(event.target, previousvalue)

		// Focus lagged click: snapshot pre-click gds value (same as onfocus arrival)
		if (event.target.type == 'radio'
			&& (!g_radio_arrival_anchor
				|| !form_radio_same_group(event.target, g_radio_arrival_anchor))) {
			g_radio_arrival_anchor = event.target
			g_radio_arrival_value = previousvalue
		} else if (event.target.type == 'checkbox'
			&& (!g_radio_arrival_anchor || g_radio_arrival_anchor.type != 'checkbox'
				|| g_radio_arrival_anchor.id != event.target.id)) {
			g_radio_arrival_anchor = event.target
			g_radio_arrival_value = previousvalue
		}

	}

	//update immediately
	//if (element.type=='radio') element.checked=true
	//if (element.type=='checkbox') element.checked=!element.checked
	//alert(event.target.getAttribute)
	//alert('onclickradiocheckbox')
	//validate or return to original
	// Like form_oninput: only mark "this element touched form" if form was untouched.
	// validateupdate nulls the marker then settouched(true); re-set after so Esc
	// can clear touched the same way as text Esc (element == gelementthatjustcalled…).
	var clickWasUntouched = !gtouched
	if (!(await validateupdate())) {
		setvalue(gpreviouselement, gpreviousvalue)
		return
	}
	// gpreviousvalue already advanced inside validateupdate (traced there)
	gpreviousvalue = getvalue(gpreviouselement)
	if (clickWasUntouched
		&& (event.target.type == 'radio' || event.target.type == 'checkbox'))
		gelementthatjustcalledsettouched = event.target

}

async function validateupdate() {

	var id = gpreviouselement ? gpreviouselement.id : 'undefined'
	//login('validateupdate ' + id)

	//return if no previouselement
	if (!gpreviouselement) {
		//logout('validateupdate - no previous element')
		return true
	}

	//return if no changes (ignoring case if lowercase not allowed)
	var newvalue = getvalue(gpreviouselement)
	//done in getvalue now
	//if (!gpreviouselement.getAttribute('exolowercase') && gpreviouselement != 'radio' && gpreviouselement.type != 'checkbox')
	//    newvalue = newvalue.toUpperCase()
	if (newvalue == gpreviousvalue) {
		//logout('validateupdate - gpreviousvalue:' + gpreviousvalue + ' same as newvalue:' + newvalue)
		// Same DOM external — still paint if conversion can canonicalise (rare drift).
		await form_paint_oconv_if_needed(gpreviouselement, getvalue_internal(gpreviouselement))
		// Value is the committed original — end quiet typeahead including miss tint.
		form_typeahead_reset()
		return true
	}

	// Leave-field validate owns the UI — cancel quiet search/panel (miss stays until pass/restore)
	form_typeahead_cancel()

	//log('User/setdefault changed ' + id + '\nfrom ' + exoquote(gpreviousvalue) + '\nto ' + exoquote(newvalue))
	//check for prior required fields if a grouped element
	var elements
	if (Number(gpreviouselement.getAttribute('exogroupno'))) {
		elements = getancestor(gpreviouselement, 'TR').getElementsByTagName('*')
		if (!(await checkrequired(elements, gpreviouselement, Number(gpreviouselement.getAttribute('exogroupno'))))) {
			setvalue(gpreviouselement, gpreviousvalue)
			return false //logout('validateupdate ' + id + ' a prior element is visible and required but is blank')
		}
	}

	//check arguments
	assertelement(gpreviouselement, 'validateupdate', 'gpreviouselement')
	if (!gpreviouselement.tagName.match(gdatatagnames)) {
		//logout('validateupdate - not an INPUT or SELECT')
		return true
	}

	//quit ok if gpreviouselement is no longer bound
	grecn = getrecn(gpreviouselement)
	if (grecn == -1) {
		//logout('validateupdate - gprevious no longer exists')
		return true
	}

	//get the old value (internal format) and grecn
	gcell = gds.getcells(gpreviouselement, grecn)[0]
	goldvalue = gcell.text
	gorigvalue = (typeof gcell.oldtext != 'undefined') ? gcell.oldtext : gcell.text

	//validate it and put back focus if it fails
	////////////////////////////////////////////
	//NOTE gets/sets gvalue
	if (!(await onbeforeupdate(gpreviouselement))) {
		focusongpreviouselement()

		//logout('validateupdate - onbeforeupdate failed')
		return false

	}

	//pressing escape will no longer cancel gtouched
	gelementthatjustcalledsettouched = null

	//update
	////////

	//update the element multiple selections
	//if reply is an array use replace all rows
	var multiplegvalue = false
	if (typeof (gvalue) == 'object') {
		if (gvalue.length > 1 && Number(gpreviouselement.getAttribute('exogroupno'))) {
			multiplegvalue = true

			var elementid = gpreviouselement.id
			await insertallrows(gpreviouselement, gvalue)

			//focus on next element AFTER table
			element = $$(elementid)
			if (element && element[0])
				element = element[0]
			focusdirection(1, element, Number(gpreviouselement.getAttribute('exogroupno')))

		}
		else {
			gvalue = gvalue[0]
		}
	}

	//update the element if not multiple selections
	if (!multiplegvalue) {
		var defaulted = gpreviouselement.dataitem && gpreviouselement.dataitem.defaulted

		await gds.setx(gpreviouselement, grecn, gvalue)

		//restore defaulted status if case it was reset by setx
		if (defaulted)
			gpreviouselement.dataitem.defaulted = defaulted
	}

	//wwww
	/* cant do this here otherwise will retrieve multipart key records when only clearing
		//switch to new key if new and user accepts to close the current one
		var nextkey
		if (gKeyNodes
		 && gpreviouselement.getAttribute('exofieldno') == 0
		 && (nextkey = await getkey())
		 && nextkey != gkey) {
			//do not change key if user chooses not to unload an existing document
			if (!(await closedoc('CHANGEKEY'))) {
				focusongpreviouselement()
				return false //logout('validateupdate - closedoc refused')
			}
	
			//return false and just allow opendoc to happen
	
			//exosettimeout('await opendoc("'+nextkey+'")',1)
			//alert('DEBUG: settimeout opendoc gkey:'+gkey+' nextkey:'+nextkey)
			//settimeout results in overlapping xmlhttp requests in FF 3.0.3
			//eg alt+P
			//1.triggers update
			//2.settimeout opendoc
			//3.settimeout printrecord
			//4.printrecord STARTS an XML request and waits synchronously for the result
			//5.opendoc timesout(?!)
			//6.opendoc issues ANOTHER XML request and we get a trap message due to this.listening being already set
			//try to avoid the above by calling opendoc directly BUT this may cause errors on return from validateupdate
			await opendoc(nextkey)
	
			//logout('validateupdate - opendoc')
			return false
		}
	*/
	//post update
	/////////////

	//flag record edited
	if (gpreviouselement.getAttribute('exofieldno') != 0
		&& !gpreviouselement.getAttribute('exonochangeswarning')) {
		settouched(true)
	}

	//calculate dependencies
	await calcfields(gdependents)
	gdependents = []

	//why is this necessary?
	// Advances Esc undo baseline to committed value (same gpreviouselement).
	var _gprev_before = gpreviousvalue
	gpreviousvalue = getvalue(gpreviouselement)
	form_trace_gprevious('validateupdate', gpreviouselement, _gprev_before, gpreviousvalue)

	//logout('validateupdate - done')

	//if (gautofitwindow)
	//    exosettimeout('exoautofitwindow()', 1)

	// Quiet miss was for live typeahead only; committed value is accepted.
	form_typeahead_clear_miss()
	return true

}

function focusongpreviouselement() {
	window.setTimeout(focusongpreviouselement2, 1)
}

function focusongpreviouselement2() {

	//log('focusing back to ' + gpreviouselement.id)

	//prevent loop since focus doesnt go back to radio button immediately
	//doesnt seem to work though
	//if (gpreviouselement.type=='radio')
	//{
	// gvalue=goldvalue
	// setvalue(element.id,gvalue)
	// gpreviouselement.select()
	//}

	if (!gpreviouselement)
		return

	//dont focus back to checkboxes because that causes instant revalidation thereby causing endless loop if invalid
	if (gpreviouselement.type != 'checkbox')
		form_focus_noscroll(gpreviouselement)

	// Same as focuson2: focus_noscroll then scroll into view (off-screen invalid
	// fields e.g. analysis code after OK must become visible).
	if (gpreviouselement.tagName
		&& gpreviouselement.tagName.match(gdatatagnames)
		&& gpreviouselement.getAttribute
		&& gpreviouselement.getAttribute('exotype')) {
		if (gpreviouselement == gstartelement
			|| gpreviouselement.getAttribute('exofieldno') === '0') {
			window.scrollTo(0, 0)
			if (typeof modalblock_note_scroll_home == 'function')
				modalblock_note_scroll_home()
		} else if (typeof scrollintoview == 'function') {
			scrollintoview(gpreviouselement)
		}
	}

	// Restore caret after focus settles.
	var bounceEl = gpreviouselement
	var snap = (gprevious_sel && gprevious_sel.el == bounceEl) ? gprevious_sel : null
	window.setTimeout(function () {
		if (gpreviouselement != bounceEl)
			return
		if (snap && form_restore_field_caret(bounceEl, snap))
			return

		// No snap: full-select so L/R navigate fields again
		if (bounceEl.tagName == 'SELECT' || bounceEl.tagName == 'OPTION')
			return
		try {
			if (bounceEl.select && !bounceEl.isContentEditable
				&& bounceEl.type != 'checkbox' && bounceEl.type != 'radio'
				&& bounceEl.type != 'button' && bounceEl.type != 'submit') {
				bounceEl.select()
				return
			}
			if (window.getSelection && document.createRange
				&& (bounceEl.isContentEditable
					|| (bounceEl.tagName == 'SPAN'
						&& bounceEl.getAttribute('contenteditable')))) {
				var sel = window.getSelection()
				sel.removeAllRanges()
				var range = document.createRange()
				range.selectNodeContents(bounceEl)
				sel.addRange(range)
			}
		} catch (e) { }
	}, 0)

}

async function earlyupdate() {

	//skip update on buttons
	//if (!gpreviouselement || !gpreviouselement.name) return
	if (!gpreviouselement || !gpreviouselement.id || (gpreviouselement.type && gpreviouselement.type == 'button'))
		return

	await gds.setx(gpreviouselement, grecn, gvalue)

}

// Tab order for "is this field before that one?" in checkrequired.
// exosetreadonly sets tabIndex -1 (saved oldtabindex); without restoring
// that sequence, focusing a readonly field (e.g. autonumber VOUCHER_NO) made
// no prior field look "before" it — skipped required checks and opendoc side effects.
function form_effective_tabindex(el) {
	if (!el)
		return exo_tabindex_default
	var t = Number(el.tabIndex)
	if (t == -1 || isNaN(t)) {
		var ot = el.getAttribute('oldtabindex')
		if (ot != null && ot !== '' && !isNaN(Number(ot)))
			return Number(ot)
		return exo_tabindex_default
	}
	return t
}

async function checkrequired(elements, element, groupno) {

	//check the given elements with the given group number
	//and prior to the given field for required
	//and try to set default otherwise return false
	//
	// Pre-open (!gloaded): only key parts (fieldno 0). Body requireds (Job etc.)
	// after load. Stops F7/gfields pre-open nags; keeps multipart key fill-in.

	grecn = getrecn(element)

	var element_tab = form_effective_tabindex(element)
	var foundelement = false
	for (var ii = 0; ii < elements.length; ii++) {
		var element2 = elements[ii]

		//if given gfields then get the equivalent DOM element
		//group 0 fields are not bound to DOM initially but are in the next paragraph
		//group 1+ fields are never bound to DOM because there are many rows
		if (!element2.parentNode) {
			element2 = $$(element2.id)
			//if $$ return more than one (nb SELECT elements may have [] for its options)
			if (element2 && !element2.tagName && element2[0])
				element2 = element2[0]
			if (!element2) {
				//no longer check this so that elements can be "unbound" for greater security etc
				//by changing the id to something like id_unbound
				//    alert(elements[ii].id+' has disappeared from the document\nPerhaps it need to be in its own HTML table')
				continue
			}
			gfields[ii] = element2
		}

		//only check input fields
		if (!element2.tagName.match(gdatatagnames))
			continue

		// Pre-open: only key parts (not JOB_NO etc.)
		if (!gloaded && element2.getAttribute('exofieldno') !== '0')
			continue

		//don't check current but continue looking for lower tabindexed fields
		//if (element2==element) //this does not work because gfields<> bound table elements for rows
		//if (element2.getAttribute('exo_screenfn')==element.getAttribute('exo_screenfn'))
		//check ids because exo_screenfn is repeated in gfields for radio 4,4,4
		//but not in the 2nd and subsequent repeated form elements 4,5,6
		if (element2.id == element.id) {
			foundelement = true
			continue
		}

		//skip elements not in desired group
		//must be after detection of foundelement above
		if (Number(element2.getAttribute('exogroupno')) != groupno)
			continue

		if (Number(element2.getAttribute('exogroupno')) == groupno) {
			var element2_tab = form_effective_tabindex(element2)
			// form_effective_tabindex never returns -1 (maps to oldtabindex or exo_tabindex_default)
			if ((!foundelement && element2_tab <= element_tab) || (element2_tab < element_tab)) {
				//if (element&&element2.getAttribute('exorequired')&&gds.getcells(element2,grecn)[0].text=='')
				//if (element&&element2.getAttribute('exorequired')&&getvalue(element2)=='')
				//if (element&&(!Number(element.getAttribute('exogroupno'))||element2.getAttribute('exorequired'))&&getvalue(element2)=='')
				if (element
					&& ((gds.isnewrecord && !Number(element2.getAttribute('exogroupno')))
						|| element2.getAttribute('exorequired'))
					&& getvalue(element2) == '') {

					//try to set the default
					// Pre-open key parts (fieldno 0, !gloaded): DOM/getkey path only —
					// setdefault donotupdate, still-empty via getvalue not gds.
					var keyEntry = gKeyNodes && !gloaded
						&& element2.getAttribute('exofieldno') === '0'
					if (!(await setdefault(element2, keyEntry)) && exoenabledandvisible(element2)) {
						focuson(element2)
						return false
					}

					//if still empty then fail
					//if (getvalue(element2)=='')
					var stillEmpty = keyEntry
						? (getvalue(element2) == '')
						: (gds.getcells(element2, grecn)[0].text == '')
					if (element2.getAttribute('exorequired') && !element2.getAttribute('exoreadonly') && stillEmpty) {

						//disabled or invisible elements may be blank and required (even after setdefault)
						if (element2.disabled || element2.getAttribute('disabled') || !exoenabledandvisible(element2))
							return true

						//put up a message unless is the first column of a row
						if (true || !(element2.getAttribute('exo_isfirstinputcolumn'))) {
							// Message then focuson only. Do not rewrite
							// gpreviouselement/gonfocuselement here: focuson →
							// document_onfocus leave-fields the real previous
							// (incl. default-painted key) and runs opendoc when
							// appropriate. Stealing previous to "suppress re-entry"
							// early-exits that path.
							await exoui_invalid(element2.getAttribute('exotitle') + ' is required..')
						}

						focuson(element2)
						//if (!(Number(element2.getAttribute('exogroupno'))))
						// focuson(element2)
						return false

					}
				}
			}
		}
	}

	return true

}

function getvalues(elementx, sepchar) {

	//return a array of values, or string separated by sepchar if defined

	var element = elementx
	if (typeof element == 'string')
		element = $$(element)
	else if (typeof element.id != 'undefined')
		element = $$(element.id)

	var values = getvalue(element)

	if (typeof (values) == 'object' && typeof (sepchar) != 'undefined')
		values = values.join(sepchar)

	return values

}

// getvalue is always DOM external (incl. NUMBER grouping). For storage/math:
// getvalue_internal = getvalue + ICONV when conversion is […]. Do not hide ICONV inside getvalue.
function getvalue_internal(element, recn) {
	var value = getvalue(element, recn)
	if (value === '' || value == null || !element || !element.getAttribute)
		return value
	var conversion = element.getAttribute('exoconversion')
	if (typeof conversion != 'string' || conversion.slice(0, 1) != '[')
		return value
	try {
		var iv = String(value).exoiconv(conversion)
		if (iv != null)
			return iv
	} catch (e) { }
	return value
}

function getvalue(element, recn) {

	if (element == null) {
		systemerror('getvalue()', ' element cannot be null')
		return ''
	}

	// if (recn==null) return getvalues(element)

	//return input value or select value(s) ... or undefined
	//returns a string or array of strings if element is array of elements

	//element is required and must be an element of type
	// INPUT text
	// INPUT radio
	// INPUT checkbox
	// SELECT
	// SPAN

	//get element if given element name
	if (typeof (element) == 'string') {
		var element0 = element
		var element = $$(element0)
		if (!element) {
			systemerror('getvalue()', exoquote(element0) + ' does not exist')
			return
		}
		//get first one only
		if (!element.tagName) {
			//element = element[0]
			if (element[0].getAttribute('exogroupno')) {
				if (typeof recn == 'undefined') recn = getrecn()
			}
			else recn = 0
			element = element[recn]
			if (!element) {
				// Grouped field row not in DOM (hidden column, row gap, etc.) — empty value
				return ''
			}
		}
	}

	//radio/checkbox buttons appear like arrays multiple elements
	//force radio/checkbox to be first element
	if (element[0] && (element[0].type == 'radio' || element[0].type == 'checkbox')) element = element[0]

	if (element.length && element[0].type != 'radio') {

		if (element.tagName) {
			//radio buttons appear like arrays multiple elements
			if (element[0].type == 'radio' || element[0].type == 'checkbox') element = element[0]
		}
		else

		//if passed an array of elements, return an array of values
		//is this used anywhere?
		//NB SELECT element on Mac but not PC has length and options[]
		{
			var values = []
			for (var ii = 0; ii < element.length; ii++) {
				values[ii] = getvalue(element[ii])
			}

			return values

		}

	}

	//ensure is an element
	assertelement(element, 'getvalue', 'element')

	switch (element.tagName) {

		case 'INPUT': {

			switch (element.type) {

				case 'text':
					//var tx = element.value.exotrimr()

					//similar code in INPUT and SPAN
					//always trim trailing white space
					var tx = element.value.replace(/\s+$/, '')
					if (!element.getAttribute('exolowercase')) {
						//may be visibly uppercase due to style but internally lowercase so change it here
						tx = tx.toUpperCase()
						//trim leading white space if lower case not allowed
						tx = tx.replace(/^\s+/, '')
					}
					return tx

				case 'radio': {

					var elements = getradiocheckboxelements(element)
					for (var ii = 0; ii < elements.length; ii++) {
						var element = elements[ii]
						if (element.checked) {
							if (typeof element.value == 'undefined')
								return ii
							else
								return element.value
						}
					}
					//nothing selected
					return ''

				}

				case 'checkbox': {

					var elements = getradiocheckboxelements(element)
					var values = []
					for (var ii = 0; ii < elements.length; ii++) {
						var element = elements[ii]
						if (element.checked) {
							values[values.length] = (typeof element.value == 'undefined') ? ii : element.value
						}
					}
					return values.join(sm)

				}

				case 'button': {

					return ''
				}

				default: {

					return systemerror('getvalue()', exoquote(element.type) + ' invalid INPUT element type')
				}

			}

		}

		case 'OPTION': return getdropdown0(element.parentNode)

		case 'SELECT': return getdropdown0(element)

		//crlf becomes space to prevent entry of tm characters in text fields
		case 'SPAN':
			//if (element.isContentEditable)
			//var value = element.innerText.replace(/([\r\n]+)/g, ' ').exotrimr()
			//allow /r by itself but replace \\r\n combinations (to allow spans to have multiple lines)
			//var value = element.innerText.replace(/\r\n/g, ' ').exotrimr()

			//var value = element.innerText // can drop spaces in some engines
			// modern browsers (selectionStart etc.)
			//alternatives are .data and .wholeText
			if (typeof element.textContent != 'undefined')
				var value = element.textContent
			else
				//the old method seems to work everywhere except later versions of IE
				//Note: .innerText property shim to textContent is added to HTMLElement.prototype in client.js
				var value = element.innerText

			// some engines use \r\n line marks; others \n
			//value = value.replace(/\r\n/g, ' ').exotrimr()
			//else
			//    var value = element.innerHTML.replace(/([\r\n]+)/g, ' ').exotrimr()
			//if (value == nbsp160)
			//    return "";
			//remove trailing spaces and end of lines
			//return value.replace(/[ \r\n]+$/, '')

			//similar code in INPUT and SPAN
			//remove trailing white space
			value = value.replace(/\s+$/, '')
			if (!element.getAttribute('exolowercase')) {
				//may be visibly uppercase due to style but internally lowercase so change it here
				value = value.toUpperCase()
				//trim leading white space if lower case not allowed
				value = value.replace(/^\s+/, '')
			}
			return value

		case 'TEXTAREA': {

			//http://wiki.ckeditor.net/Developer%27s_Guide/Javascript_API
			if (typeof CKEDITOR != 'undefined') {
				var oEditor = CKEDITOR.instances[element.id]
				if (oEditor) {
					//needs encoding set correctly eg UTF-8 in meta of html page
					var result = oEditor.getData()
					return result
				}
			}

			return element.value.exotrimr()
		}

		default: {

			return systemerror('getvalue()', exoquote(element.tagName) + ' invalid tagName')
		}
	}

}

function exosetreadonly(elements, msg, options, recn) {

	//note: cannot set readonly off if the dictionary says that it is readonly

	//if msg contains something then set the element(s) to readonly and vice versa
	//tabbing and cursor keys will skip past readonly fields
	//msg will be displayed if they click and try and change the field

	var elementx = elements

	//if elements is a number then it means set all one group (for selected recn or null for all)
	if (!(isNaN(+elementx))) {
		var elements = []
		for (var ii = 0; ii < gfields.length; ii++) {
			if (Number(gfields[ii].getAttribute('exogroupno')) == elementx
				&& Number(gfields[ii].getAttribute('exofieldno'))
				&& (!(exosetreadonly(gfields[ii].id, msg, options, recn))))
				return false
		}
		return true
	}

	//if elements is a simple string then get all the elements for it
	if (typeof elementx == 'string') {
		var elementxstring = elementx
		elementx = $$(elementx)
		//if (!elementx||elementx.length==0)
		if (!elementx) {
			//only give error if it is not even in the dictionary
			//so that we can remove fields from the screen without changing the setreadonly field lists
			if (!(gds.dictitem(elementxstring)))
				return systemerror('exosetreadonly()', exoquote(elements) + ' is not in the form')
			return false
		}

		//handle arrays of recn recursively
		//handles arrays of fieldnames and recns efficiently when items are string ids
		//since extraction of all elements for one element id is probably time consuming
		if (typeof recn == 'object' && recn.length && !elementx.tagName) {
			if (typeof elementx.length == 'undefined')
				elementx = [elementx]
			for (var ii = 0; ii < recn.length; ii++) {
				if (!(exosetreadonly(elementx[recn[ii]], msg, options)))
					return false
			}
			return true
		}

		if (!elementx.tagName && typeof recn != 'undefined' && recn != null && recn !== '') {
			elementx = elementx[recn]
			if (typeof elementx == 'undefined') {
				return systemerror('exosetreadonly(' + elements + ',' + msg + ',' + recn + ')')
			}
			elementx = [elementx]
		}
		if (!elementx.tagName && elementx.length == 1)
			elementx = elementx[0]
	}

	//handle arrays recursively
	//if (typeof elementx=='object'&&elementx.length&&!elementx.name)
	if (typeof elementx == 'object' && elementx.length && !elementx.getAttribute) {
		for (var ii = 0; ii < elementx.length; ii++) {
			//if (!(exosetreadonly(elementx[ii],msg,options,recn))) return false
			//dont stop just because one doesnt exist
			exosetreadonly(elementx[ii], msg, options, recn)
		}
		return true
	}

	//from here on we are handling one element

	if (!options) options = ''

	//skip dict items that are marked readonly
	//in chart of accounts dictitem is not available
	if (gds.dictitem) {
		var di = gds.dictitem(elementx.id)
		if (!di || di.readonly)
			return true
	}

	// hasAttribute polyfill if missing
	if (!elementx.hasAttribute) {
		elementx.hasAttribute =
			function hasAttribute(attrName) {
				return typeof this[attrName] !== 'undefined'
			}
	}

	if (msg) {
		elementx.setAttribute('exoreadonly', msg)

		//activeElement not available everywhere
		//if (elementx.id!=document.activeElement.id)
		if (gevent && (typeof gevent.target == 'undefined' || elementx.id != gevent.target.id)) {
			//remove tabindex (unless we are on the element otherwise tab key doesnt work anymore!)
			elementx.setAttribute('oldtabindex', elementx.tabIndex)
			elementx.tabIndex = -1
		}

		//if (elementx.onchange)
		// elementx.oldonchange=elementx.onchange
		//elementx.onchange=readonly_onchange
		addeventlistener(elementx, 'change', 'readonly_onchange')

		if (options && options.indexOf('BGCOLOR') >= 0) {
			if (!elementx.getAttribute('oldbgcolor'))
				elementx.setAttribute('oldbgcolor', elementx.style.backgroundColor)
			// Blend with host cell (not icon wrap SPAN); never hardcode #f6f6f6.
			var hostcell = getancestor(elementx, ' TD TH ') || elementx.parentNode
			var newbgcolor = hostcell && hostcell.style
				? hostcell.style.backgroundColor : ''
			if (!newbgcolor && hostcell && typeof getComputedStyle != 'undefined') {
				try {
					newbgcolor = getComputedStyle(hostcell).backgroundColor
				} catch (e) { }
			}
			if (!newbgcolor || newbgcolor === 'transparent'
				|| newbgcolor === 'rgba(0, 0, 0, 0)')
				newbgcolor = 'transparent'
			elementx.style.backgroundColor = newbgcolor

			//spans have no type
			if (!elementx.type || !elementx.type.match(gradiocheckboxtypes)) {
				if (!elementx.oldborderwidth)
					elementx.setAttribute('oldborderwidth', elementx.style.borderWidth)
				elementx.style.borderWidth = '1px'

				if (!elementx.hasAttribute('oldborderstyle'))
					elementx.setAttribute('oldborderstyle', elementx.style.borderStyle)
				elementx.style.borderStyle = 'solid'

				if (!elementx.hasAttribute('oldbordercolor'))
					elementx.setAttribute('oldbordercolor', elementx.style.borderColor)
				// Match form border token when present; else leave empty (no light-grey hardcode)
				var bcol = ''
				try {
					if (typeof getComputedStyle != 'undefined')
						bcol = getComputedStyle(document.documentElement)
							.getPropertyValue('--exoform-border-color').trim()
				} catch (e2) { }
				elementx.style.borderColor = bcol || 'currentColor'
			}

		}

		if (elementx.type && elementx.type.match(gradiocheckboxtypes))
			elementx.setAttribute('disabled', true)

		if (options && options.indexOf('BORDER') >= 0) {
			if (!elementx.hasAttribute('oldborderstyle'))
				elementx.setAttribute('oldborderstyle', elementx.style.borderStyle)
			// Field chrome is border-bottom underline; clear both so INPUT and SPAN
			// lose the data-field mark (style alone can leave bottom longhand visible).
			if (!elementx.hasAttribute('oldborderbottom'))
				elementx.setAttribute('oldborderbottom', elementx.style.borderBottom)
			elementx.style.borderStyle = 'none'
			elementx.style.borderBottom = 'none'
		}

	}
	else {

		elementx.removeAttribute('exoreadonly')
		//if (elementx.oldonchange)
		// elementx.onchange=elementx.oldonchange
		//addeventlistener(elementx,'change',elementx.oldonchange)
		//else
		{
			if (elementx.hasAttribute('onchange') || elementx.onchange) {
				elementx.removeAttribute('onchange')
				//removeAttribute does not seem to work on functions so also
				elementx.onchange = null
				//TODO check if this is a memory leak
				//TODO implement REMOVEEVENTLISTENER and do it here
				//addeventlistener(elementx,'change',function(){})
			}
		}
		if (elementx.hasAttribute('oldtabindex')) {
			elementx.tabIndex = elementx.getAttribute('oldtabindex')
			//elementx.removeAttribute('oldtabindex')
		}
		if (elementx.hasAttribute('oldbgcolor'))
			elementx.style.backgroundColor = elementx.getAttribute('oldbgcolor')

		elementx.removeAttribute('disabled')

		if (elementx.hasAttribute('oldborderstyle'))
			elementx.style.borderStyle = elementx.getAttribute('oldborderstyle')
		if (elementx.hasAttribute('oldborderbottom'))
			elementx.style.borderBottom = elementx.getAttribute('oldborderbottom')

	}

	//move onto next field if setting current focus field to readonly
	//but only if default tabindex since focusnext cant find the next tabindex properly
	//activeElement not available everywhere
	//if (document.activeElement.getAttribute('exoreadonly')&&gpreviouselement&&document.activeElement.tabIndex==exo_tabindex_default)
	if (gevent && typeof gevent.target != 'undefined' && gevent.target.getAttribute && gevent.target.getAttribute('exoreadonly') && gpreviouselement && document.activeElement.tabIndex == exo_tabindex_default)
		focusnext(gpreviouselement)

	return true

}

async function readonly_onchange(event) {
	event = getevent(event)

	//called when user starts to change a readonly element

	//ckeditor configured to pass element.id in event
	if (event.target && event.target.listenerData) {
		event.target = $$(event.target.listenerData)
	}

	var element = event.target
	var readonlymsg = element.getAttribute('exoreadonly')
	if (!readonlymsg || readonlymsg == 'true')
		return

	setvalue(gpreviouselement, gpreviousvalue)

	await exoui_invalid(readonlymsg)

	return exocancelevent(event)

}

function setvalue(element, valueorvalues) {

	//given an element (array or first element for radio/checkbox buttons)
	//set the value of INPUT text/INPUT radio/INPUT checkbox/SPAN/SELECT elements

	if (!element) {
		systemerror('setvalue()', 'required argument "element" is missing')
		return
	}

	//convert element name to element
	var elementid = element.id
	if (typeof element == 'string') {
		var elementid = element
		element = $$(element)
		if (!element) {
			systemerror('setvalue', exoquote(elementid) + ' element does not exist')
			return
		}
	}

	//login('setvalue ' + elementid + ' ' + valueorvalues)

	setvalue2(element, valueorvalues)

	//logout('setvalue ' + elementid + ' ' + valueorvalues)

}

function setvalue2(element, value) {

	//unprotected (faster) core of setvalue()
	value = value.toString()
	// Writing a miss-tinted field (Esc restore, setx, pick, …) drops the class
	if (form_is_miss_tinted(element))
		form_typeahead_set_miss(element, false)

	switch (element.tagName) {
		case 'INPUT': {

			switch (element.type) {

				case 'text': {

					setexolink(element, value)
					element.value = value
					// colour fields: keep swatch in step with bound text
					if (element.getAttribute('data-exo-color-field') == '1'
						&& typeof colors_sync_swatch == 'function')
						colors_sync_swatch(element)
					break
				}

				case 'radio': {

					var elements = getradiocheckboxelements(element)
					for (var ii = 0; ii < elements.length; ii++) {
						var element = elements[ii]
						if (value.exolocate(element.value.toString()) > 0) {
							element.checked = true
							value = element.value
							if (value == 'undefined')
								value = ii
						}
						else
							element.checked = false
					}

					break

				}

				case 'checkbox': {

					//multiple values as : or , separated string
					if (typeof value == 'string' && value.indexOf(':') >= 0) value = value.split(':')
					if (typeof value == 'string') value = value.split(sm)

					var elements = getradiocheckboxelements(element)
					for (var ii = 0; ii < elements.length; ii++) {
						var element = elements[ii]
						if (value.exolocate(element.value.toString()) > 0)
							element.checked = true
						else
							element.checked = false
					}

					break

				}

			}

			break//INPUT
		}

		case 'TEXTAREA': {

			setexolink(element, value)

			//http://wiki.ckeditor.net/Developer%27s_Guide/Javascript_API
			if (typeof CKEDITOR != 'undefined') {
				oEditor = CKEDITOR.instances[element.id]
				if (oEditor) {
					//oEditor.SetHTML(value)
					//oEditor.setData(value)
					// defer setData — overlapping async setData can throw "permission denied"
					window.setTimeout(function () {
						try {
							oEditor.setData(value)
							//oEditor.updateElement(value)
						} catch (e) { }
					}, 500)
				}
			}

			element.value = value
			break
		}

		case 'SPAN': {

			setexolink(element, value)

			if (element.getAttribute('exoimage')) {
				var img = element.nextSibling
				if (!img)
					img = element.parentNode.nextSibling
				if (img && img.tagName == 'IMG')
					img.src = value
			}

			//this puts a char 160 no-break-space to ensure height isnt zero
			//the other solution, putting style min-height doesnt zoom with ctrl+ ctrl- (does in ff 32)
			//if (value == '')
			//    value = '&nbsp;'

			//treat as text
			//IF element is contenteditable and the new value doesnt look like HTML
			// and browser doesnt support "Sequences of whitespace are preserved."
			if (element.isContentEditable && (value.slice(0, 1) != '<' || value.slice(-1) != '>') && element.style.whiteSpace != 'pre-wrap')

				//WARNING:
				//leading, trailing and excess spaces will be converted to character 160 (&nbsp;) in order to retain
				//their appearance since HTML spans dont show leading, trailing or more than one consecutive space
				//THEREFORE using span tags (but not input tags) CAN CONTAIN CHAR(160) IN TEXT ON THE SERVER
				element.innerText = value

			//treat is HTML
			//if element is not contenteditable, or the new value looks like HTML
			else
				element.innerHTML = value
			break
		}

		case 'SELECT': {

			setdropdown3(element, null, '', value)
			break
		}

		case 'OPTION': {

			setdropdown3(element.parentNode, null, '', value)
			break
		}

		default: {

			return systemerror('setvalue2()', exoquote(element) + ' ' + exoquote(element.tagName) + ' invalid tagName in setvalue2(' + element + ',' + value + ')')
		}

	} //of switch

}

function getradiocheckboxelements(element) {

	var scope
	var elements
	if (Number(element.getAttribute('exogroupno'))) {
		var scope = getancestor(element, 'TR')
		if (!scope) {
			return systemerror('getradiocheckboxelements', element.id + ' has no parentNode')
		}
	}
	else {
		scope = document
	}
	if (document.getElementsByClassName)
		elements = scope.getElementsByClassName('exoid_' + (element.id ? element.id : element))
	else {
		elements = scope.all[element.id ? element.id : element]
		if (elements && elements.tagName)
			elements = [elements]
	}

	if (!elements.length)
		systemerror('getradiocheckboxelements("' + element.id + '") No elements found.')

	return elements

}

function setexolink(element, value) {
	return//always display now
	/*    if (element.getAttribute('exolink')) {
			if (element.previousSibling && element.previousSibling.getAttribute('exolink')) {
				element.previousSibling.style.display = value ? '' : 'none'
			}
			else if (element.nextSibling && element.nextSibling.getAttribute('exolink')) {
				element.nextSibling.style.display = value ? '' : 'none'
			}
		}
	*/
}

async function getdefault(element) {

	//return default value string or ''

	//make sure is element
	if (typeof element == 'string') {
		element = $$(element)
		if (element && element[0])
			element = element[0]
	}
	assertelement(element, 'getdefault', 'element')

	//default value of a select item is the selected item if not already defaulted
	//suppress otherwise cannot have "" value except as first item eg (X;XXXX:;YYYY)
	//if (!element.getAttribute('exodefaultvalue')&&element.tagName=='SELECT')
	//{
	// //login('getdefault '+element.id)
	// var defaultvalue=getvalue(element)
	// logout('getdefault '+element.id+' SELECT '+defaultvalue)
	// return defaultvalue
	//}

	//return '' if no default
	defaultvalueexpression = element.getAttribute('exodefaultvalue')
	if (!defaultvalueexpression || defaultvalueexpression == '""')
		return ''

	//login('getdefault ' + element.id)

	//calculate default
	var defaultvalue = await exoevaluate(defaultvalueexpression, 'await getdefault(' + element.id + ')');

	//select elements always have a default
	if (element.tagName == 'SELECT') {
		if (defaultvalue == '' && element.tagName == 'SELECT')
			defaultvalue = getvalue(element)
	}

	//don't default if unique and already present
	if (defaultvalue && element.getAttribute('exounique')) {
		var othervalues = getvalues(element.id)
		if (othervalues.exolocate(defaultvalue))
			defaultvalue = ''
	}

	//convert numbers to strings and check is a string
	if (typeof defaultvalue == 'number')
		defaultvalue = defaultvalue.toString()
	if (typeof defaultvalue != 'string') {
		await exoui_invalid(element.id + ' default returned is ' + typeof defaultvalue + ' - "" used\nExpression:' + defaultvalueexpression)
		defaultvalue = ''
	}

	//logout('getdefault ' + element.id + ' ' + defaultvalue)
	return defaultvalue

}

// Whole-field selection (arrival / select-all). Shared by arrow nav and Tab-as-data.
// Empty field counts as all-selected (same as classic left/right in MV).
// INPUT: selectionStart/End. Contenteditable SPAN: window Selection.
function form_field_all_selected(element) {
	if (!element)
		return true
	var text = getvalue(element)
	if (!text)
		return true

	if (typeof element.selectionStart == 'number') {
		if (element.selectionStart != 0 || element.selectionEnd != text.length)
			return false
		return true
	}
	if (document.selection) {
		if (document.selection.createRange().text.replace(/[\r\n]/g, '') != text.replace(/[\r\n]/g, ''))
			return false
		return true
	}
	if (window.getSelection) {
		var selection = window.getSelection()
		if (!selection || selection.rangeCount < 1)
			return true
		// Caret or partial → not all selected (editing)
		if (selection.isCollapsed)
			return false
		if (selection.toString
			&& selection.toString().replace(/[\r\n]/g, '') == text.replace(/[\r\n]/g, ''))
			return true
		// SPAN multi-node: range covers full contents?
		try {
			var range = selection.getRangeAt(0)
			var full = document.createRange()
			full.selectNodeContents(element)
			if (range.compareBoundaryPoints(Range.START_TO_START, full) <= 0
				&& range.compareBoundaryPoints(Range.END_TO_END, full) >= 0)
				return true
		} catch (e) { }
		return false
	}
	return true
}

// Tab-as-data for [INDENTED] (e.g. charts ACCOUNT_NAME). Consumes Tab only while
// editing — same whole-field test as left/right (form_field_all_selected).
// Empty field: all-selected for arrows, but still allow indent (blank-row indent).
function form_try_insert_tab_char(element) {
	if (!element)
		return false
	var conv = element.getAttribute('exoconversion') || ''
	if (conv.slice(0, 1) != '[')
		return false
	var convname = conv.slice(1, -1).split(',')[0].toUpperCase()
	if (convname != 'INDENTED')
		return false
	if (element.getAttribute('exoreadonly') || element.disabled
		|| element.getAttribute('disabled') != null)
		return false
	// Non-empty whole-field select → navigate (Tab); caret/partial → insert
	var text = getvalue(element)
	if (text && form_field_all_selected(element))
		return false

	if (element.tagName == 'INPUT' || element.tagName == 'TEXTAREA') {
		var start
		var end
		try {
			start = element.selectionStart
			end = element.selectionEnd
		} catch (e) {
			return false
		}
		if (typeof start != 'number' || typeof end != 'number')
			return false
		var v = element.value || ''
		element.value = v.slice(0, start) + '\t' + v.slice(end)
		var pos = start + 1
		try {
			element.setSelectionRange(pos, pos)
		} catch (e) { }
		settouched(true)
		return true
	}

	// contenteditable SPAN (dict_text host)
	if (!element.isContentEditable)
		return false
	var sel = window.getSelection && window.getSelection()
	if (!sel || sel.rangeCount < 1)
		return false
	var anchor = sel.anchorNode
	if (anchor && anchor !== element && !element.contains(anchor))
		return false
	try {
		if (document.execCommand && document.queryCommandSupported
			&& document.queryCommandSupported('insertText')) {
			document.execCommand('insertText', false, '\t')
		} else {
			var range = sel.getRangeAt(0)
			range.deleteContents()
			var tn = document.createTextNode('\t')
			range.insertNode(tn)
			range.setStartAfter(tn)
			range.collapse(true)
			sel.removeAllRanges()
			sel.addRange(range)
		}
	} catch (e) {
		return false
	}
	settouched(true)
	return true
}

//var gautofitwindowpending
function settouched(value, savebuttonactive) {
	gtouched = value
	if (!gtouched)
		gelementthatjustcalledsettouched = null
	if (typeof savebuttonactive == 'undefined')
		savebuttonactive = value || !gKeyNodes
	var savebuttonstyle = savebuttonactive ? gsaveimage : gsavegreyimage
	setgraphicbutton(saverecord, null, savebuttonstyle)

	//if (gautofitwindow && !gautofitwindowpending) {
	//    gautofitwindowpending = true
	//    exosettimeout('exoautofitwindow()', 1)
	//}

}

async function setdefault(element, donotupdate) {

	//used in
	//1. newrecordfocus and cleardoc to point to 1st element
	//2. focuson (why? should be setvalue?)
	//3. onfocus at end
	//4. checkrequired

	//if no value, get default and set value

	if (typeof element == 'string')
		element = document.getElementById(element)

	// colour swatch is paint-only chrome — never invent a stored default into it
	if (element && element.type == 'color')
		return true

	//cannot update anything but key field if not locked or save button not enabled
	if (element.getAttribute('exofieldno') != 0 && gKeyNodes && (!glocked || saverecord.getAttribute('disabled'))) {
		//  return true
	}

	//return true if already defaulted

	if (element.dataitem) {
		if (element.dataitem.defaulted)
			return true
	}
	else {
		//only the first element of radio and checkboxes are bound to data
		//should really check if the first element is already defaulted
		if ('checkbox radio'.indexOf(element.type) >= 0)
			return true
	}

	//return true if already has a value
	//var value=gds.getcells(element,grecn)[0].text<A HREF="http://localhost/exodus7/EXODUS/images/">http://localhost/exodus7/EXODUS/images/</A>
	var value = getvalue(element)
	if (value)
		return true

	//signal that it has been defaulted (even if there is no default)
	if (element.dataitem)
		element.dataitem.defaulted = true

	//login('setdefault ' + element.id)

	//get the default value
	var storegrecn = grecn
	grecn = getrecn(element)
	gdefault = await getdefault(element)
	grecn = storegrecn

	//for select elements with no default, the first element is the default
	//is this not done in the binding routine now?
	if (gdefault == '' || gdefault == null || typeof (gdefault) == 'undefined') {
		if (element.childNodes.length)
			gdefault = element.childNodes[0].value
	}

	//if no default return
	if (!gdefault) {
		//logout('setdefault ' + element.id + ' no default')
		return true
	}

	//save the default for use eg by custom validation routines to avoid work
	element.setAttribute('exo_default', gdefault)

	//set the value (externally only)
	//await gds.setx(element,grecn,gdefault)

	setvalue(element, await oconvertvalue(gdefault, element.getAttribute('exoconversion')))

	//call the validate/update routine
	if (!donotupdate) {

		//changed to not mess with gpreviouselement
		savegpreviouselement = gpreviouselement
		savegpreviousvalue = gpreviousvalue

		//gpreviouselement = element
		//gpreviousvalue = ''//gpreviousvalue=value
		setgpreviouselement(element, '')

		if (!(await validateupdate())) {
			//logout('setdefault ' + element.id + ' invalid')
			return false
		}

		//restore
		if (savegpreviouselement != gpreviouselement) {
			gpreviouselement = savegpreviouselement
			gpreviousvalue = savegpreviousvalue
		}

	}

	//log(element.id + ' defaulted to ' + gdefault)

	//logout('setdefault ' + element.id + ' ' + exoquote(gdefault))

	return true

}

//''''''''''''''
//'BEFORE UPDATE
//''''''''''''''
async function onbeforeupdate(element) {
	//move to validateupdate?

	//ignore updates while closing
	if (gclosing) return false

	//login('onbeforeupdate')

	// if (typeof(element)=='undefined')
	// element=event.target

	//get grecn to be safe
	grecn = getrecn(element)

	//skip if value not changed
	var value = getvalue(element)
	if (value == gpreviousvalue) {
		//logout('onbeforeupdate')
		return true
	}

	//cannot update anything but key field if not locked or save button not enabled
	//if (element.getAttribute('exofieldno')!=0&&gKeyNodes&&(!glocked||saverecord.getAttribute('disabled')))
	if (element.getAttribute('exofieldno') != 0 && gKeyNodes && !glocked) {
		setvalue(gpreviouselement, gpreviousvalue)
		await readonlydocmsg()
		//logout('onbeforeupdate')
		return await exoui_invalid('')
	}

	//post entry processing
	if (!(await validate(element))) {
		//this should already have been done in the validate routine
		//element.focus()
		return false //logout('onbeforeupdate')
	}

	//logout('onbeforeupdate')

	return true

}

//only called from await validateupdate() so some code might be unnecessary duplicated like getvalue() etc
async function validate(element) {

	//new value
	gvalue = getvalue(element).toString()

	//login('validate ' + element.id + ' ' + gvalue)

	//extract other infor
	var name = element.name

	//extract global info

	//group number
	ggroupno = Number(element.getAttribute('exogroupno'))

	grecn = getrecn(element)

	//log('grecn=' + grecn)

	//old value
	goldvalue = gds.getcells(element, grecn)
	if (goldvalue != null)
		goldvalue = goldvalue[0].text

	//convert to uppercase
	if (!element.getAttribute('exolowercase') && element.type != 'radio' && element.type != 'checkbox') {
		var tt = gvalue.toUpperCase()
		if (tt != gvalue) {
			gvalue = tt
			//update it immediately in case something like await getkey()
			setvalue(element, gvalue)
		}
	}

	//get something to show in error messages
	var elementtitle = element.getAttribute('exotitle')
	if (!elementtitle)
		elementtitle = element.id

	if (element.tagName != 'SELECT' && element.type != 'radio') {

		/*
		//invalid character check (nearly all punctuation except - / and #)
		if (element.getAttribute('exoinvalidcharacters')) {

		var invalidcharacters=element.getAttribute('exoinvalidcharacters')+'\xF8\xF9\xFA\xFB'//\xFC\xFD\xFE\xFF'

		//ignore any valid characters
		if (element.getAttribute('exovalidcharacters')) invalidcharacters=invalidcharacters.exoconvert(element.getAttribute('exovalidcharacters'),'')

		}
		else {

		var invalidcharacters=FMs//'\xF8\xF9\xFA\xFB'//\xFC\xFD\xFE\xFF'
		}
		*/

		//special key field validation and conversion
		if (Number(element.getAttribute('exofieldno')) == 0) {

			//remove all non-printing ASCII characters (eg tab) from key fields
			gvalue = gvalue.replace(/[\x00-\x1F]/g, '')

			//prevent anything that is effectively 0 unless it is a checkbox
			if (element.type != 'checkbox' && exonum(gvalue) && !Number(gvalue)) {
				await exoui_invalid(elementtitle + ' cannot be zero')
				return false //logout('validate')
			}
		}

		//invalid character check
		//generally dont allow any field marks (can allow specific ones by putting them in valid characters)
		var invalidcharacters = FMs
		if (element.type == 'checkbox')
			invalidcharacters = invalidcharacters.exoconvert(sm, '')
		if (element.getAttribute('exoinvalidcharacters')) {
			invalidcharacters += element.getAttribute('exoinvalidcharacters')

			//valid characters override invalid characters (but be careful not to allow field marks
			if (element.getAttribute('exovalidcharacters'))
				invalidcharacters = invalidcharacters.exoconvert(element.getAttribute('exovalidcharacters'), '')

		}

		var temp = gvalue.exoconvert(invalidcharacters, '')
		if (temp != gvalue) {
			//   alert('Punctuation characters and spaces (except / - and #) are not allowed in key fields and have been removed\n')
			//var charsx=invalidcharacters
			//get unused invalid characters
			var charsx = invalidcharacters.exoconvert(gvalue, '')
			//get used invalid characters
			charsx = invalidcharacters.exoconvert(charsx, '').exoswap('|', '&#124;')
			//.exoswap(fm,'&u'+fm.charCodeAt(0)+';')
			await exoui_invalid('The following characters are not allowed in ' + elementtitle + '.\n\n&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span style="border:1px solid #DDDDDD;padding-bottom:2px"> ' + charsx.exoswap(' ', ' space ') + '&nbsp;</span><br />&nbsp;')
			return false //logout('validate')
		}

		//valid character check
		if (element.getAttribute('exovalidcharacters')) {

			var temp = gvalue.exoconvert(element.getAttribute('exovalidcharacters'), '')
			if (temp != '') {
				await exoui_invalid('Only the following characters are allowed in ' + elementtitle + '.\n\&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"' + element.getAttribute('exovalidcharacters').exoswap('|', '&#124;') + '\"')
				return false //logout('validate')
			}
		}

	}

	//required check
	//log('required check')
	if (gvalue == ''
		&& element.getAttribute('exorequired')
		&& exoenabledandvisible(element)) {
		await exoui_invalid(elementtitle + ' is required...')
		return false //logout('validate')
	}

	//log('before file check')
	//file check (skip if has dropdown)
	if (element.getAttribute('exofilename')
		&& gvalue != ''
		&& !element.getAttribute('exodropdown')) {

		var filename = element.getAttribute('exofilename')
		var key = gvalue

		//exodus hack (possibly not used anymore as .filename='ACCOUNTS' not used?
		if (filename == 'ACCOUNTS') key = '.' + gvalue

		db.request = 'CACHE\rREAD\r' + filename + '\r' + key
		if (!(await db.send())) {

			if (db.response.indexOf('NO RECORD') >= 0) db.response = exoquote(gvalue) + ' ' + element.getAttribute('exotitle') + ' is not on file.'

			await exoui_invalid(db.response)

			return false //logout('validate - not on file ' + gvalue)

		}

	}

	//log('before conversion')

	gvaluebeforeiconv = gvalue

	//if conversion is a routine. eg [NUMBER] [DATE] are standard
	var conversion = element.getAttribute('exoconversion')
	if (typeof (conversion) != 'string' || conversion.slice(0, 1) != '[')
		conversion = false

	//input conversion
	if (conversion && gvalue != '') {

		//format is '[functionname,options]'
		var convarray = conversion.slice(1, -1).split(',')
		//if no options, make them ''
		if (convarray.length == 1)
			convarray[1] = ''

		//see also OCONV below

		//var quotechar = (gvalue.indexOf('"') >= 0) ? "'" : '"'
		//var expression = convarray[0] + '(' + '"ICONV",' + quotechar + gvalue.replace(/\\/g, '\\\\') + quotechar + ',"' + convarray.slice(1) + '")'

		var value = gvalue.replace(/\\/g, '\\\\')
		value = value.replace(/\"/g, '\\"')
		value = value.replace(/[\x0D]/g, '\\r')
		value = value.replace(/[\x0A]/g, '\\n')
		// all options after the function name (e.g. NDECS,CURRENCY) — not only [1]
		var convopts = convarray.slice(1).join(',')
		var expression = convarray[0] + '(' + '"ICONV","' + value + '","' + convopts + '")'

		gmsg = ''
		ivalue = await exoevaluate(expression, 'await validate(' + element.id + ') iconv');
		if (typeof ivalue == 'undefined') {
			return false //logout('validate - system error in input conversion')
		}

		//null means failed to convert to internal value therefore invalid
		if (gvalue == null || ivalue == null) {
			//error message (use the conversion program name in the message)
			await exoui_invalid(exoquote(gvalue) + ' is not a valid ' + convarray[0].toLowerCase().replace(/_/g, ' ') + '\n\n' + gmsg)
			return false //logout('validate - input conversion returned null')
		}

		//switch the data into internal format
		gvalue = ivalue

	}

	//custom validation - data in internal format
	//log('before custom validation')
	var storegrecn = grecn
	var elementvalidation = element.getAttribute('exovalidation')
	if (elementvalidation) {

		var ok
		if (typeof elementvalidation == 'function')
			ok = elementvalidation()
		else
			ok = await exoevaluate(elementvalidation, 'await validate() functioncode');

		if (gvalue == null)
			await exoui_warning(element.id + ' validation routine returned gvalue=null')
		if (!ok || gvalue == null) {
			grecn = storegrecn
			await exoui_invalid()
			//logout('validate - validation function code returned false or gvalue as null')
			return false
		}

	}

	//get grecn again in case any prior lines have been deleted in the validation routine
	//grecn=storegrecn
	grecn = getrecn(gpreviouselement)

	//check for uniqueness for multivalues
	//log('before unique check')
	if (gvalue && ggroupno > 0 && element.getAttribute('exounique')) {
		var othervalues = await gds.getall(element.id)
		var ln
		//othervalues[grecn]='' //not needed because only validate if changed
		othervalues[grecn] = '' //put back because of a validation after a multiple choice popup fails
		if (ln = othervalues.exolocate(gvalue)) {
			gmsg = exoquote(gvaluebeforeiconv) + ' is already used in line ' + ln + '.'
			if (element.getAttribute('exononuniquewarning')) {
				if (!(confirm('Warning:\n\n' + gmsg, 1))) {
					//logout('validate - not unique warning')
					return await exoui_invalid()
				}
			}
			else {
				//logout('validate - not unique')
				return await exoui_invalid(gmsg)
			}
		}
	}

	//check for sequential multivalues
	//assumes that numbers will be in number format
	var elementsequence = element.getAttribute('exosequence')
	if (gvalue
		&& ggroupno > 0
		&& elementsequence) {
		var title = element.getAttribute('exotitle')
		if (elementsequence == 'A') {
			var temp
			if ((temp = await getpreviousrow('', { skipblanks: true, internal: true }))
				&& gvalue < temp) {
				//    alert(typeof gvalue+' '+gvalue+' < '+typeof temp+' '+temp)
				//logout('validate - not sequential')
				return await exoui_invalid(title + ' cannot be less than ' + title + ' in the previous row above')
			}
			var temp
			if ((temp = await getnextrow('', { skipblanks: true, internal: true }))
				&& gvalue > temp) {
				//    alert(typeof gvalue+' '+gvalue+' > '+typeof temp+' '+temp)
				//logout('validate - not sequential')
				return await exoui_invalid(title + ' cannot be greater than ' + title + ' in the next row below')
			}
		}
	}

	//output conversion
	//log('before output conversion')

	ovalue = await validateoconv(element, gvalue)
	// validateoconv returns false after invalid, or null/undefined on hard fail —
	// must not fall through to return true (looked like silent Esc).
	if (ovalue === false || ovalue == null || ovalue === 'undefined')
		return false //logout('validate - oconv failed')

	// Canonical external into the DOM for all [ conversions (e.g. 1,11,1.00AED → 1,111.00AED).
	// Keep gvalue internal for setx. getvalue is DOM external so grouping edits reach validate.
	if (conversion && String(ovalue) !== String(getvalue(element)))
		setvalue(element, ovalue)

	//logout('validate ' + element.id + ' ' + gvalue)

	return true

}

// OCONV ivalue (internal) onto the element if DOM still shows pre-canonical external.
async function form_paint_oconv_if_needed(element, ivalue) {
	var conversion = element && element.getAttribute && element.getAttribute('exoconversion')
	if (typeof conversion != 'string' || conversion.slice(0, 1) != '[')
		return true
	var ovalue = await validateoconv(element, ivalue == null ? '' : ivalue)
	if (ovalue === false || ovalue == null || typeof ovalue == 'undefined')
		return false
	if (String(ovalue) !== String(getvalue(element)))
		setvalue(element, ovalue)
	return true
}

async function validateoconv(element, ivalue) {

	// returns ovalue string (or '') on success; false on fail.
	// Callers must treat false (and null/undefined) as failure — not only null.

	//skip if nothing to convert
	ivalue = ivalue.toString()
	var ovalue = ivalue
	if (ovalue == '')
		return ovalue

	//if conversion is a routine. eg [NUMBER] [DATE] are standard
	var conversion = element.getAttribute('exoconversion')
	if (typeof conversion != 'string' || conversion.slice(0, 1) != '[')
		conversion = false
	if (!conversion)
		return ovalue

	//format is '[functionname,options]'
	var convarray = conversion.slice(1, -1).split(',')
	//if no options, make them ''
	if (convarray.length == 1)
		convarray[1] = ''

	//convert internal to external format
	//nb cannot cancel after updating element.value
	//var quotechar = (ivalue.indexOf('"') >= 0) ? "'" : '"'
	//var expression = convarray[0] + '(' + '"OCONV",' + quotechar + ivalue.replace(/\\/g, '\\\\') + quotechar + ',"' + convarray[1] + '")'

	ivalue = ivalue.replace(/\\/g, '\\\\')
	ivalue = ivalue.replace(/\"/g, '\\"')
	ivalue = ivalue.replace(/[\x0D]/g, '\\r')
	ivalue = ivalue.replace(/[\x0A]/g, '\\n')
	// all options after the function name (must include CURRENCY when present)
	var convopts = convarray.slice(1).join(',')
	var expression = convarray[0] + '(' + '"OCONV","' + ivalue + '","' + convopts + '")'

	gmsg = ''
	var ovalue = await exoevaluate(expression, 'validateoconv ' + (element && element.id))
	if (typeof ovalue == 'undefined') {
		await exoui_invalid(exoquote(ivalue) + ' output conversion failed (undefined)\n' + gmsg)
		return false
	}

	//null means failed to convert to external value therefore invalid
	if (ovalue == null) {
		await exoui_invalid(exoquote(ivalue) + ' is not a valid ' + convarray[0].toLowerCase() + '\n' + gmsg)
		return false
	}

	return ovalue

}

// Suspend calcfields (depth). Callers flush with calcfields_resume().
function calcfields_suspend() {
	gcalcfields_suspend++
}

// Queue fieldns while suspended (null/undefined = full calc).
function calcfields_queue(fieldns) {
	if (fieldns == null || typeof fieldns == 'undefined') {
		gcalcfields_pending = true
		return
	}
	if (gcalcfields_pending === true)
		return
	if (!gcalcfields_pending)
		gcalcfields_pending = []
	if (typeof fieldns != 'object')
		fieldns = [fieldns]
	for (var i = 0; i < fieldns.length; i++) {
		var fn = fieldns[i]
		if (fn === '' && typeof fn != 'number')
			continue
		if (gcalcfields_pending.indexOf(fn) < 0)
			gcalcfields_pending.push(fn)
	}
}

// End suspend; run queued calcfields once with grecn null (mass xlate).
async function calcfields_resume() {
	if (gcalcfields_suspend > 0)
		gcalcfields_suspend--
	if (gcalcfields_suspend > 0)
		return true
	var pending = gcalcfields_pending
	gcalcfields_pending = null
	if (pending === null || pending === undefined)
		return true
	var savegrecn = grecn
	grecn = null
	try {
		if (pending === true)
			await calcfields(null)
		else if (pending.length)
			await calcfields(pending)
	} finally {
		grecn = savegrecn
	}
	return true
}

//given an array of field numbers calculate and set their contents
async function calcfields(fieldns) {

	//login('calcfields')

	if (gKeyNodes && !gloaded)
		return false //logout('calcfields no record')

	// Multi-entry insertallrows: queue only (flush in calcfields_resume)
	if (gcalcfields_suspend > 0) {
		calcfields_queue(fieldns)
		return true
	}

	//if list is empty then do all calculated fields
	//exclude real fields (type=F) that have functioncode specified
	var origfieldns = fieldns
	if (fieldns == null) {
		fieldns = []
		var field
		for (var fn = 0; fn < gfields.length; fn++) {
			var field = gfields[fn]
			if (!field) {
				systemerror('await calcfields()', 'gfields[' + fn + '] is undefined.')
			}
			else {
				if (field.getAttribute('exofunctioncode') && field.getAttribute('exotype') != 'F') {
					fieldns[fieldns.length] = fn
				}
			}
		}
	}

	//convert fieldns to an array if necessary
	if (typeof (fieldns) != 'object') fieldns = [fieldns]

	for (var fn = 0; fn < fieldns.length; fn++) {

		if (fieldns[fn] != '' || typeof fieldns[fn] == 'number') {

			var field = gfields[fieldns[fn]]

			//only do calculated fields
			if (field.getAttribute('exofunctioncode')) {

				//add dependents of dependents to the list to recalc
				var deps = field.getAttribute('exo_dependents')
				if (deps) {
					deps = deps.split(';')
					for (var depn = 0; depn < deps.length; depn++) {
						//prevent recursion by limiting to 1000 fields
						if (fieldns.length < 1000)
							fieldns[fieldns.length] = deps[depn]
					}
				}

				//recalculate and update record
				await gds.regetx(field.id)

			}
		}
	}

	if (typeof origfieldns == 'undefined') gdependents = []

	//logout('calcfields')

}

//version to make grecn null and restore it afterwards
async function exoevaluateall(functioncode, callerfunctionname) {
	var storegrecn = grecn
	grecn = null
	var result = await exoevaluate(functioncode, callerfunctionname)
	grecn = storegrecn
	return result
}

async function exoevaluate(functionorcode, callerfunctionname, arg1name, arg1, thisobject) {

	//wrapper to call custom functions or expressions (eg validation etc)
	//arg1name and arg1 are optional way of getting data into
	// arguments or variable listed in function source if functionorcode is code (text)

	//returns result of exoevaluate
	//displays message if the function does not return something and returns ''
	//failure results in error message and returns undefined!!!
	// or if gstepping then dump to de-bugger

	var result

	if (typeof functionorcode == 'undefined') {
		systemerror('exoevaluate()', 'The required argument "functionorcode" is missing. Called from\n' + callerfunctionname)
		if (gstepping || gusername == 'EXODUS') crashhere
		return
	}

	//boolean just gets returned
	if (typeof functionorcode == 'boolean' || typeof functionorcode == 'number')
		return functionorcode

	if (typeof functionorcode == 'string') {

		//simple quoted values just get returned
		//(starting and ending with single or double quote
		//and no single or double quotes internally)
		if (functionorcode.match(/^(["'])[^'"]*\1$/))
			return functionorcode.slice(1, -1)

		//special code to return today's date
		if (functionorcode == 'TODAY')
			return exodate()

		if (functionorcode == '')
			return ''
	}

	//if (gstepping||(!ginitok&&gusername=='EXODUS'))
	// if (true||gstepping||gusername=='EXODUS')
	if (gstepping || gusername == 'EXODUS' || gusername == 'STEVE')
		result = await exoevaluate3(functionorcode, null, arg1name, arg1, thisobject)
	else {

		//var e
		try {
			result = await exoevaluate3(functionorcode, null, arg1name, arg1, thisobject)
		}
		catch (e) {
			//chrome exception is not available except inside catch clause
			//} if (e) {
			//if (typeof callerfunctionname == 'undefined') callerfunctionname = '"not specified"'
			systemerror('exoevaluate()' + functionorcode, e)
			return await exoui_invalid()
		}

	}

	return result

}

//var gcatcherrors=true//use try/catch to show errors to users on screen
var gcatcherrors = false//avoid try/catch thereby allowing javascript error line number etc to show in console and/or be caught by debugger

async function exoevaluate3(functionorcode, functionname, arg1name, arg1, thisobject) {

	//arg1name and arg1 are 'event' and event in some use cases
	//in order to pass event into exo_onclick functions
	if (!arg1name)
		arg1name = 'dummyarg1name'
	if (!arg1)
		arg1 = null

	//straight function call
	if (typeof functionorcode == 'function')
		return functionorcode(arg1)

	var functioncode = functionorcode
	if (functioncode.indexOf('return ') < 0)
		functioncode = 'return ' + functioncode

	//legacy 'yield* ' prefix in dynamic function code - strip and run via async path below
	if (functioncode.match && functioncode.match(gyieldregex)) {
		functioncode = functioncode.replace(gyieldregex, '')
	}

	//non-yielding function code
	try {

		//arg1name can be the textual name of any variable in the text of the function code
		//TODO should only add return if no return in functioncode - to allow multi-line function code with returns in later lines
		if (functioncode.match && functioncode.match(/await\b/)) {
			var AsyncFunction = (async function(){}).constructor;
			functionx = new AsyncFunction(arg1name, functioncode)
			return await functionx.call(thisobject || this, arg1)
		}
		functionx = new Function(arg1name, functioncode)
	} catch (e) {
		return systemerror('exoevaluate3()\n' + functioncode, e)
	}
	//we pass in the value of the argument when calling the function
	//return functionx.apply(this,arg1)
	return await functionx.call(thisobject || this, arg1)
}

async function oconvertvalue(ivalue, conversion, element) {
	if (!conversion) return ivalue
	if (typeof (conversion) != 'string' || conversion.slice(0, 1) != '[') return ivalue
	// NUMBER OCONV defaults display=true (grouping). Use [DECIMAL,…] for plain.
	return ivalue.exooconv(conversion)
}

async function deleterow_onclick(event) {
	return await form_deleterow(event)
}

async function insertrow_onclick(event) {
	return await form_insertrow(event)
}

//'''''''''''
//'DELETE ROW
//'''''''''''
async function form_deleterow(event, element) {

	//login('deleterow')

	event = getevent(event)

	if (!element)
		element = event.target

	//fast key repeats generates delete on the table element which can be ignored
	if (element.tagName == 'TABLE')
		return

	var row = getancestor(element, 'TR')
	var tablex = getancestor(row, 'TABLE')
	grows = tablex.tBodies[0].getElementsByTagName('tr')
	var groupno = Number(tablex.getAttribute('exogroupno'))

	if (groupno == 0)
		return false //logout('deleterow group 0')

	// prefer rowIndex over sectionRowIndex (more portable)
	var rown = row.rowIndex
	if (tablex.tHead)
		rown -= tablex.tHead.rows.length

	//make sure any previous data entered is valid EXCEPT if on the current line benig deleted
	if
		(gpreviouselement
		&& (Number(gpreviouselement.getAttribute('exogroupno')) != groupno
			|| getrecn(gpreviouselement) != rown)
		&& !(await validateupdate())
	)
		return false //logout('deleterow gprevious invalid')

	//cannot delete if keyed and not locked
	if (gKeyNodes && !glocked) {
		await readonlydocmsg()
		return false //logout('deleterow')
	}

	//alert('rown='+rown)
	var nrows = grows.length
	//var grecn=row.recordNumber-1

	//zzz should be offset when paging
	grecn = rown

	//form specific before row delete function
	var predeleterow = window['form_predeleterow' + groupno]
	if (typeof predeleterow == 'function') {
		var p = predeleterow(event);
		if (p && typeof p.then === 'function') {
			p = await p;
		} else if (p && typeof p.next === 'function') {
			systemerror('deleterow', 'form_predeleterow must be async (generators removed stage 6)')
			return false
		}
		if (!p)
			return false //logout('deleterow - predelete false')
	}

	//committed from here on

	//gpreviouselement = null
	//gpreviousvalue = ''
	setgpreviouselement(null)

	settouched(true)
	//setdisabledandhidden(saverecord,false)
	//setdisabledandhidden(editreleaserecord,false)

	var pagesize = tablex.dataPagesize ? tablex.dataPagesize : 999999
	var pagen = exoint(grecn / pagesize)

	var id
	if (event.target.name)
		id = event.target.id
	else
		id = gfields[gtables[groupno][0]].id

	gds.deleterow(groupno, rown)

	//focus on first column of new current row
	if (rown > 0 && rown == (nrows - 1)) rown--//if deleting last row then focus on previous row
	grows = tablex.tBodies[0].getElementsByTagName('tr')
	var element
	//rows[rown].getElementsByClassName('exoid_'+id)
	if (document.getElementsByClassName)
		element = grows[rown].getElementsByClassName('exoid_' + id)[0]
	else
		//using (id) instead of [id] because will return only one?
		element = grows[rown].all(id)
	focuson(element)
	//gpreviouselement = element
	//gpreviousvalue = getvalue(gpreviouselement)
	setgpreviouselement(element)

	//form specific after row delete function
	var postdeleterow = window['form_postdeleterow' + groupno]
	if (typeof postdeleterow == 'function') {
		var p = postdeleterow(event);
		if (p && typeof p.then === 'function') {
			await p;
		} else if (p && typeof p.next === 'function') {
			systemerror('deleterow', 'form_postdeleterow must be async (generators removed stage 6)')
		}
	}

	var deps = tablex.getAttribute('exo_dependents')
	if (deps) {

		await calcfields(deps.split(';'))
	}

	//logout('deleterow')

}

//TODO merge deleterows and deleteallrows
async function deleterows(groupnoorelement, rowns) {

	//login('deleterows')

	var groupno = groupnoorelement
	if (typeof groupno == 'object') {
		groupno = groupnoorelement.getAttribute && groupnoorelement.getAttribute('exogroupno')
		if (!groupno && groupnoorelement[0] && groupnoorelement[0].getAttribute)
			groupno = groupnoorelement[0].getAttribute('exogroupno')
		// Table carries exogroupno; walk past icon wraps (was broken parenNode typo).
		if (!groupno) {
			var tablex = getancestor(groupnoorelement[0] || groupnoorelement, 'TABLE')
			if (tablex)
				groupno = tablex.getAttribute('exogroupno')
		}
	}

	var rows = gds.data['group' + groupno]
	if (!rows) {
		systemerror('deleterows()', 'group number ' + groupnoorelement + ' doesnt exist')
	}

	for (var rownn = rowns.length - 1; rownn >= 0; --rownn) {
		var rown = rowns[rownn]
		//if deleting first row then a blank row (to clear formatting)
		if (rown == 0)
			gds.insertrow(groupno, 0)
		//never actually delete first row
		gds.deleterow(groupno, (rown == 0 ? 1 : rown))
	}

	//logout('deleterows')

}

async function deleteallrows(groupnoorelement, fromrecn) {

	//login('deleteallrows')

	var groupno = groupnoorelement
	if (typeof groupno == 'object') {
		groupno = groupnoorelement.getAttribute && groupnoorelement.getAttribute('exogroupno')
		if (!groupno && groupnoorelement[0] && groupnoorelement[0].getAttribute)
			groupno = groupnoorelement[0].getAttribute('exogroupno')
		if (!groupno) {
			var tablex = getancestor(groupnoorelement[0] || groupnoorelement, 'TABLE')
			if (tablex)
				groupno = tablex.getAttribute('exogroupno')
		}
	}

	if (!fromrecn)
		fromrecn = 0

	if (!gds.data['group' + groupno]) {
		systemerror('await deleteallrows()', 'group number ' + groupnoorelement + ' doesnt exist')
	}

	//insert a blank row (to clear any formatting)
	//if (!exceptfirst)
	gds.insertrow(groupno, fromrecn)

	//delete all rows but the newly inserted first row
	//var recs=gds.data['group'+groupno]
	while (gds.data['group' + groupno].length > (1 + fromrecn)) {
		//delete the last row
		gds.deleterow(groupno, gds.data['group' + groupno].length - 1)
	}

	//logout('deleteallrows')

}

async function insertallrows(elements, values, fromrecn) {

	//preserve the basics while insertallrows2 is called
	var save_gpreviouselement = gpreviouselement
	var save_gvalue = gvalue
	var save_gpreviousvalue = gpreviousvalue
	var save_grecn = grecn

	// Defer calcfields until all rows validated (or mid-fail): one mass regetx/xlate
	calcfields_suspend()
	var result
	try {
		result = await insertallrows2(elements, values, fromrecn)
	} finally {
		await calcfields_resume()
	}

	//if (gdataset.split('_')[0] == 'gravity' || gdataset.split('_')[1] == 'test') {
		gpreviouselement = save_gpreviouselement
		gvalue = save_gvalue
		gpreviousvalue = save_gpreviousvalue
		if (!save_grecn)
			grecn = save_grecn
	//}

	return result
}

async function insertallrows2(elements, values, fromrecn) {

	//login('insertallrows')

	//get first element if an array passed
	//otherwise convert elements to an array
	//var element = elements
	if (!elements.tagName && elements.length && elements[0].tagName) {
		//element = element[0]
	}
	else
		elements = [elements]
	var element = elements[0]

	if (!fromrecn)
		fromrecn = 0

	assertelement(element, 'insertallrows', 'element')

	var groupno = Number(element.getAttribute('exogroupno'))

	//if (elements.length == 1)
	//await deleteallrows(element, fromrecn)
	await deleteallrows(element, fromrecn + 1)

	//get the group
	var rows = gds.data['group' + groupno]

	//cater for passing one row/one cell value not in array form
	if (typeof values != 'object')
		values = [values]

	for (var rown = 0; rown < values.length; rown++) {

		//insert a new row (except for the last)
		if (rown > 0 && rown < (values.length - 1))
			gds.insertrow(groupno, rown + fromrecn)

		var rowvalues = values[rown]
		if (typeof rowvalues != 'object')
			rowvalues = [rowvalues]

		//set the record and form values
		for (var coln = 0; coln < elements.length && coln < rowvalues.length; coln++) {
			//await gds.setx(elements[coln], rown + fromrecn, rowvalues[coln])

			var element = elements[coln]
			var oldvalue = await gds.get1(element, rown + fromrecn)
			var newvalue = rowvalues[coln]

			//set and call validation to trigger any consequences
			if (newvalue != oldvalue) {
				grecn = rown + fromrecn
				gpreviousvalue = oldvalue
				//gpreviouselement = document.getElementsByName(element.id)[grecn]
				//if (!gpreviouselement)
				//  gpreviouselement = element
				if (element.id) {
					gpreviouselement = $$(element.id)
					if (gpreviouselement[grecn])
						gpreviouselement = gpreviouselement[grecn]
				} 
				//await gds.setx(element, grecn, newvalue)
				var ovalue = await validateoconv(gpreviouselement, newvalue)
				// validateoconv returns false on fail (not only null/undefined)
				if (ovalue === false || ovalue == null || typeof ovalue == 'undefined')
					return false
				//const conversion = gpreviouselement.getAttribute('exoconversion')
				//if (conversion && conversion.substr(0,1) == '[')
				//    newvalue = newvalue.exooconv(conversion)
				setvalue(gpreviouselement,ovalue)
				if ((!gKeyNodes || glocked) && !(await validateupdate()))
					return false
				gpreviousvalue = newvalue
			}
		}

	}

	//no because often done programmatically which requires no specific work by user
	//popup etc may set it specifically
	//settouched(true)

	//recalculate any dependents
	var dependentfieldnos = ''
	for (var ii = 0; ii < elements.length; ii++) {
		var deps = elements[ii].getAttribute('exo_dependents')
		if (deps)
			dependentfieldnos += ';' + deps
	}
	dependentfieldnos = dependentfieldnos.slice(1).split(';')
	grecn = null

	await calcfields(dependentfieldnos)

	//logout('insertallrows')

}

function exoaddrow(groupno) {
	var tablex = $$('exogroup' + groupno)
	if (!tablex || tablex.getAttribute('noinsertrow'))
		return false
	gds.addrow(groupno)
}

async function readonlydocmsg() {

	//restore any previous value
	if (gpreviouselement && gvalue != gpreviousvalue)
		setvalue(gpreviouselement, gpreviousvalue)

	//readonly after key has been entered
	if (gKeyNodes || gkey)
		return await exoui_invalid('This document is currently "read only"')

	//readonly before key has been entered
	else if (!gkey)
		return await exoui_invalid('Please open a document first')
}

// True if any bound F field on this group data row has non-blank text.
// Same cell.text test as validateall() empty-row detection.
function form_group_row_has_data(datarow) {

	if (!datarow)
		return false
	for (var propname in datarow) {
		var cell = datarow[propname]
		var element = cell && cell.element
		if (!element || element.getAttribute('exotype') != 'F')
			continue
		if (cell.text && (typeof cell.text != 'string' || cell.text.replace(/ *$/, '')))
			return true
	}
	return false
}

//'''''''''''
//'INSERT ROW
//'''''''''''
async function form_insertrow(event, append) {

	event = getevent(event)
	if (event.target.tagName == 'BODY')
		return

	//make sure any previous stuff is validatedupdated
	if (!(await validateupdate(event)))
		return false

	//login('insertrow')

	//setdisabledandhidden(saverecord,false)
	//setdisabledandhidden(editreleaserecord,false)

	var row = getancestor(event.target, 'TR')
	if (!row)
		systemerror('insertrow', event.target.id + ' has no tr ancestor')

	var tablex = getancestor(row, 'TABLE')
	if (!tablex || tablex.getAttribute('noinsertrow'))
		return false

	//var tbodies = tablex.tBodies
	var groupno = Number(tablex.getAttribute('exogroupno'))
	if (groupno == 0)
		return false

	var rown = row.rowIndex
	if (tablex.tHead) rown -= tablex.tHead.rows.length
	if (append)
		rown++
	grecn = rown

	grows = tablex.tBodies[0].getElementsByTagName('tr')
	var nrows = grows.length

	//first and only row is considered to be clicking on last row, ie appends
	//but how would we insert a line before line 1?
	//if (nrows==1)
	//     append=true

	// Before/After: ask on single, penultimate, or last row of many — but only if
	// the current line has data. Blank line → always Before (no prompt).
	if (typeof append == 'undefined') {
		var askbeforeafter = (nrows == 1 || rown == nrows - 1 || rown == nrows - 2)
		if (askbeforeafter) {
			var grouprows = gds.data['group' + groupno]
			var rowdata = grouprows && grouprows[rown]
			if (form_group_row_has_data(rowdata)) {
				// default_icons false: Before/After are alternatives, not Yes/No
				var choice = await exoui_confirm('Insert row before or after?', 1, 'Before', 'After', 'Cancel', null, null, null, false)
				if (!choice)
					return false
				if (choice == 2)
					grecn++//AFTER/BELOW
			}
			// empty: grecn unchanged = Before
		}
	}

	// Following row(s) hidden → expand instead of insert, except:
	//   • dblclick value filter (table.exo_filter_colid): always insert
	//     (c137c49b — filter hide is not fold; insert after filter)
	//   • fold-on-open [+] (exo_expand) while Show All up: expand those
	//   • not filtered: expand (indent/legacy hide)
	var valueFilter = !!(tablex && tablex.exo_filter_colid)
	var insertbtn = grows[grecn] && grows[grecn].exofields
		&& grows[grecn].exofields['insertrowbutton' + groupno]
	var expandAffordance = insertbtn && insertbtn.getAttribute('exo_expand')
	if (grecn < (nrows - 1) && grows[grecn + 1].style.display == 'none'
		&& !valueFilter
		&& (expandAffordance || !form_group_is_filtered(tablex, groupno))) {

		//return to insertrow image
		setinsertimage('insert', grows[grecn], groupno)

		for (rown = grecn + 1; rown < nrows; rown++) {
			if (grows[rown].style.display !== 'none') return true
			//grows[rown].style.display=''
			grows[rown].style.display = ''
		}
		return true
	}

	//cannot update if (locked
	if (gKeyNodes && !glocked) {
		await readonlydocmsg()
		return false //logout('insertrow')
	}

	//form specific before row insert function
	var preinsertrow = window['form_preinsertrow' + groupno]
	if (typeof preinsertrow == 'function') {
		var p = preinsertrow(event);
		if (p && typeof p.then === 'function') {
			p = await p;
		} else if (p && typeof p.next === 'function') {
			systemerror('insertrow', 'form_preinsertrow must be async (generators removed stage 6)')
			return false
		}
		if (!p)
			return false //logout('insertrow - preinsert false')
	}

	var id
	if (event.target.name)
		id = event.target.id
	else
		id = gfields[gtables[groupno][0]].id

	if (append) {
		gds.addrow(groupno)
	}
	else {
		gds.insertrow(groupno, grecn)
		//inserting a row sets gtouched, but appending does not
		settouched(true)
	}

	//get new row again
	var row = tablex.tBodies[0].getElementsByTagName('tr')[grecn]

	//ensure not copied a row with expand marker
	setinsertimage('insert', row, groupno)

	//form specific after row insert function
	var postinsertrow = window['form_postinsertrow' + groupno]
	if (typeof postinsertrow == 'function') {
		var p = postinsertrow(event);
		if (p && typeof p.then === 'function') {
			await p;
		} else if (p && typeof p.next === 'function') {
			systemerror('insertrow', 'form_postinsertrow must be async (generators removed stage 6)')
		}
	}

	//focus on first input column of new row (after postinsert — may rebind rows)
	if (!append) {
		// re-get row; postinsert (e.g. schedule_copy) may replace DOM
		row = tablex.tBodies[0].getElementsByTagName('tr')[grecn]
		var firstsfn = form_getfirstinputcolscreenfn(tablex)
		if (firstsfn != null && firstsfn !== '' && typeof firstsfn != 'undefined' && gfields[firstsfn])
			id = gfields[firstsfn].id
		var focusel = null
		if (row) {
			if (row.exofields && row.exofields[id])
				focusel = row.exofields[id]
			else if (document.getElementsByClassName)
				focusel = row.getElementsByClassName('exoid_' + id)[0]
			else if (row.all)
				focusel = row.all[id]
		}
		if (focusel)
			focuson(focusel)
	}

	//logout('insertrow')

	return true

}

function setinsertimage(mode, row, groupno) {

	//return to insertrow graphic zzz hardcoded should be located
	//var insertimage=row.childNodes[0].childNodes[0].childNodes[0]
	//var insertimage=row.childNodes[0].exofields['insertrowbutton'+groupno]
	var insertimage = row.exofields
	if (!insertimage)
		return
	insertimage = insertimage['insertrowbutton' + groupno]
	if (!insertimage)
		return

	if (mode == 'expand') {
		insertimage = exo_set_icon_element(insertimage, gexpandrowimage)
		insertimage.setAttribute('exo_expand', '1')
		//duplicate keycodes in 3 places
		insertimage.title = 'Expand hidden rows here (Ctrl+I or Ctrl+Insert)'
	}
	else {
		insertimage = exo_set_icon_element(insertimage, ginsertrowimage)
		insertimage.removeAttribute('exo_expand')
		//duplicate keycodes in 3 places
		insertimage.title = 'Insert a new row here (Ctrl+I or Ctrl+Insert)'
	}
	return
}

// True while row filter is active (dblclick state and/or Show All visible).
function form_group_is_filtered(tablex, groupno) {
	if (tablex && tablex.exo_filter_colid)
		return true
	// Prefer the bound multivalue table (exogroupN), not a nested TABLE ancestor.
	var gtable = (typeof groupno != 'undefined' && groupno !== '' && groupno != null)
		? document.getElementById('exogroup' + groupno)
		: null
	if (gtable && gtable.exo_filter_colid)
		return true
	var showall = document.getElementById('exogroup' + groupno + 'showall')
	if (!showall)
		return false
	// style.display '' means shown (default); only 'none' is hidden.
	return showall.style.display != 'none'
}

async function openrecord_onclick() {

	//login('openrecord')

	if (!(await validateupdate()))
		return false

	var reply
	//get a reply or return false
	if ((reply = await exoui_popup2(openrecord)) == null)
		return false //logout('openrecord')

	//forget it if no change
	if (reply == (await getkey()))
		return false //logout('openrecord no change')

	//do not change key if user chooses not to unload an existing document
	if (!(await closedoc('OPEN'))) {
		//logout('openrecord user cancelled')
		return false
	}

	await opendoc(reply)

	//logout('openrecord')

	return true

}

async function firstrecord_onclick(event) {
	return await nextrecord2(event, 'first')
}

async function previousrecord_onclick(event) {
	return await nextrecord2(event, -1)
}

async function selectrecord_onclick(event) {
	return await nextrecord2(event, 0)
}

async function nextrecord_onclick(event) {
	return await nextrecord2(event, 1)
}

async function lastrecord_onclick(event) {
	return await nextrecord2(event, 'last')
}

async function recordnav_wait_for_db() {

	// Defensive: wait if main db is still mid-request (should be rare under Gate A).
	while (db.requesting)
		await new Promise(function (resolve) { window.setTimeout(resolve, 25) })
}

async function nextrecord2(event, direction) {

	// direction is 'first', -1, 0, 1, 'last'
	// Gate A serializes entry (second key/click while airborne is cancelled, not merged).
	// Former grecordnav_busy / pending-merge was a pre-gate concurrent-entry patch.
	await recordnav_wait_for_db()
	return await nextrecord2_step(event, direction)
}

async function nextrecord2_step(event, direction) {

	var nextkeys = gkeys
	var nextkeyn = gkeyn

	//goto one of many ... or if select many then reduce list to those selected
	if (direction == 0) {
		var selkeys = await exoui_decide('', gkeys, [[0, 'Key']], 0, '', many = true)
		if (!selkeys) return false
		if (selkeys.length > 1) {
			nextkeys = selkeys
			nextkeyn = 1
		}
		else {
			nextkeyn = nextkeys.exolocate(selkeys[0]) - 1
		}
	}
	else {

		if (!direction) direction = 1

		//get the next key
		if (direction == 'first') nextkeyn = 0
		else if (direction == 'last') nextkeyn = nextkeys.length - 1
		else {
			nextkeyn += direction
			if (nextkeyn >= nextkeys.length) nextkeyn = 0
			if (nextkeyn < 0) nextkeyn = nextkeys.length - 1
		}
	}

	var nextkey = nextkeys[nextkeyn]

	//switch to new key if new and user accepts to close the current one
	if (nextkey != gkey) {
		// Nav-only: defer formbutton display across close/open awaits; flush always
		// (cancel close, throw, or success) so hide+show paint once — no empty bar mid-DB.
		g_formbuttons_defer_hide = true
		try {
			//do not change key if user chooses not to unload an existing document
			if (!(await closedoc('OPEN')))
				return false
			await setgkeyn(nextkeyn)
			await opendoc(nextkey)
		} finally {
			formbuttons_flush_pending_hidden()
		}
	}

	await setgkeys(nextkeys, nextkeyn)

	return true

}

async function exoui_link(event, element) {

	event = getevent(event)

	//login('exoui_link')

	exocancelevent(event)

	// Keyboard: focused host. Click: icon has data-exo-host.
	if (!element)
		element = event.target
	if (!element.getAttribute || !element.getAttribute('exolink'))
		element = form_field_from_chrome_icon(element)

	//quit if no link defined
	if (!element || !element.getAttribute('exolink'))
		return false //logout('exoui_link - no link')

	//prevent popups except on the key field unless a record is present
	if (gKeyNodes && !gloaded && element.getAttribute('exofieldno') != 0) {
		focuson(gKeyNodes[0])
		return false //logout('exoui_link - no record')
	}

	grecn = getrecn(element)

	gvalue = getvalue(element, grecn)
	if (!(element.getAttribute('exolowercase')))
		gvalue = gvalue.toUpperCase()

	var reply = await exoevaluate(element.getAttribute('exolink'), 'await exoui_link()');

	//logout('exoui_link')

	return

}

function exofieldpopupallowed(element) {

	if (!element || !element.getAttribute)
		return false

	if (element.getAttribute('exoreadonly'))
		return false

	if (element.disabled || element.getAttribute('disabled'))
		return false

	// skip fields made non-tabbable (incl. exosetreadonly)
	if (element.tabIndex === -1)
		return false

	return true
}

async function exoui_popup(event, element) {

	event = getevent(event)

	//element is only provided from f7/alt+down keyboard events

	//login('exoui_popup')

	exocancelevent(event)

	// Keyboard: focused host/SELECT. Click: icon has data-exo-host.
	if (!element) {
		element = event.target
		if ((!element.getAttribute || !element.getAttribute('exopopup'))
			&& element.tagName != 'SELECT')
			element = form_field_from_chrome_icon(element)
	}

	//quit if no element
	if (!element)
		return false //logout('exoui_popup - no element')

	//log('check no missing data in group 0 always')
	if (!(await checkrequired(gfields, element, 0)))
		return false //logout('exoui_popup' + ' ' + element.id + ' a prior element is visible and required but is blank (1)')

	//validateupdate previous field
	//if (element.id!=gpreviouselement.id)
	if (element != gpreviouselement) {
		if (!(await validateupdate()))
			return false //logout('exoui_popup - validateupdate failed')
	}

	//cannot update anything but key field if not locked or save button not enabled
	if (element.getAttribute('exofieldno') != 0 && gKeyNodes && (!glocked || saverecord.getAttribute('disabled'))) {
		await readonlydocmsg()
		return false //logout('exoui_popup - read only document')
	}

	// quit if field is not editable (readonly, disabled, or non-tabbable)
	if (!exofieldpopupallowed(element)) {
		var readonly = element.getAttribute('exoreadonly')
		if (readonly && readonly != 'true')
			await exoui_invalid(readonly)
		return false //logout('exoui_popup - read only')
	}

	//quit if no popup defined
	if (!element.getAttribute('exopopup') && element.tagName != 'SELECT')
		return false //logout('exoui_popup - no popup')

	//prevent popups except on the key field unless a record is present
	if (gKeyNodes && !gloaded && element.getAttribute('exofieldno') != 0) {
		focuson(gKeyNodes[0])
		return false //logout('exoui_popup - no record')
	}

	grecn = getrecn(element)
	gvalue = getvalue(element)
	//gpreviouselement = element
	//gpreviousvalue = gvalue
	setgpreviouselement(element, gvalue)

	//get a reply or return false
	/////////////////////////////
	if ((reply = await exoui_popup2(element)) == null) {
		//    alert('xxx')
		focuson(element)
		return false //logout('exoui_popup - no reply')
	}

	//update the element multiple selections qqq
	//if reply is an array use replace all rows
	if (reply.length > 1
		&& Number(element.getAttribute('exogroupno'))
		&& typeof (reply) == 'object') {
		//var elementid = element.id

		var elementid = element.id

		//deselect items already selected
		if (grecn > 0) {
			var prevalues = ((await gds.getall(elementid))).slice(0, grecn)
			for (var ii = 0; ii < prevalues.length; ++ii) {
				var replyn
				if (replyn = reply.exolocate(prevalues[ii]))
					reply.splice(replyn - 1, 1)
			}
			if (!reply.length)
				return false //logout('exoui_popup - no new selections')
		}

		await insertallrows(element, reply, grecn)
		settouched(true)//should this be done in insertallrows to ensure Save button is enabled in other cases too?

		//focus on next element AFTER table
		element = $$(elementid)
		if (element && element[0])
			element = element[0]
		focusdirection(1, element, Number(element.getAttribute('exogroupno')))

		//logout('exoui_popup - insert all rows')
		return true

	}

	//if reply is still an array use the first one
	if (typeof (reply) == 'object')
		reply = reply[0]

	//forget it if no change (changed to allow early update on popup eg pop_target)
	//if (reply==getvalue(element))
	if (reply == gpreviousvalue) {
		focusnext()
		return false //logout('exoui_popup - no change')
	}

	//do not change key if user chooses not to unload an existing document
	if (element.getAttribute('exofieldno') == 0 && gloaded && !(await closedoc('OPEN')))
		return false //logout('exoui_popup - user cancelled unloaddoc')

	//output convert it
	if (element.getAttribute('exoconversion')) {
		reply = await validateoconv(element, reply)
		// validateoconv returns false on fail (not only null/undefined)
		if (reply === false || reply == null || typeof reply == 'undefined')
			return false //logout('exoui_popup - oconv failed')
	}

	//setup next onfocus to validateupdate
	//done before calling popup now
	//gpreviouselement=element
	//gpreviousvalue=getvalue(element)

	//update the screen
	setvalue(element, reply)

	//move focus to next field (should trigger validation etc)
	//have to force validation since the focus may already be on the next field
	//if clicked on the popup
	gvalidatingpopup = true
	if (await validateupdate())
		focusnext(element)
	gvalidatingpopup = false
	//exosettimeout('focusnext()',10)
	//await validateupdate()

	//logout('exoui_popup')

}

async function exoui_popup2(element) {

	//given a SELECT item or element with a popupfunction to evaluate, returns a reply or null

	//quit if no popup defined and not SELECT
	var expression = element.getAttribute('exopopup')
	if (!expression && element.tagName != 'SELECT')
		return false

	//evaluate popup expression if provided
	if (expression) {
		var reply = await exoevaluate(expression, 'await exoui_popup2()');
	}

	//otherwise build a list and select from the SELECT
	else {

		//build a popup from the SELECT tag
		var selectvalues = []
		for (var ii = 0; ii < element.childNodes.length; ii++) {
			var option = element.childNodes[ii]
			var optionvalue = option.value
			if (!optionvalue)
				optionvalue = ''
			var optiontext = option.innerText
			if (!optiontext)
				optiontext = ''
			selectvalues[selectvalues.length] = [optionvalue, optiontext]
		}

		//allow multiple selection if element is in a group with only one input column
		var multipleselection = false
		var elgroupno = Number(element.getAttribute('exogroupno'))
		if (elgroupno) {
			var tableelement = gtables[elgroupno].tableelement
			var multipleselection = tableelement && tableelement.getAttribute('exo_lastinputcolscreenfn') == tableelement.getAttribute('exo_firstinputcolscreenfn')
		}

		//get the response(s)
		reply = await exoui_decide2('', selectvalues, '1', '', '', multipleselection)

		//have to do this to cancel the standard dropdown if they press alt+down then press escape on the popup
		if (!reply)
			focuson(element)

		//convert an array of row number(s) to an array of value(s)
		for (var ii = 0; ii < reply.length; ii++) {
			reply[ii] = selectvalues[reply[ii] - 1][0]
		}

		//remove empty values
		reply = reply.exotrim('')

		//minimum one value
		if (!reply.length)
			reply = ['']

	}

	//undefined, null or '' all return as null
	if
		(typeof (reply) == 'undefined'
		|| (typeof (reply) == 'boolean' && reply == false)
		|| reply == null
		|| (typeof (reply) == 'string' && reply == '')
	) {
		reply = null

		//if popup returns nothing but gdependents is set then the popup must have updated
		//something itself so recalc dependents
		if (gdependents.length) {
			await calcfields(gdependents)
		}
	}

	//if sole key field or the open function, setup a list of keys
	if (reply && ((openrecord && element == openrecord) || (element.getAttribute('exofieldno') == 0 && gKeyNodes.length == 1))) {
		if (typeof reply == 'object') {

			if (reply.length > 1 && reply.length <= 50) {
				// Always One for now (skip One/Many confirm). Restore prompt to re-enable Many.
				var openall = 1
				// default_icons false: One/Many are alternatives, not Yes/No
				//openall = await exoui_confirm('Open all in one tab?', 1, 'One', 'Many', '', null, null, null, false)
				//if (!openall)
				//    return false
				if (openall == 2) {
					//open 2nd and subsequent keys in tabs
					for (var keyn = 1; keyn <= reply.length; ++keyn) {
						var key = reply[keyn]
						if (key)
							//doesnt work when multiple .. due to using cookies to communicate? perhaps chain opening passing remaining keys to each window
							await windowopen(window.location.href.toString().split('?')[0] + '?key=' + key)
					}
					//window.location.assign(window.location.href.toString().split('?')[0] + '?key=' + reply[0])
					reply = [reply[0]]//return the first key only
				}
			}

			await setgkeys(reply, 0)
			reply = reply[0]
		}
		else {
			await setgkeys([reply], 0)
		}
	}

	return reply

}

async function setgkeys(keys, keyn) {
	if (typeof keys == 'string') {
		keys = [keys]
	}
	gkeys = keys
	// Respect g_formbuttons_defer_hide (pending_hidden) — same seam as setdisabledandhidden
	var navhidden = gkeys.length <= 1
	formbuttons_apply_display(firstrecord, navhidden)
	formbuttons_apply_display(previousrecord, navhidden)
	formbuttons_apply_display(selectrecord, navhidden)
	formbuttons_apply_display(nextrecord, navhidden)
	formbuttons_apply_display(lastrecord, navhidden)
	if (typeof keyn == 'undefined') {
		for (keyn = 0; keyn < gkeys.length; ++keyn) {
			if (keys[keyn] == gkey) {
				break
			}
		}
	}
	if (keyn < gkeys.length) {
		await setgkeyn(keyn)
	}
}

async function setgkeyn(keyn) {
	gkeyn = keyn
	firstrecord.disabled = gkeyn == 0 ? true : false
	// Update label only — CSS ::before strut matches nav icon height for v-center.
	var navtext = (gkeyn + 1) + ' of ' + gkeys.length
	var navlabel = $$('selectrecordbutton_label')
	if (navlabel)
		navlabel.innerHTML = navtext
	else if (selectrecord)
		selectrecord.innerHTML = navtext
	lastrecord.disabled = (gkeyn == (gkeys.length - 1)) ? true : false
}

async function getkeyexternal() {
	return await getkey('oconv')
}

async function debug(v) {
	if (!(confirm(v))) {
		var _b = exobreak();
		if (exoisasyncfunction(_b))
			await exo_begin(_b, 'break')
		else if (_b && typeof _b.next === 'function')
			systemerror('debug', 'function* break handler removed (stage 6)')
	}
}

async function getkey(mode) {

	//login('getkey ' + mode)

	//returns a string containing the key or '' if any key parts are required and ''
	var key = new Array(gKeyNodes.length)
	for (var ii = 0; ii < gKeyNodes.length; ii++) {
		var temp = getvalue(gKeyNodes[ii])
		if (temp == '' && gKeyNodes[ii].getAttribute('exorequired') != false) {
			//logout('getkey ""')
			return ''
		}
		if (mode != 'oconv') {
			var conversion = gKeyNodes[ii].getAttribute('exoconversion')
			if (conversion.slice(0, 1) == '[') temp = exoiconv(temp, conversion)
		}
		key[gKeyNodes[ii].getAttribute('exokeypart') - 1] = temp
	}

	var key
	if (mode != 'oconv') {
		key = key.join('*')
	}
	else {
		key = key.join(' ')
	}
	//await debug(key)

	//logout('getkey ' + key)

	return key

}

async function setkeyvalues(key) {
	//given a string updates the keyfield(s)
	if (key == null || key === undefined)
		key = ''
	else if (typeof key != 'string') {
		if (typeof key.join == 'function')
			key = key.join('*')
		else
			key = String(key)
	}
	for (var ii = 0; ii < gKeyNodes.length; ii++) {
		var temp = key.exofield('*', Number(gKeyNodes[ii].getAttribute('exokeypart')))
		//var conversion=gKeyNodes[ii].getAttribute('exoconversion')
		//if (conversion.slice(0,1)=='[') temp=temp.exooconv(conversion)
		//setvalue(gKeyNodes[ii],temp)
		await gds.setx(gKeyNodes[ii].id, '', temp)
	}
}

function mergeattributes(sourceelement, targetelement) {

	//does not work on mac despite docs
	//targetelement.mergeAttributes(sourceelement)

	//targetelement.id=sourceelement.id
	////targetelement.name=sourceelement.name

	//merges all strin and number attributes except id, outerText and outerHTML
	for (var attribname in sourceelement) {
		var attribute = sourceelement[attribname]
		//alert(attribname)
		try {
			//if (attribname!='id'&&attribname!='name'&&attribname!='outerText'&&attribname!='outerHTML'&&(typeof(attribute)=='string'||typeof(attribute)=='number'))
			if (attribname != 'innerText' && attribname != 'innerHTML' && attribname != 'outerText' && attribname != 'outerHTML' && (typeof (attribute) == 'string' || typeof (attribute) == 'number' || attribute == true || attribute == false)) {
				//    alert(attribname+':'+attribute)
				targetelement.setAttribute(attribname, attribute)
				//    targetelement[attribname]=attribute
			}
		}
		catch (e) {
		}
	}

}

function getrecn(element) {

	if (typeof element == 'undefined') {
		//global variable set in any getevent(event)
		element = gevent.target
	}
	if (typeof element != 'object') {
		systemerror('getrecn', element[0].id + ' ' + element[0].tagName + ' ' + element[0].innerHTML + ' is not an object')
	}

	exoassertobject(element, 'getrecn', 'element')
	//returns null if element not part of a table

	var recn
	if (element.getAttribute && !(Number(element.getAttribute('exogroupno')))) {
		recn = null
	}
	else {
		var rowx = getancestor(element, 'TR')
		if (!rowx) {
			return null
		}
		var tablex = getancestor(rowx, 'TABLE')
		recn = rowx.rowIndex
		if (tablex && tablex.tHead) {
			recn -= tablex.tHead.rows.length
		}
	}

	//partially solve a bug that deleteallrows deletes our row and leaves us unable to find recn from its rowIndex
	//since rowIndex of a deleted row (removed from DOM) is -1
	if (recn < 0) {
		//var msg='getrecn'+recn
		//if (rowx)
		//    msg+=' rowx.rowIndex'+rowx.rowIndex
		//    msg+=' ' + row.innerHTML
		//if (tablex)
		//    msg+=' tablex.tHead.rows.length'+tablex.tHead.rows.length
		//alert(msg)
		recn = 0
	}

	return recn

}

// getpreviousrow / getnextrow / getrowx
// ------------------------------------
// Return value from another multivalue row (dictid, or gpreviouselement.id if '').
//
//   opts.skipblanks — if true, walk past empty cells
//   opts.internal   — false/omitted → getvalue / getvalues (DOM, external / paint)
//                     true          → gds.get1 / gds.getall (getx — storage form)
//                     NOT getvalue+ICONV. Two layers: DOM vs gds.
//
// Default is DOM external. Use { internal: true } for defaultvalue/setdefault
// (storage then OCONV for paint), gds compares, file keys, full ACCOUNT_NO, etc.
// Alt+Y / screen copy: omit internal (DOM form).
//
//   getpreviousrow(id, { skipblanks: true, internal: true })

async function getnextrow(dictid, opts) {
	return await getrowx(dictid, true, opts)
}

async function getpreviousrow(dictid, opts) {
	return await getrowx(dictid, false, opts)
}

async function getrowx(dictid, forward, opts) {

	// See API note above getnextrow/getpreviousrow.
	// forward true = next row, false = previous. grecn is current row.
	if (!opts)
		opts = {}
	var skipblanks = opts.skipblanks
	var internal = opts.internal

	if (!dictid)
		dictid = gpreviouselement.id
	var value = ''

	//quick previous search
	if (!forward) {
		if (grecn == 0)
			return ''
		if (internal)
			// gds storage (get1 → getx)
			value = await gds.get1(dictid, grecn - 1)
		else
			value = getvalue(dictid, grecn - 1)
		if (value)
			return value
		if (!skipblanks)
			return ''
	}

	//more comprehensive search
	// var values=getvalue(dictid, null)
	//var values=getvalues(dictid)
	var values
	if (internal)
		// gds storage (getall → getx)
		values = await gds.getall(dictid)
	else
		//values = getvalues(document.getElementsByName(dictid))
		values = getvalues(dictid)

	//return undefined if no next row
	if (forward) {
		var ln = grecn + 1
		var ln2 = skipblanks ? values.length : ln + 1
		for (ln = ln; ln <= ln2; ln++) {
			if ((value = values[ln]) != '')
				return value
		}
	}
	else {
		var ln = grecn - 1
		var ln2 = skipblanks ? 0 : ln
		for (ln = ln; ln >= ln2 && ln >= 0; ln--) {
			if ((value = values[ln]) != '')
				return value
		}
	}
	return ''

}

async function form_ondblclick(event) {
	event = getevent(event)
	return await form_filter('filter')
}

async function form_onrightclick(event) {
	event = getevent(event)
	return true
}

// Lead-in col 0 cells: class "exogroup_col0 exogroupN_col0". Residual hide is
// table.exogroup_col0_hide (CSS) — one class toggles thead+tbody+tfoot together.
function form_group_col0_class(groupno) {
	return 'exogroup_col0 exogroup' + groupno + '_col0'
}

function form_group_tag_col0(tablex, groupno) {
	if (!tablex)
		return
	var col0class = form_group_col0_class(groupno)
	function tag(cell) {
		if (!cell || cell.querySelector('[exoname], [exotype], input[name]'))
			return
		if ((' ' + (cell.className || '') + ' ').indexOf(' exogroup_col0 ') >= 0)
			return
		cell.className = (cell.className ? cell.className + ' ' : '') + col0class
	}
	if (tablex.tBodies && tablex.tBodies[0]) {
		var brows = tablex.tBodies[0].rows
		for (var bri = 0; bri < brows.length; bri++)
			tag(brows[bri].cells[0])
	}
	tag(tablex.tHead && tablex.tHead.rows[0] && tablex.tHead.rows[0].cells[0])
	tag(tablex.tFoot && tablex.tFoot.rows[0] && tablex.tFoot.rows[0].cells[0])
}

// Residual lead-in only (no ins/del). Show All must reveal all col0 cells at once.
function form_group_leadin_display(tablex, show) {
	if (!tablex)
		return
	if (!(tablex.getAttribute('noinsertrow') && tablex.getAttribute('nodeleterow')))
		return
	if (tablex.querySelector('[id^="insertrowbutton"], [id^="deleterowbutton"]'))
		return
	if (show)
		tablex.classList.remove('exogroup_col0_hide')
	else
		tablex.classList.add('exogroup_col0_hide')
}

// form_filter: hide multivalue rows (dblclick value, regexp, maxrecn) + Show All.
// No type-in filter box — that was schedules-only (gallowfilter), incomplete, and
// carried ad-hoc onfocus/onblur + VEHICLE_CODE hardcode.
async function form_filter(mode, colidorgroupno, regexp, maxrecn, elem) {

	//NB regexp to be filtered OUT not IN

	if (mode == 'refilter') {
		colidorgroupno2 = colidorgroupno
		await form_filter('unfilter', colidorgroupno2)
		mode = 'filter'
	}

	//get colid
	var colid = colidorgroupno
	if (!colid) {
		colid = gevent.target.id
		if (!colid) {
			var element = gevent.target.parentNode
			while (element) {
				colid = element.name
				if (colid)
					break
				element = element.parentNode
			}
		}
	}

	//get dictitem
	//if (mode == 'filter' || mode == 'unfilter') {
	//}

	//get groupno
	var groupno
	if (typeof colidorgroupno != 'number' && (mode == 'filter' || mode == 'unfilter')) {
		var dictitem = gds.dictitem(colid)
		//if (!dictitem)
		// return await exoui_invalid(colid+' dictitem does not exist in await form_filter()')
		groupno = dictitem.groupno
		if (!groupno)
			return await exoui_invalid()//colid+' is not multivalued for sorting'
	}
	else {
		groupno = colidorgroupno
	}

	//ensure the function name is recognised by the yield converter
	//async function prefilter(){}

	//prefilter
	if (typeof form_prefilter == 'function') {
		if (!(form_prefilter(mode, colid)))
			return await exoui_invalid()
	}

	//get the table rows
	var tablex = $$('exogroup' + groupno)
	if (!tablex) {
		// systemerror (not exoui_invalid): full stack — usually table never bound (exogroupno / id)
		systemerror('form_filter', String(colid) + ' is not part of a table'
			+ ' (mode=' + mode + ' groupno=' + groupno + ' id=exogroup' + groupno + ')')
		return false
	}
	grows = tablex.tBodies[0].rows

	//unfilter and exit (not tested or used anywhere)
	if (mode == 'expand') {
		for (var rown = grecn + 1; rown < grows.length; rown++) {
			if (grows[rown].style.display != '')
				break
			//grows[rown].style.display=''
			grows[rown].style.display = ''
		}
		await calcfields()
		return true
	}

	//get the show all button
	var tablexshowall = $$('exogroup' + groupno + 'showall')
	if (!tablexshowall) {
		//syserror('await form_filter()','Cannot find showall button, are you missing a thead?')
		return true
	}

	//unfilter and exit
	if (mode == 'unfilter') {
		for (var rown = 0; rown < grows.length; rown++) {
			//change to expandrow image
			setinsertimage('insert', grows[rown], groupno)

			//grows[rown].style.display=''
			grows[rown].style.display = ''
		}
		//hide the show all buttons (can be two - in THEAD and TFOOT)
		//tablexshowall.style.display='none'
		showhide('exogroup' + groupno + 'showall', false)
		// residual lead-in (no ins/del): re-hide thead/tbody/tfoot together
		form_group_leadin_display(tablex, false)
		// clear dblclick filter snapshot on the ruling table
		tablex.exo_filter_colid = ''
		tablex.exo_filter_value = ''
		await calcfields()
		return true
	}

	//window.status = 'Filtering, please wait ...'
	console.log('Filtering, please wait ...');

	//get the value and values to be filtered
	var value
	var values
	values = await gds.regetx(colid, null)
	if (!regexp && !maxrecn) {
		grecn = getrecn()
		var value = values[grecn]
	}

	// Dblclick same col + same value again → Show All (unfilter).
	// State lives on the multivalue TABLE (exogroupN).
	if (mode == 'filter' && !regexp && !maxrecn) {
		var prevCol = tablex.exo_filter_colid
		var prevVal = tablex.exo_filter_value
		if (prevCol === colid && String(prevVal) === String(value))
			return await form_filter('unfilter', groupno)
		tablex.exo_filter_colid = colid
		tablex.exo_filter_value = value
	}

	//hide unmatched rows
	var lastunhiddenrow = ''
	var anyunhiddenrows = false
	var hiderowns = []
	var lastunhiddenrows = []

	var anyhiddenrows = false
	for (var rown = 0; rown < values.length; rown++) {

		//grows[rown].style.display=(values[rown]==value)?'':'none'
		var hiderow = false
		if (grows[rown].style.display == 'none') {
			continue
		}
		else if (regexp) {
			if (values[rown].match(regexp)) hiderow = true
			//console.log(hiderow+' '+values[rown]+' '+regexp)
		}
		else if (maxrecn) {
			hiderow = rown > maxrecn
		}
		else if (values[rown] != value)
			hiderow = true

		if (hiderow) {
			hiderowns.push(rown)
			if (lastunhiddenrow) {
				lastunhiddenrows.push(lastunhiddenrow)
				lastunhiddenrow = ''
			}
		}
		else {
			var lastunhiddenrow = grows[rown]
			anyunhiddenrows = true
		}

	}

	if (!anyunhiddenrows && !maxrecn) {
		//await form_filter('unfilter', colidorgroupno)
	}
	else {
		for (rownn = 0; rownn < hiderowns.length; ++rownn) {
			var rown = hiderowns[rownn]
			grows[rown].style.display = 'none'
			if (tablexshowall) {
				tablexshowall.style.display = ''
				// residual lead-in (no ins/del): thead was only shown for Show All —
				// must open tbody/tfoot first cells too or headers shift one column
				form_group_leadin_display(tablex, true)
			}
		}
		// Fold-on-open (regexp / maxrecn): expand [+] on last visible before a
		// hidden run. Dblclick value filter: green insert only —
		// clear any leftover fold expand marks so insert after filter works.
		if (regexp || maxrecn) {
			for (var ern = 0; ern < lastunhiddenrows.length; ++ern)
				setinsertimage('expand', lastunhiddenrows[ern], groupno)
		}
		else {
			for (var irn = 0; irn < grows.length; ++irn)
				setinsertimage('insert', grows[irn], groupno)
		}
		await calcfields()
	}

	//window.status = ''
	console.log('');

	return true

}

var calendar_checkInDatePicker

async function form_pop_calendar() {
	// Non-modal UI that must outlive this click flight. Opening inside the same
	// flight flashes: LANDING → exoui_popup focuson(date) → form_closepopups hides it.
	// Contract: return null so exoui_popup refocuses the date field first; open after
	// that focus chain settles (same timing as the old setTimeout open).
	exo_begin_when_idle(form_popcalendar2, 'form_popcalendar2', { delay_ms: 100 })
	return null
}

function calendar_checkInDatePicker_onchange_sync() {

	// Commit path only: day click, Enter, Today, or Clear — not Escape/dismiss
	// Sync DOM handler (Calendar.onchange) — name ends _sync by convention.
	if (!calendar_checkInDatePicker || !gpreviouselement)
		return true

	setvalue(gpreviouselement, calendar_checkInDatePicker.formatDate())

	focusnext(gpreviouselement)
	return true
}

async function form_popcalendar2() {

	if (!gpreviouselement || !exofieldpopupallowed(gpreviouselement))
		return false

	var datevalue = gvalue.toString().exoiconv('[DATE]')
	var msdate = null
	if (datevalue) {
		msdate = new Date()
		msdate.setDate(Number(datevalue.exooconv('[DATE,DOM]')))
		msdate.setMonth(Number(datevalue.exooconv('[DATE,MONTH]') - 1))
		msdate.setFullYear(Number(datevalue.exooconv('[DATE,YEAR]')))
	}

	if (!calendar_checkInDatePicker) {
		calendar_checkInDatePicker = msdate ? new Calendar(msdate) : new Calendar()
		calendar_checkInDatePicker.create()
		//dont use addeventlistener here because onchange is special to DatePicker
		calendar_checkInDatePicker.onchange = calendar_checkInDatePicker_onchange_sync
	} else {
		if (calendar_checkInDatePicker._showing)
			calendar_checkInDatePicker.hide()
		calendar_checkInDatePicker.resetForOpen(msdate)
	}

	calendar_checkInDatePicker.setFormat(gdateformat);
	calendar_checkInDatePicker.setFirstDayOfWeek(Number(gfirstdayofweek));
	calendar_checkInDatePicker.setMinimalDaysInFirstWeek(1);
	calendar_checkInDatePicker.setIncludeWeek(false);

	calendar_checkInDatePicker.show(gpreviouselement)
	calendar_checkInDatePicker._calDiv.focus()
	//calendar_checkInDatePicker._calDiv.setActive()
	//NB activeElement not available everywhere
	//xyz=exosetinterval('window.status=document.activeElement.outerHTML',10)
	return false

}

// Drop form-owned popup chrome (not OS dialogs, not exoui_confirm).
// Return true if any closed (so Esc does not continue to field-undo / close record).
//
// Calendar-class popups: owned DOM + form_closepopups. Confirm uses its own
// modal stack (blockmodalui / resolvePendingConfirm) — not listed here.
// Future: optional calendar_popup_is_open() helper for symmetry with colour;
// do not change calendar “stray key hides” product behaviour casually.
async function form_closepopups() {

	var anyclosed = false

	if (calendar_checkInDatePicker && calendar_checkInDatePicker._showing) {
		try {
			calendar_checkInDatePicker.hide()
		}
		catch (e) {
			if (gusername == 'EXODUS')
				await exoui_note('couldnt drop calendar\n' + e.description)
		}
		anyclosed = true

	}

	// colour popup (colors.js) — calendar-class: owned DOM, Esc dismisses first
	if (typeof colors_popup != 'undefined' && colors_popup && colors_popup._showing
		&& typeof colors_popup_hide == 'function') {
		try {
			if (typeof colors_popup_cancel == 'function')
				await colors_popup_cancel()
			else
				colors_popup_hide()
		}
		catch (e) {
			if (gusername == 'EXODUS')
				await exoui_note('couldnt drop colour popup\n' + e.description)
		}
		anyclosed = true
	}

	return anyclosed

}

async function form_val_index(filename, fieldname, allownew) {

	//check if an existing

	if (!filename)
		filename = gdatafilename
	if (!fieldname)
		fieldname = gpreviouselement.name

	//get the existing values
	var indexvalues = await form_get_index(filename, fieldname)
	if (!indexvalues)
		return true
	indexvalues = indexvalues.group1

	//ok if is an existing indexvalue (Exact Case)
	for (var ii = 0; ii < indexvalues.length; ii++) {
		if (indexvalues[ii][fieldname].text == gvalue)
			return true
	}

	//optionally change case
	for (var ii = 0; ii < indexvalues.length; ii++) {
		if (indexvalues[ii][fieldname].text.toUpperCase() == gvalue.toUpperCase()) {
			if (!(reply = await exoui_confirm('Change the capitalisation of your entry?\n\nFrom: ' + gvalue.exoquote() + '\n\n  To: ' + indexvalues[ii][fieldname].text.exoquote(), 1)))
				return await exoui_invalid()
			if (reply == 1)
				gvalue = indexvalues[ii][fieldname].text
			return true
		}
	}

	//option to allow/prevent new index values
	if (!allownew)
		return await exoui_invalid(gvalue + ' ' + fieldname.toLowerCase() + ' does not exist')

	//confirm is new index value
	if (gvalue && ((await exoui_yesno(' Is ' + exoquote(gvalue) + ' to be a new ' + fieldname.toLowerCase() + ' for ' + filename.toLowerCase() + ' ?', 1)) != 1))
		return false

	return true

}

async function form_get_index(filename, fieldname) {

	db.request = 'CACHE\rGETINDEX.XML\r' + filename + '\r' + fieldname
	if (!(await db.send())) {
		await exoui_invalid(db.response)
		return ''
	}

	if (!db.data || db.data == '<records></records>')
		return ''

	return exoxml2obj(db.data)

}

async function form_pop_index(filename, fieldname, many) {

	//provides a simple way to select previously entered values on an indexed field

	if (!filename)
		filename = gdatafilename
	if (!fieldname)
		fieldname = gpreviouselement.name

	//get index values
	var indexvalues = await form_get_index(filename, fieldname)
	if (!indexvalues || indexvalues.group1.length == 0)
		return await exoui_invalid('No ' + fieldname.toLowerCase().exoconvert('_', ' ') + '(s) have been entered on ' + filename.toLowerCase().exoconvert('._', '  ') + ' file yet')

	//user selects index value(s)
	var result = await exoui_decide2('', indexvalues, [[fieldname, fieldname.exocapitalise()]], 0, '', many)

	return result

}

async function copyrecord_onclick() {

	if (!gkey || !glastkey || !gds.isnewrecord || gtouched)
		return await exoui_invalid('To copy a record you must:\n\n1. Open the record to copy\n2. Start a new record&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n3. Click the Copy button&nbsp;&nbsp;&nbsp;&nbsp;')

	//read the record to be copied
	var copyrecord = []
	if (!(await copyrecord.exoread(gdatafilename, glastkey)))
		return await exoui_invalid(copyrecord.exoresponse)

	//remove any uncopyable data
	for (var dictname in gds.dict) {
		var dictitem = gds.dictitem(dictname)
		if (dictitem && dictitem.type && dictitem.type == 'F' && typeof dictitem.copyable != 'undefined' && !dictitem.copyable)
			copyrecord[dictitem.fieldno] = ''
	}

	//validate copy
	if (!(await form_copyrecord(copyrecord)))
		return await exoui_invalid()

	//load the new record
	gro.revstr = copyrecord.slice(1).join(fm)
	gro.rev2obj()
	await gds.load(gro)

	return true

}

function copydictitem(dictitem, element) {

	// Dict attrs (validpropnames in db.js): write exo* only (no dual).
	// groupno → exogroupno; exostyle → exostyle. Drop legacy name if HTM seed had it.
	for (var propertyname in dictitem) {
		if (typeof dictitem[propertyname] == 'undefined')
			continue
		var shortn = propertyname == 'groupno' ? 'exogroupno'
			: (propertyname == 'exostyle' ? 'exostyle' : ('exo' + propertyname))
		var legacyn = propertyname == 'groupno' ? 'exogroupno'
			: (propertyname == 'exostyle' ? 'exostyle' : ('exodus' + propertyname))
		// Skip if attr already an expando (HTML property collision).
		if (typeof element[shortn] != 'undefined')
			continue
		if (legacyn != shortn && typeof element[legacyn] != 'undefined')
			continue
		// setAttribute so row cloneNode keeps dict attrs (expandos are not cloned)
		// false → '' so attributes are not the string "false"
		var value = dictitem[propertyname]
		// popup/link: false/null = suppress (no icon, no pad) — attribute must be absent.
		// '' = pad request (hasAttribute + empty). non-empty = real F7/F6.
		if ((propertyname == 'popup' || propertyname == 'link')
			&& (value === false || value === null)) {
			element.removeAttribute(shortn)
			if (legacyn != shortn)
				element.removeAttribute(legacyn)
			continue
		}
		if (value === false || value === null)
			value = ''
		element.setAttribute(shortn, value)
		// no dual: drop legacy so DOM is not hybrid
		if (legacyn != shortn)
			element.removeAttribute(legacyn)
	}

	element.name = dictitem.name

	//will be setup later
	//element.id=dictitem.name
	////element.name=dictitem.name

	//key elements default to required
	//(no longer as the next seq key can be provided by the database)
	//if (element.getAttribute('exofieldno')=='0'&&(typeof(element.getAttribute('exorequired'))=='undefined'||element.getAttribute('exorequired')=='false'))
	//{
	// //element.setAttribute('exorequired','true')
	// element.setAttribute('exorequired',true)
	//}

	if (!element.getAttribute('exorequired') || element.getAttribute('exorequired') == 'false') {
		element.setAttribute('exorequired', '')
		element.removeAttribute('exodusrequired') // HTM seed cleanup only
	}

}

function form_setdefault(str) {
	gro.defaultrevstr = unescape(str.exoconvert('`^]}\~', rm + fm + vm + sm + tm + stm))
	return 'ok'
}

async function form_postread_noteifdeleted(descending) {

	//skip if not deleted. deleted = gds.isnewrecord but has some version
	if (!gds.isnewrecord || !(await gds.getx('VERSION')))
		return false

	//descending means the latest log is inserted as first mv, which is the least common option, in PLAN/SCHEDULE/BATCHES
	var logn = descending ? 0 : -1

	var note = 'This record was deleted by ' + (await gds.get1('USERNAME_UPDATED', logn)) + ' on ' + (await gds.get1('DATETIME_UPDATED', logn)).exooconv('[DATE_TIME]') + '.'
	note += '\n\nYou can restore it by saving it again'
	await exoui_note(note)

	gallowsavewithoutchanges = true
	settouched(false, true)//change style of Save button

	//indicate is deleted
	return true
}

// Sync: copy selected text from INPUT/TEXTAREA as text/plain only.
// Must run before any await — clipboardData is only valid during the event, and
// browser default often also fills text/html with the <input> markup (worse when
// a popup icon wraps the field, e.g. CLIENT_CODE). Returns true if handled.
function form_copy_text_field_sync(event) {
	event = getevent(event)
	var el = event.target
	if (!el || !el.tagName)
		return false
	if (el.tagName != 'INPUT' && el.tagName != 'TEXTAREA')
		return false
	if (el.tagName == 'INPUT') {
		var typ = (el.type || 'text').toLowerCase()
		if (typ != 'text' && typ != 'search' && typ != 'tel' && typ != 'url'
			&& typ != 'password' && typ != 'email' && typ != 'number')
			return false
	}
	var start
	var end
	try {
		start = el.selectionStart
		end = el.selectionEnd
	} catch (e) {
		return false
	}
	if (typeof start != 'number' || typeof end != 'number' || start === end)
		return false
	var text = String(el.value || '').slice(start, end)
	var clip = event.clipboardData || window.clipboardData
	if (!clip || !clip.setData)
		return false
	try {
		// IE uses 'Text'; modern browsers 'text/plain'
		if (window.clipboardData && clip === window.clipboardData)
			clip.setData('Text', text)
		else {
			clip.setData('text/plain', text)
			// Prefer plain when paste targets check HTML first (Word, mail, etc.)
			clip.setData('text/html', text
				.replace(/&/g, '&amp;')
				.replace(/</g, '&lt;')
				.replace(/>/g, '&gt;'))
		}
	} catch (e) {
		return false
	}
	return exocancelevent(event) || true
}

async function document_oncopy(event) {

	event = getevent(event)

	// Decide list (F7/popup): same select-all / copy contract as typeahead
	if (typeof decide_oncopy == 'function') {
		var decideCopy = decide_oncopy(event)
		if (decideCopy === true)
			return true
		if (decideCopy === false)
			return false
	}

	// Typeahead: Ctrl+A then copy → full list TSV (all rows in panel data, not viewport only).
	// Partial drag-select in list → browser default.
	if (gform_typeahead_select_all && gform_typeahead_div && gform_typeahead_div.style.display != 'none') {
		if (form_typeahead_copy_text_sync(event, form_typeahead_list_tsv()))
			return false
	}
	if (form_typeahead_selection_in_list())
		return true

	// Before any await: plain field selection → text only (not <input> HTML).
	if (form_copy_text_field_sync(event))
		return false

	//choose
	//form_oncopy if present
	//form_oncopy_generic
	if (typeof form_oncopy == 'undefined')
		form_oncopy = form_oncopy_generic

	//return true if handled EVEN IF FAILED
	//return false to allow normal handler below
	if (await form_oncopy(event)) {
		return exocancelevent(event)
	}

	return true

}

async function form_oncopy_generic(event) {

	event = getevent(event)

	//resolve any data entry issues first
	if (!(await validateupdate()))
		return true//prevent normal copy

	//if no cols to copy then return false to trigger normal copy
	var cols = form_copypaste_getcols(event, false)
	if (!cols)
		return false

	var elementid = cols[0][0]

	//determine the lns that are selected
	var selection = window.getSelection()
	//var elements=document.getElementsByName('ITEM_DESCRIPTION')
	//getElementByName doesnt work on span tags
	var elements = document.getElementsByClassName('exoid_' + elementid)
	var lns = []
	if (!selection.containsNode)
		return false // no containsNode — fall through to normal copy
	for (var ln = 0; ln < elements.length; ++ln) {
		if (selection.containsNode(elements[ln], true))
			lns.push(ln)
	}

	//if nothing to copy, false = allow normal copy
	if (!lns.length)
		return false

	var ncols = cols.length
	var data = []

	//first line output is tab separated column titles (dictids)
	var txln = []
	for (var coln = 0; coln < ncols; ++coln) {
		txln.push(cols[coln][1])
	}
	data.push(txln.join('\t'))

	//ln used in gds.get1(dictid,ln) below
	for (var lnn = 0; lnn < lns.length; ++lnn) {
		var ln = lns[lnn]

		//build a line of text using the column names
		var txln = []
		for (var coln = 0; coln < ncols; ++coln) {
			var colpars = cols[coln]
			var dictid = colpars[0]
			if (!dictid)
				//empty col for letters and dates multi-row generated below
				txln.push('')
			else {
				var cell = await gds.get1(dictid, ln)
				if (colpars[2] && colpars[2].slice(0, 1) == '[')
					cell = cell.exooconv(colpars[2])
				txln.push(cell)
			}
		}

		//tab separated columns
		data.push(txln.join('\t'))

	}

	//using dos/windows style line endings to be compatible with windows notepad
	//everything else seems to handle them properly
	event.clipboardData.setData('text/plain', data.join('\r\n'));
	///event.clipboardData.setData('text/html', '<b>Hello, world!</b>');

	//prevent any data from selection being copied normally
	exocancelevent(event)

	return false

}

async function document_onpaste(event) {

	event = getevent(event)
	var element = event.target

	// Miss tint: no paste unless replacing a selection (same as typed insert)
	if (form_is_miss_tinted(element)) {
		var missPasteOk = false
		try {
			missPasteOk = typeof element.selectionStart === 'number'
				&& element.selectionEnd > element.selectionStart
		} catch (e) { }
		if (!missPasteOk) {
			exocancelevent(event)
			return false
		}
	}

	//prevent paste into readonly
	var msg = element.getAttribute('exoreadonly')
	if (msg) {
		//Generic message for read-only fields
		if (msg == 'true') {
			msg = 'This is a read-only field'
		}
		exocancelevent()
   		return await exoui_invalid(msg)
	}

	//only supporting form_paste in first column
	if (!element.getAttribute('exo_isfirstinputcolumn')) {
		//perform normal paste before any yielding is done which loses it
		// paste Gate A skips following input — re-run form_oninput after insert
		window.setTimeout(function () { void form_oninput({ target: element }) }, 0)
		return true
	}

	var clipboarddata = event.clipboardData || window.clipboardData || event.originalEvent.clipboardData
	if (!clipboarddata)
		return false

	//var data = clipboarddata.getData('text/html')
	var text = clipboarddata.getData('text/plain')

	//standardise on \n
	text = text.replace(/\r\n/g, '\n')

	// Single value paste (incl. trailing newline from Excel/clipboard) → normal
	// insert + typeahead. Multi-line import needs ≥2 non-empty lines.
	var nonempty = 0
	var parts = text.split('\n')
	for (var pi = 0; pi < parts.length; pi++) {
		if (String(parts[pi]).replace(/^\s+|\s+$/g, '') !== '')
			nonempty++
	}
	if (nonempty < 2) {
		window.setTimeout(function () { void form_oninput({ target: element }) }, 0)
		return true
	}
	//use form_onpaste_generic if form_onpaste not defined
	if (typeof form_onpaste == 'undefined')
		form_onpaste = form_onpaste_generic

	//return true to suppress normal handler EVEN IF FAILED
	if (await form_onpaste(event))
		return exocancelevent(event)

	//in case we selected a text node
	if (!element.getAttribute && element.parentNode.getAttribute)
		element = element.parentElement

	//the following doesnt seem to simulate pasting anymore
	//so we rely on early return above to perform normal paste
	/*
	//prevent pasting HTML into SPANS
	if (element.tagName!='SPAN')
		return true

	var clipboarddata = event.clipboardData || window.clipboardData || event.originalEvent.clipboardData
	if (!clipboarddata)
		return false

	//var data = clipboarddata.getData('text/html')
	var text = clipboarddata.getData('text/plain')

	//standardise on \n
	text = text.replace(/\r\n/g, '\n')

	// these dont work on INPUT or TEXTAREA
	//document.execCommand("insertText", false, text);
	document.execCommand("insertHTML", false, text);
	document.execCommand('paste', false, text)

	//dont continue to normal paste
	return exocancelevent(event)

	*/
}

function form_copypaste_getcols(event, pasting) {

	event = getevent(event)

	var element = event.target
	if (!element || !element.getAttribute) {
		element = element.parentElement
		if (!element || !element.getAttribute)
			return false
	}

	var selection = window.getSelection()
	var tablex = getancestor(selection.anchorNode, 'TABLE')
	if (!tablex)
		return false
	var groupn = tablex.getAttribute('exogroupno')
	if (groupn < 1)
		return false

	//only copy/paste on rows - get groupn or quit
	//var groupn=element.getAttribute('exogroupno')
	//if (groupn<1)
	//    return false

	//only cut/paste on first col
	//if (!element.getAttribute('exo_isfirstinputcolumn'))
	//    return false

	//get table columns
	var tablefieldns = gtables[groupn]
	var cols = []
	for (var fn = 0; fn < tablefieldns.length; ++fn) {
		var element = gfields[tablefieldns[fn]]
		//skip pasting values of non field type columns like symbolics
			if (pasting && Number(element.getAttribute('exofieldno')) == 0)
				continue
		var elementtitle = element.id.exoconvert('_', ' ')
		var conversion = element.getAttribute('exoconversion')
		var col = [element.id, elementtitle, conversion]
		cols.push(col)
	}

	if (!cols.length)
		return false

	return cols

}

async function form_onpaste_generic(event, elementid, validatedata_function, importdata_function) {

	//return false - to indicate normal paste should occur
	//return true - to avoid normal paste
	/////////////////////////////////////
	if (!validatedata_function)
		validatedata_function = form_onpaste_generic_validatedata
	if (!importdata_function)
		importdata_function = form_onpaste_generic_importdata

	event = getevent(event)

	//do normal paste if not on a row
	if (getrecn() == null)
		return false

	//determine columns to paste otherwise quit
	if (!elementid) {
		var cols = form_copypaste_getcols(event, true)
		if (!cols)
			return false
		elementid = cols[0][0]
	}

	//    //indicate normal paste should occur except on first column of rows
	//    if (!event.target||event.target.id!=elementid)
	//      return false
	//
	//    //rather horrible way to reverse default paste which happens later
	//    var beforepaste_element=gpreviouselement
	//    var beforepaste_value=gpreviousvalue
	//    window.setTimeout(
	//        function () {
	//            setvalue(beforepaste_element,beforepaste_value)
	//        }
	//    ,1)
	//    
	//resolve any data entry issues first
	if (!(await validateupdate()))
		return true

	//from here on we do NOT want normal paste to happen afterwards
	exocancelevent()

	//skip if no clipboard
	var clipboarddata = gevent.clipboardData ? gevent.clipboardData : window.clipboardData
	if (!clipboarddata)
		return true

	//var data = clipboarddata.getData('Text/HTML')
	//var data = clipboarddata.getData('Text/Plain')
	var data = clipboarddata.getData('Text')

	//standardise on \n and remove trailing \n
	data = data.replace(/\r\n/g, '\n').replace(/\n+$/, '')

	//normal paste if not multi-line
	if (data.indexOf('\n') < 0)
		return false

	//    //user pastes data
	//    if (!data) {
	//        //args.SCHEDULE_TEXT = gscheduleimportdata.exoconvert(tm,vm)
	//        var args = {}
	//        data = await exoui_showmodaldialog('../media/schedules_import.htm', args)
	//        if (!data)
	//            return true
	//        data = data.replace(tm, '\n')
	//        //gscheduleimportdata = data
	//    }

	//convert pairs of double quotes to one double quote and remove surrounding double quotes
	//eg "30""" becomes 30"
	//data = data.exoswap('""', '&quote;').exoconvert('"', '').exoswap('&quote;', '"')

	//convert data to array and tidy up
	data = data.split('\n')
	var nlines = data.length
	var line1 = data[0].split('\t')
	var ncols = line1.length
	for (var ln = nlines - 1; ln >= 0; --ln) {

		var line = data[ln].split('\t')

		//trim leading and trailing spaces and space-like characters
		//line = line.replace(/^ +/, '').replace(/ +$/, '')
		//trim all space before and after all tabs
		//line = line.replace(/ +\t/g, '\t').replace(/\t +/g, '\t')
		//unfortunately array.exotrim removes empty cells
		//line=line.exotrim()
		for (var ii = line.length - 1; ii >= 0; --ii)
			line[ii] = line[ii].exotrim()

		// skip blank lines
		//if (!line.join('')) {
		//    data.splice(ln,1)
		//    --nlines
		//    continue
		//}

		//ensure full set of columns
		while (line.length < ncols)
			line.push('')

		data[ln] = line
	}

	data.startln = 1
	if (nlines < data.startln + 1) {
		//await form_undopaste(beforepaste_element,beforepaste_value)
		await exoui_invalid('Not enough lines to import')
		return true
	}

	//pass the desired target cols with the data
	data.cols = cols

	//////////        
	//validate
	//////////
	var ninvalid = await validatedata_function(data)
	if (ninvalid === false)
		return true

	if (data.length <= 1) {
		await exoui_invalid('Nothing to import')
		return true
	}

	//option to quit if any invalid items
	if (ninvalid) {
		if (!(await exoui_yesno('There were ' + ninvalid + ' invalid items\nDo you want to continue?\n\n(Invalid items will be imported as blank)')))
			return true
	}

	////////
	//update
	////////
	var savegrecn = grecn
	var p = importdata_function(data);
	var result = await exoawaitresult(p, 'importdata');

	await calcfields()

	//focus on new row
	focuson(document.getElementsByName(elementid)[savegrecn])

	return true

}

async function form_onpaste_ignore_cancel(rown, coln, coltitle, data, msg) {
	return await exoui_okcancel('WARNING! in Row:' + (rown + 1) + ' Col:' + (coln + 1) + '\n\n' + coltitle + ' = "' + data + '"\n\n' + msg, 'Ignore', 'Cancel')
}

async function form_onpaste_generic_validatedata(data) {

	//returns ninvalid (or false to cancel)

	var ninvalid = 0
	var nlns = data.length

	//configuration

	var line1 = data[0]
	var rowtitles = line1
	var ncols = line1.length

	//this translates form coln to coln in the input/pasted coln
	var colns = []
	data.colns = colns

	//data.cols=gcostestimatecols
	var ncols = data.cols.length

	//add conversions and validations to data.cols[]
	for (var coln = 0; coln < ncols; ++coln) {
		var col = data.cols[coln]
		var element0 = $$(col[0])
		if (element0[0])
			element0 = element0[0]

		col.conversion = element0.getAttribute('exoconversion')
		if (!col.conversion)
			col.conversion = element0.getAttribute('exocheckbox')
		if (col.conversion && col.conversion.slice(0, 1) != '[')
			col.conversion = col.conversion.exosplit(':;')

		col.filename = element0.getAttribute('exofilename')
		col.validation = element0.getAttribute('exovalidation')
	}

	//parse line1 for column headings and find the datacoln for each screencoln
	var nimportablecols = 0
	for (var datacoln = 0; datacoln < line1.length; ++datacoln) {
		var coltitle = line1[datacoln].toUpperCase()
		for (var coln = 0; coln < ncols; ++coln) {
			if (data.cols[coln][1] == coltitle) {
				data.colns[coln] = datacoln
				++nimportablecols
				break
			}
		}
	}
	//ensure at least one column to be imported
	if (!nimportablecols) {
		var allcoltx = []
		for (var coln = 0; coln < ncols; ++coln)
			allcoltx.push(data.cols[coln][1])
		return await exoui_invalid('No recognisable columns in data\n\nPossible columns: ' + allcoltx.join(', '))
	}

	//for each line of pasted data
	for (var ln = 1; ln < nlns; ++ln) {

		var line = data[ln]

		//for each screen coln
		for (coln = 0; coln < ncols; ++coln) {

			//find the data col
			var datacoln = data.colns[coln]
			if (datacoln) {

				var ivalue = null
				var ovalue = line[datacoln]
				if (!ovalue)
					continue

				var col = data.cols[coln]

				//basic input conversion/validation
				if (col.conversion) {
					//things like [NUMBER,NDECS]
					if (col.conversion.slice(0, 1) == '[') {
						ivalue = ovalue.exoiconv(col.conversion)
					}
					//things like dropdowns and checkboxes
					else {
						ivalue = null
						for (var ii = 0; ii < col.conversion.length; ++ii) {
							var option = col.conversion[ii]
							if (ovalue == option[1])
								ivalue = option[0]
							else if (ovalue == option[0])
								ivalue = ovalue
						}
						if (ivalue == null) {
							gmsg = 'Allowable values are ' + col.conversion.exojoin(',=')
						}
					}
					//blank any invalid values
					if (ivalue === null) {
						//'Format must be '+conversion.slice(1,-1).split(',')[0]
						if (!(await form_onpaste_ignore_cancel(ln, datacoln, data.cols[coln][1], ovalue, gmsg)))
							return false
						ivalue = ''
						ninvalid++
					}
				}
				else
					ivalue = ovalue

				//basic file check
				if (col.filename) {
					ivalue = ivalue.toUpperCase()

					//warn and blank if not on file
					var rec = []
					if (!(await rec.exoread(col.filename, ivalue))) {
						//'Format must be '+conversion.slice(1,-1).split(',')[0]
						if (!(await form_onpaste_ignore_cancel(ln, datacoln, data.cols[coln][1], ivalue, 'Code does not exist or cannot be accessed')))
							return false
						ivalue = ''
						ninvalid++
					}
				}

				line[datacoln] = ivalue
			}
		}
	}

	return ninvalid

}

async function form_onpaste_generic_importdata(data) {

	//returns undefined if successful or false if any invalid vehicle code


	var ninserted = 0
	var nlines = data.length
	var ncols = data.cols.length

	//for each line of pasted data
	for (var ln = data.startln; ln < nlines; ++ln) {

		var line = data[ln]

		//add a new row if last row is not blank
		//var rown = gds.data.group1.length - 1
		//if (await gds.get1('VEHICLE_CODE', rown)) {
		//    exoaddrow(1)
		//    rown += 1
		//}
		if (ninserted)
			++grecn
		else
			ninserted = 1
		gds.insertrow(1, grecn)

		//for each screen coln
		for (coln = 0; coln < ncols; ++coln) {

			//find the data col
			var datacoln = data.colns[coln]
			if (typeof datacoln == 'number') {

				var colid = data.cols[coln][0]
				gpreviouselement = $$(colid)[grecn]
				gpreviousvalue = getvalue(gpreviouselement)
				setvalue(gpreviouselement, line[datacoln])

				if (!(await validateupdate()))
					return false

			}
		}
	}

	return

}
