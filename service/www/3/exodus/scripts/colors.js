
// Preview / apply LM form body (session only). Cookies written only on users Save.
// Empty / Default → CSS :root default (exo_chrome_apply_color removes override).
function colors_apply_screencolor(value) {
	if (typeof exo_chrome_apply_color == 'function')
		exo_chrome_apply_color(value)
	else if (typeof exo_set_style == 'function')
		exo_set_style('screencolor', value)
}

// Discard / clear / postdisplay: full chrome from cookies (fc + ff + fs).
function colors_restore_saved_chrome() {
	if (typeof exo_chrome_from_cookies == 'function')
		exo_chrome_from_cookies()
}
// Old name — colour-only callers
function colors_restore_saved_screencolor() {
	colors_restore_saved_chrome()
}

// Custom validators: always call base val first (di.validation replaces base).
async function colors_val_screencolor() {
	if (!(await exo_val_color())) return await exoui_invalid()
	// Preview only. Empty after Default → system body via CSS fallback.
	colors_apply_screencolor(gvalue)
	return true
}

async function colors_val_screenfont() {
	if (!(await exo_val_font())) return await exoui_invalid()
	if (typeof exo_chrome_apply_font == 'function')
		exo_chrome_apply_font(gvalue, await gds.getx('SCREEN_FONT_SIZE'))
	else if (typeof exo_set_style == 'function')
		exo_set_style('screenfont', gvalue, await gds.getx('SCREEN_FONT_SIZE'))
	return true
}

async function colors_val_screenfontsize() {
	if (typeof exo_chrome_apply_font == 'function')
		exo_chrome_apply_font(await gds.getx('SCREEN_FONT'), gvalue)
	else if (typeof exo_set_style == 'function')
		exo_set_style('screenfont', await gds.getx('SCREEN_FONT'), gvalue)
	return true
}

async function exo_val_font() {
	if (gvalue == null)
		gvalue = ''
	gvalue = String(gvalue)
	if (gvalue.toUpperCase() == 'DEFAULT')
		gvalue = ''
	return true
}

// True if v is a valid CSS <color> (name, #hex, rgb(), …). Does not rewrite gvalue.
function colors_is_valid_css_color(v) {
	if (v == null || v === '')
		return false
	v = String(v)
	if (typeof CSS != 'undefined' && CSS.supports)
		return CSS.supports('color', v)
	// Fallback: invalid assignments leave style.color empty
	try {
		var s = new Option().style
		s.color = ''
		s.color = v
		return !!s.color
	} catch (e) {
		return false
	}
}

// Base colour field validation. Wired by exo_dict_color when di.validation is
// unset. Custom di.validation must call this first.
// Empty / "Default" → "". Named colours and other valid CSS colours stay as-is
// (framework / exo_set_style have long accepted them). Invalid rejected.
// Length capped to the longest name in the allowed colour list.
async function exo_val_color() {
	if (gvalue == null)
		gvalue = ''
	gvalue = String(gvalue)
	if (gvalue.toUpperCase() == 'DEFAULT')
		gvalue = ''
	if (!gvalue)
		return true
	var maxLen = colors_max_name_length()
	if (gvalue.length > maxLen)
		return await exoui_invalid('Colour must be at most ' + maxLen + ' characters')
	if (colors_is_valid_css_color(gvalue))
		return true
	return await exoui_invalid(gvalue + ' is not a recognised color')
}

function exo_dict_font(di) {

	di.lowercase = true
	di.allowcursor = true

	//get fonts
	var tt = ''
	if (!di.required) tt += ';Default:'
	tt = exo_get_fonts(tt)

	di.conversion = tt

	di.popup = 'await exo_pop_font('
	if (di.required) di.popup += 'true'
	di.popup += ')'

	// Base validation unless a custom di.validation is already set (or set after).
	if (!di.validation)
		di.validation = 'await exo_val_font()'

	return

}

async function exo_pop_font(required, many) {

    //get colors
    var tt = ''
    if (!required) tt += ';Default:'
    tt = exo_get_fonts(tt)

    tt = tt.split(':')
    for (ii = tt.length - 1; ii >= 0; ii--) {
        var ttt = tt[ii].split(';')
        ttt[1] = ''
        ttt[2] = '<div width=100% style="font:' + ttt[0] + '">' + ttt[0]
        //exclude this since it still works without it and it triggers XML mode in exoui_decide2
        //ttt[2]+='</div>'
        tt[ii] = ttt.join(vm)
    }
    tt = tt.join(fm)

    return await exoui_decide2('', tt, [[2, '']], 0, '', many)

}

// Longest label in exo_get_colors (names + #rrggbb). Cached after first call.
function colors_max_name_length() {
	if (colors_max_name_length._n)
		return colors_max_name_length._n
	var max = 7 // #rrggbb
	try {
		var tt = exo_get_colors('')
		var parts = tt.split(':')
		for (var i = 0; i < parts.length; i++) {
			var segs = parts[i].split(';')
			// name is second segment when present; strip trailing spaces in list
			var name = (segs.length > 1 ? segs[1] : segs[0] || '').replace(/\s+$/, '')
			if (name.length > max)
				max = name.length
		}
	} catch (e) { }
	colors_max_name_length._n = max
	return max
}

function exo_dict_color(di) {

	di.lowercase = true
	di.allowcursor = true
	// align T → dbform replaces INPUT with contenteditable SPAN (autosize; same as text fields)
	di.align = 'T'

	// Magic conversion: bound text + swatch sibling (colors_install_swatch).
	// Empty paints current form body — not di.defaultvalue.
	di.conversion = 'color'

	di.popup = 'await exo_pop_color('
	if (di.required) di.popup += 'true'
	di.popup += ')'

	// Span min-width only (align T): ~8ch floor; content may grow. val_color still caps names.
	if (!di.length)
		di.length = 8

	// Base validation for every conversion="color" field from this helper.
	// If a custom di.validation is set after (or already set), that custom
	// routine must call await exo_val_color() first (see colors_val_screencolor).
	if (!di.validation)
		di.validation = 'await exo_val_color()'

	return

}

// Multi-select (or spectrum unavailable): named list with swatch preview (legacy decide).
async function colors_pop_color_decide(required, many) {

    var tt = ''
    if (!required) tt += ';Default:'
    tt = exo_get_colors(tt)

    tt = tt.split(':')
    for (var ii = tt.length - 1; ii >= 0; ii--) {
        var ttt = tt[ii].split(';')
        ttt[2] = '<div width=100% style="background-color:' + ttt[0] + '">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'//+ttt[1]
        //exclude this since it still works without it and it triggers XML mode in exoui_decide2
        //ttt[2]+='</div>'
        tt[ii] = ttt.join(vm)
    }
    tt = tt.join(fm)

    return await exoui_decide2('', tt, [[2, ''], [1, ''], [0, '']], 1, '', many)

}

// ---------------------------------------------------------------------------
// Colour popup — calendar-class form UI (sibling of calendar, not of confirm).
//
// Continuum: N×N hue×sat grid + N brightness samples (N = 1..32, default 16).
// Ctrl+wheel over continuum changes N. Event delegation (not 1k listeners).
// Lightness mapped 75%–100% (form body colours stay pale).
//
// F7 returns null (calendar contract) + opens when idle; Esc / form_closepopups /
// F7 toggle dismisses. Bound text tabbable; swatch click = F7.
// Key isolation: colors_popup_startevent / document_keydown (+ modal_on/off).
// Styles: colors.css (loaded here — not global.css; colour UI is optional).
// ---------------------------------------------------------------------------

// Inject colors.css once when this script loads (same directory as global.css).
function colors_ensure_stylesheet() {
	if (document.getElementById('exo_colors_css'))
		return
	var href = ''
	try {
		var scripts = document.getElementsByTagName('script')
		for (var i = scripts.length - 1; i >= 0; i--) {
			var src = scripts[i].src || ''
			if (/\/colors\.js(\?|#|$)/.test(src)) {
				href = src.replace(/scripts\/colors\.js(\?.*)?(#.*)?$/, 'colors.css')
				break
			}
		}
	} catch (e) { }
	if (!href && typeof EXODUSlocation != 'undefined' && EXODUSlocation)
		href = EXODUSlocation + 'colors.css'
	if (!href)
		return
	var link = document.createElement('link')
	link.id = 'exo_colors_css'
	link.rel = 'stylesheet'
	link.type = 'text/css'
	link.href = href
	var head = document.head || document.getElementsByTagName('head')[0]
	if (head)
		head.appendChild(link)
	else
		document.documentElement.appendChild(link)
}
colors_ensure_stylesheet()

var colors_popup = {
	_showing: false,
	_field: null,
	_openedStore: '',
	_hex: '',
	_div: null,
	_preview: null,
	_grid: null,
	_bright: null,
	_continuum: null,
	_nLabel: null,
	_hexInput: null,
	_monitor: null,
	_hexThead: null,
	_n: 16, // continuum resolution (1..32)
	_hi: 0, // 0..n-1 hue index
	_si: 15, // 0..n-1 sat index (full sat default)
	_vi: 15, // 0..n-1 brightness index (bright default)
	// Click locks hover-select; next click unlocks (and selects) — separate for grid / bright
	_hsLocked: false,
	_vLocked: false,
	_popGen: 0,
	_closedAt: 0,
	_closing: false,
	_modalOn: false
}

// Lightness floor for continuum (form body colours stay pale)
var COLORS_POPUP_LIGHT_MIN = 75
// Continuum resolution steps only (Ctrl+wheel jumps among these)
var COLORS_POPUP_N_STEPS = [8, 16, 24, 32]
var COLORS_POPUP_N_DEFAULT = 16
// Reference cell size at N=16 so the continuum keeps roughly the same footprint
var COLORS_POPUP_REF_N = 16
var COLORS_POPUP_REF_CELL_EM = 0.85

function colors_popup_n() {
	var n = colors_popup_n_snap(colors_popup._n)
	colors_popup._n = n
	return n
}

// Snap to nearest allowed step (prefer higher when midway).
function colors_popup_n_snap(n) {
	var steps = COLORS_POPUP_N_STEPS
	n = Math.round(Number(n))
	if (!(n > 0))
		return COLORS_POPUP_N_DEFAULT
	var best = steps[0]
	var bestDist = Math.abs(n - best)
	for (var i = 1; i < steps.length; i++) {
		var d = Math.abs(n - steps[i])
		if (d < bestDist || (d === bestDist && steps[i] > best)) {
			best = steps[i]
			bestDist = d
		}
	}
	return best
}

// stepDir: +1 finer (next higher step), -1 coarser
function colors_popup_n_step(stepDir) {
	var steps = COLORS_POPUP_N_STEPS
	var n = colors_popup_n()
	var idx = 0
	for (var i = 0; i < steps.length; i++) {
		if (steps[i] === n) {
			idx = i
			break
		}
	}
	idx += stepDir < 0 ? -1 : 1
	if (idx < 0)
		idx = 0
	if (idx >= steps.length)
		idx = steps.length - 1
	return steps[idx]
}

// Match exoconfirm2: modal shield + form_blockevents.
function colors_popup_modal_on() {
	if (colors_popup._modalOn)
		return
	colors_popup._modalOn = true
	if (typeof blockmodalui_sync == 'function')
		blockmodalui_sync()
	if (typeof form_blockevents == 'function')
		form_blockevents(true, 'colors_popup')
}

function colors_popup_modal_off() {
	if (!colors_popup._modalOn)
		return
	colors_popup._modalOn = false
	if (typeof form_blockevents == 'function')
		form_blockevents(false, 'colors_popup')
	if (typeof unblockmodalui_sync == 'function')
		unblockmodalui_sync()
}

function colors_hsl_to_hex(h, s, l) {
	h = Number(h) / 360
	s = Number(s) / 100
	l = Number(l) / 100
	function hue2rgb(p, q, t) {
		if (t < 0) t += 1
		if (t > 1) t -= 1
		if (t < 1 / 6) return p + (q - p) * 6 * t
		if (t < 1 / 2) return q
		if (t < 2 / 3) return p + (q - p) * (2 / 3 - t) * 6
		return p
	}
	var r, g, b
	if (s === 0) {
		r = g = b = l
	} else {
		var q = l < 0.5 ? l * (1 + s) : l + s - l * s
		var p = 2 * l - q
		r = hue2rgb(p, q, h + 1 / 3)
		g = hue2rgb(p, q, h)
		b = hue2rgb(p, q, h - 1 / 3)
	}
	function hx(n) {
		return ('0' + Math.round(n * 255).toString(16)).slice(-2)
	}
	return ('#' + hx(r) + hx(g) + hx(b)).toLowerCase()
}

function colors_hex_to_hsl(hex) {
	hex = colors_css_to_hex6(hex) || '#fdf5e6'
	var r = parseInt(hex.slice(1, 3), 16) / 255
	var g = parseInt(hex.slice(3, 5), 16) / 255
	var b = parseInt(hex.slice(5, 7), 16) / 255
	var max = Math.max(r, g, b)
	var min = Math.min(r, g, b)
	var h = 0
	var s = 0
	var l = (max + min) / 2
	if (max !== min) {
		var d = max - min
		s = l > 0.5 ? d / (2 - max - min) : d / (max + min)
		switch (max) {
			case r: h = (g - b) / d + (g < b ? 6 : 0); break
			case g: h = (b - r) / d + 2; break
			default: h = (r - g) / d + 4; break
		}
		h /= 6
	}
	return { h: h * 360, s: s * 100, l: l * 100 }
}

// Indices → colour. Brightness strip = lightness 75%..100%.
function colors_popup_cell_hex(hi, si, vi) {
	var n = colors_popup_n()
	var h = (hi % n) * (360 / n)
	var s = (n <= 1) ? 100 : (si / (n - 1)) * 100
	var l = COLORS_POPUP_LIGHT_MIN
		+ ((n <= 1) ? 0 : (vi / (n - 1)) * (100 - COLORS_POPUP_LIGHT_MIN))
	return colors_hsl_to_hex(h, s, l)
}

function colors_popup_current_hex() {
	return colors_popup_cell_hex(colors_popup._hi, colors_popup._si, colors_popup._vi)
}

function colors_popup_indices_from_hex(hex) {
	var hsl = colors_hex_to_hsl(hex)
	var n = colors_popup_n()
	var hi = Math.round(hsl.h / (360 / n)) % n
	if (hi < 0) hi = 0
	var si = Math.round((hsl.s / 100) * (n - 1))
	if (si < 0) si = 0
	if (si > n - 1) si = n - 1
	var l = hsl.l
	if (l < COLORS_POPUP_LIGHT_MIN)
		l = COLORS_POPUP_LIGHT_MIN
	var vi = Math.round(((l - COLORS_POPUP_LIGHT_MIN) / (100 - COLORS_POPUP_LIGHT_MIN)) * (n - 1))
	if (vi < 0) vi = 0
	if (vi > n - 1) vi = n - 1
	return { hi: hi, si: si, vi: vi }
}

// exact=true: keep this hex in chrome (field seed may be outside continuum range,
// e.g. #888888 is darker than LIGHT_MIN — still show it until the user picks).
function colors_popup_set_from_hex(hex, exact) {
	hex = colors_css_to_hex6(hex) || colors_default_swatch_hex()
	var ix = colors_popup_indices_from_hex(hex)
	colors_popup._hi = ix.hi
	colors_popup._si = ix.si
	colors_popup._vi = ix.vi
	colors_popup_paint_grid()
	colors_popup_paint_bright()
	if (exact)
		colors_popup_sync_chrome(hex)
	else
		colors_popup_sync_chrome()
}

// Contrast text for a #rrggbb sample (Rec. 601 luma).
function colors_popup_monitor_text_color(hex) {
	try {
		var r = parseInt(hex.slice(1, 3), 16)
		var g = parseInt(hex.slice(3, 5), 16)
		var b = parseInt(hex.slice(5, 7), 16)
		return (0.299 * r + 0.587 * g + 0.114 * b) >= 140 ? '#111' : '#fff'
	} catch (e) {
		return '#111'
	}
}

// Sticky-thead head direction for this form-body sample (same threshold as exo_set_form_head_direction).
function colors_popup_monitor_head_direction(hex) {
	try {
		var r = parseInt(hex.slice(1, 3), 16)
		var g = parseInt(hex.slice(3, 5), 16)
		var b = parseInt(hex.slice(5, 7), 16)
		return (0.299 * r + 0.587 * g + 0.114 * b) >= 128 ? 'deeper' : 'lighter'
	} catch (e) {
		return 'deeper'
	}
}

// hexOverride: show this value instead of continuum snap (open-from-field seed).
function colors_popup_sync_chrome(hexOverride) {
	var hex = hexOverride
		? (colors_css_to_hex6(hexOverride) || colors_popup_current_hex())
		: colors_popup_current_hex()
	colors_popup._hex = hex
	var inp = colors_popup._hexInput
	var thead = colors_popup._hexThead
	var mon = colors_popup._monitor
	if (!inp)
		return
	inp.value = hex
	var textCol = colors_popup_monitor_text_color(hex)
	var headDir = colors_popup_monitor_head_direction(hex)
	// Sample block — own fill; beat theme INPUT rules
	if (mon)
		mon.style.setProperty('--exocolor-monitor-bg-color', hex)
	inp.style.setProperty('background-color', hex, 'important')
	inp.style.setProperty('background-image', 'none', 'important')
	inp.style.setProperty('color', textCol, 'important')
	inp.style.setProperty('-webkit-text-fill-color', textCol, 'important')
	// Thead-tint sample under form-body sample (CSS oklch / color-mix matches global sticky thead)
	if (thead) {
		thead.setAttribute('data-monitor-head', headDir)
		thead.style.setProperty('color', textCol, 'important')
		thead.style.setProperty('-webkit-text-fill-color', textCol, 'important')
		thead.textContent = 'thead'
		thead.title = 'Sticky column-head tint of this form body colour (' + headDir + ')'
	}
}

function colors_popup_live_body() {
	var hex = colors_popup_current_hex()
	colors_popup_sync_chrome()
	var field = colors_popup._field
	if (!field || field.id != 'SCREEN_BODY_COLOR')
		return
	// Sync apply — must not queue via exo_begin or a late hover flight
	// re-applies after Default/OK and leaves the form body on a continuum colour.
	// Session form body only; cookie still waits for users form_postwrite on save.
	colors_apply_screencolor(hex)
}

function colors_popup_paint_grid() {
	var grid = colors_popup._grid
	if (!grid)
		return
	var n = colors_popup_n()
	var cells = grid.children
	var k = 0
	for (var si = n - 1; si >= 0; si--) {
		for (var hi = 0; hi < n; hi++) {
			var cell = cells[k++]
			if (!cell)
				continue
			var hex = colors_popup_cell_hex(hi, si, colors_popup._vi)
			cell.style.backgroundColor = hex
			cell.setAttribute('data-hi', String(hi))
			cell.setAttribute('data-si', String(si))
			var sel = (hi === colors_popup._hi && si === colors_popup._si)
			if (sel)
				cell.classList.add('exocolor-cell-selected')
			else
				cell.classList.remove('exocolor-cell-selected')
		}
	}
}

function colors_popup_paint_bright() {
	var strip = colors_popup._bright
	if (!strip)
		return
	var n = colors_popup_n()
	var cells = strip.children
	for (var vi = n - 1; vi >= 0; vi--) {
		var cell = cells[n - 1 - vi]
		if (!cell)
			continue
		// Show brightness of selected hue/sat
		cell.style.backgroundColor = colors_popup_cell_hex(colors_popup._hi, colors_popup._si, vi)
		cell.setAttribute('data-vi', String(vi))
		if (vi === colors_popup._vi)
			cell.classList.add('exocolor-cell-selected')
		else
			cell.classList.remove('exocolor-cell-selected')
	}
}

function colors_popup_select_hs(hi, si) {
	colors_popup._hi = hi
	colors_popup._si = si
	colors_popup_paint_grid()
	colors_popup_paint_bright()
	colors_popup_live_body()
}

function colors_popup_select_v(vi) {
	colors_popup._vi = vi
	colors_popup_paint_grid()
	colors_popup_paint_bright()
	colors_popup_live_body()
}

function colors_popup_cycle_buttons(backwards) {
	var row = colors_popup._div && colors_popup._div.querySelector('.exocolor-popup-actions')
	if (!row)
		return
	var buttons = row.querySelectorAll('button')
	if (!buttons.length)
		return
	var i
	var active = document.activeElement
	var idx = -1
	for (i = 0; i < buttons.length; i++) {
		if (buttons[i] === active || buttons[i].contains(active)) {
			idx = i
			break
		}
	}
	if (idx < 0)
		idx = backwards ? 0 : buttons.length - 1
	if (backwards)
		idx = (idx - 1 + buttons.length) % buttons.length
	else
		idx = (idx + 1) % buttons.length
	try {
		buttons[idx].focus()
	} catch (e) { }
}

function colors_popup_focusables() {
	// Tab order: continuum → OK / Default / Cancel (hex is display-only)
	var list = []
	if (colors_popup._bright)
		list.push(colors_popup._bright)
	if (colors_popup._grid)
		list.push(colors_popup._grid)
	var row = colors_popup._div && colors_popup._div.querySelector('.exocolor-popup-actions')
	if (row) {
		var buttons = row.querySelectorAll('button')
		for (var i = 0; i < buttons.length; i++)
			list.push(buttons[i])
	}
	return list
}

// Selected cell in the brightness strip (top = high vi).
function colors_popup_bright_cell(vi) {
	var strip = colors_popup._bright
	if (!strip)
		return null
	var n = colors_popup_n()
	if (vi == null)
		vi = colors_popup._vi
	return strip.children[n - 1 - vi] || null
}

// Selected cell in the N×N grid (paint: high sat rows first).
function colors_popup_grid_cell(hi, si) {
	var grid = colors_popup._grid
	if (!grid)
		return null
	var n = colors_popup_n()
	if (hi == null)
		hi = colors_popup._hi
	if (si == null)
		si = colors_popup._si
	return grid.children[(n - 1 - si) * n + hi] || null
}

function colors_popup_active_pane() {
	var active = document.activeElement
	if (!active)
		return null
	if (colors_popup._bright
		&& (active === colors_popup._bright || colors_popup._bright.contains(active)))
		return 'bright'
	if (colors_popup._grid
		&& (active === colors_popup._grid || colors_popup._grid.contains(active)))
		return 'grid'
	return null
}

// Tab/arrow focus lands on the selected (locked-or-current) cell, not an empty pane shell.
function colors_popup_focus_stop(el) {
	if (!el)
		return
	if (el === colors_popup._bright) {
		var bc = colors_popup_bright_cell()
		if (bc) {
			try { bc.focus() } catch (e) { }
			return
		}
	}
	if (el === colors_popup._grid) {
		var gc = colors_popup_grid_cell()
		if (gc) {
			try { gc.focus() } catch (e2) { }
			return
		}
	}
	try {
		el.focus()
	} catch (e3) { }
}

function colors_popup_tab_cycle(shiftKey) {
	var list = colors_popup_focusables()
	if (!list.length)
		return
	var active = document.activeElement
	var idx = -1
	for (var i = 0; i < list.length; i++) {
		// Cells inside bright/grid count as that pane
		if (list[i] === active || (list[i].contains && list[i].contains(active))) {
			idx = i
			break
		}
	}
	if (idx < 0) {
		colors_popup_focus_stop(list[shiftKey ? list.length - 1 : 0])
		return
	}
	if (shiftKey)
		idx = (idx - 1 + list.length) % list.length
	else
		idx = (idx + 1) % list.length
	colors_popup_focus_stop(list[idx])
}

function colors_popup_is_open() {
	return !!(colors_popup && colors_popup._showing && colors_popup._div)
}

// ---------------------------------------------------------------------------
// Key isolation — contract null / true / false (see client.js popup isolation).
// ---------------------------------------------------------------------------

function colors_popup_startevent(event) {
	if (!colors_popup_is_open())
		return null

	if (event.type == 'copy')
		return true

	var div = colors_popup._div
	var tgt = event.target
	var inside = div.contains(tgt)

	if (event.type == 'keydown') {
		var keycode = event.keyCode ? event.keyCode : event.which

		if (event.ctrlKey && (event.which == 67 || keycode == 67))
			return true

		// Confirm-style hotkeys (see exoconfirm_startevent)
		// Esc / F7 = Cancel
		if (keycode == 27 || keycode == 118) {
			void colors_popup_cancel()
			return false
		}
		// Tab trap
		if (keycode == 9) {
			colors_popup_tab_cycle(!!event.shiftKey)
			return false
		}

		var actions = colors_popup._div
			&& colors_popup._div.querySelector('.exocolor-popup-actions')
		// Enter / Ctrl+Enter: focused action button, else OK (confirm: Enter = positive)
		if (keycode == 13) {
			if (!event.ctrlKey && tgt && actions && actions.contains(tgt) && tgt.tagName == 'BUTTON') {
				if (tgt.click)
					tgt.click()
				return false
			}
			void colors_popup_ok()
			return false
		}
		// F9 = OK, F8 = Default (confirm F9/F8)
		if (keycode == 120) {
			void colors_popup_ok()
			return false
		}
		if (keycode == 119) {
			void colors_popup_clear()
			return false
		}
		// Letter hotkeys O / D / C
		if (!event.ctrlKey && !event.altKey && !event.metaKey) {
			var letter = String.fromCharCode(keycode).toUpperCase()
			if (letter == 'O') {
				void colors_popup_ok()
				return false
			}
			if (letter == 'D') {
				void colors_popup_clear()
				return false
			}
			if (letter == 'C') {
				void colors_popup_cancel()
				return false
			}
		}
		// Action-row buttons only — colour cells use arrows to move the selection
		if (inside && tgt && tgt.tagName == 'BUTTON' && actions && actions.contains(tgt)
			&& (keycode == 37 || keycode == 38 || keycode == 39 || keycode == 40)) {
			colors_popup_cycle_buttons(keycode == 37 || keycode == 38)
			return false
		}
		// Arrow keys on grid / brightness: nudge from current (locked) selection
		if (inside && (keycode == 37 || keycode == 38 || keycode == 39 || keycode == 40)) {
			colors_popup_nudge_selection(keycode)
			return false
		}
		if (keycode == 32 && inside && tgt && tgt.tagName == 'BUTTON')
			return true
		if (inside)
			return true
		return false
	}

	if (inside)
		return true
	return false
}

// pane: 'bright' | 'grid' | null (infer from focus). Optional for wheel-over.
// Arrows and wheel enter lock mode (like a click) so mouse-over no longer reselects.
function colors_popup_nudge_selection(keycode, pane) {
	var n = colors_popup_n()
	if (pane == null)
		pane = colors_popup_active_pane()
	// Brightness strip: move vi; lock hover
	if (pane === 'bright') {
		// Top of strip is high vi; Up/Right lighter, Down/Left darker
		if (keycode == 38 || keycode == 39)
			colors_popup._vi = Math.min(n - 1, colors_popup._vi + 1)
		else if (keycode == 40 || keycode == 37)
			colors_popup._vi = Math.max(0, colors_popup._vi - 1)
		colors_popup._vLocked = true
		colors_popup_paint_grid()
		colors_popup_paint_bright()
		colors_popup_live_body()
		colors_popup_focus_stop(colors_popup._bright)
		return
	}
	// Grid (default when grid-focused or dialog shell): from current hi/si
	// 37 L 38 U 39 R 40 D — sat rows (high sat top), hue cols
	if (keycode == 37)
		colors_popup._hi = (colors_popup._hi - 1 + n) % n
	else if (keycode == 39)
		colors_popup._hi = (colors_popup._hi + 1) % n
	else if (keycode == 38)
		colors_popup._si = Math.min(n - 1, colors_popup._si + 1)
	else if (keycode == 40)
		colors_popup._si = Math.max(0, colors_popup._si - 1)
	colors_popup._hsLocked = true
	colors_popup_paint_grid()
	colors_popup_paint_bright()
	colors_popup_live_body()
	// Always land focus on the grid after nudge (arrows or wheel)
	colors_popup_focus_stop(colors_popup._grid)
}

// Wheel over continuum = arrows: plain = up/down, Shift = left/right, Ctrl = change N.
function colors_popup_pane_under(el) {
	if (!el)
		return null
	if (colors_popup._bright
		&& (el === colors_popup._bright || colors_popup._bright.contains(el)))
		return 'bright'
	if (colors_popup._grid
		&& (el === colors_popup._grid || colors_popup._grid.contains(el)))
		return 'grid'
	// Continuum chrome (label / gap) still counts for Ctrl+wheel N changes
	if (colors_popup._continuum
		&& (el === colors_popup._continuum || colors_popup._continuum.contains(el)))
		return 'continuum'
	return null
}

function colors_popup_on_wheel(ev) {
	if (!colors_popup_is_open())
		return
	var div = colors_popup._div
	var inside = div && div.contains(ev.target)
	if (!inside)
		return

	var dy = ev.deltaY || 0
	var dx = ev.deltaX || 0

	// Ctrl+wheel anywhere on the popup: change continuum resolution N (1..32).
	// Scroll up = finer. Blocks browser zoom.
	if (ev.ctrlKey) {
		if (ev.preventDefault)
			ev.preventDefault()
		if (ev.stopPropagation)
			ev.stopPropagation()
		var step = 0
		if (dy)
			step = dy < 0 ? 1 : -1
		else if (dx)
			step = dx < 0 ? 1 : -1
		if (step)
			colors_popup_set_n(colors_popup_n_step(step))
		return
	}

	// Plain / Shift wheel only on strip or grid cells
	var pane = colors_popup_pane_under(ev.target)
	if (pane !== 'bright' && pane !== 'grid')
		return
	if (ev.preventDefault)
		ev.preventDefault()
	if (ev.stopPropagation)
		ev.stopPropagation()

	// One step per event — same as a single arrow key
	// plain wheel → up/down; Shift+wheel (or trackpad horizontal) → left/right
	var keycode
	if (ev.shiftKey) {
		var hs = dy || dx
		if (!hs)
			return
		keycode = hs < 0 ? 37 : 39
	} else if (Math.abs(dx) > Math.abs(dy) && dx) {
		keycode = dx < 0 ? 37 : 39
	} else {
		if (!dy)
			return
		keycode = dy < 0 ? 38 : 40
	}
	colors_popup_nudge_selection(keycode, pane)
}

function colors_popup_document_keydown(event) {
	if (!colors_popup_is_open())
		return null

	var keycode = event.keyCode ? event.keyCode : event.which
	var div = colors_popup._div

	// Confirm-style: Esc/F7 Cancel; F9 OK; F8 Default
	if (keycode == 27 || keycode == 118) {
		void colors_popup_cancel()
		return false
	}
	if (keycode == 120) {
		void colors_popup_ok()
		return false
	}
	if (keycode == 119) {
		void colors_popup_clear()
		return false
	}
	// Secondary Tab trap (div handler is primary; this catches rare outside paths)
	if (keycode == 9) {
		colors_popup_tab_cycle(!!event.shiftKey)
		return false
	}
	if (div.contains(event.target))
		return true
	try {
		div.focus()
	} catch (e) { }
	return false
}

// Grid / brightness: hover selects until a click locks; next click unlocks and selects.
// Event delegation on pane — cells have no per-node listeners (fast rebuild when N changes).
function colors_popup_pick_hs(cell, isClick) {
	if (!cell)
		return
	if (!isClick && colors_popup._hsLocked)
		return
	colors_popup_select_hs(
		Number(cell.getAttribute('data-hi')),
		Number(cell.getAttribute('data-si'))
	)
	if (isClick)
		colors_popup._hsLocked = !colors_popup._hsLocked
}

function colors_popup_pick_v(cell, isClick) {
	if (!cell)
		return
	if (!isClick && colors_popup._vLocked)
		return
	colors_popup_select_v(Number(cell.getAttribute('data-vi')))
	if (isClick)
		colors_popup._vLocked = !colors_popup._vLocked
}

function colors_popup_cell_from_event(ev, pane) {
	var t = ev.target
	while (t && t !== pane) {
		if (t.classList && t.classList.contains('exocolor-cell'))
			return t
		t = t.parentNode
	}
	return null
}

function colors_popup_bind_pane(pane, kind) {
	// kind: 'grid' | 'bright'
	pane.addEventListener('click', function (ev) {
		var cell = colors_popup_cell_from_event(ev, pane)
		if (!cell)
			return
		if (kind == 'grid')
			colors_popup_pick_hs(cell, true)
		else
			colors_popup_pick_v(cell, true)
	})
	pane.addEventListener('mouseover', function (ev) {
		var cell = colors_popup_cell_from_event(ev, pane)
		if (!cell)
			return
		if (kind == 'grid')
			colors_popup_pick_hs(cell, false)
		else
			colors_popup_pick_v(cell, false)
	})
}

function colors_popup_fill_grid_cells(grid, n) {
	// DocumentFragment + no listeners — one paint when appended
	var frag = document.createDocumentFragment()
	for (var si = n - 1; si >= 0; si--) {
		for (var hi = 0; hi < n; hi++) {
			var cell = document.createElement('button')
			cell.type = 'button'
			cell.className = 'exocolor-cell'
			cell.tabIndex = -1
			cell.setAttribute('data-hi', String(hi))
			cell.setAttribute('data-si', String(si))
			frag.appendChild(cell)
		}
	}
	grid.appendChild(frag)
}

function colors_popup_fill_bright_cells(strip, n) {
	var frag = document.createDocumentFragment()
	for (var vi = n - 1; vi >= 0; vi--) {
		var cell = document.createElement('button')
		cell.type = 'button'
		cell.className = 'exocolor-cell exocolor-bright-cell'
		cell.tabIndex = -1
		cell.setAttribute('data-vi', String(vi))
		frag.appendChild(cell)
	}
	strip.appendChild(frag)
}

function colors_popup_build_grid() {
	var grid = document.createElement('div')
	grid.className = 'exocolor-popup-grid'
	grid.setAttribute('role', 'grid')
	grid.tabIndex = 0
	grid.title = 'Hue × saturation — click to lock/unlock; Ctrl+scroll changes resolution'
	colors_popup_bind_pane(grid, 'grid')
	return grid
}

function colors_popup_build_bright() {
	var strip = document.createElement('div')
	strip.className = 'exocolor-popup-bright'
	strip.tabIndex = 0
	strip.title = 'Brightness — click to lock/unlock; Ctrl+scroll changes resolution'
	colors_popup_bind_pane(strip, 'bright')
	return strip
}

// Apply continuum CSS vars and (re)build cell nodes for colors_popup._n.
// Preserves current colour via HSL re-index. Focus stays on bright if it was there.
// N is always one of COLORS_POPUP_N_STEPS (8, 16, 24, 32).
function colors_popup_set_n(n) {
	n = colors_popup_n_snap(n)
	var prev = colors_popup._n
	var hex = colors_popup_current_hex()
	var focusPane = colors_popup_active_pane()
	colors_popup._n = n

	var grid = colors_popup._grid
	var bright = colors_popup._bright
	var continuum = colors_popup._continuum
	if (!grid || !bright)
		return

	// Keep overall continuum size ~constant (scale cells with N)
	var cellEm = (COLORS_POPUP_REF_N * COLORS_POPUP_REF_CELL_EM) / n
	if (continuum) {
		continuum.style.setProperty('--exocolor-n', String(n))
		continuum.style.setProperty('--exocolor-cell', cellEm.toFixed(3) + 'em')
	}
	if (colors_popup._nLabel)
		colors_popup._nLabel.textContent = n + '×' + n

	// Only rebuild DOM when N changes (or first fill)
	if (prev !== n || grid.children.length !== n * n) {
		while (grid.firstChild)
			grid.removeChild(grid.firstChild)
		while (bright.firstChild)
			bright.removeChild(bright.firstChild)
		colors_popup_fill_grid_cells(grid, n)
		colors_popup_fill_bright_cells(bright, n)
	}

	// Re-map selection to same colour at new resolution
	if (hex)
		colors_popup_set_from_hex(hex)
	else {
		if (colors_popup._hi > n - 1)
			colors_popup._hi = n - 1
		if (colors_popup._si > n - 1)
			colors_popup._si = n - 1
		if (colors_popup._vi > n - 1)
			colors_popup._vi = n - 1
		colors_popup_paint_grid()
		colors_popup_paint_bright()
		colors_popup_sync_chrome()
	}

	if (focusPane === 'bright' || focusPane === 'grid')
		colors_popup_focus_stop(focusPane === 'bright' ? bright : grid)
	else if (colors_popup._showing)
		colors_popup_focus_stop(bright)

	// N change can grow the popup — re-clamp to the viewport
	if (colors_popup._showing && colors_popup._field)
		colors_popup_place(colors_popup._field)
}

function colors_popup_create() {
	if (colors_popup._div)
		return
	var div = document.createElement('div')
	div.id = 'exo_color_popup'
	div.className = 'exocolor-popup calendar'
	div.tabIndex = 1
	div.setAttribute('role', 'dialog')
	div.setAttribute('aria-label', 'Colour')

	// Face + sticky-thead monitor (continuum pick only — no typing)
	var monitor = document.createElement('div')
	monitor.className = 'exocolor-popup-monitor'
	var hexInput = document.createElement('input')
	hexInput.type = 'text'
	hexInput.className = 'exocolor-popup-hex'
	hexInput.readOnly = true
	hexInput.tabIndex = -1
	hexInput.spellcheck = false
	hexInput.title = 'Form body colour (hex)'
	hexInput.setAttribute('aria-label', 'Form body colour')
	hexInput.setAttribute('aria-readonly', 'true')
	var hexThead = document.createElement('div')
	hexThead.className = 'exocolor-popup-hex-thead'
	hexThead.setAttribute('aria-label', 'Sticky thead tint of this form body colour')
	hexThead.textContent = 'thead'
	monitor.appendChild(hexInput)
	monitor.appendChild(hexThead)

	var continuum = document.createElement('div')
	continuum.className = 'exocolor-popup-continuum'
	var nLabel = document.createElement('div')
	nLabel.className = 'exocolor-popup-nlabel'
	nLabel.title = 'Continuum resolution (Ctrl+scroll: 8, 16, 24, 32)'
	var grid = colors_popup_build_grid()
	var bright = colors_popup_build_bright()
	// Brightness on the left, N×N grid on the right
	continuum.appendChild(bright)
	continuum.appendChild(grid)

	var row = document.createElement('div')
	row.className = 'exocolor-popup-actions'
	// Confirm-style: underline hotkey letter, title "Press X or Fkey" (exoui_confirm)
	function mkBtn(label, cls, letter, funckey) {
		var b = document.createElement('button')
		b.type = 'button'
		b.className = cls || ''
		letter = String(letter || label.charAt(0)).toUpperCase()
		var plain = String(label)
		var i = plain.toUpperCase().indexOf(letter)
		if (i >= 0)
			b.innerHTML = plain.slice(0, i) + '<u>' + plain.charAt(i) + '</u>' + plain.slice(i + 1)
		else
			b.textContent = plain
		b.title = 'Press ' + letter + (funckey ? ' or ' + funckey : '')
		b.setAttribute('data-exo-letter', letter)
		return b
	}
	// Order: OK (positive/F9/Ctrl+Enter) · Default (F8) · Cancel (Esc) — like exoui_confirm
	var btnOk = mkBtn('OK', 'exocolor-popup-ok', 'O', 'F9 or Ctrl+Enter')
	var btnClear = mkBtn('Default', 'exocolor-popup-clear', 'D', 'F8')
	var btnCancel = mkBtn('Cancel', 'exocolor-popup-cancel', 'C', 'Esc')
	row.appendChild(btnOk)
	row.appendChild(btnClear)
	row.appendChild(btnCancel)

	div.appendChild(monitor)
	div.appendChild(continuum)
	div.appendChild(nLabel)
	div.appendChild(row)
	div.style.display = 'none'
	// Wheel: plain/Shift on continuum cells; Ctrl anywhere = change N (block zoom/scroll)
	div.addEventListener('wheel', colors_popup_on_wheel, { passive: false })
	document.body.appendChild(div)

	colors_popup._div = div
	colors_popup._preview = hexInput
	colors_popup._monitor = monitor
	colors_popup._hexThead = hexThead
	colors_popup._continuum = continuum
	colors_popup._nLabel = nLabel
	colors_popup._grid = grid
	colors_popup._bright = bright
	colors_popup._hexInput = hexInput

	// Initial continuum at default N (cells + CSS vars)
	if (!colors_popup._n)
		colors_popup._n = COLORS_POPUP_N_DEFAULT
	colors_popup_set_n(colors_popup._n)

	btnOk.addEventListener('click', function () {
		void colors_popup_ok()
	})
	btnClear.addEventListener('click', function () {
		void colors_popup_clear()
	})
	btnCancel.addEventListener('click', function () {
		void colors_popup_cancel()
	})

	div.addEventListener('keydown', function (ev) {
		if (ev.stopPropagation)
			ev.stopPropagation()
		ev.cancelBubble = true
		var key = ev.key || ev.keyCode
		var kc = ev.keyCode || ev.which
		var tgt = ev.target

		// Esc / F7 = Cancel (F7 while open is not re-open)
		if (key == 'Escape' || kc == 27 || kc == 118 || key == 'F7') {
			if (ev.preventDefault)
				ev.preventDefault()
			void colors_popup_cancel()
			return false
		}
		if (kc == 9 || key == 'Tab') {
			if (ev.preventDefault)
				ev.preventDefault()
			colors_popup_tab_cycle(!!ev.shiftKey)
			return false
		}
		// F9 OK / F8 Default
		if (kc == 120 || key == 'F9') {
			if (ev.preventDefault)
				ev.preventDefault()
			void colors_popup_ok()
			return false
		}
		if (kc == 119 || key == 'F8') {
			if (ev.preventDefault)
				ev.preventDefault()
			void colors_popup_clear()
			return false
		}
		// Letter hotkeys O / D / C
		if (!ev.ctrlKey && !ev.altKey && !ev.metaKey) {
			var ch = (typeof key == 'string' && key.length == 1)
				? key.toUpperCase()
				: String.fromCharCode(kc).toUpperCase()
			if (ch == 'O') {
				if (ev.preventDefault)
					ev.preventDefault()
				void colors_popup_ok()
				return false
			}
			if (ch == 'D') {
				if (ev.preventDefault)
					ev.preventDefault()
				void colors_popup_clear()
				return false
			}
			if (ch == 'C') {
				if (ev.preventDefault)
					ev.preventDefault()
				void colors_popup_cancel()
				return false
			}
		}
		if (tgt && tgt.tagName == 'BUTTON' && row.contains(tgt)
			&& (kc == 37 || kc == 38 || kc == 39 || kc == 40
				|| key == 'ArrowLeft' || key == 'ArrowUp' || key == 'ArrowRight' || key == 'ArrowDown')) {
			if (ev.preventDefault)
				ev.preventDefault()
			colors_popup_cycle_buttons(kc == 37 || kc == 38 || key == 'ArrowLeft' || key == 'ArrowUp')
			return false
		}
		if ((kc == 37 || kc == 38 || kc == 39 || kc == 40)
			&& !(tgt && tgt.tagName == 'INPUT')) {
			if (ev.preventDefault)
				ev.preventDefault()
			colors_popup_nudge_selection(kc)
			return false
		}
		// Enter / Ctrl+Enter: focused action button (plain Enter), else OK
		if (key == 'Enter' || kc == 13) {
			if (ev.preventDefault)
				ev.preventDefault()
			if (!ev.ctrlKey && tgt && tgt.tagName == 'BUTTON' && row.contains(tgt) && tgt.click)
				tgt.click()
			else
				void colors_popup_ok()
			return false
		}
		return true
	})
}

// Bounds of the colour control for placement: wrap (text+swatch) plus any
// F7 find icon (id + '_popup') to the left — so we never cover that icon.
function colors_popup_base_rect(field) {
	if (!field)
		return null
	var els = []
	var p = field.parentNode
	if (p && p.classList && p.classList.contains('exocolor-field-wrap'))
		els.push(p)
	else {
		els.push(field)
		var sw = colors_field_swatch(field)
		if (sw)
			els.push(sw)
	}
	if (field.id) {
		var icon = document.getElementById(field.id + '_popup')
		if (icon)
			els.push(icon)
	}
	var left = Infinity
	var top = Infinity
	var right = -Infinity
	var bottom = -Infinity
	for (var i = 0; i < els.length; i++) {
		var r = els[i].getBoundingClientRect()
		if (r.width <= 0 && r.height <= 0)
			continue
		if (r.left < left)
			left = r.left
		if (r.top < top)
			top = r.top
		if (r.right > right)
			right = r.right
		if (r.bottom > bottom)
			bottom = r.bottom
	}
	if (!(left < right && top < bottom))
		return field.getBoundingClientRect()
	return { left: left, top: top, right: right, bottom: bottom }
}

// Same placement contract as Calendar.prototype._place (date picker):
//   left half  → fully RIGHT of base (field+swatch+F7 icon)
//   right half → fully LEFT of base
//   top half   → align tops; bottom half → align bottoms (grows up)
//   if still over base: stack above (bottom half) or below (top half)
//   stay fully on-screen; position:fixed (viewport coords)
function colors_popup_rects_overlap(a, b) {
	return a.left < b.right && a.right > b.left && a.top < b.bottom && a.bottom > b.top
}

function colors_popup_place(field) {
	var div = colors_popup._div
	if (!div || !field)
		return
	div.style.display = 'block'
	div.style.position = 'fixed'
	void div.offsetHeight

	var ar = colors_popup_base_rect(field)
	if (!ar)
		return
	var w = div.offsetWidth || 0
	var h = div.offsetHeight || 0
	var vw = window.innerWidth || document.documentElement.clientWidth || 0
	var vh = window.innerHeight || document.documentElement.clientHeight || 0
	try {
		if (window.visualViewport) {
			if (window.visualViewport.width)
				vw = window.visualViewport.width
			if (window.visualViewport.height)
				vh = window.visualViewport.height
		}
	} catch (e) { }
	if (!w || !h || !vw || !vh)
		return

	var midX = vw / 2
	var midY = vh / 2
	var ax = (ar.left + ar.right) / 2
	var ay = (ar.top + ar.bottom) / 2
	var gap = 4
	var pad = 4
	var minL = pad
	var minT = pad
	var maxR = vw - pad
	var maxB = vh - pad
	var bottomHalf = ay >= midY
	var leftHalf = ax < midX

	function fitsRight() {
		return ar.right + gap + w <= maxR
	}
	function fitsLeft() {
		return ar.left - gap - w >= minL
	}
	function fitsBelow() {
		return ar.bottom + gap + h <= maxB
	}
	function fitsAbove() {
		return ar.top - gap - h >= minT
	}
	function popupRect(l, t) {
		return { left: l, top: t, right: l + w, bottom: t + h }
	}
	function clampOnScreen(l, t) {
		if (l + w > maxR)
			l = maxR - w
		if (l < minL)
			l = minL
		if (t + h > maxB)
			t = maxB - h
		if (t < minT)
			t = minT
		return { left: l, top: t }
	}

	var leftPos
	var topPos

	// Horizontal: fully beside base
	if (leftHalf) {
		if (fitsRight())
			leftPos = ar.right + gap
		else if (fitsLeft())
			leftPos = ar.left - gap - w
		else
			leftPos = minL
	} else {
		if (fitsLeft())
			leftPos = ar.left - gap - w
		else if (fitsRight())
			leftPos = ar.right + gap
		else
			leftPos = minL
	}

	// Vertical while beside
	if (!bottomHalf)
		topPos = ar.top
	else
		topPos = ar.bottom - h

	var c = clampOnScreen(leftPos, topPos)
	leftPos = c.left
	topPos = c.top

	// Covering field/swatch/icon → stack clear of it
	if (colors_popup_rects_overlap(popupRect(leftPos, topPos), ar)) {
		if (bottomHalf) {
			if (fitsAbove())
				topPos = ar.top - gap - h
			else if (fitsBelow())
				topPos = ar.bottom + gap
			else
				topPos = (ar.top - minT) >= (maxB - ar.bottom)
					? Math.max(minT, ar.top - gap - h)
					: ar.bottom + gap
		} else {
			if (fitsBelow())
				topPos = ar.bottom + gap
			else if (fitsAbove())
				topPos = ar.top - gap - h
			else
				topPos = (maxB - ar.bottom) >= (ar.top - minT)
					? ar.bottom + gap
					: Math.max(minT, ar.top - gap - h)
		}
		leftPos = ar.left
		c = clampOnScreen(leftPos, topPos)
		leftPos = c.left
		topPos = c.top

		if (colors_popup_rects_overlap(popupRect(leftPos, topPos), ar)) {
			if (bottomHalf || fitsAbove())
				topPos = ar.top - gap - h
			else
				topPos = ar.bottom + gap
			leftPos = ar.left
			if (leftPos + w > maxR)
				leftPos = maxR - w
			if (leftPos < minL)
				leftPos = minL
			if (topPos + h <= ar.top) {
				if (topPos < minT)
					topPos = minT
				if (topPos + h > ar.top - gap)
					topPos = ar.top - gap - h
			} else if (topPos >= ar.bottom) {
				if (topPos + h > maxB)
					topPos = maxB - h
				if (topPos < ar.bottom + gap)
					topPos = ar.bottom + gap
			}
		}
	}

	// Fully inside viewport
	if (leftPos + w > maxR)
		leftPos = maxR - w
	if (leftPos < minL)
		leftPos = minL
	if (topPos + h > maxB)
		topPos = maxB - h
	if (topPos < minT)
		topPos = minT

	// Clamp re-cover → force clear (bottom half → above)
	if (colors_popup_rects_overlap(popupRect(leftPos, topPos), ar)) {
		if (bottomHalf) {
			topPos = ar.top - gap - h
			if (topPos + h > maxB)
				topPos = maxB - h
			if (topPos + h > ar.top - gap)
				topPos = ar.top - gap - h
		} else {
			topPos = ar.bottom + gap
			if (topPos < minT)
				topPos = minT
			if (topPos + h > maxB)
				topPos = maxB - h
			if (topPos < ar.bottom + gap)
				topPos = ar.bottom + gap
		}
		if (leftPos + w > maxR)
			leftPos = maxR - w
		if (leftPos < minL)
			leftPos = minL
	}

	div.style.left = Math.round(leftPos) + 'px'
	div.style.top = Math.round(topPos) + 'px'
}

// Snapshot live form/pane colour (--exoform-bg-color / cookie) for cancel restore.
// The popup must not leave the form body changed unless the user OK/Defaults.
function colors_popup_capture_form_body() {
	try {
		var formBody = getComputedStyle(document.documentElement)
			.getPropertyValue('--exoform-bg-color').trim()
		if (formBody)
			return formBody
	} catch (e) { }
	if (typeof exogetcookie2 == 'function') {
		try {
			var fc = exogetcookie2('fc')
			if (fc != null && String(fc) !== '')
				return String(fc)
		} catch (e2) { }
	}
	return colors_system_default_body_hex()
}

function colors_popup_show(field) {
	if (!field)
		return false
	if (colors_popup._closedAt && (Date.now() - colors_popup._closedAt) < 450)
		return false

	colors_popup_create()

	var reopening = colors_popup._showing
	colors_popup._field = field

	// Do not touch field.value / gpreviousvalue — form popups must not wipe typing.
	// Seed UI from current field text (may be mid-edit); only continuum hover/OK
	// changes the form body, and cancel restores the form body captured here.
	if (!reopening)
		colors_popup._openedFormBody = colors_popup_capture_form_body()

	var raw = field.value
	if (typeof getvalue == 'function') {
		try {
			raw = getvalue(field)
		} catch (e) {
			raw = field.value
		}
	}
	var seed = colors_css_to_hex6(raw)
	var fromField = !!seed
	if (!seed)
		seed = colors_default_swatch_hex()

	colors_popup._showing = true
	colors_popup._closedAt = 0
	// Field value: lock hover so seed selection sticks; empty = unlocked
	colors_popup._hsLocked = fromField
	colors_popup._vLocked = fromField
	colors_popup_modal_on()
	// exact seed in chrome; do NOT live-preview form body until continuum pick
	colors_popup_set_from_hex(seed, fromField)
	colors_popup_place(field)
	colors_popup_focus_stop(colors_popup._bright)
	return true
}

function colors_popup_hide() {
	if (!colors_popup._showing)
		return false
	var was = true
	colors_popup._popGen++
	colors_popup._showing = false
	colors_popup._closedAt = Date.now()
	colors_popup_modal_off()
	if (colors_popup._div)
		colors_popup._div.style.display = 'none'
	var field = colors_popup._field
	colors_popup._field = null
	// Return focus to bound text (like calendar → date field), not the swatch
	if (field) {
		try {
			if (field.focus)
				field.focus()
		} catch (e) { }
	}
	return was
}

async function colors_popup_ok() {
	if (colors_popup._closing)
		return
	colors_popup._closing = true
	try {
		var field = colors_popup._field
		if (!field) {
			colors_popup_hide()
			return
		}
		// Prefer chrome hex (exact field seed until user picks a continuum cell)
		var hex = colors_popup._hex || colors_popup_current_hex()
		// Empty store only for the fixed system form body — not live --exoform-bg-color
		// (live form body follows preview, so hex===formBody would always store "").
		var sys = colors_system_default_body_hex()
		var store = (hex && hex === sys) ? '' : hex
		colors_popup_hide()
		await colors_field_store(field, store)
	} finally {
		colors_popup._closing = false
	}
}

async function colors_popup_clear() {
	if (colors_popup._closing)
		return
	colors_popup._closing = true
	try {
		var field = colors_popup._field
		if (!field) {
			colors_popup_hide()
			return
		}
		// Return "Default"; field validation turns it into "" and applies system
		// form-body (colors_val_screencolor / exo_val_color).
		var prev = colors_field_text(field)
		colors_popup_hide()
		await colors_field_store(field, 'Default')
		if (typeof validateupdate == 'function') {
			// Force leave-field validate: previous ≠ Default so the change is seen
			if (typeof gpreviouselement != 'undefined')
				gpreviouselement = field
			if (typeof gpreviousvalue != 'undefined')
				gpreviousvalue = prev
			await validateupdate()
		}
	} finally {
		colors_popup._closing = false
	}
}

async function colors_popup_cancel() {
	if (colors_popup._closing)
		return
	colors_popup._closing = true
	try {
		var field = colors_popup._field
		// Undo any live form-body preview only — leave field text alone
		// (including mid-edit typing; standard popup cancel does not validate).
		var formBody = colors_popup._openedFormBody
		colors_popup_hide()
		if (field && field.id == 'SCREEN_BODY_COLOR')
			colors_apply_screencolor(formBody == null ? '' : formBody)
	} finally {
		colors_popup._closing = false
	}
}

// F7 / find — calendar contract: return null, open when idle. Toggle closes.
async function exo_pop_color(required, many) {

	if (many)
		return await colors_pop_color_decide(required, many)

	var gen = ++colors_popup._popGen

	if (colors_popup._showing) {
		void colors_popup_cancel()
		return null
	}

	if (colors_popup._closedAt && (Date.now() - colors_popup._closedAt) < 450)
		return null

	if (typeof exo_begin_when_idle == 'function')
		exo_begin_when_idle(function () {
			return colors_pop_color_open(gen)
		}, 'colors_pop_color_open', { delay_ms: 100 })
	else
		void exo_begin(function () {
			return colors_pop_color_open(gen)
		}, 'colors_pop_color_open')
	return null
}

async function colors_pop_color_open(gen) {
	if (gen != null && gen !== colors_popup._popGen)
		return false
	if (colors_popup._closedAt && (Date.now() - colors_popup._closedAt) < 450)
		return false
	var field = (typeof gpreviouselement != 'undefined') ? gpreviouselement : null
	if (field && field.getAttribute && field.getAttribute('data-exo-color-swatch-for'))
		field = document.getElementById(field.getAttribute('data-exo-color-swatch-for')) || field
	if (!field)
		return false
	if (typeof exofieldpopupallowed == 'function' && !exofieldpopupallowed(field)
		&& field.getAttribute('data-exo-color-field') != '1')
		return false
	colors_popup_show(field)
	return false
}

function exo_get_colors(tt) {
    if (!tt) tt = ''
    tt += '#000000;Black:'
    tt += '#000080;Navy:'
    tt += '#00008B;DarkBlue:'
    tt += '#0000CD;MediumBlue:'
    tt += '#0000FF;Blue:'
    tt += '#006400;DarkGreen:'
    tt += '#008000;Green:'
    tt += '#008080;Teal:'
    tt += '#008B8B;DarkCyan:'
    tt += '#00BFFF;DeepSkyBlue:'
    tt += '#00CED1;DarkTurquoise:'
    tt += '#00FA9A;MediumSpringGreen:'
    tt += '#00FF00;Lime:'
    tt += '#00FF7F;SpringGreen:'
    tt += '#00FFFF;Aqua:'
    tt += '#00FFFF;Cyan:'
    tt += '#191970;MidnightBlue:'
    tt += '#1E90FF;DodgerBlue:'
    tt += '#20B2AA;LightSeaGreen:'
    tt += '#228B22;ForestGreen:'
    tt += '#2E8B57;SeaGreen:'
    tt += '#2F4F4F;DarkSlateGray:'
    tt += '#2F4F4F;DarkSlateGrey:'
    tt += '#32CD32;LimeGreen:'
    tt += '#333399;NavyBlue:'
    tt += '#3CB371;MediumSeaGreen:'
    tt += '#40E0D0;Turquoise:'
    tt += '#4169E1;RoyalBlue:'
    tt += '#4682B4;SteelBlue:'
    tt += '#483D8B;DarkSlateBlue:'
    tt += '#48D1CC;MediumTurquoise:'
    tt += '#4B0082;Indigo :'
    tt += '#556B2F;DarkOliveGreen:'
    tt += '#5F9EA0;CadetBlue:'
    tt += '#6495ED;CornflowerBlue:'
    tt += '#66CDAA;MediumAquaMarine:'
    tt += '#696969;DimGray:'
    tt += '#696969;DimGrey:'
    tt += '#6A5ACD;SlateBlue:'
    tt += '#6B8E23;OliveDrab:'
    tt += '#708090;SlateGray:'
    tt += '#708090;SlateGrey:'
    tt += '#778899;LightSlateGray:'
    tt += '#778899;LightSlateGrey:'
    tt += '#7B68EE;MediumSlateBlue:'
    tt += '#7CFC00;LawnGreen:'
    tt += '#7FFF00;Chartreuse:'
    tt += '#7FFFD4;Aquamarine:'
    tt += '#800000;Maroon:'
    tt += '#800080;Purple:'
    tt += '#808000;Olive:'
    tt += '#808080;Gray:'
    tt += '#808080;Grey:'
    tt += '#87CEEB;SkyBlue:'
    tt += '#87CEFA;LightSkyBlue:'
    tt += '#8A2BE2;BlueViolet:'
    tt += '#8B0000;DarkRed:'
    tt += '#8B008B;DarkMagenta:'
    tt += '#8B4513;SaddleBrown:'
    tt += '#8FBC8F;DarkSeaGreen:'
    tt += '#90EE90;LightGreen:'
    tt += '#9370D8;MediumPurple:'
    tt += '#9400D3;DarkViolet:'
    tt += '#98FB98;PaleGreen:'
    tt += '#9932CC;DarkOrchid:'
    tt += '#9ACD32;YellowGreen:'
    tt += '#A0522D;Sienna:'
    tt += '#A52A2A;Brown:'
    tt += '#A9A9A9;DarkGray:'
    tt += '#A9A9A9;DarkGrey:'
    tt += '#ADD8E6;LightBlue:'
    tt += '#ADFF2F;GreenYellow:'
    tt += '#AFEEEE;PaleTurquoise:'
    tt += '#B0C4DE;LightSteelBlue:'
    tt += '#B0E0E6;PowderBlue:'
    tt += '#B22222;FireBrick:'
    tt += '#B8860B;DarkGoldenRod:'
    tt += '#BA55D3;MediumOrchid:'
    tt += '#BC8F8F;RosyBrown:'
    tt += '#BDB76B;DarkKhaki:'
    tt += '#C0C0C0;Silver:'
    tt += '#C71585;MediumVioletRed:'
    tt += '#CD5C5C;IndianRed :'
    tt += '#CD853F;Peru:'
    tt += '#D2691E;Chocolate:'
    tt += '#D2B48C;Tan:'
    tt += '#D3D3D3;LightGray:'
    tt += '#D3D3D3;LightGrey:'
    tt += '#D87093;PaleVioletRed:'
    tt += '#D8BFD8;Thistle:'
    tt += '#DA70D6;Orchid:'
    tt += '#DAA520;GoldenRod:'
    tt += '#DC143C;Crimson:'
    tt += '#DCDCDC;Gainsboro:'
    tt += '#DDA0DD;Plum:'
    tt += '#DEB887;BurlyWood:'
    tt += '#E0FFFF;LightCyan:'
    tt += '#E6E6FA;Lavender:'
    tt += '#E9967A;DarkSalmon:'
    tt += '#EE82EE;Violet:'
    tt += '#EEE8AA;PaleGoldenRod:'
    tt += '#F08080;LightCoral:'
    tt += '#F0E68C;Khaki:'
    tt += '#F0F8FF;AliceBlue:'
    tt += '#F0FFF0;HoneyDew:'
    tt += '#F0FFFF;Azure:'
    tt += '#F4A460;SandyBrown:'
    tt += '#F5DEB3;Wheat:'
    tt += '#F5F5DC;Beige:'
    tt += '#F5F5F5;WhiteSmoke:'
    tt += '#F5FFFA;MintCream:'
    tt += '#F8F8FF;GhostWhite:'
    tt += '#FA8072;Salmon:'
    tt += '#FAEBD7;AntiqueWhite:'
    tt += '#FAF0E6;Linen:'
    tt += '#FAFAD2;LightGoldenRodYellow:'
    tt += '#FDF5E6;OldLace:'
    tt += '#FF0000;Red:'
    tt += '#FF00FF;Fuchsia:'
    tt += '#FF00FF;Magenta:'
    tt += '#FF1493;DeepPink:'
    tt += '#FF4500;OrangeRed:'
    tt += '#FF6347;Tomato:'
    tt += '#FF69B4;HotPink:'
    tt += '#FF7F50;Coral:'
    tt += '#FF8C00;Darkorange:'
    tt += '#FFA07A;LightSalmon:'
    tt += '#FFA500;Orange:'
    tt += '#FFB6C1;LightPink:'
    tt += '#FFC0CB;Pink:'
    tt += '#FFD700;Gold:'
    tt += '#FFDAB9;PeachPuff:'
    tt += '#FFDEAD;NavajoWhite:'
    tt += '#FFE4B5;Moccasin:'
    tt += '#FFE4C4;Bisque:'
    tt += '#FFE4E1;MistyRose:'
    tt += '#FFEBCD;BlanchedAlmond:'
    tt += '#FFEFD5;PapayaWhip:'
    tt += '#FFF0F5;LavenderBlush:'
    tt += '#FFF5EE;SeaShell:'
    tt += '#FFF8DC;Cornsilk:'
    tt += '#FFFACD;LemonChiffon:'
    tt += '#FFFAF0;FloralWhite:'
    tt += '#FFFAFA;Snow:'
    tt += '#FFFF00;Yellow:'
    tt += '#FFFFE0;LightYellow:'
    tt += '#FFFFF0;Ivory:'
    tt += '#FFFFFF;White:'

    //others
    tt += '#A3FF99;somelightgreen2:'
    tt += '#DBFEF8;mintblue:'
    tt += '#B4D7BF;vanillamint:'
    tt += '#C5E3BF;minticecream:'
    //(Safe Hex3)
    tt += '#CCFFCC;offwhitegreen:'

    tt += '#00CCCC;StrongCyan:'
    //Empire
    tt += '#00CCBD;StrongCyan2:'

    //numeric colors
    //tt = ''
    for (ii = 0; ii <= 256; ii += 32) {
        if (ii > 255) ii = 255
        for (jj = 0; jj <= 256; jj += 32) {
            if (jj > 255) jj = 255
            for (kk = 0; kk <= 256; kk += 32) {
                if (kk > 255) kk = 255
                hexcolor = ('0' + ii.toString(16)).slice(-2) + ('0' + jj.toString(16)).slice(-2) + ('0' + kk.toString(16)).slice(-2)
                tt += '#' + hexcolor + ';#' + hexcolor + ':'
            }
        }
    }

    if (tt.slice(-1) == ':') tt = tt.slice(0, -1)
    if (tt.slice(0, 1) == ':') tt = tt.slice(1)

    return tt
}

function exo_get_fonts(tt) {
    if (!tt) tt = ''

    tt += 'SANS SERIF,HELVETICA:'
    tt += ' Verdana:'
    tt += ' Arial:'
    tt += ' Arial Black:'
    tt += ' Arial Narrow:'
    tt += ' Century Gothic:'
    tt += ' Comic Sans MS:'
    tt += ' Franklin Gothic Medium:'
    tt += ' Haettenschweiler:'
    tt += ' Impact:'
    tt += ' Lucida Console:'
    tt += ' Lucida Sans Unicode:'
    tt += ' Microsoft Sans Serif:'
    tt += ' MS Reference Sans Serif:'
    tt += ' Nina:'
    tt += ' Tahoma:'
    tt += ' Trebuchet MS:'

    tt += 'SERIF,TIMES:'
    tt += ' Book Antiqua:'
    tt += ' Bookman Old Style:'
    tt += ' Courier New:'
    tt += ' Garamond:'
    tt += ' Georgia:'
    tt += ' Palatino Linotype:'
    tt += ' Sylfaen:'
    tt += ' Times New Roman:'
    tt += ' Monotype Corsiva:'

    if (tt.slice(-1) == ':') tt = tt.slice(0, -1)
    if (tt.slice(0, 1) == ':') tt = tt.slice(1)

    return tt

}

function exo_dict_colorfontsize(dict, fn) {

    var din = dict.length - 1

    // DONT CHANGE ORDER UNLESS YOU CHANGE ORDER IN BACKEND

    di = dict[++din] = dictrec('REPORT_HEAD_COLOR', 'F', fn)
    di.wordsep = vm
    di.wordno = 1
    di.nwords = 1
    exo_dict_color(di)

    di = dict[++din] = dictrec('REPORT_BODY_COLOR', 'F', fn)
    di.wordsep = vm
    di.wordno = 2
    di.nwords = 1
    exo_dict_color(di)

    di = dict[++din] = dictrec('REPORT_FONT_NAME', 'F', fn)
    di.wordsep = vm
    di.wordno = 3
    di.nwords = 1
    exo_dict_font(di)
    //di.required=true

    // SCREEN_HEAD_COLOR — UNUSED in the live UI (no cookie, no set_style, no HTM
    // input on Screens row). Kept as word 4 so SYSTEM 46,* field layout stays stable
    // (see initgeneral formheadcolor). Sticky thead tint is CSS: slightly darker
    // mix of --exoform-data-bg-color (field-cell colour), not this field.
    di = dict[++din] = dictrec('SCREEN_HEAD_COLOR', 'F', fn)
    di.wordsep = vm
    di.wordno = 4
    di.nwords = 1
    exo_dict_color(di)

    // SCREEN_BODY_COLOR — users preview → --exoform-bg-color; cookie fc on Save.
    di = dict[++din] = dictrec('SCREEN_BODY_COLOR', 'F', fn)
    di.wordsep = vm
    di.wordno = 5
    di.nwords = 1
    exo_dict_color(di)
    di.validation = 'await colors_val_screencolor()'

    di = dict[++din] = dictrec('SCREEN_FONT', 'F', fn)
    di.wordsep = vm
    di.wordno = 6
    di.nwords = 1
    exo_dict_font(di)
    di.validation = 'await colors_val_screenfont()'

    di = dict[++din] = dictrec('SCREEN_FONT_SIZE', 'F', fn)
    di.wordsep = vm
    di.wordno = 7
    di.nwords = 1
    di.validation = 'await colors_val_screenfontsize()'
    // Ghost only (not filldefaults): empty field + empty fs cookie → 90% apply
    di.placeholder = '90'
    exo_dict_integer(di, { min: 50, max: 200 })
    di.allowcursor = true

    di = dict[++din] = dictrec('REPORT_FONT_SIZE', 'F', fn)
    di.wordsep = vm
    di.wordno = 8
    di.nwords = 1
    di.placeholder = '90'
    exo_dict_integer(di, { min: 50, max: 200 })
    di.allowcursor = true

    //report styles need to be ignored in LISTEN2 see task USER UPDATE "REPORT"

}

// ---------------------------------------------------------------------------
// Colour field chrome: bound text (data) + sibling type=color swatch (paint only).
// dbform conversion "color" calls colors_install_swatch after the field is set up.
// ---------------------------------------------------------------------------

// CSS/name/hex → #rrggbb, or '' if empty/default/unparseable
function colors_css_to_hex6(v) {
	if (v == null)
		return ''
	v = String(v).replace(/\s+/g, '')
	if (!v || v.toUpperCase() == 'DEFAULT')
		return ''
	if (/^#[0-9a-fA-F]{6}$/.test(v))
		return v.toLowerCase()
	if (/^[0-9a-fA-F]{6}$/.test(v))
		return ('#' + v).toLowerCase()
	if (/^#[0-9a-fA-F]{3}$/.test(v))
		return ('#' + v.charAt(1) + v.charAt(1) + v.charAt(2) + v.charAt(2)
			+ v.charAt(3) + v.charAt(3)).toLowerCase()
	var el = document.createElement('span')
	el.style.cssText = 'position:absolute;visibility:hidden;color:' + v
	document.documentElement.appendChild(el)
	var m = /(\d+)\s*,\s*(\d+)\s*,\s*(\d+)/.exec(window.getComputedStyle(el).color)
	document.documentElement.removeChild(el)
	if (!m)
		return ''
	function hx(n) {
		return ('0' + Number(n).toString(16)).slice(-2)
	}
	return ('#' + hx(m[1]) + hx(m[2]) + hx(m[3])).toLowerCase()
}

// System form body — global.css :root LM default. Not live preview / cookie.
function colors_system_default_body_hex() {
	return '#fdf5e6'
}

// Empty field swatch paint = live form/pane colour (cookie fc / --exoform-bg-color).
// After live preview this follows the popup selection — do not use for "is default?".
function colors_default_swatch_hex() {
	try {
		var formBody = getComputedStyle(document.documentElement)
			.getPropertyValue('--exoform-bg-color').trim()
		var hex = colors_css_to_hex6(formBody)
		if (hex)
			return hex
	} catch (e) { }
	return colors_system_default_body_hex()
}

function colors_field_swatch(field) {
	if (!field || !field.id)
		return null
	return document.getElementById(field.id + '_swatch')
}

// Read bound colour text (INPUT value or contenteditable SPAN).
function colors_field_text(field) {
	if (!field)
		return ''
	if (typeof getvalue == 'function') {
		try {
			var v = getvalue(field)
			return v == null ? '' : String(v)
		} catch (e) { }
	}
	if (field.tagName == 'SPAN')
		return String(field.textContent != null ? field.textContent : field.innerText || '')
	return field.value == null ? '' : String(field.value)
}

// Bound text → swatch paint. Empty text paints form body (not written back to text).
function colors_sync_swatch(field) {
	var swatch = colors_field_swatch(field)
	if (!swatch)
		return
	var hex = colors_css_to_hex6(colors_field_text(field))
	if (!hex)
		hex = colors_default_swatch_hex()
	hex = hex.toLowerCase()
	if (swatch.tagName == 'INPUT' && swatch.type == 'color') {
		if (swatch.value !== hex)
			swatch.value = hex
	} else {
		// Solid fill + checkerboard underlay (form-body-matching colours still visible)
		swatch.style.backgroundColor = hex
		swatch.style.backgroundImage =
			'linear-gradient(' + hex + ',' + hex + '),'
			+ 'linear-gradient(45deg,#ccc 25%,transparent 25%),'
			+ 'linear-gradient(-45deg,#ccc 25%,transparent 25%),'
			+ 'linear-gradient(45deg,transparent 75%,#ccc 75%),'
			+ 'linear-gradient(-45deg,transparent 75%,#ccc 75%)'
		swatch.style.backgroundSize = 'auto,8px 8px,8px 8px,8px 8px,8px 8px'
		swatch.style.backgroundPosition = '0 0,0 0,0 4px,4px -4px,-4px 0'
	}
}

// Write store into bound colour text + gds (+ live body style).
// store is "" | "Default" | #rrggbb. "Default" is for field validate → "" + body apply.
async function colors_field_store(field, store) {
	if (!field)
		return
	store = store == null ? '' : String(store)

	var cur = field.value
	if (typeof getvalue == 'function') {
		try {
			cur = getvalue(field)
		} catch (e) {
			cur = field.value
		}
	}
	var same = String(cur) === String(store)
	if (!same) {
		if (typeof setvalue == 'function')
			setvalue(field, store)
		else
			field.value = store

		if (typeof gds != 'undefined' && gds && field.id && typeof gds.setx == 'function') {
			var recn = 0
			if (typeof getrecn == 'function')
				recn = getrecn(field)
			else if (typeof grecn != 'undefined')
				recn = grecn
			await gds.setx(field.id, recn, store)
		}
		if (typeof settouched == 'function')
			settouched(true)
	}

	// Preview only. Cookie written on users Save; discard → colors_restore_saved_chrome.
	// "Default" skipped here — validation → "" → apply (CSS system body).
	if (field.id == 'SCREEN_BODY_COLOR' && store.toUpperCase() != 'DEFAULT')
		colors_apply_screencolor(store)

	colors_sync_swatch(field)
}

// Visible bound text (INPUT or contenteditable SPAN) + swatch after it. Click = F7.
function colors_install_swatch(field) {
	// dbform align T converts INPUT → SPAN before conversion "color" install
	if (!field || (field.tagName != 'INPUT' && field.tagName != 'SPAN'))
		return false
	// Need a stable id for field_swatch (dbform must set id before install)
	if (!field.id)
		return false
	if (field.getAttribute('data-exo-color-field') == '1') {
		colors_sync_swatch(field)
		return true
	}

	field.setAttribute('data-exo-color-field', '1')
	field.className = (field.className ? field.className + ' ' : '') + 'exocolor-text'

	// Swatch is chrome only — not tabbable; sits after the text; click = F7.
	// Inline size so the chip is visible even if colors.css fails to load
	// (never size with em + font-size:0 — that is 0×0).
	var swatch = document.createElement('button')
	swatch.type = 'button'
	swatch.id = field.id + '_swatch'
	swatch.className = 'exonative-color'
	swatch.title = (field.getAttribute('exotitle') || field.id || 'Colour') + ' (F7)'
	swatch.tabIndex = -1
	swatch.setAttribute('tabindex', '-1')
	swatch.setAttribute('aria-label', 'Colour swatch')
	swatch.setAttribute('data-exo-color-swatch-for', field.id)
	swatch.style.cssText = 'display:inline-block;width:2.7rem;height:1.35rem;'
		+ 'min-width:2.7rem;min-height:1.35rem;padding:0;margin:0 0 0 0.35em;'
		+ 'border:2px solid #888;border-radius:0.3rem;vertical-align:middle;'
		+ 'cursor:pointer;box-sizing:border-box;overflow:hidden;'

	var wrap = document.createElement('span')
	wrap.className = 'exocolor-field-wrap'
	if (!field.parentNode)
		return false
	field.parentNode.insertBefore(wrap, field)
	wrap.appendChild(field)
	wrap.appendChild(swatch)

	colors_ensure_stylesheet()
	colors_sync_swatch(field)

	// Typing updates the swatch (INPUT and contenteditable SPAN)
	field.addEventListener('input', function () {
		colors_sync_swatch(field)
	})
	field.addEventListener('change', function () {
		colors_sync_swatch(field)
	})

	// Click swatch → same as F7 find icon (exo_pop_color / HSL panel)
	swatch.addEventListener('click', function (ev) {
		ev.preventDefault()
		ev.stopPropagation()
		void exo_begin(async function () {
			if (typeof setgpreviouselement == 'function')
				setgpreviouselement(field)
			// Same entry as F7 when panel already open = toggle close
			if (typeof exo_pop_color == 'function')
				await exo_pop_color(!!field.getAttribute('exorequired'))
			else
				colors_popup_show(field)
		}, 'color swatch open')
	})

	return true
}
