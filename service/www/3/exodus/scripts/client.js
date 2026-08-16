//Copyright NEOSYS All Rights Reserved.
//undocumented functions
//CollectGarbage() in all versions
//"finally" block in v5.0

// HTML element has a parentNode which is not usual element so be careful in "ancestor finder" routines
// elements cannot be referred to simply as variables like idx
// (must use getElementById or set an equivalent global variable)
// insertBefore secondparameter is mandatory (put null)
// document.body cannot be referred to in heading scripts until the document body is started
// (so put document.onload function in the body tag. onload function cannot be called onload)
// selectelement.options() should
//parentElement nonstandard use parentNode

//default location of exodus scripts etc
if (typeof EXODUSlocation == 'undefined')
	EXODUSlocation = '../exodus/'

//define global variables and initialise some of them
//many global variables are defined throughout the code outside functions ... ususally just preceeding them
//some global variable might be defined inside function by assignment without a defining var
//most global variables start with g except some very commonly used ones like fm, vm, sm
//a more complete set of global variables may be found using a debugger

//var gautofitwindow = true
var gisdialog
var glocked//mainly required in dbform but used in logout

var gdomainname = window.location.toString().split('/')[2]
//if (document.cookie.indexOf('username') < 0)
//	alert('qno username')

var geventlog = ''
var geventstack = ['', '']
var geventdepth = 1
var gonunload = false
var gonbeforeunload = false

var glogging, gstepping

var glogevents//related to yield
var glogcookie
var glogsettimeout
// Gate A/B flight console: set true (or ?logflights=1) to trace TAKEOFF/LANDING/SKIP.
var glogflights

//various images
var gimagetheme = '../../exodus/images/theme2/'
/*
 * Icon values are either:
 *   string URL  — painted multicolour SVG (New/Edit/Delete, theme, …)
 *   { mask, color } — black monochrome SVG, tinted via CSS --exodus-icon-*
 * Colour names: green, red, orange, blue, darkgrey, lightgrey, neutral
 */
function exodus_icon_spec(mask, color) {
	return { mask: mask, color: color || 'darkgrey' }
}
function exodus_is_icon_spec(x) {
	return !!(x && typeof x == 'object' && x.mask)
}

var gmenuimage = exodus_icon_spec('shell-menu.svg', 'darkgrey')
var glogoutimage = exodus_icon_spec('shell-logout.svg', 'red')
var gloginimage = exodus_icon_spec('shell-login.svg', 'green')
var grefreshimage = exodus_icon_spec('shell-refresh.svg', 'green')
var gprintimage = exodus_icon_spec('file-print.svg', 'darkgrey')
var gthemeimage = gimagetheme + 'shell-theme_lm.svg' // painted sun/moon chrome
// company icon uses a patterned SVG — keep painted (LM/DM twins)
var gcompanyimage = gimagetheme + 'shell-company_lm.svg'

var gisdarktheme
// LM/DM preference cookie — global per browser (not glogincode / dataset / user)
var gthemecookiekey = 'EXODUStheme'

// First paint: apply EXODUStheme/dt before the rest of this file parses and before
// body HTML is reached. Hard refresh otherwise flashes browser-default white
// (especially with OS + Exodus both in night mode). Full theme_toggle runs later.
;(function exodus_theme_firstpaint() {
	try {
		var dark = false
		var raw = document.cookie || ''
		var cookies = raw.split('; ')
		for (var i = 0; i < cookies.length; i++) {
			var eq = cookies[i].indexOf('=')
			if (eq < 0)
				continue
			var name = cookies[i].slice(0, eq)
			if (name !== 'EXODUStheme' && name !== gthemecookiekey)
				continue
			var crumbs = cookies[i].slice(eq + 1).split('&')
			for (var j = 0; j < crumbs.length; j++) {
				var kv = crumbs[j].split('=')
				if (kv[0] === 'dt' && kv[1] && kv[1] !== '0') {
					dark = true
					break
				}
			}
			break
		}
		if (!dark)
			return
		var html = document.documentElement
		html.setAttribute('data-theme', 'dark_mode')
		html.style.colorScheme = 'dark'
		html.style.background = '#000'
		// Style block before body exists — stops white canvas on hard refresh
		var st = document.createElement('style')
		st.id = 'exodus_dm_firstpaint'
		st.textContent = '@media screen{'
			+ 'html[data-theme=dark_mode],html[data-theme=dark_mode] body{background:#000!important;color:#fff}'
			+ '}'
		var head = document.head || document.getElementsByTagName('head')[0]
		if (head)
			head.insertBefore(st, head.firstChild)
	} catch (e) { }
})()

function exodus_sortimage(order) {

	// Unsorted chevron: lightgrey (secondary). Active up/down: text colour.
	var name = (order == 'up') ? 'sort-up.svg' : (order == 'down') ? 'sort-down.svg' : 'sort.svg'
	var color = (order == 'up' || order == 'down') ? 'darkgrey' : 'lightgrey'
	return exodus_icon_spec(name, color)
}

function exodus_sort_order_of(el) {
	if (!el)
		return ''
	var d = el.getAttribute && el.getAttribute('data-sort-order')
	if (d === 'up' || d === 'down')
		return d
	if (d === '')
		return ''
	var blob = (el.src || '')
	try {
		if (el.style)
			blob += ' ' + (el.style.maskImage || '') + ' ' + (el.style.webkitMaskImage || '')
	} catch (e) { }
	if (blob.indexOf('sort-up') >= 0)
		return 'up'
	if (blob.indexOf('sort-down') >= 0)
		return 'down'
	return ''
}

function exodus_apply_sort_icon(el, order) {
	el = exodus_set_icon_element(el, exodus_sortimage(order))
	if (el && el.setAttribute)
		el.setAttribute('data-sort-order', order || '')
	return el
}

function exodus_refresh_sortimages() {

	document.querySelectorAll('.exodus-icon[id^="sortbutton_"], img[id^="sortbutton_"]').forEach(function (el) {
		exodus_apply_sort_icon(el, exodus_sort_order_of(el))
	})
}

var gcache

// Client READ cache (gcache.values — one reader/writer around db.send):
//   undefined  → not in cache → network
//   null       → known NO RECORD → fail without network
//   string     → body ('' = empty existing record, not a miss)
// null is only stored on .values (object store). Attribute store is legacy/string-only.

// Apply a cache hit for db.send. Returns undefined if not in cache;
// true = data hit; false = known NO RECORD (sets response).
function dblink_cache_apply(self, request2) {
	var temp = readcache(request2)
	if (typeof temp == 'undefined')
		return undefined
	if (temp === null) {
		self.data = ''
		self.response = 'Error: NO RECORD'
		return false
	}
	self.data = temp
	self.response = 'OK'
	return true
}

// Store NO RECORD only for READ-like keys (and CACHE\r callers).
function dblink_cache_store_norecord(request2, trycache) {
	if (!(trycache || (request2 && request2.slice(0, 4) == 'READ')))
		return
	if (!request2 || request2.indexOf('READ\r') != 0)
		return
	writecache(request2, null)
}

var glogincode
var gDialogArguments//similar to window.dialogArguments

var gusername
var gdataset
var gsystem

var gcompanycode
var gperiod
var gbasecurr
var gbasefmt
var gthousands_regex
var gmarketcode
var gmaincurrcode
var gdatasetname
var gtz = [0, 0]
gtz[0] = new Date().getTimezoneOffset() * -60

var gnpendingscripts = 0

// Filled in exodus_client_init from modal bag (and URL query). Prefer this over dialogArguments.
var gparameters

//var grecn should only be used in dbform.js. used here in ICONV special case

//Revelation/Pick field separator characters
var rm, fm, vm, sm, tm, stm
var STMre, TMre, SMre, VMre, FMre, RMre, ALLFMre
var XMLXXre
var FMs

var e//mac safari 3.1.2 cannot tolerate "catch(e)" without this

//prevent framing?
//if (window != top)
//	top.location.href = location.href

var gkeepalivemins = 10
//gkeepalivemins=1
var gkeepalive_timer = null

// Stage 6: no live function*/yield* in app code — only async/await + Gate A/B.
// A repo-wide yield* count is almost all *commented* legacy (and docs/error strings).
// Do not re-audit those as active generators. This regex is only for stripping old
// 'yield* ' from dynamic strings (settimeout / functioncode attrs).
var gyieldregex = /yield ?\*/g

exodus_client_init()

//any global variable defined in this function must not of course be declare var here otherwise would be local function variables
function exodus_client_init() {

	// Browser zoom keys: sync capture, out of form/Gate A key path (see client.js)
	exodus_ensure_browser_chrome_keydown()

	// swapNode polyfill — still used by sort + ledgerprint
	if (!document.swapNode) {
		Node.prototype.swapNode = function (node) {
			var p = node.parentNode;
			var s = node.nextSibling;
			this.parentNode.replaceChild(node, this);
			p.insertBefore(this, s);
			return this;
		}
	}

	// $$ is an Exodus DOM helper (not a browser polyfill).
	// Primary lookup: class exodusid_<id> (so multi-row/multi-instance fields share one logical id).
	// Return shape (legacy document.all-like): one match → element; many → collection; none → getElementById then undefined.
	// getElementById alone does not match this: it always returns at most one node even when duplicate ids exist.
	// Required by login/dbform/etc. — do not drop or replace with plain getElementById.
	$$ = $class

	document.protocolcode = document.location.toString().slice(0, 4)

	//global constants for revelation high end separator characters

	//new style flexible but chose unusual characters not likely to be required by clients and yet visible for debugging
	//characters can be changed if required by require a change in the http server message handler too

	//c0,c1, f5-ff do not occur in utf8 at all.
	//see http://www.ifi.unizh.ch/mml/mduerst/papers/PDF/IUC11-UTF-8.pdf for graphical layout of utf8
	//00-7f means one ascii byte
	//80-BF means is a trailing byte
	//c0-c1 is illegal
	//c2-df is a two byte sequence first byte
	//e0-ef is a three byte sequence first byte
	//f0-f4 is a four byte sequence first byte
	//c0,c1, f5-ff do not occur in utf8 at all.
	//0xxxxxxx (US-ASCII)
	//110xxxxx 10xxxxxx
	//1110xxxx 10xxxxxx 10xxxxxx
	//11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	//111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	//1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

	//using NKO code page 07FA-07FF for our 6 field separator characters

	//prestored global regular expressions for speed
	//STMre = /\u07FA/g
	//TMre = /\u07FB/g
	//SMre = /\u07FC/g
	//VMre = /\u07FD/g
	//FMre = /\u07FE/g
	//RMre = /\u07FF/g
	//ALLFMre = /[\u07FA\u07FB\u07FC\u07FD\u07FE\u07FF]/g
	STMre = /\x1A/g
	TMre = /\x1B/g
	SMre = /\x1C/g
	VMre = /\x1D/g
	FMre = /\x1E/g
	RMre = /\x1F/g
	ALLFMre = /[\x1A\x1B\x1C\x1D\x1E\x1F]/g

	//\x0A-\x0F in the back end
	//(no longer) also block character F8 and F9
	//F9 is used as a field mark in DOS eg PRINT ("XXX":\F9\:"YYY") 'L#20' ... formats both to 20 characters
	//also block character F8 since to make logical block of eight field mark characters
	//XMLXXre = /([\x25\x3C\x3E\x26\u07FF\u07FE\u07FD\u07FC\u07FB\u07FA\u07F9\u07F8])/g
	//FMs = '\u07FF\u07FE\u07FD\u07FC\u07FB\u07FA\u07F9\u07F8'
	//XMLXXre = /([\x25\x3C\x3E\x26\u07FF\u07FE\u07FD\u07FC\u07FB\u07FA])/g
	//FMs = '\u07FF\u07FE\u07FD\u07FC\u07FB\u07FA'
	XMLXXre = /([\x25\x3C\x3E\x26\x1F\x1E\x1D\x1C\x1B\x1A])/g
	FMs = '\x1F\x1E\x1D\x1C\x1B\x1A'

	//calculate all the global field separator character variables
	rm = FMs[0]
	fm = FMs[1]
	vm = FMs[2]
	sm = FMs[3]
	tm = FMs[4]
	stm = FMs[5]

	// Modal dialog args (if this window is a modal child). See
	// exodus_acquire_modal_dialog_arguments / exoui_showmodaldialog.
	// Page code should read gparameters; window.dialogArguments is rebound for legacy.
	gDialogArguments = exodus_acquire_modal_dialog_arguments()
	if (gDialogArguments)
		window.dialogArguments = gDialogArguments

	gisdialog = !!gDialogArguments

	if (gDialogArguments && gDialogArguments.logincode) {
		glogincode = gDialogArguments.logincode
		//alert('debug client.js glogincode=gDialogArguments.logincode '+gDialogArguments.logincode)
	}

	//can extract cookied immediately
	if (!glogincode) glogincode = ''
	if (!glogincode) {
		glogincode = exodusgetcookie2('logincode', 'EXODUSlogincode')
	}
	if (!gdataset) gdataset = ''
	if (!gdataset) {
		gdataset = exodusgetcookie2('dataset')
	}
	setdateformat()

	//gexodus_server = gdataset.toLowerCase() == gdataset
	gexodus_server = typeof exodusread != 'undefined'

	if (typeof gparameters == 'undefined')
		gparameters = new Object
	if (gDialogArguments) {
		for (var param in gDialogArguments)
			gparameters[param] = gDialogArguments[param]
	}
	if (gparameters.gtasks && !gtasks)
		gtasks = gparameters.gtasks

	//add gparameters from URL if present eg. neosys.com/xyz.htm?param1=1&param2=2 etc.
	var temp = window.location.toString()
	if (temp.indexOf('?') >= 0) {
		//temp = unescape(temp.slice(temp.indexOf('?') + 1)).split('&')
		temp = unescape(temp.slice(temp.indexOf('?') + 1)).replace(/\?/g, '&').split('&')
		for (i = 0; i < temp.length; i++)
			gparameters[temp[i].split('=')[0]] = temp[i].split('=').slice(1).join('=')
	}
	if (typeof gparameters.savemode == 'undefined')
		gparameters.savemode = gparameters.SAVEMODE

	//file access functions
	if (document.protocolcode == 'file') {
		document.writeln('<scr' + 'ipt type="text/javascript" src="' + EXODUSlocation + 'scripts/server.js"></scr' + 'ipt>')
	}

	// Pre-emptively set CSS screen theme to avoid flash of switching from
	// default theme (light or user color) mode to dark theme if dark theme is active
	if (document.title.toUpperCase() == 'EXODUS LOGIN') {
		var login_theme = function () {
			return window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark_mode' : 'default'
		}
		theme_toggle(login_theme())
		if (window.matchMedia)
			window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', function () { theme_toggle(login_theme()) })
	} else {
		theme_toggle(exodusgetcookie2('dt', gthemecookiekey, null) ? 'dark_mode' : 'default')
	}

	// Before global.css: page/form body only. No field color/border !important —
	// that stays after load and blocks focus underline and miss Highlight.
	if (gisdarktheme) {
		// @media screen only — must not win over @media print (white paper + light text = blank)
		document.writeln('<style id="exodus_dm_flashguard">'
			+ '@media screen{'
			+ ':root[data-theme=dark_mode],:root[data-theme=dark_mode] BODY{background:#000!important;color:#fff}'
			// No TABLE.exodusform body fill — pane chrome + .exodata only (global.css)
			+ '}'
			+ '</style>')
	}

	//style sheet
	document.writeln('<link id="exodus_global_css" rel="stylesheet" type="text/css" href="' + EXODUSlocation + 'global.css">')

	//general functions
	if (!($$('generalfunctions'))) {
		document.writeln('<scr' + 'ipt type="text/javascript" id=generalfunctions src="' + EXODUSlocation + 'scripts/exodus.js"></scr' + 'ipt>')
	}

	//form functions
	if (typeof gdatafilename != 'undefined' || typeof gdictfilename != 'undefined') {
		document.writeln('<scr' + 'ipt type="text/javascript" src="' + EXODUSlocation + 'scripts/db.js"></scr' + 'ipt>')
		document.writeln('<scr' + 'ipt type="text/javascript" src="' + EXODUSlocation + 'scripts/gds.js"></scr' + 'ipt>')
		document.writeln('<scr' + 'ipt type="text/javascript" src="' + EXODUSlocation + 'scripts/dbform.js"></scr' + 'ipt>')
	}

	//if (!window.onload)
	//save any custom onload function to be executed by clientfunctions_windowonload when it finishes
	gwindowonload = window.onload
	//window.onload = clientfunctions_windowonload
	window.onload = null
	addeventlistener(window, 'load', 'clientfunctions_windowonload')
	//addeventlistener(window,'unload','clientfunctions_windowonunload_sync')

	//ensure print preview styles are set during print/preview
	//window.onbeforeprint = window_onbeforeprint
	addeventlistener(window, 'beforeprint', 'window_onbeforeprint')

	//save location except if logging in
	if (typeof gnosavelocation == 'undefined' && !window.dialogArguments && EXODUSlocation != './exodus/') {
		//if (gdataset) exodussetcookie('','EXODUS',escape(location),'ll',true)
	}

	glogging = false
	gstepping = 0
	// Flight console off by default; enable with ?logflights=1 or glogflights=true in console.
	if (typeof glogflights == 'undefined' || glogflights == null) {
		glogflights = false
		try {
			if (location.search && location.search.indexOf('logflights=1') >= 0)
				glogflights = true
		} catch (e) { }
	}

	//ensure http session is kept alive (optional — skip if Gate A busy; never queue)
	exodus_start_keepalive()

	loadcache()

}

//end of main initialisation.
//what follows should be functions and their global variables only

async function window_onbeforeprint() {
	await clientfunctions_setstyle()
}

async function exoduslogout_onclick() {

	//cancel any automatic login
	exodussetcookie('', 'EXODUS', '', 'a')

	//decide where to login again
	var newwindowlocation = '../index.html'
	var system = exodusgetcookie2('', 'EXODUSsystem', null)
	if (system && system != 'ADAGENCY')
		newwindowlocation += '??' + system

	//clear various session variable
	await dblogout()

	//switch to login window
	try {
		window.location.assign(newwindowlocation)
	}
	catch (e) { }

	return false

}

var gmsg
var gtasks
async function exoui_security(task) {

	//return empty gmsg if authorised
	gmsg = ''

	//look for ancient source code
	if (task.indexOf(' FILE ') >= 0)
		await exoui_warning('FILE should not be in task ' + task)

	//make sure task list is loaded (clearing cache also clears gtasks for convenience)
	if (!gtasks) {
		db.request = 'CACHE\rEXECUTE\rGENERAL\rGETTASKS\rNOT'
		if (!(await db.send())) {
			gtasks = ''
			gmsg = db.response
			await exoui_invalid(gmsg)
			return false
		}
		gtasks = db.data.split(fm)[0].split(vm)

		//expand leading + signs
		for (var i = 1; i < gtasks.length; i++) {
			if (gtasks[i].match(/(^\++)/)) {
				gtasks[i] = gtasks[i].replace(/(^\++)/, gtasks[i - 1].split(' ').slice(0, RegExp.$1.length).join(' ') + ' ')
			}
		}

	}

	//fail if task not allowed
	//if (task.slice(0,3)=='!!!'||gtasks.exoduslocate(task)||(gstepping&&!(confirm(task))))
	if (task.slice(0, 3) == '!!!' || gtasks.exoduslocate(task)) {
		gmsg = 'Sorry ' + gusername.exoduscapitalise() + ', you are not authorised to do\r' + task.exoduscapitalise()
		return false
	}

	return true

}

// Start/restart keepalive interval from current gkeepalivemins.
// Call after changing gkeepalivemins in the console: gkeepalivemins=0.1; exodus_start_keepalive()
function exodus_start_keepalive() {

	if (gkeepalive_timer) {
		window.clearInterval(gkeepalive_timer)
		gkeepalive_timer = null
	}

	if (document.protocolcode == 'file' || !gkeepalivemins)
		return

	var ms = gkeepalivemins * 60 * 1000
	if (ms < 1000)
		ms = 1000

	exodus_flight_log(
		'keepalive interval ' + ms + 'ms (gkeepalivemins=' + gkeepalivemins + ')'
	)

	gkeepalive_timer = window.setInterval(function () {
		void exodus_begin_if_idle(sessionkeepalive, 'sessionkeepalive')
	}, ms)

}

async function sessionkeepalive() {

	//last connection (updated on every db.send — so active forms rarely need a pure KEEPALIVE)
	var lastconnection = exodusgetcookie2('lc', 'EXODUSlc', '')
	if (lastconnection == 'undefined')
		lastconnection = ''
	lc = lastconnection
	lastconnection = lastconnection ? new Date(lastconnection) : lastconnection = new Date(0)

	//next connection
	var nextconnection = lastconnection
	nextconnection.setMilliseconds(lastconnection.getMilliseconds() + (gkeepalivemins * 60 * 1000))

	//call server if time to keep alive
	var time = new Date
	if (time >= nextconnection && typeof db != 'undefined') {
		var tempdb = new exodusdblink()
		tempdb.request = 'KEEPALIVE'
		await tempdb.send()
		exodus_flight_log('keepalive SEND')
		console.log(time + ' Keep Alive')
	} else {
		exodus_flight_log(
			'keepalive tick not due yet (next '
			+ nextconnection.toISOString() + ', gkeepalivemins=' + gkeepalivemins + ')'
		)
	}

}

function exodussetexpression(elementsorelementid, attributename, expression) {

	//check element exists
	if (!elementsorelementid) {
		void exodus_begin(function () {
			return exoui_invalid('missing element in exodussetexpression ' + attributename + ' ' + expression)
		}, 'exodussetexpression missing element')
		return
	}

	//elements can be elementnames too

	//get an array of elements
	var elements
	if (typeof elementsorelementid == 'string') {
		elements = document.getElementsByName(elementsorelementid)
		if (!elements.length) {
			elements = ''
			elements = $$(elementsorelementid)
			if (!elements)
				try {
					elements = eval(elementsorelementid)
				}
				catch (e) { }
			if (!elements)
				return
			// TODO: multi-element $$ result (collection) if many matches
			elements = [elements]
		}
	}
	else {
		elements = elementsorelementid
		//make sure is an array
		if (!elements.length) elements = [elements]
	}

	//set the attribute expression for all elements
	var style = attributename.slice(0, 6) == 'style:'
	var attributepart = style ? attributename.slice(6) : attributename
	for (var ii = 0; ii < elements.length; ii++) {
		if (typeof elements[ii] == 'string')
			exodussetexpression(elements[ii], attributename, expression)
		else {

			// some engines leave a stub that throws when used
			try {
				// force error if missing or unusable stub
				if (elements[ii].setExpression) {
					var element = elements[ii]
					if (style)
						element = element.style
					element.setExpression(attributepart, expression)
					continue;
				}
			}
			catch (e) {
				//TODO replace setexpression with some kind of callback events
			}

		}
	}

}

var gsetexpressioninterval = []
function exodussetexpression2(elementids, attributename, expression) {

	var elementids2
	if (typeof elementids == 'string')
		elementids2 = elementids.split(',')
	else
		elementids2 = elementids

	var allelements = []
	for (var ii = 0; ii < elementids2.length; ++ii) {
		var elementid = elementids2[ii]

		var elements = document.getElementsByName(elementid)
		if (!elements.length) {
			elements = $$(elementid)
			if (!elements)
				try {
					elements = eval(elementid)
				}
				catch (e) { }
			if (!elements)
				continue
			// $$() returns a collection only when more than one match
			if (!elements.length)
				elements = [elements]
		}
		// rearray so $$ multi-match collection is a real array (concat-safe)
		allelements = allelements.concat(rearray(elements))
	}

	var style = attributename.slice(0, 6) == 'style:'
	var attributepart = style ? attributename.slice(6) : attributename

	exodussetexpression2b(elementid, allelements, style, attributepart, expression)

}

function exodussetexpression2b(expressionid, elements, style, attributename, expression) {

	gsetexpressioninterval[expressionid] = exodussetinterval(

		//build a closure containing all the elements to be updated
		//and to be called at intervals
		function anon_from_exodussetexpression2b() {
			// Interval tick: Gate A only when idle (skip if busy — expression UI is optional).
			void exodus_begin_if_idle(function () {
				return exodussetexpression2c(elements, style, attributename, expression)
			}, 'exodussetexpression2c')
		}
		, 250)//every quarter second
}

//this is called at intervals
async function exodussetexpression2c(elements, style, attributename, expression) {
	//set the attribute expression for all elements
	var result = await exodusevaluate(expression)
	//console.log(expression+' is '+result+')
	if (attributename == 'disabled')
		1 == 1
	for (var ii = 0; ii < elements.length; ii++) {
		var element = elements[ii]
		if (style)
			element = element.style
		element[attributename] = result
	}

}

function exodusenabledandvisible(element0, allowreadonly) {
	var element = element0
	if (typeof element == 'string') {
		element = document.getElementsByName(element0)[0]
		if (!element) {
			element = document.getElementById(element0)
			if (!element) {
				element = $$(element0)
				if (!element) {
					systemerror('exodusenabledandvisible() cannot getElementsByName ' + element0)
				}
			}
		}
	}
	if (!allowreadonly && element.getAttribute('exoreadonly'))
		return false

	if (!element.offsetWidth || element.style.display == 'none')
		return false

	// Hidden ancestor (display:none) — not focusable
	var p = element.parentNode
	while (p && p.tagName != 'BODY') {
		if (p.style && p.style.display == 'none')
			return false
		if (p.getAttribute && p.getAttribute('disabled'))
			return false
		p = p.parentNode
	}

	return true
}

function showhide(element, show) {

	var elementid = element

	if (typeof element == 'string') {
		element = $$(element)
		if (!element) {
			// Try to get elements by class name
			var elements = document.querySelectorAll('.' + elementid)
			if (elements.length == 0)
				systemerror('showhide("' + elementid + '")', ' window element does not exist')
			element = elements
		}
	}

	//recursive
	//if (typeof element=='object'&&!element.tagName)
	if ((typeof element == 'object' || typeof element == 'function') && !element.tagName) {
		for (var ii = 0; ii < element.length; ii++)
			showhide(element[ii], show)
		return
	}

	if (element.style) {
		if (!show)
			element.style.display = 'none'
		else {
			// use '' — 'inline' misaligns columns; display:table-row is widely supported now
			element.style.display = ''
		}
	}
	return true
}

function insertafter(element, newelement) {
	if (element.nextSibling)
		return element.parentNode.insertBefore(newelement, element.nextSibling)
	return element.parentNode.insertBefore(newelement, null)
}

function setinnerHTML(elementsorid, html) {
	if (typeof elementsorid == 'string') {
		elementsorid = $$(elementsorid)
		if (elementsorid.tagName) {
			elementsorid.innerHTML = html
			return
		}
	}
	for (var i = 0; i < elementsorid.length; i++)
		elementsorid[i].innerHTML = html
}

function loguiblockerwaitcancel_event(event, action) {

	event = getevent(event)
	var target = event.target
	var msg = 'uiblocker Wait/Cancel: ' + action
	if (event.type)
		msg += ' type=' + event.type
	if (typeof event.isTrusted != 'undefined')
		msg += ' isTrusted=' + event.isTrusted
	if (typeof event.detail != 'undefined')
		msg += ' detail=' + event.detail
	if (event.timeStamp)
		msg += ' timeStamp=' + event.timeStamp
	if (typeof event.clientX != 'undefined')
		msg += ' clientXY=' + event.clientX + ',' + event.clientY
	if (target) {
		msg += ' target=' + (target.tagName || '')
		if (target.id)
			msg += '#' + target.id
	}
	console.log(msg)
	logevent(msg)

}

var gprocessing_waitcancel_active

// Gate B — in-DOM Wait/Cancel while Gate A (or any owner) is in db.send lazy XHR wait.
// Concurrent with Gate A by design. Must NEVER call exodus_begin or main-line dbio.
// Allowed: exoui_confirm UI, xhttp.abort(), fire-and-forget CANCEL on a separate link.
var g_exodus_waitcancel = null
var g_exodus_waitcancel_n = 0

// PHP-FPM: XHR abort is not reliably seen by xhttp.php — send an explicit CANCEL request too.
function dbsend_cancel_xhttp(requestid) {

	// CANCEL with request id, or bare CANCEL (xhttp resolves id from session _active_xhttp).
	// Separate exodusdblink + fire-and-forget: must not use main db or enter Gate A.
	var canceldb = new exodusdblink()
	canceldb.request = requestid ? ('CANCEL\r' + requestid) : 'CANCEL'
	exodusfireandforget(canceldb.send(), 'dbsend_cancel_xhttp')
}

// Release per-request modal wait state owned by db.send (not KEEPALIVE/RELOCK/CANCEL).
function dbsend_release_modal(xhttp, dbmodalblocked) {

	if (!dbmodalblocked)
		return

	if (gchildwin && gchildwin.lazy && gchildwin.xhttp === xhttp) {
		gchildwin.xhttprequestid = false
		gchildwin = false
	}

	// Close Wait/Cancel confirm only if it owns the pending resolver (Gate B).
	// Never resolve a Gate A business confirm from db.send completion.
	if ($$('exodusconfirmdiv') && gpendingConfirmResolve)
		resolvePendingConfirm(1, 'db.send complete', 'B')

	unblockmodalui_sync()

}

// PUBLIC ASYNC COMMENCEMENT API (frozen — stage 2)
// =================================================
// Only these three start concurrent async work from the framework:
//   exodus_begin(asyncFn, label[, event])     — Gate A business (exclusive)
//   exodus_begin_if_idle(asyncFn, label)      — optional background; skip if busy
//   exodus_begin_waitcancel(source)           — Gate B Wait/Cancel only
// Everything else (starteventhandler, *_sync bridges, timeouts) must funnel
// into one of these. Do not add a fourth commencement path.
//
// Deferral helper (not a fourth gate — always ends in exodus_begin):
//   exodus_begin_when_idle(fn, label[, { delay_ms, max_wait_ms }])
// Use for post-land work (opendoc next key, popup → openrecord, DATE peer setx).
// Retries until Gate A is free; systemerror if still busy after max_wait_ms.
// Prefer this over one-shot setTimeout→exodus_begin (silent SKIP with queue_max 0).
//
// Do NOT convert historical setTimeout post-open hooks into same-flight await
// from form_postread — that runs before gds.load. Prefer form_postdisplay.
//
// Fail loud: required Gate A takeoff that cannot run (busy + full wait list)
// calls systemerror. Optional work uses exodus_begin_if_idle (silent skip OK).
//
// Gate B entry — only from uiblocker while a lazy db.send wait is active.
// Does not go through exodus_begin (Gate A is airborne during db.send; Gate B must run now).
function exodus_begin_waitcancel(source) {

	source = source || 'uiblocker'

	if (g_exodus_waitcancel || gprocessing_waitcancel_active) {
		exodus_flight_log(
			'WAITCANCEL ignored (already open'
			+ (g_exodus_waitcancel ? ' B#' + g_exodus_waitcancel.n : '') + ')'
		)
		return
	}

	if ($$('exodusconfirmdiv')) {
		exodus_flight_log('WAITCANCEL ignored (other confirm up)')
		return
	}

	// Operational condition: modal db.send wait (gchildwin.lazy), not "Gate A only".
	// Prefer A airborne; if idle, still allow (db.send may still bypass A via timeouts).
	if (!gchildwin || !gchildwin.lazy || !gchildwin.xhttp) {
		exodus_flight_log('WAITCANCEL ignored (no lazy db.send wait)')
		return
	}

	if (!g_exodus_flow)
		exodus_flight_log('WAITCANCEL while Gate A idle (db.send outside exodus_begin?)')

	// Fire-and-forget second stack — intentional dual-stack with Gate A.
	void exodus_run_waitcancel(source)
}

async function exodus_run_waitcancel(source) {

	var n = ++g_exodus_waitcancel_n
	g_exodus_waitcancel = { n: n, source: source }
	gprocessing_waitcancel_active = true

	var ainfo = g_exodus_flow
		? ('A#' + g_exodus_flow.n + ' "' + g_exodus_flow.location + '"')
		: 'A idle'
	exodus_flight_log('WAITCANCEL OPEN B#' + n + ' (' + ainfo + ', via ' + source + ')')

	// Snapshot XHR for this wait — gchildwin may clear when the request completes.
	var xhttp = gchildwin.xhttp
	var requestid = gchildwin.xhttprequestid

	try {
		var response = await exoui_confirm('Processing. Please wait.', 1, 'Wait', 'Cancel')

		// Wait=1: keep request running (also when dbsend_release_modal force-resolves on complete)
		if (response != 1) {
			if (gchildwin && gchildwin.xhttprequestid)
				requestid = gchildwin.xhttprequestid
			exodus_flight_log('WAITCANCEL CANCEL B#' + n)
			try {
				xhttp.abort()
			} catch (e) { }
			dbsend_cancel_xhttp(requestid)
			// db.send releases modal state when the XHR abort completes.
		} else {
			exodus_flight_log('WAITCANCEL WAIT/COMPLETE B#' + n)
		}
	} catch (e) {
		exodus_flight_log(
			'WAITCANCEL error B#' + n + ': ' + (e && (e.message || e.description || e))
		)
		throw e
	} finally {
		gprocessing_waitcancel_active = false
		g_exodus_waitcancel = null
		exodus_flight_log('WAITCANCEL CLOSE B#' + n)
	}
}

// Legacy name — blocker and any old callers enter Gate B only.
function uiblocker_waitcancel_dialog() {
	exodus_begin_waitcancel('uiblocker_waitcancel_dialog')
}

var gmodalblockdepth = 0
// Body pin while modal is open: { position, top, left, width, x, y }
var gmodalblock_pin = null
var gmodalblock_capturebound
// Set while a decide list is open; wheel over options moves selection/focus (classic list).
var gdecide_onwheel = null
// Ctrl+A / context-menu Select All → full list selection; copy uses decide_list_tsv().
var gdecide_select_all = false
var gdecide_ctx_menu = false
var gdecide_ctx_timer = null

function modalblock_scrollpane_under(event) {

	var confirm = $$('exodusconfirmdiv')
	if (!confirm || !confirm.contains(event.target))
		return null

	var scrollpane = confirm.querySelector('.exodusconfirm_body')
	if (!scrollpane || !(scrollpane === event.target || scrollpane.contains(event.target)))
		return null

	return scrollpane

}

function modalblock_onwheel(event) {

	if (gmodalblockdepth <= 0)
		return

	// Ctrl/Cmd+wheel is browser zoom (same as Ctrl+/- keys). Never steal it.
	// After zoom, decide popups reflow via visualViewport resize (see fit_decide).
	if (event.ctrlKey || event.metaKey)
		return

	// Decide list: wheel = next/prev option (radio checks; multi only moves focus).
	// Runs before body scroll so the list behaves like a classic listbox, not a free scroller.
	if (typeof gdecide_onwheel == 'function' && gdecide_onwheel(event)) {
		event.preventDefault()
		if (event.stopPropagation)
			event.stopPropagation()
		return
	}

	var scrollpane = modalblock_scrollpane_under(event)
	if (scrollpane) {
		var delta = event.deltaY
		if (!delta)
			return
		var atTop = scrollpane.scrollTop <= 0
		var atBottom = scrollpane.scrollTop + scrollpane.clientHeight >= scrollpane.scrollHeight - 1
		// Edge of scrollable body: stop page behind from scrolling
		if ((delta < 0 && atTop) || (delta > 0 && atBottom))
			event.preventDefault()
		return
	}

	// Wheel over modal chrome / shield: block page scroll only
	event.preventDefault()

}

function modalblock_ontouchmove(event) {

	if (gmodalblockdepth <= 0)
		return

	if (modalblock_scrollpane_under(event))
		return

	event.preventDefault()

}

function modalblock_bind_capture() {

	if (gmodalblock_capturebound)
		return

	document.addEventListener('wheel', modalblock_onwheel, { capture: true, passive: false })
	document.addEventListener('touchmove', modalblock_ontouchmove, { capture: true, passive: false })
	gmodalblock_capturebound = true

}

function modalblock_unbind_capture() {

	if (!gmodalblock_capturebound)
		return

	document.removeEventListener('wheel', modalblock_onwheel, { capture: true })
	document.removeEventListener('touchmove', modalblock_ontouchmove, { capture: true })
	gmodalblock_capturebound = false

}

function modalblock_create() {

	//YIELD//console.log('BLOCKING UI')

	var blocker = document.createElement('div')
	blocker.style.width = '100vw'
	blocker.style.height = '100vh'
	blocker.style.background = gisdarktheme
		? 'rgba(0, 0, 0, 0.08)'//near-transparent tint, matches --exodus-page-bg-color
		: 'rgba(255,255,255,0.25)'//white overlay with only 25% opacity
	blocker.style.position = 'fixed'
	blocker.style.top = '0'
	blocker.style.left = '0'
	blocker.style.zIndex = '1000'//above #exodus_menu (999), below .exodusconfirmdiv (1001)
	blocker.style.pointerEvents = 'auto'
	blocker.id = 'uiblockerdiv'

	document.body.insertBefore(blocker, null)

	// Gate A mounts this on non-focus flights. Focus/activate skips it (native
	// <select> would close under the overlay).
	// Scroll hold: pin body position:fixed at -scroll (overflow:hidden jumps to 0,0).
	// Width = clientWidth px only — not 100%+left+right (that painted a right white band).
	// Wheel/touch capture still blocks gestures under the shield.
	modalblock_pin_body()
	modalblock_bind_capture()

	//keep focus off parent window and on child window or exodusdiv
	var guiblockermousedown = false
	blocker.onmousedown = function uiblockerdiv_onmousedown() {
		guiblockermousedown = true
	}
	blocker.onclick = function uiblockerdiv_onclick_sync(event) {

		if ($$('exodusconfirmdiv')) {
			window.setTimeout(exodus_confirm_outside_click_sync, 10)
		}
		// Date picker: click outside (on modal shield) dismisses without commit
		else if (typeof calendar_checkInDatePicker != 'undefined' && calendar_checkInDatePicker
			&& calendar_checkInDatePicker._showing) {
			calendar_checkInDatePicker.hide()
		}
		// Colour popup: kill OS spectrum immediately (sync) — do not queue on Gate A
		else if (typeof colors_popup != 'undefined' && colors_popup && colors_popup._showing
			&& typeof colors_popup_cancel == 'function') {
			void colors_popup_cancel()
		}
		else if (gchildwin) {
			if (gchildwin.lazy) {
				//ignore spurious click events without a prior mousedown on this blocker
				//eg when a child popup closes after selection and the click falls through
				if (!guiblockermousedown) {
					loguiblockerwaitcancel_event(event, 'ignored (no mousedown on blocker)')
					return
				}
				guiblockermousedown = false
				loguiblockerwaitcancel_event(event, 'opening in-dom wait/cancel')
				exodus_begin_waitcancel('uiblockerdiv')
			} else {
				var actualwin = gchildwin

				//focus on child window
				window.setTimeout(function () { try { actualwin.focus() } catch (e) { } }, 10)

				//also focus on child's child recursively
				var winuiblocker = actualwin.document.getElementById('uiblockerdiv')
				if (winuiblocker)
					winuiblocker.click()
			}
		}
	}

}

function modalblock_destroy() {

	// Orphaned confirm while last modal layer tears down — resolve by owner only.
	// Gate B Wait/Cancel → Wait/complete (1). Gate A business → Cancel (0), never auto-OK.
	if ($$('exodusconfirmdiv') && gpendingConfirmResolve) {
		if (gpendingConfirmOwner === 'B')
			resolvePendingConfirm(1, 'modalblock_destroy', 'B')
		else if (gpendingConfirmOwner === 'A') {
			exodus_flight_log('CONFIRM orphan A force-cancel on modal destroy')
			resolvePendingConfirm(0, 'modalblock_destroy', 'A')
		} else
			resolvePendingConfirm(0, 'modalblock_destroy')
	}

	var blocker = $$('uiblockerdiv')
	if (blocker) {
		//YIELD//console.log('UNBLOCKING UI')
		exodusremovenode(blocker)
		//console.log('parent window ui unblocked')
	}

	modalblock_unpin_body()
	modalblock_unbind_capture()

}

// Pin body so the form stays visually at (sx,sy) while popup/db wait is up.
// window.scrollY may read 0 while pinned — restore real scroll on unpin.
function modalblock_pin_body() {
	var sx = window.pageXOffset || document.documentElement.scrollLeft || 0
	var sy = window.pageYOffset || document.documentElement.scrollTop || 0
	var layoutW = document.documentElement.clientWidth
	gmodalblock_pin = {
		position: document.body.style.position,
		top: document.body.style.top,
		left: document.body.style.left,
		width: document.body.style.width,
		x: sx,
		y: sy
	}
	document.body.style.position = 'fixed'
	document.body.style.top = (-sy) + 'px'
	document.body.style.left = (-sx) + 'px'
	document.body.style.width = layoutW + 'px'
}

function modalblock_unpin_body() {
	if (!gmodalblock_pin)
		return
	var sx = gmodalblock_pin.x || 0
	var sy = gmodalblock_pin.y || 0
	document.body.style.position = gmodalblock_pin.position
	document.body.style.top = gmodalblock_pin.top
	document.body.style.left = gmodalblock_pin.left
	document.body.style.width = gmodalblock_pin.width
	gmodalblock_pin = null
	window.scrollTo(sx, sy)
}

// Key-field home while a modal pin is active: remember 0,0 for unpin + move pin.
function modalblock_note_scroll_home() {
	if (!gmodalblock_pin)
		return
	gmodalblock_pin.x = 0
	gmodalblock_pin.y = 0
	if (document.body.style.position == 'fixed') {
		document.body.style.top = '0px'
		document.body.style.left = '0px'
	}
}

function blockmodalui_sync() {

	if (gmodalblockdepth === 0)
		modalblock_create()
	++gmodalblockdepth

}

var gchildwin

function unblockmodalui_sync() {

	//close legacy child 'please wait' window if present
	if (gchildwin && gchildwin.actual && !gchildwin.actual.closed)
		gchildwin.actual.close()
	// Do NOT clear gprocessing_waitcancel_active here — nested unblocks (confirm depth,
	// Gate A finally) must not strip Gate B ownership mid-wait. Cleared only in
	// exodus_run_waitcancel finally / resolve of owner B.

	if (gmodalblockdepth > 0)
		--gmodalblockdepth

	if (gmodalblockdepth === 0)
		modalblock_destroy()

}

function getdialogstyle_sync(dialogstyle) {

	var standardstyle = 'Center:yes'
	standardstyle += ',Help:no'
	standardstyle += ',Resizable:yes'
	standardstyle += ',Scroll:yes'
	standardstyle += ',Status:no'
	standardstyle += ',scrollbars=1'//mozilla?
	//standardstyle+=',modal=yes'//no longer supported in chrome
	standardstyle += ',alwaysRaised=yes'

	var max = getmaxwindow_sync()
	//var maxwidth = window.outerWidth//window.innerWidth//screen.availWidth
	//var maxheight = window.outerHeight//window.innerHeight//screen.availHeight
	//var maxwidth = max.width
	//var maxheight = max.height

	// some engines minimise instead of open features as expected
	//maxwidth=0
	//maxheight=0
	if (!dialogstyle) {
		//var maxwidth=window.screen.availWidth
		//var maxheight=window.screen.availHeight
		//var dialogstyle = 'DialogHeight:' + maxheight + 'px;DialogWidth:' + maxwidth + 'px'
		//var dialogstyle = 'Center:yes,Help:yes,Resizable:yes;Status:No'
		var dialogstyle = standardstyle

		//IE
		//if (maxwidth)
		//	dialogstyle += ',DialogWidth:' + maxwidth + 'px'
		//if (maxheight)
		//	dialogstyle += ',DialogHeight:' + maxheight + 'px'

		//mozilla
		if (max.width)
			dialogstyle += ',width=' + max.width
		if (max.height)
			dialogstyle += ',height=' + max.height

		if (typeof window.screenX != 'undefined')
			//dialogstyle+=', left='+window.screenX
			dialogstyle += ',left=' + max.left
		if (typeof window.screenY != 'undefined')
			//dialogstyle+=', top='+window.screenY
			dialogstyle += ',top=' + max.top

		//prevents centering in ff3
		//dialogstyle+='; DialogHeight: 100px; DialogWidth: 500px'
		//manual centering (TODO calculate it from maxheight and maxwidth)
		//dialogstyle+='; DialogTop: 100px; DialogLeft: 100px'
	}
	else if (dialogstyle == 'max') {
		dialogstyle = standardstyle + ',height:' + max.height + 'px,width:' + max.width + 'px;'
	}

	// return with comma AND semicolon separators (browsers differ on which they accept)
	//dialogstyle = dialogstyle + ';' + dialogstyle.replace(/,/g, ';')

	return dialogstyle
}

// ---------------------------------------------------------------------------
// Modal dialog arguments — one bag, one live child at a time.
//
// Parent (exoui_showmodaldialog):
//   dialogArgumentsForChild = bag   // primary; child can re-pull after hard refresh
//   gchildwin = window.open(...)
//   gchildwin.dialogArguments = bag // secondary; can race first load; lost on refresh
//   finally: dialogArgumentsForChild = null
//
// Child (exodus_client_init via exodus_acquire_modal_dialog_arguments):
//   1. If opener.gchildwin === this window and not the lazy Wait/Cancel stub
//      → use opener.dialogArgumentsForChild
//   2. Else → window.dialogArguments (secondary inject)
//   Then: rebind window.dialogArguments and copy properties into gparameters.
//
// Page code: prefer gparameters. Do not invent page-local acquire helpers.
// Non-modal opens use gwindowopenparameters (dbform), not this bag.
// Nested modals: each child reads only its own opener’s bag.
// ---------------------------------------------------------------------------
function exodus_acquire_modal_dialog_arguments() {

	var bag = null

	if (window.opener) {
		try {
			// opener may be closed or cross-origin; access can throw
			if (!window.opener.closed
				&& window.opener.gchildwin === window
				&& !window.opener.gchildwin.lazy
				&& window.opener.dialogArgumentsForChild)
				bag = window.opener.dialogArgumentsForChild
		}
		catch (e) { }
	}

	if (!bag && window.dialogArguments)
		bag = window.dialogArguments

	return bag
}

async function exoui_showmodaldialog(url, dialogargs, dialogstyle) {

	if (!dialogargs)
		var dialogargs = new Object
	if (!dialogargs.gtasks)
		dialogargs.gtasks = gtasks

	dialogstyle = getdialogstyle_sync(dialogstyle)

	//always send login code
	dialogargs.logincode = glogincode

	// Publish before open so a fast-loading or hard-refreshed child can pull.
	// Cleared in finally so a finished modal cannot leak args to later pages.
	dialogArgumentsForChild = dialogargs
	try {

		// modern async path using window.open + promise (no more showModalDialog or guseyield branching)
		//example
		//../media/schedulefind.htm?FILENAME=SCHEDULES
		/*
		Center: yes, Help: no, Resizable: yes, Scroll: yes, Status: no, scrollbars=1,
		alwaysRaised=yes, DialogWidth:1870px, DialogHeight:1053px, width=1870, height=1053, left=50, top=27
		;
		Center: yes; Help: no; Resizable: yes; Scroll: yes; Status: no; scrollbars=1; 
		alwaysRaised=yes; DialogWidth:1870px; DialogHeight:1053px; width=1870; height=1053; left=50; top=27
		*/
		dialogstyle = '' // now always show in a tab

		//open the child window async
		gchildwin = window.open(url, '', dialogstyle)

		if (!gchildwin) {
			alert('Unable to show popup window - please enable popups; disable your popup blocker.')
			return
		}

		// Secondary: inject after open (may race load; not durable on refresh)
		gchildwin.dialogArguments = dialogargs

		// auto resume if the child window disappears — poll every n ms
		window.setTimeout(exodus_autoresume, 100)

		//wait here until exodus_autoresume detects that the child window is closed
		// and passes its return value here
		var dialogResolve
		var dialogPromise = new Promise((resolve) => {
			dialogResolve = resolve
		})
		gpendingDialogResolve = dialogResolve
		gpendingDialogOwner = 'A'

		var result = await dialogPromise
		gpendingDialogResolve = null
		gpendingDialogOwner = null
		console.log('exoui_showmodaldialog result is ' + result)

		// Closed / blocked popup — no return value (was Safari-only UA sniff)
		if (typeof result == 'undefined')
			throw (url)

		return result

	}
	catch (e) {
		console.log('caught error in exoui_showmodaldialog: ' + e)
		//alert('Please enable popups for this site (1)\n\nError:'+(e.description?e.description:e))
		//alert('Please enable popups for this site (1)\n\nError:'+(e.description?e.description:e)+'\n\n'+url+'\n\n'+arguments)
		return
	}
	finally {
		dialogArgumentsForChild = null
	}

}

//called by child windows to return result to the parent before closing
function exoui_windowclose(returnvalues) {

	//window.opener.gchildwin_returnvalue = returnvalues
	if (window.opener) {
		window.opener.focus()//for MSEDGE
		if (window.opener.exodus_setchildwin_returnvalue) {
			if (typeof returnvalues == 'undefined')
				returnvalues = ''
			returnvalues.exodusisarray = true
			window.opener.exodus_setchildwin_returnvalue(returnvalues)
		}
	}
	window.returnValue = returnvalues
	window.close()
}

//parent window function called by childwindow to return result to parent window
var gchildwin_returnvalue
function exodus_setchildwin_returnvalue(returnvalue) {
	//gchildwin_returnvalue=returnvalue
	//shallow copy array to avoid permissions issue when child windows closes in edge
	if (returnvalue.exodusisarray) {
		gchildwin_returnvalue = []
		for (var ii = 0; ii < returnvalue.length; ++ii) {
			//gchildwin_returnvalue[ii]=returnvalue[ii].toString()
			gchildwin_returnvalue[ii] = returnvalue[ii]
		}
	}
	else {
		gchildwin_returnvalue = returnvalue
	}
}

function exodus_autoresume() {

	// if child window still active then schedule another check later
	if (gchildwin && !gchildwin.lazy && !gchildwin.closed) {
		window.setTimeout(exodus_autoresume, 100)
		return
	}

	console.log('EXODUS_AUTORESUME since gchildwin is closed or missing')

	//childwin doesnt exist or has closed
	//get the returnvalue from it or return an empty string
	try {
		var returnvalue = gchildwin.returnValue
	} catch (e) {
		gchildwin = false
		returnvalue = ''
	}

	//alert('2 gchildwin_returnvalue='+gchildwin_returnvalue)
	if (gchildwin_returnvalue) {
		returnvalue = gchildwin_returnvalue
		//		alert('gchildwin_returnvalue='+gchildwin_returnvalue)
		gchildwin_returnvalue = undefined
	}

	//exodus_resume(returnvalue, 'exodus_autoresume')
	resolvePendingDialog(returnvalue, 'exodus_autoresume', 'A')

}

// Generator resume path removed (rationalisation stage 6, 2026-07-17).
// Framework and app modules are async/await only. Stubs remain so accidental
// calls fail loudly instead of silently no-op.
function exodus_resume(value, source) {
	systemerror(
		'exodus_resume',
		'Generator resume removed (stage 6). Source: ' + source
		+ '. Convert remaining function* to async/await.'
	)
	return value
}

function exodus_next(value, source) {
	systemerror(
		'exodus_next',
		'Generator step removed (stage 6). Source: ' + source
	)
	return { done: true, value: value }
}

function displayresponsedata_sync(request, data) {
	exodusinvokesynctarget(displayresponsedata, [request, data], 'displayresponsedata')
}

async function displayresponsedata(request, data) {

	if (!data)
		data = ''

	db.request = request
	if (!(await db.send(data))) {
		await exoui_invalid(db.response)
		return false
	}
	if (db.data)
		await exoui_note(db.data)
	else
		await exoui_note(db.response.slice(2))

	return true

}

function openwindow_sync(request, data) {
	//LEAVE SPACE AFTER FUNCTION NAME TO PREVENT CONVERSION TO YIELD
	return exodusinvokesynctargetreturn(openwindow, [request, data], 'openwindow_sync ' + request)
}

//function to simplify passing a db request (with optional data)
//that returns a URL to be opened in an independent window
async function openwindow(request, data) {

	if (!data)
		data = ''
	db.request = request
	if (!(await db.send(data))) {
		await exoui_invalid(db.response)
		return false
	}

	if (db.response != 'OK')
		await exoui_note(db.response.slice(3))

	if (db.data) {
		var urls = db.data.split(fm)
		var result
		for (ii = 0; ii < urls.length; ++ii)
			result = await windowopen(urls[ii])
		return result
	}
}

async function windowopenkey(url, key) {
	if (!key)
		key = gvalue
	return await windowopen(url, { 'key': key })
}

function windowopen_sync(url, parameters, style) {
	//LEAVE SPACE AFTER FUNCTION NAME TO PREVENT CONVERSION TO YIELD
	return exodusinvokesynctargetreturn(windowopen, [url, parameters, style], 'windowopen_sync')
}

var gwindowopenparameters
async function windowopen(url, parameters, style) {

	url = url || ''
	parameters = parameters || ''
	style = style || ''

	//url may arrive like ..\DATA\DEVDTEST\~3499268.htm or ../data/DEVDTEST/~3499268.htm
	// and will be converted here to ../../DATA/DEVDTEST/~3499268.htm
	//which browsers will interpret as as the full URL http://hostname/DATA/DEVDTEST/~3499268.htm
	//if (url.replace(/\\/g, '/').toUpperCase().substr(0, 8) == '../DATA/')
	//	url = '../' + url.replace(/\\/g, '/')

	// replace backslashes with slashes and anything before /data/ with ../..
	url = url.replace(/\\/g, '/').replace(/^.*[\/\\]data[\/\\]/,'../../data/')

	//if no style or "tab" parameter, windows opened from dialogs are opened in new maximised windows
	if (style == '' && gisdialog) {
		var max = getmaxwindow_sync()

		style += ';DialogHeight:' + max.height + 'px; DialogWidth:' + max.width + 'px; Resizable: yes'
		//mozilla
		style += ',toolbar=yes,menubar=yes,resizable=yes,scrollbars=yes,status=yes'
		style += ',width=' + max.width
		style += ',height=' + max.height
		style += ',left=' + max.left
		style += ',top=' + max.top
	}
	//"tab" used to force opening in a tab despite being in a dialog (because the dialog is closing)
	if (style == 'tab')
		style = ''

	//normalise windows style urls with backslashes to slashes.
	//firefox wont normalise window style relative urls like ..\ but will if they are like ../
	// and this may cause problems for any other relative urls in the page like images
	url = url.replace(/\\/g, '/')

	// Parent bag for non-modal children (dbform copies into gparameters on load *and*
	// refresh). Always *replace* the whole bag here — never leave a prior open's
	// key if this open has none. Child must not null this (F5 same form re-reads).
	// Stamp _openhtm so a bag left for vouchers is not applied to journals (etc.).
	// URL query avoided so the form URL stays cacheable.
	if (!parameters || typeof parameters != 'object')
		gwindowopenparameters = {}
	else
		gwindowopenparameters = parameters
	gwindowopenparameters.logincode = glogincode
	try {
		var openpath = String(url || '').replace(/\\/g, '/').split('?')[0]
		gwindowopenparameters._openhtm = openpath.split('/').pop().toLowerCase()
	} catch (e) {
		gwindowopenparameters._openhtm = ''
	}
	try {

		var result = window.open(url, '', style)

		// Closed / blocked popup — no window handle (null or undefined)
		if (result == null)
			throw (url)

		// Empty url → about:blank. Some browsers pair dark canvas with black text
		// under prefers-color-scheme: dark (broken default contrast). Fix the blank
		// document once; callers just write content.
		if (!url)
			exodus_ensure_readable_blank(result)

		return result

	}
	catch (e) {
		//alert('Please enable popups for this site (2)\n\nError:'+(e.description?e.description:e))
		return false
	}

}

// Readable defaults for a blank document (about:blank or raw window.open()).
// Use system Canvas/CanvasText so light and dark both contrast — not hard-coded
// light, and not a debug-only patch on individual dump sites.
function exodus_ensure_readable_blank(win) {
	if (!win || !win.document)
		return
	try {
		var doc = win.document
		if (doc.documentElement)
			doc.documentElement.style.colorScheme = 'light dark'
		var body = doc.body
		if (!body)
			return
		body.style.backgroundColor = 'Canvas'
		body.style.color = 'CanvasText'
	} catch (e) {
	}
}

//wrapper function to replace window.open()
//to give warning if cannot open ... because of pop blockers etc.
function exodusbreak(cmd, funcname, position) {

	if (!gstepping) return

	//window.status = funcname + ' ' + position + ' ' + cmd
	console.log(funcname + ' ' + position + ' ' + cmd);

	if (!cmd) cmd = ' '

	var msg = ''
	while (cmd) {
		if (cmd != ' ') {

			var result
			try {
				result = this.parent.eval(cmd)
			}
			catch (e) {
				result = e.number + ' ' + e.description
			}
			msg = cmd + '=\n' + result + '\n\n' + msg

			//display result and optionally quit
			if (!(confirm(msg))) {
				//FF3 about:config
				//extensions.firebug-service.breakOnErrors
				//breakrighthere
				debugger
				return
			}

		}
		cmd = prompt(funcname + ' ' + position, cmd)
	}
}

function assertelement(element, funcname, varname) {
	exodusassertobject(element, funcname, varname)
	if (!element.tagName) {
		return systemerror('assertelement()', 'In ' + funcname + ', ' + varname + ' is not an element.')
	}
	return true
}

// User-facing text when backend (or rare client) surface is a system/backtrace error.
// Refine wording later; special exoui_confirm icon/mode may follow.
var gexodus_system_error_user_msg =
	'A system error has occurred.\r\n\r\n' +
	'Technical support has been informed.\r\n\r\n' +
	'You may try to ignore the message or contact technical support for more info.'

// System errors contain "System Error" near the start.
// Server (listen) often wraps as "Error: System Error: …"; client systemerror uses
// "System Error: …". Inspect first 50 chars so a short server prefix still matches.
function exodus_looks_like_system_error(msg) {
	if (msg == null || msg === '')
		return false
	return String(msg).slice(0, 50).indexOf('System Error') >= 0
}

function exodus_user_facing_msg(msg) {
	if (!exodus_looks_like_system_error(msg))
		return msg
	// Always log technical text (friendly UI may hide it from the alert/note)
	try {
		console.log('EXODUS system error (hidden from user):\n' + String(msg))
	} catch (e) { }
	return gexodus_system_error_user_msg
}

async function exoui_note(msg, mode) {

	//if (!msg) return false
	//allow return await exoui_note() to be opposite of return await exoui_invalid()
	if (!msg)
		return true

	//depends on number of buttons now 1=info >1=question
	//if (!mode) mode='info'

	if (typeof msg == 'string' && msg.slice(0, 6) == 'Error:')
		msg = msg.slice(6)

	//msg=msg.replace(/\r/,'\r')
	//msg=msg.replace(/\n/,'\n')
	msg = msg.toString().replace(FMre, '\r\n').replace(VMre, '\r\n')
	msg = msg.replace(/\|/g, '\r\n')

	msg = exodus_user_facing_msg(msg)

	await exoui_confirm(msg, 1, 'OK', '', '', null, false, mode)

	return true

}

//''''''''
//'INVALID
//''''''''
async function exoui_invalid(msg) {
	//displays a message if provided and returns false
	await exoui_note(msg, 'critical')
	return false
}

//''''''''
//'WARNING
//''''''''
async function exoui_warning(msg) {
	//displays a message if provided and returns true
	//so you can use it like "return await exoui_warning(msg)" to save a line
	return await exoui_note(msg, 'warning')
}

function theme_toggle_title(dark) {
	return dark ? 'Switch to light mode' : 'Switch to dark mode'
}

// Menubar theme control: theme-sun / theme-moon glyph inside shell-theme host.
// Show current mode: sun in light, moon in dark (same as knob.innerHTML before).
// Optional img/btn: required while building the control (not in document yet — getElementById fails).
function exodus_sync_theme_btn_icon(img, btn) {

	img = img || document.getElementById('theme_toggle_icon')
	btn = btn || document.getElementById('theme_toggle_btn')
	if (!img)
		return
	img.src = gimagetheme + (gisdarktheme ? 'theme-moon.svg' : 'theme-sun.svg')
	if (btn)
		btn.title = theme_toggle_title(gisdarktheme)
}

function exodus_swap_tool_icons() {

	// Static toolbar icons (e.g. reports.htm). Mono masks → tinted .exodus-icon spans.
	var tools = [
		{ file: 'row-insert.svg', color: 'green' },
		{ file: 'row-delete.svg', color: 'red' },
		{ file: 'shell-refresh.svg', color: 'green' },
		{ file: 'media-play.svg', color: 'green' },
		{ file: 'media-pause.svg', color: 'blue' }
	]
	for (var ti = 0; ti < tools.length; ti++) {
		var tool = tools[ti]
		var sel = 'img[src*="/' + tool.file + '"]'
		document.querySelectorAll(sel).forEach(function (img) {
			var span = exodus_set_icon_element(img, exodus_icon_spec(tool.file, tool.color))
			if (span && span.style)
				span.style.cursor = 'pointer'
		})
	}
}

function exodus_update_auth_button() {

	var btn = $$('exoduslogoutbutton')
	if (!btn)
		return
	var label = $$('exoduslogoutbutton_label')
	if (!gusername) {
		if (label)
			label.innerHTML = 'Login'
		btn.title = 'Login. Alt+L'
		setgraphicbutton(btn, null, gloginimage)
	} else {
		if (label)
			label.innerHTML = 'Lo<u>g</u>out'
		btn.title = 'Logout. Alt+G'
		setgraphicbutton(btn, null, glogoutimage)
	}
}

function exodus_set_theme_icons() {

	// Monochrome icons: colours come from CSS vars (no path swap).
	// Painted multi-colour: New/Open/Edit/Delete, Copy, theme, company
	gthemeimage = gimagetheme + (gisdarktheme ? 'shell-theme_dm.svg' : 'shell-theme_lm.svg')
	gcompanyimage = gimagetheme + (gisdarktheme ? 'shell-company_dm.svg' : 'shell-company_lm.svg')
	if (typeof gcopyimage != 'undefined')
		gcopyimage = gimagetheme + (gisdarktheme ? 'record-copy_dm.svg' : 'record-copy_lm.svg')
	if (typeof gnewimage != 'undefined') {
		gnewimage = gimagetheme + (gisdarktheme ? 'record-new_dm.svg' : 'record-new_lm.svg')
		gdeleteimage = gimagetheme + (gisdarktheme ? 'record-delete_dm.svg' : 'record-delete_lm.svg')
		geditimage = gimagetheme + (gisdarktheme ? 'record-edit_dm.svg' : 'record-edit_lm.svg')
	}
	if (typeof gopenimage != 'undefined')
		gopenimage = gimagetheme + (gisdarktheme ? 'record-open_dm.svg' : 'record-open_lm.svg')
	if (typeof gsortimage != 'undefined')
		gsortimage = exodus_sortimage()
	exodus_update_auth_button()
	// Refresh painted <img> srcs if already in the DOM
	if (typeof gnewimage != 'undefined') {
		;['newrecord', 'openrecord', 'editreleaserecord', 'deleterecord', 'copyrecord'].forEach(function (id) {
			var b = $$(id + 'button')
			if (!b)
				return
			var img = b.getElementsByTagName('IMG')[0]
			if (!img)
				return
			if (id == 'newrecord')
				img.src = gnewimage
			else if (id == 'openrecord' && typeof gopenimage == 'string')
				img.src = gopenimage
			else if (id == 'deleterecord')
				img.src = gdeleteimage
			else if (id == 'copyrecord' && typeof gcopyimage == 'string')
				img.src = gcopyimage
			else if (id.indexOf('edit') >= 0 && b.querySelector && !b.querySelector('.exodus-icon'))
				img.src = geditimage // Edit mode; Release uses lock mono
		})
	}
}

// Right-side menubar cluster: session | theme | logout as one float:right flex row.
// Equal CSS gap — no independent float:right packing (that made session↔theme↔logout uneven).
function exodus_menubar_trailing_cluster() {

	if (!gexodus_menubar)
		return null
	var trail = gexodus_menubar.querySelector('.exodus_menubar_trailing')
	if (trail)
		return trail
	trail = document.createElement('span')
	trail.className = 'exodus_menubar_trailing'
	// Before clear:left spacer if present; else at end of menubar
	var clear = gexodus_menubar.querySelector('.exodus_menubar_clear')
	if (clear)
		gexodus_menubar.insertBefore(trail, clear)
	else
		gexodus_menubar.appendChild(trail)
	return trail

}

function add_theme_toggle_btn() {

	// Simple sun/moon button (no slider track — sun lost contrast on the grey track).
	var btn = document.createElement('span')
	btn.id = 'theme_toggle_btn'
	btn.className = 'theme_button'
	btn.setAttribute('role', 'button')
	btn.tabIndex = 0
	btn.title = theme_toggle_title(gisdarktheme)

	var img = document.createElement('img')
	img.id = 'theme_toggle_icon'
	img.alt = ''
	img.width = 18
	img.height = 18
	btn.appendChild(img)
	// Pass nodes: not in document yet, so getElementById would leave src empty (empty box)
	exodus_sync_theme_btn_icon(img, btn)

	function flip() {
		theme_toggle(gisdarktheme ? 'default' : 'dark_mode')
		exodussetcookie('', gthemecookiekey, (gisdarktheme ? 1 : ''), 'dt', true)
		exodus_sync_theme_btn_icon(img, btn)
	}
	btn.addEventListener('click', flip)
	btn.addEventListener('keydown', function (e) {
		if (e.key == 'Enter' || e.key == ' ') {
			e.preventDefault()
			flip()
		}
	})

	return btn
}

function exodus_global_css_link() {

	return document.getElementById('exodus_global_css')
		|| document.querySelector("link[href$='global.css']")

}

function exodus_clear_form_inline_theme() {

	var tables = document.getElementsByTagName('TABLE')
	for (var ii = 0; ii < tables.length; ii++) {
		if ((' ' + tables[ii].className + ' ').indexOf(' exodusform ') >= 0) {
			tables[ii].style.removeProperty('color')
			tables[ii].style.removeProperty('background-color')
		}
	}

}

function theme_toggle(theme = 'default') {

	// Switch between dark and light (color) modes using CSS themes
	const html = document.documentElement;

	if (theme == 'default') {
		gisdarktheme = false
		html.removeAttribute('data-theme')
		html.style.removeProperty('--exodus-cardcolor')
		// Drop firstpaint DM inline (color-scheme / black bg) so LM is not stuck dark
		html.style.removeProperty('color-scheme')
		html.style.removeProperty('background')
		var fp = document.getElementById('exodus_dm_firstpaint')
		if (fp && fp.parentNode)
			fp.parentNode.removeChild(fp)
		var fg = document.getElementById('exodus_dm_flashguard')
		if (fg && fg.parentNode)
			fg.parentNode.removeChild(fg)
		// Full LM chrome from cookies (colour + font + size)
		exodus_chrome_from_cookies()
		exodus_clear_form_inline_theme()
	} else {
		gisdarktheme = true
		html.setAttribute('data-theme', theme)
		html.style.removeProperty('--exodus-cardcolor')
		// Inline LM colour on <html> would override DM :root tokens
		html.style.removeProperty('--exodus-form-bg-color')
		html.style.removeProperty('--exodus-form-border-color')
		html.removeAttribute('data-form-head')
	}

	// Monochrome icons recolor via CSS vars automatically.
	// Painted multicolour (New/Edit/Delete, theme, …) still swap files.
	// Static reports.htm add/delete/refresh: converted to mono tints in exodus_swap_tool_icons.
	const xform_postload = document.readyState === 'complete'
	if (xform_postload) {
		exodus_swap_tool_icons()
		exodus_refresh_sortimages()
	}

	exodus_set_theme_icons()

	return true
}

// Sticky thead tint direction for LM (see global.css “LM sticky thead tint”).
// Deeper vs lighter from body luma; CSS owns the two formulas.
// Call when setting a non-empty --exodus-form-bg-color.
function exodus_set_form_head_direction(cssColor) {
	var s = String(cssColor == null ? '' : cssColor).replace(/\s+/g, '')
	if (/^[0-9a-fA-F]{3}$/.test(s) || /^[0-9a-fA-F]{6}$/.test(s))
		s = '#' + s
	var el = document.createElement('span')
	el.style.cssText = 'position:absolute;visibility:hidden;color:' + s
	document.documentElement.appendChild(el)
	var m = /(\d+)\s*,\s*(\d+)\s*,\s*(\d+)/.exec(window.getComputedStyle(el).color)
	document.documentElement.removeChild(el)
	// Rec. 601 luma; ≥128 → deepen head, else lighten head
	var light = true
	if (m)
		light = (0.299 * Number(m[1]) + 0.587 * Number(m[2]) + 0.114 * Number(m[3])) >= 128
	document.documentElement.setAttribute('data-form-head', light ? 'deeper' : 'lighter')
}

// ---------------------------------------------------------------------------
// Screen chrome (cookies fc / ff / fs → CSS vars only)
//
// Source of truth after login: cookies. Empty cookie/field = no inline override
// (global.css :root / BODY fallbacks win). users.htm may preview via apply_*;
// only users form_postwrite writes cookies.
// ---------------------------------------------------------------------------

// Empty / Default / CSS keywords → treat as "not available"
function exodus_chrome_is_empty(v) {
	if (v == null)
		return true
	v = String(v).trim()
	if (!v)
		return true
	if (v.toUpperCase() == 'DEFAULT')
		return true
	if (/^(inherit|initial|unset|revert)$/i.test(v))
		return true
	return false
}

// Value stored in cookie: empty string when not available
function exodus_chrome_cookie_store(v) {
	return exodus_chrome_is_empty(v) ? '' : String(v).trim()
}

// LM form body only. Empty → removeProperty so CSS default #fdf5e6 applies.
function exodus_chrome_apply_color(value) {
	if (typeof gisdarktheme != 'undefined' && gisdarktheme)
		return
	var html = document.documentElement
	if (exodus_chrome_is_empty(value)) {
		html.style.removeProperty('--exodus-form-bg-color')
		html.style.removeProperty('--exodus-form-border-color')
		html.removeAttribute('data-form-head')
		html.style.removeProperty('--exodus-cardcolor')
		return
	}
	value = String(value).trim()
	html.style.setProperty('--exodus-form-bg-color', value)
	html.style.setProperty('--exodus-form-border-color', '#d0d0d0')
	exodus_set_form_head_direction(value)
	html.style.removeProperty('--exodus-cardcolor')
}

// Font family + size %. Empty family/size → CSS / browser default (remove override).
function exodus_chrome_apply_font(family, size) {
	var html = document.documentElement
	if (exodus_chrome_is_empty(family))
		html.style.removeProperty('--exodus-screen-font-family')
	else
		html.style.setProperty('--exodus-screen-font-family', String(family).trim())

	if (exodus_chrome_is_empty(size)) {
		html.style.removeProperty('font-size')
		return
	}
	var n = Number(size)
	if (!n) {
		html.style.removeProperty('font-size')
		return
	}
	if (typeof gformfontscale != 'undefined' && gformfontscale)
		n *= gformfontscale
	if (n == 100)
		html.style.removeProperty('font-size')
	else
		html.style.fontSize = n + '%'
}

// Entry / theme→day / discard / post-save: cookies only.
function exodus_chrome_from_cookies() {
	var ff = ''
	var fs = ''
	var fc = ''
	if (typeof exodusgetcookie2 == 'function') {
		ff = exodusgetcookie2('ff')
		fs = exodusgetcookie2('fs')
		fc = exodusgetcookie2('fc')
	}
	exodus_chrome_apply_font(ff, fs)
	exodus_chrome_apply_color(fc)
}

// Thin aliases — prefer exodus_chrome_* at new call sites.
function exodus_set_style(mode, value, value2) {
	if (mode == 'screencolor')
		exodus_chrome_apply_color(value)
	else if (mode == 'screenfont')
		exodus_chrome_apply_font(value, value2)
}

// Early in decide / decide2 / print
async function clientfunctions_setstyle() {
	exodus_chrome_from_cookies()
}

async function clientfunctions_getglobals() {
	gcompanycode = exodusgetcookie2('cc')
	gncompanies = exodusgetcookie2('nc')
	gperiod = exodusgetcookie2('pd')
	gbasecurr = exodusgetcookie2('bc')
	gshowzeros = exodusgetcookie2('bc')
	gbasefmt = exodusgetcookie2('bf')
	gmarketcode = exodusgetcookie2('mk')
	gmaincurrcode = exodusgetcookie2('mc')
	gdatasetname = exodusgetcookie2('db')
	gmenucodes = exodusgetcookie2('m')
	gmaxstrsize = Number(exodusgetcookie2('ms'))
	gtz = exodusgetcookie2('tz').split(fm)
	if (gtz.join('')) {
		gtz[0] = Number(gtz[0])
		if (gtz[1])
			gtz[1] = Number(gtz[1])
		else
			gtz[1] = 0
	}
	else {
		//gtz = [0, 0]
		gtz = [new Date().getTimezoneOffset() * -60, 0]
	}

	// gbasefmt from cookie 'bf' = server BASEFMT, e.g. MD20P / MC20P / MD20P, / MC20P,
	// MD = '.' decimal (',' thousands); MC = ',' decimal ('.' thousands);
	// trailing ',' = thousands grouping enabled (company number format).
	if (typeof gbasefmt == 'string' && gbasefmt.substr(0, 2) == 'MC')
		gthousands_regex = /\./gi
	else
		gthousands_regex = /,/gi
}

function add_exodus_menubar() {

	//if no exodus_menubar element
	//create a exodus_menu span
	// at the beginning of the body
	// or after the first navbar element

	gexodus_menubar = document.getElementById('exodus_menu')
	if (!gexodus_menubar) {
		var span = document.createElement('SPAN')
		span.id = 'exodus_menu'
		// menubar design
		//span.style.backgroundColor = '#f4f4f4';
		span.style.backgroundColor = '#f0f0f0';
		span.style.color = 'black';		   // Text color
		span.style.display = 'block';
		span.style.width = '100%';			// Span full width
		span.style.position = 'fixed';
		span.style.top = '0';				 // Align to the top
		span.style.left = '0';				// Align to the left
		//span.style.zIndex = '100';			// Cannot be overlapped
		span.style.padding = '5px 0';
		span.style.margin = '0';
		span.style.boxSizing = 'border-box';  // Ensure padding doesn't affect width
		span.style.outline = '1px solid lightgrey'

		var navbar1 = document.getElementsByClassName('navbar')[0];
		if (navbar1)
			navbar1.parentNode.insertBefore(span, navbar1.nextSibling)
		else
			document.body.insertBefore(span, document.body.firstChild)
		gexodus_menubar = document.getElementById('exodus_menu')
	}
	// Modals (e.g. search.htm) create the bar late via form_place_menubar_session —
	// still need body offset + resize/mutation wiring so content is not under the bar.
	wire_exodus_bodymargin()
}

// Adjust the body's top margin dynamically so that body is never overlapped by the menubar
function adjust_bodymargin() {
	gexodus_menubar = document.getElementById('exodus_menu')
	if (!gexodus_menubar)
		return
	var menuheight = gexodus_menubar.offsetHeight
	// Floated session/trailing can stick out if clearfix is incomplete — use visual extent
	var barTop = gexodus_menubar.getBoundingClientRect().top
	var kids = gexodus_menubar.children
	for (var i = 0; i < kids.length; i++) {
		var extent = kids[i].getBoundingClientRect().bottom - barTop
		if (extent > menuheight)
			menuheight = extent
	}
	document.body.style.marginTop = (menuheight + 10) + 'px'
	// stick flush under fixed menubar (body margin +10 is page spacing, not sticky offset)
	document.documentElement.style.setProperty('--exodus-sticky-top', (barTop + menuheight) + 'px')
}

// Once per window: resize + MutationObserver so late-built bars (modals) keep body clear.
var g_exodus_bodymargin_wired = false
function wire_exodus_bodymargin() {
	gexodus_menubar = document.getElementById('exodus_menu')
	if (!gexodus_menubar)
		return
	adjust_bodymargin()
	if (g_exodus_bodymargin_wired)
		return
	g_exodus_bodymargin_wired = true
	window.addEventListener('resize', adjust_bodymargin)
	if (typeof MutationObserver == 'function') {
		var observer = new MutationObserver(function () {
			adjust_bodymargin()
		})
		observer.observe(gexodus_menubar, {
			attributes: true,
			childList: true,
			subtree: true
		})
	}
}

async function clientfunctions_windowonload() {

	////login('clientfunctions_windowonload')

	if (document.getElementsByClassName)
		id2classname()

	//already done in global script
	//	if (!glogincode)
	//		glogincode = exodusgetcookie2('logincode', 'EXODUSlogincode', null)

	//loginalert('wol'+glogincode)
	//	gdataset = exodusgetcookie2('dataset')

	await clientfunctions_getglobals()

	db = new exodusdblink
	//gusername is set in exodusdblink

	//In case previous window didnt successfully unlock its record in onbeforeunload
	//Could be turned off hoping synchronous unlock in window_onunload is sufficiently reliable
	// to avoid need for double unlocking
	var pending = exodusgetcookie(glogincode, 'EXODUSpending')
	if (pending) {
		exodussetcookie(glogincode, 'EXODUSpending', '')
		console.log(pending)
		db.request = pending
		await db.send()
	}

	await clientfunctions_setstyle()

	//if (gautofitwindow && document.getElementById('autofitwindowelement'))
	//	exodussettimeout('exodusautofitwindow()', 10)
	//exodussetinterval('exodusautofitwindow()', 10)

	// Fixed top menubar for main windows only (not modal dialogs).
	// Form actions: dbform — under the form when on-screen, else top menubar.
	// Dont add menubar on login page where link doesnt end in .htm)
	if (document.URL.includes(".htm") && !window.dialogArguments) {
		// Also no menubar in other pages
		if (!document.URL.match(/index|confirm|upload/)) {
			add_exodus_menubar();
		}
	}

	// Before form_functions_onload so content is not under a pre-existing bar.
	// Modals (search.htm) often create #exodus_menu later via add_exodus_menubar —
	// wire_exodus_bodymargin runs again from there.
	wire_exodus_bodymargin()

	//trigger formfunctions_onload; wrap panes; only then reveal forms
	// (html:not(.exodus-panes-ready) keeps bare/unmerged layout invisible — global.css).
	// Wide: cleardoc skeleton (bound+unbound) before first record; re-decide after wrap.
	try {
		if (typeof formfunctions_onload == 'function')
			await formfunctions_onload()
		exoduswrapformpanes()
		if (typeof form_update_wide_layout == 'function')
			form_update_wide_layout()
	} finally {
		exodus_reveal_form_panes()
	}

	// Unbound form actions under the form: form_keep runs once inside formfunctions_onload,
	// but wrap/pane padding can push the bar below the fold *after* that check.
	// Re-run after wrap, and once more after the next paint.
	if (typeof form_keep_action_buttons_on_screen == 'function') {
		form_keep_action_buttons_on_screen()
		if (typeof requestAnimationFrame == 'function') {
			requestAnimationFrame(function () {
				requestAnimationFrame(form_keep_action_buttons_on_screen)
			})
		} else {
			window.setTimeout(form_keep_action_buttons_on_screen, 0)
		}
	}

	//add menu, logout and refresh buttons if not a popup, depending on gshowmenu, not /exodus/ location and no navbar elements
	//if no exodus_menu span (even if no menu, it is a holder for EXODUS form buttons New/Save etc.)
	if (!window.dialogArguments && (typeof gshowmenu == 'undefined' || gshowmenu) && EXODUSlocation != './exodus/' && document.getElementsByClassName('navbar').length == 0) {

		// Trailing cluster: theme | refresh | logout (equal gap)
		var trailing = exodus_menubar_trailing_cluster()

		//button to theme toggle (sun/moon icon; no slider)
		trailing.appendChild(add_theme_toggle_btn())

		//button to refresh (clear cache) — left of logout
		if (typeof gshowrefreshcachebutton == 'undefined' || gshowrefreshcachebutton) {
			var refresh_span = document.createElement('span')
			refresh_span.classList.add('refresh_wrapper')
			refresh_span.innerHTML = menubuttonhtml('refreshcache', grefreshimage, '', 'Refresh the Database Cache. Alt+R', 'X')
			trailing.appendChild(refresh_span)
			//if no dbform
			if (typeof gdictfilename == 'undefined')
				addeventlistener(refresh_span, 'click', 'refreshcache_onclick')
		}

		//button to logout
		var temp2 = document.createElement('span')
		temp2.classList.add('logout_wrapper')
		temp2.innerHTML = menubuttonhtml('exoduslogout', glogoutimage, 'Lo<u>g</u>out', 'Logout. Alt+G', 'G')
		trailing.appendChild(temp2)

		exodus_update_auth_button()
		//if no dbform
		if (typeof gdictfilename == 'undefined')
			addeventlistener(temp2, 'click', 'exoduslogout_onclick')

		//button for menu
		if (gmenucodes && gmenucodes != 'EXIT2') {
			var menu_span = document.createElement('span')
			menu_span.classList.add('hamburger_menu')
			menu_span.innerHTML = menubuttonhtml('menu', gmenuimage, '<u>M</u>enu', 'Menu. Alt+M', 'M')
			//document.body.insertBefore(menu_span, document.body.firstChild)
			gexodus_menubar.insertBefore(menu_span, gexodus_menubar.firstChild)

			//div to retrieve menu structure (insert into end of Menu button span)
			var dropdown_menu_span = document.createElement('span')
			dropdown_menu_span.id = 'menux'
			//dropdown_menu_span.style.display = 'none'
			dropdown_menu_span.innerHTML = '<iframe id=menuframe src="../../data/menu.htm" tabindex=-1 height=1px marginheight=1 marginwidth=1 frameborder=0 scrolling=no></iframe>'
			var menuelement = $$('menubutton')
			//why insert it into the menu button?
			menuelement.insertBefore(dropdown_menu_span, null)
			//gexodus_menubar.insertBefore(dropdown_menu_span, gexodus_menubar.firstChild)

			//dont rely on onload event ... poll every 100ms to see if iframe is loaded
			gmenuonloader = exodussetinterval('menuonload()', 100)
		}

		// Form pages use dbform document_onkeydown for Alt+letter. Pure client pages
		// (helpkeyboard, backup, …) need only the top-bar subset.
		if (typeof gdictfilename == 'undefined')
			exodus_menubar_ensure_keydown()

	}

	//execute any custom window onload function
	//avoid error "Can't execute code from a freed script"
	//gwindowonload comes from any window.onload ... not used atm
	if (gwindowonload)
		try { gwindowonload() }
		catch (e) { }

	exodus_swap_tool_icons()

	//logout('clientfunctions_windowonload')

	return

}

//var mmm=0
gmenuloaded = false
function menuonload() {

	//$$('menuframe').contentDocument.getElementById('menucompleted')
	var iframe = $$('menuframe')
	idocument = iframe.contentDocument
	var menuhtml
	if (idocument) {
		if (!(idocument.getElementById('menucompleted')))
			return

		//enable exodus support menu items
		if (gusername=='EXODUS') {
			var elements = idocument.getElementsByClassName('exodussupportmenuitem')
			for (var ii=0;ii<elements.length;++ii)
				elements[ii].style.display = ''
		}

		menuhtml = idocument.body.innerHTML

	}
	else {
		//menuframe=$$('menuframe')
		// iframe.onload
		//if (typeof menuframe.menudiv=='undefined'||menuframe.menucompleted.readyState!='complete') return
		if (!menuframe.menucompleted)
			return

		//menux.innerHTML=menuframe.document.body.innerHTML
		//for some reason the iframe contains the span and iframe
		//so the above does not work!!!
		//so loop through the elements of the iframe skipping menux and menuframe
		var menuhtml = ''
		var childNodes = menuframe.document.body.childNodes
		for (var i = 0; i < childNodes.length; i++) {
			if (childNodes[i].outerHTML && childNodes[i].id != 'menux' && childNodes[i].id != 'menuframe')
				menuhtml += childNodes[i].outerHTML
		}

	}

	//login('menuonload')

	window.clearInterval(gmenuonloader)

	//menux.zIndex=999 //done in stylesheet but does not work over select elements (see MS docs)

	gmenuloaded = true

	//store the menu in the menu button
	$$('menux').innerHTML = menuhtml
	var menudiv = $$('menudiv')
	var target = menudiv.parentNode.parentNode
	insertafter(target, menudiv)

	// remove legacy Alt+M capture button (old menu.htm left it inside #menudiv)
	var menuAccesskeyBtn = document.querySelector('#menudiv > button[accesskey="M"]')
		|| ($$('menux') && $$('menux').querySelector('button[accesskey="M"]'))
	if (menuAccesskeyBtn)
		menuAccesskeyBtn.remove()

	//enable the menu button events
	var menubuttonx = $$('menubutton')
	menubuttonx.onmouseover = menuonmouseover
	menubuttonx.onmouseout = menuonmouseout

	//do both?!
	//addeventlistener(menubuttonx, 'onclick', 'menuonclick')
	//addeventlistener(menubuttonx, 'onmouseover', 'menuonmouseover')
	//addeventlistener(menubuttonx, 'onmouseout', 'menuonmouseout')

	//logout('menuonload')

}

Array.prototype.exodusread = async function array_exodusread(filename, key, fieldno, cache) {

	//unless returning one fieldno, always return at least n fields
	//so that accessing fields that do not exist by [] returns ''
	var minnfields = 100

	this.exodusresponse = exodusquote(key) + ' does not exist in the ' + filename.toLowerCase() + ' file.'
	if (key == '')
		return false

	if (typeof cache == 'undefined' || cache)
		db.request = 'CACHE\r'
	else
		db.request = ''
	db.request += 'READ\r' + filename + '\r' + key
	if (!(await db.send())) {
		if (db.response.indexOf('NO RECORD') >= 0) {
			//var temp=filename.toLowerCase().exodussingular().replace(/_/,' ')
			return false
		}
		else {
			if (db.response.indexOf('file is not available') >= 0) systemerror('exodusread', db.response)
			this.exodusresponse = db.response
		}
		return false
	}

	var data = (key + fm + db.data).split(fm)

	//flag to extract function that fields are 1 based (key in [0])
	this.base = 1

	if (typeof (fieldno) == 'undefined' || fieldno == '' || fieldno == null) {
		var n = data.length
		if (n < minnfields) n = minnfields
		for (var i = 0; i < n; i++) this[i] = typeof data[i] != 'undefined' ? data[i] : ''
	}
	else {
		if (typeof data[fieldno] == 'undefined') data[fieldno] = ''
		this[0] = data[fieldno]
	}
	return true
}

//xlate method for array of keys
Array.prototype.exodusxlate = async function arrayxlate(filename, fieldno, mode) {

	var keys = this
	var results = []

	//make a list of keys not in cache
	var uncachedkeys = []
	for (var keyn = 0; keyn < keys.length; keyn++) {

		//return '' if record cannot be read or key if blank
		results[keyn] = ''
		var key = keys[keyn]
		if (!key) continue

		//get a record from the cache
		var cachekey = 'READ\r' + filename + '\r' + key
		var rec = readcache(cachekey)
		if (typeof rec != 'undefined') {
			// Known miss (null) → leave results[keyn] '' (same as NO RECORD)
			if (rec === null)
				continue
			// Hit with body — xlate logic on cached record
			results[keyn] = await exodusxlatelogic(filename, (key + fm + rec).split(fm), fieldno, mode, key)
		}
		// Not in cache — batch SELECT later
		else {
			if (!uncachedkeys.exoduslocate(key)) uncachedkeys[uncachedkeys.length] = key
		}

	}

	//select all the uncached records
	if (uncachedkeys.length) {

		//WARNING !!!
		//unfortunately SELECT\r\r\rRECORD returns raw record without
		//the usual postread processing and it puts this into the cache for
		//later processing as well (perhaps it should not put it in the cache)
		//it is difficult to call the usual postread processing from SELECT2
		//on the server since postreads require DOS.COMMON (OREC etc)
		//perhaps one should have a special POSTREADX which only touches @record

		//select the (deduplicated) records or return systemerror
		db.request = 'SELECT\r' + filename + '\r\rRECORD'
		if (!(await db.send(uncachedkeys.join(fm)))) {
			// Client abort/cancel (typeahead supersede, unload) — not a system failure.
			// Keep any cached partials already filled above.
			this.exodusresponse = db.response
			var r = String(db.response || '')
			if (r == 'Cancelled' || r.indexOf('Client cancelled') >= 0)
				return results
			systemerror(db.response)
			return []
		}

		//if no data returned then all uncached keys are misses
		if (!db.data) {
			for (var uk = 0; uk < uncachedkeys.length; uk++)
				writecache('READ\r' + filename + '\r' + uncachedkeys[uk], null)
			return results
		}

		//process the multiple records separated by rm char characters
		var foundkeys = {}
		var recset = db.data.split(rm)
		for (ii = 0; ii < recset.length; ii++) {

			//do xlate logic on the record
			var keyrec = recset[ii].split(fm)
			var key = keyrec[0]
			foundkeys[key] = true
			var result = await exodusxlatelogic(filename, keyrec, fieldno, mode, key)

			//store the results whereever they are needed
			var keyn = 0
			while (keyn = keys.exoduslocate(key, '', keyn + 1)) {
				results[keyn - 1] = result
			}

			//save in the cache regardless
			var cachekey = 'READ\r' + filename + '\r' + key
			writecache(cachekey, keyrec.slice(1).join(fm))

		}

		// SELECT only returns existing keys — mark the rest as known misses (null)
		for (var uk = 0; uk < uncachedkeys.length; uk++) {
			if (!foundkeys[uncachedkeys[uk]])
				writecache('READ\r' + filename + '\r' + uncachedkeys[uk], null)
		}

	} //any uncachedkeys

	return results

}

//get a (cached) database value given a filename, fieldno
//fieldno 0 means return whole record as simple array
//mode can be undefined, C (means return key if no record) and SUM means add up mvs
//zzz SHOULD return '' if no record and null if there is any error
String.prototype.exodusxlate = async function stringxlate(filename, fieldno, mode) {

	key = this.toString()
	if (key == '') return ''

	exodusassertnumeric(fieldno, 'xlate', filename + ' ' + key)

	var record = []
	await record.exodusread(filename, this)
	if (db.response.indexOf('file is not available') >= 0) systemerror('xlate', db.response)

	return await exodusxlatelogic(filename, record, fieldno, mode, key)

}

async function exodusxlatelogic(filename, record, fieldno, mode, key) {
	if (record.length) {
		if (typeof fieldno != 'undefined') {
			if (fieldno) {
				record = record[fieldno]
				if (typeof record == 'undefined') record = ''
			}
		}
		//option to sum the result (good for adding up multivalues)
		if (mode && mode == 'SUM') {
			record = record.exodussum()
		}
	}
	else {

		//option to return the key if failed to get record
		//zzz ?! C mode doesnt work since record length is zero here
		//record=(mode&&mode=='C')?record[0]:''
		record = (mode && mode == 'C') ? key : ''
	}

	return record

}

async function exodusfilepopup(filename, cols, coln, sortselectionclause, many, filtertitle, maxnrecs) {
	//filename is required
	//cols is required (array of arrays)
	//eg [['COMPANY_NAME','Company Name'],['COMPANY_CODE','Company Code']]

	//sortselectionclause is optional
	//eg 'WITH TYPE1 = "X" AND WITH = TYPE2 "Y" BY TYPE1 BY NAME'
	if (!sortselectionclause)
		sortselectionclause = ''

	if (!filtertitle)
		filtertitle = ''

	//sortselectionclause can be an array of keys
	if (typeof sortselectionclause == 'object')
		sortselectionclause = sortselectionclause.join('" "').exodusquote()

	//get a list of col names sep by spaces
	var collist = ''
	for (var i = 0; i < cols.length; i++)
		collist += ' ' + cols[i][0]
	collist = collist.slice(1)
	if (!collist.split(' ').exoduslocate('ID'))
		collist += ' ID'

	//increased from 1000 since changelog records>1000 and no way to
	if (typeof maxnrecs == 'undefined')
		maxnrecs = 2000

	//get the data from the server
	sortselectionclause = sortselectionclause.replace(/[\r\n]/g, ' ')
	collist = collist.replace(/[\r\n]/g, ' ')
	db.request = 'CACHE\rSELECT\r' + filename.toUpperCase() + '\r' + sortselectionclause + '\r' + collist + '\rXML\r' + maxnrecs
	//db.request='CACHE\rSELECT\r'+filename.toUpperCase()+'\r'+sortselectionclause+'\r'+collist+' ID'
	if (!(await db.send())) {
		await exoui_invalid(db.response)
		return null
	}

	//cancel if no records found
	if (db.data.indexOf('<RECORD>') < 0) {
		var msg = 'Sorry, no records found'
		if (filtertitle)
			msg += '\nfor ' + filtertitle
		else if (sortselectionclause.indexOf('WITH COMPANY_CODE') >= 0)
			msg += '\nfor the chosen company'
		await exoui_invalid(msg)
		return null
	}

	var question
	if (filtertitle)
		question = 'Which do you want?' + filtertitle

	return await exoui_decide2(question, db.data, cols, coln, '', many)

}

//////////////////////
// global functions //
//////////////////////

//similar function in xhttp.asp
function exodusdblink() {

	this.request = ''
	this.data = ''
	this.response = ''
	this.documentprotocolcode = document.protocolcode
	this.login = exodusdblink_login

	//maybe preset by php in some heading script

	//used in cache and elsewhere
	if (!gdataset) gdataset = exodusgetcookie2('dataset')
	//gusername = exodusgetcookie2('username')
	gusername = exodusgetcookie2('username') || gusername
	//alert('xyz2 EXODUS2/username='+exodusgetcookie2('username'))
	//alert(document.cookie)
	gsystem = exodusgetcookie2('', 'EXODUSsystem', null) || gsystem

	// alert(gdataset+'*'+gusername+'*'+gsystem)
	this.dataset = gdataset
	this.system = gsystem
	this.username = gusername

	//setup environment for http messaging
	if (this.documentprotocolcode != 'file') {

		// native XMLHttpRequest
		try {
			this.XMLHTTP = new XMLHttpRequest()
			this.send = exodusdblink_send_byhttp_using_xmlhttp
			return
		}
		catch (e) { }

		// ActiveX XMLHTTP fallback
		try {
			this.XMLHTTP = new ActiveXObject('Microsoft.XMLHTTP')
			this.send = exodusdblink_send_byhttp_using_xmlhttp
		}

		//asp forms
		catch (e) {
			this.send = exodusdblink_send_byhttp_using_forms
		}

		return

	}

	//otherwise setup environment for file messaging

	//try and get the username, password and dataset
	this.password = exodusgetcookie2('password')
	this.timeout = exodusgetcookie2('timeout')

	//default timeout is 10 minutes (NB GIVEWAY timeout is hard coded to 10 mins?)
	var defaulttimeoutmins = 10
	//var defaulttimeoutmins=.25//15 seconds
	if (!this.timeout)
		this.timeout = defaulttimeoutmins * 60 * 1000


	this.send = exodusdblink_send_byfile
	this.start = exodusdblink_startdb

	try {
		gfso = new ActiveXObject('Scripting.FileSystemObject')
	}
	catch (e) {
		alert('Error: While creating Scripting.FileSystemObject\n' + e.number + ' ' + e.description)
		window.location.assign(EXODUSlocation + 'securityhowto.htm')
		return
	}

	this.exodusrootpath = getexodusrootpath(document.location.toString())

	//localhostname
	//remove space . " ' characters
	//and convert _ to -
	this.localhostname = new ActiveXObject('WScript.Network').ComputerName.slice(0, 8)
	this.wscriptshell = new ActiveXObject('WScript.Shell')

	this.remoteaddr = ''
	this.remotehost = new ActiveXObject('WScript.Network').ComputerName.toString()
	//remove space . " ' characters
	this.remotehost = this.remotehost.replace(/[\ |\.|\"|\']/g, '')
	this.https = 'off'//Request.ServerVariables("HTTPS")

}

async function dblogout() {

	//remove username etc
	//exodussetcookie('','EXODUS',gdataset,'dataset',true)

	//remove last page
	//exodussetcookie('','EXODUS','','ll',true)

	exodussetcookie(glogincode, 'EXODUS2', '', 'username')
	//alert('xyz EXODUS2/username='+exodusgetcookie2('username'))

	//remove settings
	//alert('before clear '+document.cookie)
	exodussetcookie(glogincode, 'EXODUS2', '')
	exodussetcookie(glogincode, 'EXODUS', '')
	//alert('after clear '+document.cookie)

}

async function exodusdblink_login(username, password, dataset, system) {

	//get list of datasets from server
	var logindb = new exodusdblink
	var datasets = ''

	if (!dataset && gdataset)
		dataset = gdataset
	if (!dataset)
		dataset = exodusgetcookie2('dataset', 'EXODUS', '')
	if (!system)
		system = exodusgetcookie2('', 'EXODUSsystem', null)
	var arguments = ['', '', dataset, '', '', '', system]
	var failed = false

	// Drop typeahead panel before Resume/modal login (DOMUI under onchange)
	if (typeof form_typeahead_hide == 'function')
		form_typeahead_hide()

	//maybe relogging after request to relogin (eg expired session on server)
	if (gusername) {
		var question = 'Your session has timed out\nor been lost to another login or another computer or browser\nor the server has been restarted.'
		//current work is cleared without option to recover if detect login on another computer or browser
		if (glocked && gtouched)
			question += '\n\nWarning! Your current work on ' + gkey + ' will be lost if you dont resume login.'
		question += '\n\nResume login as ' + gusername + '?'
		if (!(await exoui_yesno(question, 1))) {

			// Refuse resume: leave this form, full login shell (same for all callers,
			// including typeahead under evaluate — no force_an_exit throw).
			// Yes path: cookie auto-login or modal index.html, then send retries.
			exodussettimeout('window.location.assign("../index.html")', 1)

			//try to avoid unlocking on exit
			glocked = false
			settouched(false)
			gkey = ''
			this.requesting = false
			db.requesting = false

			await exoui_invalid()
			// Async callers (send → typeahead → evaluate) return false cleanly;
			// page navigation follows. Do not throw undefined force_an_exit___…
			// (that became System Error under typeahead onchange evaluate).
			return false

		}

		arguments[0] = gusername
	}

	while (true) {

		arguments[4] = datasets
		if (!(typeof event != 'undefined' && event && event.shiftKey) && !failed && (exodusgetcookie2('a', 'EXODUS', '') == 'true' || username)) {
			arguments[0] = username ? username : exodusgetcookie2('u', 'EXODUS', '')
			arguments[1] = password ? password : exodusgetcookie2('p', 'EXODUS', '')
			arguments[2] = dataset
			arguments[5] = exodusgetcookie2('a', 'EXODUS', '')
			arguments[6] = system
		}
		else {

			//if refreshing login
			if (dataset) {
				datasetx = new Object
				datasetx.code = new Object
				datasetx.code.text = dataset
				datasetx.name = new Object
				datasetx.name.text = dataset
				datasets = new Object
				datasets.group1 = [datasetx]
			}

			//otherwise get all datasets
			if (!datasets) {
				logindb.request = 'GETDATASETS'
				if (!(await logindb.send())) {
					await exoui_invalid(logindb.response)
					return 0
				}
				datasets = exodusxml2obj(logindb.data)
			}

			arguments[4] = datasets

			url = '../index.html'
			arguments = await exoui_showmodaldialog(url, arguments)

		}

		//quit if user cancels
		if (!arguments) return 0

		glogincode = (arguments[2] + '*' + arguments[0] + '*').replace(/ /g, '')
		exodussetcookie('', 'EXODUSlogincode', glogincode, 'logincode')

		logindb.request = 'LOGIN\r' + arguments[0] + '\r' + arguments[1] + '\r' + arguments[2] + '\r' + arguments[3] + '\r\r' + arguments[5]

		if (!(await logindb.send())) {
			var msg = logindb.response
			if (!msg)
				msg = 'Invalid username or password'
			var response = await exoui_invalid(msg)
			failed = true
		}
		else {

			//set the username and dataset
			gusername = arguments[0]
			gdataset = arguments[2]
			gsystem = arguments[6]
			this.dataset = gdataset
			this.system = gsystem
			this.username = gusername

			//permanent cookie
			var temp = 'dataset=' + dataset
			if (arguments[5].toString() == 'true') {
				temp += '&u=' + gusername
				temp += '&p=' + arguments[1]
				temp += '&a=' + arguments[5]
				temp += '&s=' + arguments[6]
			}
			else {
				//temp+='&u='
				//temp+='&p='
				//temp+='&a='
			}
			exodussetcookie('', 'EXODUS', temp, '')

			//temporary cookie for menu
			exodussetcookie(glogincode, 'EXODUS2', logindb.data)

			await clientfunctions_getglobals()

			//temporary cookie for the dataset and username (and password for file protocol)
			var temp = 'dataset=' + gdataset + '&username=' + gusername + '&system=' + gsystem
			if (document.protocolcode == 'file') {
				this.password = arguments[1]
				temp += '&password=' + this.password
				//this.timeout=exodusgetcookie2('timeout')
			}
			exodussetcookie(glogincode, 'EXODUS2', temp)

			//quit success
			return 1

		}
	}
}

async function exodusdblink_send_byhttp_using_forms(data) {

	//log(this.request)
	//alert('exodusdblink_send_byhttp_using_forms\n...\n'+this.request+'\n...\n'+ data)
	this.data = data ? data : ''

	//request is required
	if (this.request == '') {
		this.data = ''
		this.response = ('ERROR: NO REQUEST 2')
		this.response = this.response.replace(/"/g, "'")
		this.result = ''
		return (0)
	}

	//try to use cache
	var request2 = this.request
	var trycache = (request2.slice(0, 6) == 'CACHE\r')
	if (trycache) {
		request2 = request2.slice(6)
		var cached = dblink_cache_apply(this, request2)
		if (typeof cached != 'undefined')
			return cached
	}

	var gotresponse = false
	while (!gotresponse) {

		//var params='dialogHeight:100px; dialogWidth:200px; center:Yes; help:No; resizable:No; status:No'
		//params='dialogHeight: 201px; dialogWidth: 201px; dialogTop: px; dialogLeft: px; center: Yes; help: Yes; resizable: Yes; status: Yes;'
		var reply = await exoui_showmodaldialog(EXODUSlocation + 'rs/index.html', [this.timeout, this.request, this.data])  // now async path
		if (!reply) {
			this.data = ''
			this.response = ('ERROR: Request to server failed')
			this.result = ''
			return 0
		}

		//extract the response parameters
		var result = reply[0].toString()
		this.response = reply[1].toString()
		this.data = reply[2].toString()

		var lcresponse = this.response.toLowerCase()
		if (lcresponse.indexOf('Please login') >= 0) {
			//if (lcresponse.indexOf('automatic') >= 0) {
			//	glocked=false
			//	gtouched=false
			//}
			// Universal reauth — PROGRAMMERS_OVERVIEW §4 Session lost mid-request
			// (typeahead quiet: same login(); quiet ≠ skip reauth)
			if (!(await this.login() )) {
				this.data = ''
				this.response = ('ERROR: Please login')
				this.result = ''
				return (0)
			}

		}
		else {
			gotresponse = true
		}

	}

	if (result.split(' ')[0] == 'OK' || result == 1) {

		//save in cache
		//perhaps only cache READO requests to avoid
		// initiating cache to speed things up
		if (this.documentprotocolcode == 'file' && (trycache || request2.slice(0, 4) == 'READ')) {
			writecache(request2, this.data)
		}

		return true
	}
	else {
		if (this.response && this.response.indexOf('NO RECORD') >= 0)
			dblink_cache_store_norecord(request2, trycache)
		return false
	}

}

var gxhttp

async function exodusdblink_send_byhttp_using_xmlhttp(data) {

	//log(this.request)

	var ignoreresult = (typeof this.request == 'string') && (this.request.slice(0, 6) == 'RELOCK' || this.request.slice(0, 9) == 'KEEPALIVE' || this.request.slice(0, 6) == 'CANCEL')
	//indicate to refresher when last activity was
	if (ignoreresult)
		exodussetcookie('', 'EXODUSlc', new Date(), 'lc')

	//prevent reuse
	if (this.requesting) {
		this.data = ''
		this.response = ('ERROR: ALREADY REQUESTING:\r' + this.requesting + '\r --- \rNEW REQUEST:\r' + this.request + this.data)
		this.response = this.response.replace(/"/g, "'")
		this.result = ''
		return 0
	}
	//this.requesting=true
	this.requesting = this.request

	this.data = data ? data : ''

	//request is required
	if (this.request == '') {
		this.data = ''
		this.response = ('ERROR: NO REQUEST 3')
		this.response = this.response.replace(/"/g, "'")
		this.result = ''
		this.requesting = false
		return 0
	}

	//try to use cache
	var request2 = this.request
	var trycache = (request2.slice(0, 6) == 'CACHE\r')
	if (trycache) {
		request2 = request2.slice(6)
		var cached = dblink_cache_apply(this, request2)
		if (typeof cached != 'undefined') {
			this.requesting = false
			return cached
		}
	}

	var dbwaitingwindow
	var gotresponse = false
	var remoteprocedurecallfailed = false
	var rpcserverunavailable = false

	var origrequest = this.request

	while (!gotresponse) {

		var token = glogincode
		var timeout = ''
		var xtext = '<root>'
		xtext += '<token>' + escape(token) + '</token>'
		xtext += '<timeout>' + escape(timeout) + '</timeout>'
		xtext += '<request>' + escape(request2) + '</request>'
		var thisrequest = this.request
		this.request = ''
		//xtext+='<data>' + escape(this.data) + '</data>'
		//dont escape everything otherwise unicode will arrive in the database as encoded text
		//instead of being converted to system default single byte code page in server filesystem/asp
		//convert any ascii incompatible with xml text eg < > &
		//convert the escape character "%" FIRST
		//also convert revelation delimiters so that they arrive unconverted
		//XMLre is something like [\x25\x3C\x3E\x26\ plus the field and value marks etc
		temp = this.data.replace(XMLXXre, function ($0) { return escape($0) })

		xtext += '<data>' + temp + '</data>'
		xtext += '</root>'
		//if (gusername=='EXODUS') alert(xtext)
		/*
		//decide microsoft or moz/netscape
		var moz=false
		try {

		var xhttp = new ActiveXObject('Microsoft.XMLHTTP')
		//var xhttp = new ActiveXObject('Msxml2.XMLHTTP.3.0')
		}
		catch(e) {

		var xhttp=new XMLHttpRequest()
		moz=true
		}
		*/

		var xdoc = xtext

		//package description the request parameters into an xml object
		//if (!moz)
		//{
		// var xdoc = new ActiveXObject('Microsoft.XMLDOM')
		// xdoc.loadXML(xtext)
		//}

		var dbwaitingwindow
		var gotresponse = false
		var remoteprocedurecallfailed = false
		var rpcserverunavailable = false

		var xhttp = this.XMLHTTP
		//var servererrormsg='ERROR: Could not communicate with server. Try again later\r\r'
		var servererrormsg = 'Network or server failure. (No response)\r\rPlease try again later\rand/or get technical assistance\r\r'
		var xhttpaborted = false

		//g because perhaps will be a global variable
		var gasynchronous = !(gonunload || gonbeforeunload)

		// STEP 1 of incremental async/await migration:
		// The XHR network I/O is the isolated leaf. We drive ONLY the wait using
		// a real Promise + the existing fromPromise adapter. This introduces await-capable
		// transport without touching geventhandler, gblockevents, (legacy yield), exodus_resume,
		// starteventhandler, UI modals, or any business logic yield* call sites. (legacy notes)
		var netPromise
		var dbmodalblocked = false
		if (gasynchronous) {

			// KEEPALIVE/RELOCK must not touch modal state or overwrite an in-flight request.
			// quiet: typeahead private dblink — no modal shield (scroll lock
			// pins body; still pointless thrash every keystroke).
			if (!ignoreresult && !this.quiet) {
				gchildwin = { lazy: true }
				//xhttp reference for in-dom Wait/Cancel on uiblockerdiv click (uiblocker_waitcancel_dialog)
				gchildwin.xhttp = xhttp
				gchildwin.xhttprequestid = false
				gxhttp = xhttp
				blockmodalui_sync()
				dbmodalblocked = true
			}

			var self = this;  // for setting response/result from inside XHR callbacks
			netPromise = new Promise((resolve) => {
				xhttp.onreadystatechange = function () {
					if (xhttp.readyState >= 2 && gchildwin && gchildwin.xhttp === xhttp) {
						try {
							var requestid = xhttp.getResponseHeader('X-Exodus-Request')
							if (requestid)
								gchildwin.xhttprequestid = requestid
						} catch (e) { }
					}
				}
				xhttp.onload = function (e) {
					if (xhttp.readyState === 4) {
						if (xhttp.status === 200) {
							//console.log(xhttp.responseText);
							resolve('ok');
						} else {
							// HTTP error status but server may still have returned a body (or not).
							// Preserve old signalling of 'ok' here so control flow proceeds to parse/responseText checks,
							// but make the console + resolved value carry the detail that used to go into resume source.
							const detail = 'OK-with-http-error ' + xhttp.status + ' ' + xhttp.statusText;
							console.error(detail);
							resolve('ok');  // keep original signalling for this branch
						}
					}
				};
				xhttp.onerror = function (e) {
					const detail = 'ERROR exodusdblink_send_byhttp_using_xmlhttp ' + (xhttp.status || '0') + ' ' + (xhttp.statusText || 'network error');
					console.error(detail);
					if (self) {
						self.response = detail;
						self.result = '';
					}
					// Resolve with the rich detail (instead of bare 'error') so it appears in
					// logevent("exodus_resume <in " + value + " from fromPromise") and any
					// higher trace. The caller only uses this for logs / final !OK return false.
					// Also populates this.response immediately so callers see useful db.response.
					resolve(detail);
				};
				xhttp.ontimeout = function () {
					const detail = 'TIMEOUT exodusdblink_send_byhttp_using_xmlhttp ' + thisrequest;
					console.error(detail);
					if (self) {
						self.response = detail;
						self.result = '';
					}
					resolve(detail);
				};
				xhttp.onabort = function (e) {
					const detail = 'ABORT exodusdblink_send_byhttp_using_xmlhttp ' + (xhttp.statusText || '');
					console.error(detail);
					xhttpaborted = true
					if (self) {
						self.response = detail;
						self.result = '';
					}
					if (!gonunload) {
						if (gchildwin && gchildwin.xhttp === xhttp)
							dbsend_cancel_xhttp(gchildwin.xhttprequestid)
						resolve(detail);
					}
				};
			});
		}

		//open
		//console.log('gasynchronous='+gasynchronous)

		//send the xmldoc to the server and get the response
		try {
			//false for synchronous (wait for response before continuing to next statement)
			//xhttp.open('POST', EXODUSlocation + 'scripts/xhttp.asp', async = gasynchronous || ignoreresult)
			xhttp.open('POST', EXODUSlocation + 'scripts/xhttp.php', async = gasynchronous || ignoreresult)
			//this was never required but inserted in the hope that it will
			//avoid unknown problems. Perhaps it is not necessary for active pages like .asp.
			xhttp.setRequestHeader("Pragma", "no-cache");
			xhttp.setRequestHeader("Cache-control", "no-cache");
			//xhttp.setRequestHeader('Content-type','application/x-www-form-urlencoded');
			//consider also putting the following in asp web pages
			//<% Response.CacheControl = "no-cache" %>
			//<% Response.AddHeader "Pragma", "no-cache" %>
			//<% Response.Expires = -1 %>

			//following will asynchronously handle relock failed due to login where
			//by preventing further relocking
			/* actually relocking on duplicate sessions is allowed for the time being TODO block it
			if (ignoreresult) {
				xhttp.onreadystatechange=
				//TODO consider what happens in cases other than 200 and no response at all
				function() {
					if (xmlhttp.readyState==4 && xmlhttp.status==200) {
						if (http.responseText.indexOf('Please login')>=0) {
							stoprelocker()
						}
					}
				}
			}
			*/

		}
		catch (e) {
			dbsend_release_modal(xhttp, dbmodalblocked)
			this.data = ''
			this.response = servererrormsg + e.number + ' ' + e.description + ' in .open()'
			this.result = ''
			dbready(dbwaitingwindow)
			this.requesting = false
			return 0
		}

		//send

		try {
			//needs Tools, Internet Options, Security, Trusted Sites, Sites, Add, Close, OK
			var action = 'Requested ' + new Date() + ' ' + thisrequest
			//dont show pass!
			if (!glogging && thisrequest.indexOf('LOGIN') < 0)
				//window.status = action
				console.log(action);

			var timesent = new Date
			//alert('DEBUG: pre:'+thisrequest)
			xhttp.send(xdoc)
			//alert('DEBUG: post:'+thisrequest)

			//log(action + ' ' + (new Date() - timesent) + 'ms')
			//window.status = ''

			//force an error if ignoreresult since ignoreresult ignores all errors or success
			if (ignoreresult)
				forceanerrorhere = forceanerrorhere

		}
		catch (e) {

			//ignore any errors while ignoreresult
			if (ignoreresult) {
				this.data = ''
				this.response = 'OK'
				this.result = ''
				this.requesting = false
				return true
			}

			//if (e.number==-2146697211) is "The system cannot locate the resource specified"
			dbsend_release_modal(xhttp, dbmodalblocked)
			this.data = ''
			this.response = servererrormsg + ' in .send()\n\n' + (e.number ? (e.number + ' ' + e.description) : e.toString())
			this.response = this.response.replace(/"/g, "'")
			this.result = ''
			dbready(dbwaitingwindow)
			this.requesting = false
			return 0
		}

		//yield here and wait for .send to complete
		if (gasynchronous) {

			///////////////////////////////////////////////////////////////
			// PAUSE HERE via fromPromise bridge.
			// The netPromise resolves from XHR handlers; fromPromise feeds the value
			// into the existing global geventhandler via exodus_resume exactly as before.
			// This is the reliable first conversion of an async leaf to Promise-based code.
			// All callers continue to use "await db.send(...)" in new paths (yield* for legacy generator paths).
			///////////////////////////////////////////////////////////////
			var result = await netPromise

			//match master exodus_resume: close Wait/Cancel confirm and uiblocker when db access completes
			dbsend_release_modal(xhttp, dbmodalblocked)
			dbmodalblocked = false

			// The transport signal (result) is 'ok' on success path or a descriptive
			// string (e.g. "ERROR exodusdblink...") on network failure. We still largely
			// ignore the signal value itself (original code did too) and rely on:
			// - xhttp.responseXML / responseText for normal responses
			// - this.response having been populated in the XHR error handlers above
			// - the post-yield error handling + retry confirm dialog
			// The detail is now visible in logevent traces via the value passed through fromPromise.

		}

		dbready(dbwaitingwindow)

		//response

		//check for valid response
		if (xhttp.responseXML && xhttp.responseXML.firstChild != null) {

			//extract the response parameters
			var responsex = xhttp.responseXML.firstChild
			if (true || responsex.childNodes[0].childNodes) {
				//moz
				//this.data=unescape(responsex.childNodes[0].childNodes[0].nodeValue)
				//this.response=unescape(dbgetnodevalues(responsex.childNodes[1]))
				//var result=unescape(dbgetnodevalues(responsex.childNodes[2]))
				this.data = unescape(dbgetnodesvalue(xhttp.responseXML.getElementsByTagName('data')[0].childNodes))
				this.response = unescape(dbgetnodesvalue(xhttp.responseXML.getElementsByTagName('response')[0].childNodes))
				var result = xhttp.responseXML.getElementsByTagName('result')
				if (result.length)
					result = unescape(dbgetnodesvalue(result[0].childNodes))
				else
					result = ''
				//alert('result '+result+' typeof result'+typeof result)
				if (typeof result == 'undefined') {
					debugger
				}
			}
			else {
				//iexplorer
				this.data = unescape(responsex.childNodes[0].text)
				this.response = unescape(responsex.childNodes[1].text)
				var result = unescape(responsex.childNodes[2].text)
			}

			if (this.response.indexOf('Please login') >= 0) {
				//				var tt=origrequest.slice(0,5)!='LOGIN'?gusername:''
				//				tt=gusername
				//alert(origrequest+' gusername='+tt)
				// Universal reauth — PROGRAMMERS_OVERVIEW §4 Session lost mid-request
				// (typeahead quiet: same login(); quiet ≠ skip reauth)
				tt = ''
				if (!(await this.login(tt) )) {
					this.data = ''
					this.response = ('ERROR: Please login')
					this.result = ''
					this.requesting = false
					return 0
				}

			}
			else {
				gotresponse = true
			}

		}
		else {

			//if iis process is killed (dllhost.exe/IWAM_xxx) then it doesnt work on the first call
			if (!remoteprocedurecallfailed && xhttp.responseText.indexOf('remote procedure') >= 0) {
				remoteprocedurecallfailed = true
				continue
			}

			//if iis process is killed then it doesnt work on the first call
			if (!rpcserverunavailable && xhttp.responseText.indexOf('RPC server') >= 0) {
				rpcserverunavailable = true
				continue
			}

			//aborting/unloading. try to avoid error message in caller
			if (gonunload || gonbeforeunload || xhttpaborted) {
				this.data = ''
				if (xhttpaborted)
					this.response = 'Cancelled'
				else
					this.response = ''
				this.result = ''
				this.requesting = false
				return false
			}

			//system failure, retry/abort?
			//Could be failure to convert server response to xml ... check browser debugger console for errors
			//sometimes caused by unusual data encoding in back end from old data entry systems. Latin versus UTF8.
			var tt = origrequest.split('\r')
			if (tt[0] == 'LOGIN')
				//remove password from message
				tt[2] = ''
			var errormsg = 'System failure. Do you want to retry?\r' + tt.join('\r')
			if (xhttp.responseText.indexOf('ASP 0113') > -1)
				errormsg += '\n\nThe server failed to get a response from the EXODUS database engine'
			errormsg += '\n\n' + unescape(striptags(xhttp.responseText))
			console.log(errormsg)
			if (!(confirm(errormsg))) {
				//xxx
				this.data = ''
				this.response = ('ERROR: ' + unescape(xhttp.responseText))
				this.response = this.response.replace(/"/g, "'")
				this.result = ''
				this.requesting = false
				return 0
			}

		}

	}

	//indicate to refresher when last activity was
	exodussetcookie('', 'EXODUSlc', new Date(), 'lc')

	///if (result.split(' ')[0]=='OK' || result==1)
	if (result.split(' ')[0] == 'OK' || result == 1) {

		//save in cache
		//perhaps only cache READO requests to avoid
		// initiating cache to speed things up
		if (trycache || request2.slice(0, 4) == 'READ')
			writecache(request2, this.data)

		this.requesting = false
		return true
	}
	else {
		if (this.response && this.response.indexOf('NO RECORD') >= 0)
			dblink_cache_store_norecord(request2, trycache)
		this.requesting = false
		return false
	}

}

function dbgetnodesvalue(nodes) {
	var text = ''
	var nn = nodes.length
	for (var ii = 0; ii < nn; ii++)
		text = text + nodes[ii].nodeValue
	return text
}

function dbwaiting() {

	document.body.style.cursor = 'wait'
	var dialogstyle = 'dialogHeight: 50px; dialogWidth: 200px; dialogTop: px; dialogLeft: px; center: Yes; help: No; resizable: Yes; status: No;'
	var windowx = ''//window.showModelessDialog(EXODUSlocation+'wait.htm','',dialogstyle)
	wstatus('Networking, please wait ...')
	return windowx
}

function dbready(windowx) {

	document.body.style.cursor = 'auto'
	wstatus('')
	if (windowx) try { windowx.close() } catch (e) { }
}

// some browsers clear non-permanent cookies when a window.open child is closed
function exodusfixcookie() {
	var cookies = document.cookie.split('; ')
	var npreservedcookies = 0
	var ntopreserve = 10
	//preserve first ntopreserve ASPSESSION type cookies
	for (var ii = 0; ii < cookies.length && npreservedcookies < ntopreserve; ii++) {
		npreservedcookies++
		if (cookies[ii].slice(0, 10) == 'ASPSESSION') {
			cookies[ii] += '; expires=Sun, 31 Dec 2100 23:59:59 GMT'//; path=/'
			document.cookie = cookies[ii]
			//do all
			//return true
		}
	}
	return true
}

function exodussetcookie(loginsessionid, name, value, subkey, permanent) {

	if (glogcookie)
		console.log('EXODUSSETCOOKIE(' + loginsessionid + ', ' + name + ', ' + value + ', ' + subkey + ', ' + permanent + ')')

	//exodusfixcookie()

	//for any particular permanent cookie it must consistently be set true otherwise possible loss of following cookies including ASPSESSION

	/*
	var cookie0='set cookie session:'+loginsessionid+' name:'+name+' subkey:'+subkey+' value:'+value+'\r'
	alert(cookie0)
	var tt=document.cookie.split('; ')
	for (var ii=0;ii<tt.length;ii++) cookie0+='\r'+ii+' '+tt[ii]
	if (permanent&&!(confirm(cookie0))) xyx
	if (permanent&&typeof permanent!='boolean') xyxxxxxxxxx
	*/
	if (!permanent)
		permanent = false

	//value can be multiple values and keys in cookie format x=1&y=2 etc
	//to update many without replacing all
	value = value.toString()
	if (value.indexOf('=') >= 0) {
		var values = value.split('&')
		for (var valuen = values.length - 1; valuen >= 0; valuen--) {
			var valuex = values[valuen]
			if (valuex) {
				valuex = valuex.split('=')
				if (valuex.length < 2) valuex[1] = ''
				exodussetcookie(loginsessionid, name, valuex.slice(1).join('='), valuex[0], permanent)
			}
		}
		return
	}

	if (subkey) {
		// crumbs are separated by ampersands
		var crumbs = exodusgetcookie2('', name, loginsessionid).split('&')
		var emptycrumbn
		for (var i2 = 0; i2 < crumbs.length; i2++) {
			var crumb0 = crumbs[i2].split('=')[0]
			if (crumb0 == subkey) {
				crumbs[i2] = subkey + '=' + value
				subkey = ''
				break
			}
			if (crumb0 == '') emptycrumbn = i2
		}
		if (subkey) {
			if (typeof emptycrumbn != 'undefined') i2 = emptycrumbn
			crumbs[i2] = subkey + '=' + value
		}
		value = crumbs.join('&')
	}
	//if (value.indexOf('&&')>=0) xxxxxx

	//document.cookie='EXODUSsystem=ACCOUNTS'//; expires=Fri 31 Dec 2100 23:59:59 GMT; path=/'
	//value+=';expires=Fri 31 Dec 2000 23:59:59 GMT;'
	var cookie = loginsessionid + name + '=' + escape(value)
	//if (permanent==true)
	if (value.length)
		//x = 1
		cookie += '; expires=Fri, 31 Dec 2100 23:59:59 GMT; SameSite=Strict'
	else
		cookie += '; expires=Fri, 31 Dec 2000 23:59:59 GMT; SameSite=Strict'
	//else if (permanent) cookie+=permanent

	// path=/ so login page and app pages share the same cookie
	cookie += '; path=/'


	//cookie=cookie.replace(/=/,'!')
	document.cookie = cookie
	//alert(loginsessionid+name+'\n'+value+'\n'+document.cookie)
	//document.cookie = name + '=' + value+ 'domain='+window.location.host//+'; path=/;'

	//if (name=='EXODUS2')
	// alert(cookie+'\r\r'+document.cookie)

}

function exodusgetcookie2(subkey, key, loginsessionid) {
	if (!loginsessionid && loginsessionid !== null)
		loginsessionid = glogincode
	else if (loginsessionid === null)
		loginsessionid = ''
	if (!key)
		key = 'EXODUS2'
	var result = exodusgetcookie(loginsessionid, key, subkey)
	///console.log('exodusgetcookie2('+loginsessionid+', '+key+', '+subkey+')='+result)
	return result
}

// Retrieve the value of the cookie with the specified name
function exodusgetcookie(loginsessionid, key, subkey) {

	if (glogcookie)
		var log = 'exodusgetcookie(' + loginsessionid + ', ' + key + ', ' + subkey + ')'

	//var cookie0='GET COOKIE session:'+loginsessionid+' key:'+key+' subkey:'+subkey
	//alert(cookie0)
	//cookies are separated by semicolons
	//accessing cookies in modaldialog windows when there is port number in the URL
	// can throw unspecified security error in some engines
	var cookies = (window.dialogArguments && window.dialogArguments.cookie) ? window.dialogArguments.cookie : document.cookie
	//if (window.dialogArguments&&window.dialogArguments.cookie)
	//alert('W='+window.dialogArguments&&window.dialogArguments.cookie)
	//else
	//alert('C='+document.cookie)
	//cookies=cookies.replace(/#/g,'&')
	//cookies=cookies.replace(/!/,'=')
	cookies = unescape(cookies)
	cookies = cookies.split('; ')
	var fullkey = loginsessionid + key
	for (var i = 0; i < cookies.length; i++) {
		if (cookies[i].split('=')[0] == fullkey) {

			if (typeof subkey == 'undefined' || subkey == '') {
				//return unescape(cookies[i].split('=')[1])
				//return unescape(cookies[i].split('=').slice(1).join('='))
				var cookie = cookies[i].split('=').slice(1).join('=')
				if (glogcookie)
					console.log(log + ' = "' + cookie + '"')
				return cookie
			}

			// crumbs are separated by ampersands
			var crumbs = cookies[i].substr(fullkey.length + 1).split('&')
			for (var i2 = 0; i2 < crumbs.length; i2++) {
				if (crumbs[i2].split('=')[0] == subkey) {
					//return unescape(crumbs[i2].split('=')[1])
					var cookie = unescape(crumbs[i2].split('=').slice(1))
					if (glogcookie)
						console.log(log + ' = "' + cookie + '"')
					return cookie
				}
			}
		}
	}
	if (glogcookie)
		console.log(log + ' = ""')
	return ''
}

//from "client.js" may also be copied in some "client.js" less windows
async function exoui_decide(question, data, cols, returncoln, defaultreply, many, inverted) {
	//data and cols are [[]] or [] or revstr or a;1:b;2 string
	//data cells .text property will be used if present
	//returncoln '' means return row number(s) - 1 based
	//returncoln means column number or property name in data
	// (data columns are usually numeric and 0 based)
	//defaultreply=if returncoln then cell(s) of returncoln otherwise 1 based rown(s)

	if (typeof data == 'undefined')
		data = 'Yes:No'

	//the data might be a db request prefixed with '@'
	if (typeof data == 'string' && data.slice(0, 1) == '@') {
		db.request = data.slice(1)
		if (!(await db.send())) {
			await exoui_invalid(db.response)
			return null
		}
		data = db.data

	}

	//abort if no records found
	if (data == '' || data == '<records>\r\n</records>')
		return await exoui_invalid('No records found')

	if (typeof data == 'string' && data.slice(0, 8) == '<records')
		data = exodusxml2obj(data)

	//xml2obj returns records in .group1
	if (data.group1)
		data = data.group1

	if (defaultreply) {
		if (typeof defaultreply == 'string')
			defaultreply = defaultreply.split(':')
		if (typeof defaultreply != 'object')
			defaultreply = [defaultreply]
	}

	if (!inverted)
		inverted = false
	var dialogargs = exoduscloneobj([question, data, cols, returncoln, defaultreply, many, inverted])
	dialogargs.logincode = glogincode

	//var dialogstyle='dialogHeight: 400px; dialogWidth: 600px; dialogTop: px; dialogLeft: px; center: Yes; help: Yes; resizable: Yes; status: Yes;'

	var results = await exodusconfirm2(dialogargs)
	if (typeof results == 'undefined')
		results = ''

	return rearray(results)

}

function rearray(array) {
	//fix a bug where arrays returned from modaldialogs are missing the method .join()

	if (!array || typeof array != 'object')
		return array

	var rearray = []
	for (var i = 0; i < array.length; i++)
		rearray[i] = array[i]

	return rearray

}

// Quiet find-as-you-type (non-modal). Framework only — no app knowledge.
//
//   await exodus_typeahead(request, cols)
//   await exodus_typeahead(request, cols, coln)
//   await exodus_typeahead(request, cols, coln, options)
//
// request — full db.request string (caller builds module/file/val/select/cmd);
//           omit when options.rows or options.data supplies the list
// cols    — same shape as exoui_decide: [[fieldn|id, title], ...] or 'ID NAME'
// coln    — data field index (or dict id string) written on pick; default 0
// options —
//   wordstart  true → keep only rows where any word in any cell starts with key
//              (master files that SELECT all then filter client-side)
//   rows       prebuilt [[cell,…],…] — skip send/parse (e.g. DEFINITIONS taxes)
//   data       raw response string — skip send, parse only (e.g. pre-filtered multi-hit)
//   filter     function(rows, key) → rows — extra filter after parse / wordstart
//   prefer_prefix  default true — promote identity cells starting with key.
//              Set false when the server already ordered (e.g. schedules/jobs
//              BY-DSND YEAR_PERIOD). Otherwise SKODA* keys float over newer
//              SKCLC… rows and quiet list no longer matches F7/validate order.
//
// Always: STOPPED column drop, prefer_prefix (unless false), form_typeahead_show.
// Response auto-detected: /ACCOUNTLIST/, XML <RECORD>, or FM/VM multi-hit.
// Quiet: no decide/invalid. Empty/fail → hide (+ optional miss tint). Returns true always.
async function exodus_typeahead(request, cols, coln, options) {

	if (typeof form_typeahead_show != 'function' || typeof form_typeahead_hide != 'function')
		return true

	if (!options || typeof options != 'object')
		options = {}
	// Explicit rows: [] is a deliberate miss (e.g. post-filter emptied the list)
	var hasExplicitRows = Object.prototype.hasOwnProperty.call(options, 'rows')
	var hasdata = options.data != null && options.data !== ''
	if ((!request && !hasExplicitRows && !hasdata) || !cols) {
		form_typeahead_hide()
		return true
	}

	var el = (typeof gform_onchange_element != 'undefined') ? gform_onchange_element : null
	var setMiss = typeof form_typeahead_set_miss == 'function'
		? form_typeahead_set_miss
		: function () { }

	// empty typed key → no search (caller usually embeds gvalue in request)
	var key = (typeof gvalue != 'undefined' && gvalue != null) ? String(gvalue) : ''
	key = key.replace(/^\s+|\s+$/g, '')
	if (!key) {
		setMiss(el, false)
		form_typeahead_hide()
		return true
	}

	if (typeof cols == 'string')
		cols = cols.split(' ')
	if (!cols || !cols.length) {
		form_typeahead_hide()
		return true
	}
	var normcols = []
	var colids = []
	for (var c = 0; c < cols.length; c++) {
		var col = cols[c]
		if (typeof col == 'string')
			col = [col, col]
		normcols[c] = col
		colids[c] = col[0]
	}

	var rcoln = coln
	if (typeof rcoln == 'string') {
		var matched = false
		for (var i = 0; i < colids.length; i++) {
			if (colids[i] == rcoln) {
				if (typeof colids[i] == 'number'
					|| (typeof colids[i] == 'string' && colids[i] !== ''
						&& String(Number(colids[i])) === String(colids[i])))
					rcoln = Number(colids[i])
				else
					rcoln = i
				matched = true
				break
			}
		}
		if (!matched)
			rcoln = 0
	}
	if (rcoln == null || typeof rcoln == 'undefined' || rcoln === '')
		rcoln = 0
	rcoln = Number(rcoln)
	if (isNaN(rcoln))
		rcoln = 0

	// Snapshot so a slower older request (INC) cannot overwrite a newer one (INCO)
	var keyAtStart = key
	var seqAtStart = (typeof gform_onchange_seq != 'undefined') ? gform_onchange_seq : 0

	function stillActive() {
		return typeof gform_onchange_seq == 'undefined' || seqAtStart == gform_onchange_seq
	}

	// Quiet empty/fail: hide list. Red miss, or green allownew (new key/record).
	function missOut() {
		if (!stillActive())
			return true
		var allowNew = (typeof form_field_is_allownew == 'function' && form_field_is_allownew(el))
		setMiss(el, allowNew ? 'new' : true)
		form_typeahead_hide()
		return true
	}

	var rows
	if (hasExplicitRows) {
		rows = options.rows || []
	} else if (hasdata) {
		rows = exodus_typeahead_parserows(options.data, colids)
	} else {
		var tdb = (typeof form_typeahead_dblink == 'function') ? form_typeahead_dblink() : db
		// Client CACHE by full request string (gcache cleared on page refresh / Alt+R).
		var req = String(request)
		if (req.slice(0, 6) != 'CACHE\r')
			req = 'CACHE\r' + req
		tdb.request = req
		var sent = await tdb.send()
		// Abort from form_typeahead_dblink_reset (newer keystroke): not a miss.
		if (!sent || !tdb.data) {
			var resp = String(tdb.response || '')
			if (resp == 'Cancelled' || resp.indexOf('ABORT') >= 0
				|| resp.indexOf('Client cancelled') >= 0)
				return true
			return missOut()
		}
		rows = exodus_typeahead_parserows(tdb.data, colids)
	}

	// Superseded by newer typeahead or leave-field validate
	if (!stillActive())
		return true

	// Focus left (e.g. Enter → validate/invalid): do not overlay the panel.
	// Keep miss tint if any — leave-field validation owns the final answer.
	if (!el || document.activeElement !== el) {
		form_typeahead_hide()
		return true
	}

	// Field text moved on since this search (e.g. INC response after user typed INCO)
	var now = (typeof getvalue == 'function') ? getvalue(el) : null
	if (now != null) {
		now = String(now).replace(/^\s+|\s+$/g, '')
		if (!el.getAttribute || !el.getAttribute('exolowercase'))
			now = now.toUpperCase()
		if (now !== keyAtStart)
			return true
	}

	// Master files (supplier/vehicle/…): drop stopped from quiet list only.
	// Exact key still validates on Enter if allowed. Brand typeahead uses SELECT WITH STOPPED EQ "".
	if (rows && rows.length) {
		var stopi = -1
		for (var si = 0; si < colids.length; si++) {
			if (colids[si] == 'STOPPED') {
				stopi = si
				break
			}
		}
		if (stopi >= 0) {
			var kept = []
			for (var ri = 0; ri < rows.length; ri++) {
				var cell = rows[ri] ? rows[ri][stopi] : ''
				if (cell != null && String(cell).replace(/^\s+|\s+$/g, '') !== '')
					continue
				kept.push(rows[ri])
			}
			rows = kept
		}
	}
	// Master SELECT-all then filter: code/name word starts with typed key
	if (options.wordstart)
		rows = exodus_typeahead_wordstart(rows, keyAtStart)
	if (typeof options.filter == 'function')
		rows = options.filter(rows, keyAtStart) || []
	// Prefer rows whose identity cells start with key: pick col (the key/code
	// written on choose), plus first two cells (code+name). Always include the
	// pick key so name-heavy lists still rank e.g. G12 → G123. Do not scan
	// market/supplier/… — vehicle market "SA" was scrambling name order.
	// Skip when prefer_prefix === false (server YEAR_PERIOD order for docs).
	if (options.prefer_prefix !== false) {
		var pcols = [0]
		if (rows[0] && rows[0].length > 1)
			pcols.push(1)
		// Always include pick col (even when 0 — already in list; when >1, add it)
		if (rcoln != null && !isNaN(Number(rcoln))) {
			var rc = Number(rcoln)
			if (rc >= 0 && pcols.indexOf(rc) < 0)
				pcols.push(rc)
		}
		rows = exodus_typeahead_prefer_prefix(rows, keyAtStart, pcols)
	}
	if (!rows || !rows.length)
		return missOut()

	setMiss(el, false)
	form_typeahead_show(el, normcols, rows, rcoln)
	return true
}

// Keep rows where any cell matches typed key as:
//   • whole-cell prefix (account/ledger codes: G12 → G123), or
//   • any whitespace-word prefix (names: "DUBAI" in "DUBAI FZ").
// Covers both code keys and name search in one filter.
function exodus_typeahead_wordstart(rows, key) {
	if (!rows || !rows.length)
		return rows || []
	if (key == null || key === '')
		return rows
	var ku = String(key).replace(/^\s+|\s+$/g, '').toUpperCase()
	if (!ku)
		return rows
	var kept = []
	for (var i = 0; i < rows.length; i++) {
		var row = rows[i]
		if (!row)
			continue
		var hit = false
		for (var c = 0; c < row.length; c++) {
			var cell = String(row[c] == null ? '' : row[c]).toUpperCase()
			cell = cell.replace(/^\s+|\s+$/g, '')
			if (!cell)
				continue
			// whole cell prefix (codes without spaces)
			if (cell.indexOf(ku) === 0) {
				hit = true
				break
			}
			var words = cell.split(/\s+/)
			for (var w = 0; w < words.length; w++) {
				if (words[w] && words[w].indexOf(ku) === 0) {
					hit = true
					break
				}
			}
			if (hit)
				break
		}
		if (hit)
			kept.push(row)
	}
	return kept
}

// Promote rows where a key cell starts with typed key.
// colns — optional list of row indices to test (default: all cells).
// Callers/typeahead pass [0, pick] so market/supplier/etc. do not reorder.
function exodus_typeahead_prefer_prefix(rows, key, colns) {
	if (!rows || rows.length <= 1 || key == null || key === '')
		return rows
	var ku = String(key).replace(/^\s+|\s+$/g, '').toUpperCase()
	if (!ku)
		return rows
	var prefer = []
	var rest = []
	for (var pri = 0; pri < rows.length; pri++) {
		var prow = rows[pri]
		var phit = false
		if (prow) {
			var idxs
			if (colns && colns.length) {
				idxs = colns
			} else {
				idxs = []
				for (var ai = 0; ai < prow.length; ai++)
					idxs.push(ai)
			}
			for (var ii = 0; ii < idxs.length; ii++) {
				var pci = Number(idxs[ii])
				if (isNaN(pci) || pci < 0)
					continue
				// cols may use field-index ids (ACCOUNTLIST); resolve via row length
				var pcell = prow[pci]
				if (pcell == null && prow.length && pci >= prow.length)
					continue
				pcell = String(pcell == null ? '' : pcell)
				pcell = pcell.replace(/^\s+|\s+$/g, '').toUpperCase()
				if (pcell.indexOf(ku) === 0) {
					phit = true
					break
				}
			}
		}
		if (phit)
			prefer.push(prow)
		else
			rest.push(prow)
	}
	return prefer.concat(rest)
}

// Parse quiet typeahead payload → array of row arrays (auto-detect shape).
// /ACCOUNTLIST/  — prefix then FM rows / VM cells (e.g. FINDACCOUNT multi-hit)
// XML            — <RECORD> / group1
// else           — FM rows / VM cells (VAL multi-hit); fm+fm exact body → []
function exodus_typeahead_parserows(data, colids) {

	if (data == null || data === '')
		return []
	data = String(data)

	var alist = data.indexOf('/ACCOUNTLIST/')
	if (alist >= 0) {
		data = data.substr(alist + 13)
		var sepA = (typeof fm != 'undefined') ? fm : '\x1E'
		var vsepA = (typeof vm != 'undefined') ? vm : '\x1D'
		var linesA = data.split(sepA)
		var outA = []
		for (var ja = 0; ja < linesA.length; ja++) {
			if (linesA[ja] == null || linesA[ja] === '')
				continue
			outA[outA.length] = linesA[ja].split(vsepA)
		}
		return outA
	}

	// Single-record payload (RM) — not a pick list
	if (typeof rm != 'undefined' && data.indexOf(rm) >= 0)
		return []

	if (data.indexOf('<RECORD>') >= 0 || data.indexOf('<records') >= 0) {
		if (typeof exodusxml2obj != 'function')
			return []
		var obj = exodusxml2obj(data)
		if (!obj || !obj.group1 || !obj.group1.length)
			return []
		var rows = []
		for (var i = 0; i < obj.group1.length; i++) {
			var rec = obj.group1[i]
			var row = []
			for (var c = 0; c < colids.length; c++) {
				var cell = rec[colids[c]]
				if (cell && typeof cell == 'object' && cell.text != null)
					row[c] = cell.text
				else if (cell != null && typeof cell != 'object')
					row[c] = cell
				else
					row[c] = ''
			}
			rows[rows.length] = row
		}
		return rows
	}

	if (typeof fm != 'undefined' && data.indexOf(fm + fm) >= 0)
		return []
	var sep = (typeof fm != 'undefined') ? fm : '\x1E'
	var vsep = (typeof vm != 'undefined') ? vm : '\x1D'
	var lines = data.split(sep)
	var out = []
	for (var j = 0; j < lines.length; j++) {
		if (lines[j] == null || lines[j] === '')
			continue
		out[out.length] = lines[j].split(vsep)
	}
	return out
}

async function exoui_decide2(question, data, cols, returncoln, defaultreply, many) {

	//new in-window style popup
	//if row columns are not numeric then convert numeric return column number into named column in data rows
	//also done in decide_onload() after splitting string into columns (could remove from here)
	//alert('data:'+data+'\ncols:'+cols+'\nreturncoln:'+returncoln)
	if (typeof cols != 'string') {
		var tt = cols[returncoln]
		if (tt && tt[0])
			returncoln = tt[0]
		//empty returncoln means return row number(s) of option(s) selected
		//if (!returncoln)
		//	returncoln=0
	}
	var results = await exoui_decide(question, data, cols, returncoln, defaultreply, many)

	//callers of decide2 expect reply in array
	if (results && (typeof results == 'string' || typeof results == 'number'))
		results = [results]

	return results

}

async function setdropdown2(element, dataobj, colnames, selectedvalues, requiredvalues, noautoselection) {

	//1st element is automatically selected unless noautoselection (or selectedvalues overrides it)

	if (!(checkisdropdown(element)))
		return 0

	cleardropdown(element)

	//if (no(xmldata.text)) return(0)
	if (no(dataobj.group1))
		return 0

	cleardropdown(element, true)

	var valuecolname
	var textcolname

	if (typeof colnames == 'string' && colnames.indexOf('\r'))
		colnames = colnames.split('\r')

	if (typeof (colnames) == 'string') {
		textcolname = colnames
		valuecolname = colnames
	}
	else {
		valuecolname = colnames[0]
		textcolname = colnames[1]
		if (colnames[2])
			noautoselection = colnames[2]
	}

	//var records=xmldata.firstChild.childNodes
	var records = dataobj.group1

	//var options=[]
	var anyselected = false

	for (var i = 0; i < records.length; i++) {
		var cell = records[i][valuecolname]
		if (typeof (cell) == 'undefined') {
			await exoui_invalid('Error: "' + valuecolname + '" not in data line ' + i + ' for setdropdown2 for "' + element.id + '" (1')
			systemerror('Error: "' + valuecolname + '" not in data line ' + i + ' "' + records[i] + '" for setdropdown2 for "' + element.id + '" (1)')
			return (0)
		}

		var value = cell.text

		if (no(requiredvalues) || requiredvalues.exoduslocate(value)) {
			var option1 = document.createElement('option')
			option1.value = value

			var cell = records[i][textcolname]
			if (typeof (cell) == 'undefined') {
				await exoui_invalid('Error: "' + textcolname + '" not in data line ' + i + ' for setdropdown2 for "' + element.id + '" (2')
				systemerror('Error: "' + textcolname + '" not in data line ' + i + ' for setdropdown2 for "' + element.id + '" (2)')
				return (0)
			}

			//start a new option
			//var option='<OPTION value='+value

			//indicate if selected
			var selected = false
			if (selectedvalues && selectedvalues.exoduslocate(value)) {
				//option+=' selected=true'
				option1.selected = true
				anyselected = true
			}

			var description = decodehtmlcodes(cell.text)

			//add the option description
			// must be done before insertion so width is computed correctly
			option1.innerHTML = description
			//option+=cell.text

			//add into the array of options
			element.insertBefore(option1, null)
			//element.options.add(option1)
			//options[options.length]=option+'</OPTION>'+'\r\n'

		}

	}

	if (noautoselection && !anyselected) {
		var blankoption = document.createElement('option')
		blankoption.value = ''
		blankoption.innerHTML = ''
		element.insertBefore(blankoption, element.childNodes[0])
	}

	//select first option if none selected
	// if (element.selectedIndex==-1) element.selectedIndex=0

	// force element to recalculate width after style change
	//this work around probably no longer needed
	//element.parentNode.replaceChild(element,element)

	//does not seem to do anything
	//getancestor(element,'TABLE').refresh()

}

function cleardropdown(element, all) {

	//login('cleardropdown')

	if (!(checkisdropdown(element))) return (0)

	//remove existing selections (leave one but blank it to avoid "0" showing)
	while (element.childNodes.length > (all ? 0 : 1)) {
		//  element.childNodes[0].removeNode(true)
		//element.removeChild(element.childNodes[0])
		exodusremovenode(element.childNodes[0])
	}
	if (element.childNodes.length) {
		element.childNodes[0].value = ""
		element.childNodes[0].text = ""
	}
	//logout('cleardropdown')
}

function cleardropdownselections(element) {

	if (!(checkisdropdown(element)))
		return (0)

	//	element.selectedIndex=-1
	if (element.childNodes.length)
		element.childNodes[0].selected = true

	//remove existing selections
	for (var i = 0; i < element.childNodes.length; i++) {
		element.childNodes[i].selected = false
	}
}

function checkisdropdown(element) {
	assertelement(element, 'checkisdropdown', 'element')

	if (typeof (element) != 'object' || element.tagName != 'SELECT') {
		void exodus_begin(function () {
			return exoui_invalid('Error: The target is not a SELECT tag')
		}, 'checkisdropdown')
		return false
	}
	return true
}

function setdropdown3(element, dropdowndata, colns, selectedvalues, requiredvalues) {

	//NB cannot update elements options if currently bound to data because IE busts?
	//try clonenode+replacenode?

	//parameters
	////////////

	//dropdowndata can be a string (even '') with character seps :; or fm vm (used if vm or fm found)
	//if null will not set new options, only select the selectedoptions
	//exodusrequired false means add a blank option at the beginning

	//element must be a select element
	if (!(checkisdropdown(element))) {
		//log('setdropdown3 - not a select element')
		return false
	}

	////login('setdropdown3 '+element.id+' '+exodusquote(dropdowndata))
	//login('setdropdown3 ' + element.id)

	//dropdowndata may be an array of strings or a string with ;: seps for cols/rows respectively
	//if [] then dropdowndata are not updated otherwise existing dropdowndata are deleted
	if (typeof (dropdowndata) == 'undefined' || dropdowndata == null) dropdowndata = []

	//convert conversion string to an array
	if (typeof (dropdowndata) == 'string') {

		//make sure we get at least one conversion
		if (dropdowndata == '') dropdowndata = ';'

		//convert into an array
		var sepchars = (dropdowndata.indexOf(vm) >= 0 || dropdowndata.indexOf(fm) >= 0) ? fm + vm : ':;'
		dropdowndata = dropdowndata.exodussplit(sepchars)

	}

	//if not required make sure that the first dropdown option is ""
	if (dropdowndata.length && !element.getAttribute('exorequired')) {
		if (dropdowndata[0][0] != '' && dropdowndata[dropdowndata.length - 1][0] != '') {
			//ripple up, then replace first element
			for (var i = dropdowndata.length; i > 0; i--) dropdowndata[i] = dropdowndata[i - 1]
			dropdowndata[0] = ['', '']
		}
	}

	//colns may be array(2) being pointers into dropdowndata for option value and text
	//otherwise [0,1]
	if (!(is(colns))) colns = [0, 1]
	valuecoln = colns[0]
	textcoln = colns[1]

	//selectvalues is optionally the value(s) that are to be "selected"
	//optional, can be array or single
	if (typeof (selectedvalues) == 'undefined') selectedvalues = []
	if (typeof (selectedvalues) != 'object') selectedvalues = [selectedvalues]

	//requiredvalues is optionally the value(s) that are to be included in the dropdown
	//optional, can be array or single
	if (typeof (requiredvalues) == 'undefined') requiredvalues = []
	if (typeof (requiredvalues) != 'object') requiredvalues = [requiredvalues]

	//await exoui_invalid(selectedvalues.join())
	//method
	////////

	if (dropdowndata.length) {

		cleardropdown(element, true)

		for (var i = 0; i < dropdowndata.length; i++) {
			var value = dropdowndata[i][valuecoln]
			if (!value)
				value = ''
			if (no(requiredvalues) || requiredvalues.exoduslocate(value)) {

				var text = dropdowndata[i][textcoln]
				if (typeof (text) == 'undefined' || text == '') {
					text = value
					if (typeof text == 'undefined') text = ''
					else text = text.toString().exodusconvert('_', ' ').exoduscapitalise()
				}

				addoption(element, value, text)

			}
		}
	}

	//mark selected options
	if (selectedvalues.length) {

		//clear existing selections
		if (!dropdowndata.length) cleardropdownselections(element)

		for (var j = 0; j < selectedvalues.length; j++) {
			var selectedvalue = selectedvalues[j]
			for (var i = 0; i < element.childNodes.length; i++) {
				var option = element.childNodes[i]
				//if (selectedvalues.exoduslocate(option.value)) option.selected=true
				//if (selectedvalues.exoduslocate(option.text)) option.selected=true
				if (option.value == selectedvalue || option.text == selectedvalue) {
					try {
						option.selected = true
					} catch (e) { } // some engines throw here
					break
				}
			}

			//add option if setting to a value not in the dropdown
			//(or error)
			if (i >= element.childNodes.length) {
				//except setting to '' means reset to first option

				if (selectedvalue == '') {
					// (also dataitem.text! why???)
					// but this has the result that
					//all first row drop downs are reset to first option
					//in the binding blankrowx so do not do it
					//ok do it only if bound and make sure not bound while clearing line 1
					if (element.dataitem) element.dataitem.text = element.value
				}
				else {
					//	 systemerror('setdropdown3()','"'+selectedvalue+'" is not in the dropdown')
					option = addoption(element, selectedvalue, selectedvalue)
					option.selected = true
				}
			}

		}

	}

	//force option 1 if no options
	// if (element.getAttribute('exofieldno')!='0'&&element.selectedIndex==-1) element.selectedIndex=0

	//logout('setdropdown3')

	return true

}

function addoption(element, value, text) {

	//build an option
	var option1 = document.createElement('option')
	option1.value = value
	option1.text = text

	//insert it into the dropdown
	//element.add(option1)//does not work on the mac for option 1 after clearing
	element[element.length] = option1

	return element[element.length - 1]

}

function exodusxml2obj(xmltext) {

	var dataobj = new Object
	dataobj.group1 = []
	var recn = -1
	var currentrow
	xml = xmltext.split('<')
	for (var fragn = 1; fragn < xml.length; fragn++) {
		var frag = xml[fragn].split('>')
		if (frag[0].toLowerCase() != 'records' && frag[0].toLowerCase() != '/records') {
			if (frag[0].toLowerCase() == 'record') {
				dataobj.group1[++recn] = currentrow = new Object
			}
			else {
				if (frag[0].slice(0, 1) != '/') {
					//currentrow[frag[0]]=(new Object).text=frag[1]
					var cell = new Object
					var fragdecoded = HTMLDecode(frag[1]).replace(/%25/g, '%')
					cell.text = fragdecoded
					currentrow[frag[0]] = cell
				}
			}
		}
	}

	return dataobj

}

async function exodussetdropdown(element, request, colarray, selectedvalues, noautoselection) {

	if (!(checkisdropdown(element)))
		return (0)

	db.request = request
	if (await db.send()) {

		//console.log('exodussetdropdown:'+db.data)

		dataobj = exodusxml2obj(db.data)

		//xmltemp=new ActiveXObject('Microsoft.XMLDOM')
		//xmltemp.loadXML(db.data.replace(/\&/g,'+'))
		////xmltemp.loadXML(db.data)
		await setdropdown2(element, dataobj, colarray, selectedvalues, null, noautoselection)
		if (db.data == '' || !dataobj.group1.length)
			element.setAttribute('exodropdown', '')
	}
	else {
		await exoui_invalid(db.response)
	}
	return true
}

function getdropdown0(element) {

	var index = element.selectedIndex

	// some engines use index=length when only one option
	if (index >= element.length) index = 0

	// -1 means not selected
	if (index < 0) return ''

	return element[index].value

}

function exodusgetdropdown(element, mode) {
	if (!(is(mode)))
		mode = 'selected'//otherwise 'all'
	var selectedvalues = []
	for (var i = 0; i < element.options.length; i++) {
		if (mode == 'all' || element.options[i].selected) {
			selectedvalues[selectedvalues.length] = element.options[i].value
			if (mode != 'all' && !element.multiple)
				return selectedvalues.join()
		}
	}
	if (mode != 'all' && !element.multiple)
		selectedvalues = selectedvalues.join()
	return selectedvalues
}

// One-line text prompt (OK / Cancel). See AGENTS.md HIGH PRIORITY confirm/exoui_input.
// DO NOT change return values without grepping every exoui_input caller:
//   OK → string (may be ''); Cancel → false
// WRONG when empty OK is intentional (e.g. material code "OK if not known"):
//   if (!reply) { /* default path */ }   // Cancel takes default path → "no records found"
// RIGHT:
//   if (typeof reply != 'string') return …  // Cancel only
//   // reply is string, maybe ''
// Similar: media.js media_pop_materials, allocation2.js filtertext.
async function exoui_input(question, text, texthidden) {
	if (!text) text = ''
	return await exoui_confirm(question, '', 'OK', '', 'Cancel', text, texthidden)
}

async function exoui_yesno(question, defaultbutton) {
	return ((await exoui_confirm(question, defaultbutton, 'Yes', 'No')) == 1);
}

async function exoui_okcancel(question, defaultbutton) {
	return await exoui_confirm(question, defaultbutton, 'OK', '', 'Cancel')
}

// default_icons (9th, default true): role icons on footer buttons — 1=check, 2=cross, 3=goback.
// Pass false when labels are alternatives (Before/After, One/Many, Save only vs …) not Yes/No/Cancel.
async function exoui_confirm(question, defaultbutton, yesbuttontitle, nobuttontitle, cancelbuttontitle, text, texthidden, image, default_icons) {

	//clean up question
	if (!question)
		question = 'OK to continue?'
	question = question.toString()
	if (question.slice(0, 6) == 'Error:')
		question = question.slice(6)

	console.log(question)

	// In-window popup (exodusconfirm2) for questions and text/password input.
	return await exodusconfirm2(question, defaultbutton, yesbuttontitle, nobuttontitle, cancelbuttontitle, text, texthidden, image, default_icons)

}

function striptags(string) {
	var temp = document.createElement('div')
	temp.innerHTML = string
	return temp.innerText
}

function getancestor(startelement, ancestorTag) {

	if (startelement == null) return (null)

	var ancestor = startelement.parentNode
	//while (ancestor!=null && ancestor.tagName!=ancestorTag)
	ancestorTag = ancestorTag.toUpperCase()
	while (ancestor != null && ancestorTag.indexOf(ancestor.tagName) == -1) {
		ancestor = ancestor.parentNode
	}
	return (ancestor)
}

/////////////////
//DEBUG FUNCTIONS
/////////////////

function wstatus(msg) {
	//needs Tools, Internet Options, Security, Trusted Sites, Sites, Add, Close, OK
	//or "Allow scripting to update status bar"
	//window.status = msg
	// Skip empty — old status-bar clear; console spam only
	if (msg == null || msg === '')
		return
	console.log(msg);
}

function loadcache() {

	//if gcache available already
	try {
		// parent may be closed — gcache.values access can throw permission denied
		if (typeof gcache == 'object' && gcache != null && gcache.values)
			return true
	} catch (e) {
		gcache = null
	}

	//login('loadcache')
	//sometimes error on accessing window.opener.gcache
	//Error: Permission denied to access property "gcache"
	try {
		//if (window.opener && window.opener.gcache) {
		if (window.opener && window.opener.gcache && gdataset && window.opener.gdataset == gdataset) {
			gcache = window.opener.gcache
			// parent may be closed — gcache.values access can throw permission denied
			if (gcache.values)
				temp = true
		}
	} catch (e) {
		gcache = null
	}

	if (!gcache) {
		//create a global variable span to hold the userdata
		gcache = document.createElement('span')
		gcache.values = new Object
	}

	//logout('loadcache - ok loaded')

	return true

}

function showcache() {

	if (!gcache) return

	try {
		var values = gcache.values
	}
	catch (e) {
		return
	}

	//work out the total cache size (values only)
	var cachesize = 0
	var cachen = 0
	var html = ''
	for (var key in values) {
		if (key.slice(0, 6) != 'exodus') {
			html += '<tr><td>' + key + '</td><td>' + values[key] + '</td></tr>'
		}
	}
	if (html) {
		var win = window.open()
		exodus_ensure_readable_blank(win)
		if (win && win.document && win.document.body)
			win.document.body.innerHTML = '<table>' + html + '</table>'
	}

}

//prune cache every x seconds
var gcachepruneintervalsecs = 100
var gmaxcachesize = 1024000
var gmaxcachen = 100
exodussetinterval('trimcache()', gcachepruneintervalsecs * 1000)
function trimcache() {

	if (!gcache) return

	try {
		var values = gcache.values
	}
	catch (e) {
		return
	}

	//work out the total cache size (values only)
	var cachesize = 0
	var cachen = 0
	for (var key in values) {
		if (key.slice(0, 6) != 'exodus') {
			cachesize += values[key].length
			cachen++
		}
	}

	var result = [cachesize, cachen]
	if (gusername == 'EXODUS') wstatus('trimcache() cachesize:' + cachesize + ' cacheitems:' + cachen)

	//delete initial keys until the size is below the desired limit
	//hopefully the initial keys are the oldest
	for (var key in values) {
		if (cachesize <= gmaxcachesize && cachen <= gmaxcachen) break
		if (key.slice(0, 6) != 'exodus') {
			cachesize -= values[key].length
			cachen--
			delete values[key]
		}
	}

	return result

}

function prunecache(request) {

	//prunes all entries starting with request and returns the number of entries pruned

	//also prunes in window.opener and window.parent (and up chain if any)
	if (window.opener) {
		try {
			window.opener.prunecache(request)
		}
		catch (e) { }
	}
	else if (window.parent && window.parent != window) {
		try {
			window.parent.prunecache(request)
		}
		catch (e) { }
	}

	if (!gcache) return

	try {
		var values = gcache.values
	}
	catch (e) {
		return
	}

	var npruned = 0
	var prunekey = cachekey(request)

	for (var key in values) {
		if (key.slice(0, prunekey.length) == prunekey) {
			delete values[key]
			npruned += 1
		}
	}

	return npruned

}

function readcache(request) {

	//login('readcache')
	// Returns: undefined = not in cache; null = known NO RECORD; string = body.

	if (!(loadcache())) {
		//logout('readcache - loadcache failed')
		return undefined
	}

	var key = cachekey(request)
	// Prefer object store (can hold null for misses). Attribute store is string-only.
	if (gcache.values) {
		if (!Object.prototype.hasOwnProperty.call(gcache.values, key))
			return undefined
		return gcache.values[key]
	}

	var result = gcache.getAttribute(key)
	// DOM: null = attribute absent → not in cache (cannot store null miss here)
	if (result === null)
		return undefined
	return result

}

function cachekey(request) {

	//login('cachekey')

	//return escape(this.dataset+'\r'+this.username+'\r'+request).replace(/%/g,'_')
	// var temp=escape(gdataset+'_'+request).replace(/%/g,'_').replace(/@/g,'_AT_').replace(/\//g,'_SLASH_').replace(/\*/g,'_STAR_')
	var temp = escape(request).replace(/%/g, '_').replace(/@/g, '_AT_').replace(/\+/g, '_PLUS_').replace(/\//g, '_SLASH_').replace(/\*/g, '_STAR_')

	//alert(temp)

	//logout('cachekey ' + temp)

	return temp

}

function writecacherecord(data, filename, key) {
	//if data is an array then assume data[0] is the key and remove it
	//default key from data[0]
	if (typeof data == 'object') {
		if (!key) key = data[0]
		data = data.slice(1).join(fm)
	}
	writecache('READ\r' + filename + '\r' + key, data)
}

function deletecacherecord(filename, key) {
	deletecache('READ\r' + filename + '\r' + key)
}

function writecache(request, data) {
	//login('writecache')
	// data: string body, or null = known NO RECORD (object store only)

	if (!(loadcache())) {
		//logout('writecache loadcache failed')
		return false
	}

	if (gcache.values)
		gcache.values[cachekey(request)] = data
	else if (data === null)
		// cannot represent miss in string attributes — leave uncached
		return false
	else
		gcache.setAttribute(cachekey(request), data)

	gcache.cacheupdated = true

	//logout('writecache')

	return true

}

function deletecache(request) {
	//login('deletecache')

	if (!(loadcache())) {
		//logout('deletecache loadcache failed')
		return false
	}

	if (gcache.values) {
		var cacheitem = gcache.values[cachekey(request)]
		if (typeof cacheitem != 'undefined')
			delete gcache.values[cachekey(request)]
	} else
		gcache.removeAttribute(cachekey(request))

	gcache.cacheupdated = true

	//logout('deletecache')

	return true

}

function clearcache() {

	//login('clearcache')

	//force refresh security table
	gtasks = null

	if (!(loadcache())) {
		//logout('clearcache loadcache failed')
		return false
	}

	try {
		//will fail on mac
		if (typeof gcache.load != 'undefined')
			gcache.load('emptycache')

		gcache.values = new Object

		if (typeof gcache.save != 'undefined')
			gcache.save('\exoduscache')
	}
	catch (e) {
		//logout('clearcache fail ' + e.description)
		return false
	}

	gcache.cacheupdated = false
	//logout('clearcache ok')

	return true

}

async function sorttable(event, order) {

	event = getevent(event)

	if (gKeyNodes && !glocked) {
		await readonlydocmsg()
		return false
	}

	var colid = event.target.sorttableelementid

	if (typeof form_presort == 'function') {
		if (!(await form_presort(colid)))
			return await exoui_invalid()
	}

	//determine the groupno
	var dictitem = gds.dictitem(colid)
	if (!dictitem) return await exoui_invalid()
	var groupno = dictitem.groupno
	if (!groupno)
		return await exoui_invalid(colid + ' is not multivalued for sorting')

	//window.status = 'Sorting, please wait ...'
	console.log('Sorting, please wait ...');

	//get the data rows
	if (!groupno) groupno = 1
	var datarows = gds.data['group' + groupno]
	var fromrown = 0
	var uptorown = datarows.length - 1

	var up2down = false
	try {

		var clickedelement = event.target || event.srcElement

		//var clickedelement = document.getElementsByName('sortbutton_' + groupno)[0]

			// Cycle: neutral/down → up → down (data-sort-order or mask URL)
		var cur = exodus_sort_order_of(clickedelement)
		up2down = (cur == 'up')
		var order = (cur == 'down') ? 'up' : 'down'
	}
	catch (e) {
		if (typeof order == 'unassigned')
			order = 'down'
	}

	//get the data
	var data = []
	var dictitemlength
	if (dictitem.align == 'R') {
		dictitemlength = dictitem.length
		if (!dictitemlength) dictitemlength = 20
	}
	var sortdatalevels = []
	//for (var rown=0;rown<datarows.length;rown++)
	rown1 = 0
	rown2 = uptorown
	rowinc = 1
	if (up2down) {
		rown1 = uptorown
		rown2 = 0
		rowinc = -1
	}
	for (var rown = rown1; rown >= 0 && rown < datarows.length; rown += rowinc) {
		var sortdata = datarows[rown][colid].text
		if (dictitemlength) {
			if (sortdata) {
				var temp = new exodusamount(sortdata)
				sortdata = temp.amount.split('.')
				if (!sortdata[1]) sortdata[1] = ''
				sortdata = temp.unit + exodusspace(dictitemlength - sortdata[0].length) + sortdata[0] + temp.amount
			}
		}
		else {
			//if indented then work out prefix from higher levels
			if (sortdata.match(/(^\s+)/)) {

				//refuse to sort in reverse if indented
				if (order == 'up') {
					return await exoui_invalid('Cannot reverse sort when any data is indented')
				}

				var prefix = ''
				var indent = RegExp.$1.length
				for (var i = 0; i < indent; i++) {
					if (sortdatalevels[i]) prefix += sortdatalevels[i]
				}
				//save the current level
				sortdatalevels = sortdatalevels.slice(0, indent)
				sortdatalevels[indent] = sortdata
				//add the prefix
				if (prefix) sortdata = prefix + sortdata
			}
			else {
				sortdatalevels = [sortdata]
			}

		}

		//right justify the rown number assume max 999999
		var temp = rown.toString()
		//temp = exodusspace(6 - temp.length) + temp
		temp = ('000000' + temp).slice(-6)

		//save the two column array for sorting
		//need the +temp so that indented fields sort correctly
		data[rown] = [sortdata + ' ' + temp, temp]

	}

	//get the table rows
	var tablex = $$('exogroup' + groupno)
	if (tablex)
		var tablerows = tablex.tBodies[0].rows

	//sort the data and table rows
	//////////////////////////////
	//NB .sort() will sort arrays of arrays using all elements of the subarray in turn
	//in our case we make subarray 2nd element
	// the original rown number (right justified) so .sort() is STABLE!!!
	//ie original row order is preserved for duplicate sorted items
	//this means that you can sort on one column and then another
	//and the first column sorted *remains sorted* within duplicate values of
	//of the second column sorted.
	//eg to sort by company AND name .. sort on name first then company
	data = data.sort()
	//QuickSort(data,fromrown,uptorown)

	//reverse if sorting descending
	if (order == 'up')
		data.reverse()

	//get an array of the tablerows
	var oldrows = []
	for (var oldrown = 0; oldrown < tablerows.length; oldrown++) oldrows[oldrown] = tablerows[oldrown]

	//change the sort image now confirmed
	try {
		await resetsortimages(groupno)
		exodus_apply_sort_icon(clickedelement, order)
	} catch (e) { }

	//reorder data and table rows
	var newdatarows = []
	for (var newrown = 0; newrown < data.length; newrown++) {

		var oldrown = (+Math.floor(data[newrown][1]))

		//build new ordered data
		newdatarows[newrown] = datarows[oldrown]

		//rearrange table rows
		if (newrown != oldrown) tablerows[newrown].swapNode(oldrows[oldrown])

	}

	//replace ordered data
	gds.data['group' + groupno] = newdatarows

	if (typeof form_postsort == 'function') {
		if (!(await form_postsort(colid)))
			return await exoui_invalid()
	}

	//window.status = ''
}

//******//
//*MENU*//
//******//

function menuhide(element) {

	var okmenus = false

	if (!element)
		element = $$('menudiv').parentNode

	//gmenucodes FINANCE,SUPPORT,MEDIAANALYSIS,MEDIA,JOBS,TIMESHEETS,HELP
	if (!element.menuok) {
		okmenus = exodusgetcookie2('m')
		//if (gusername!='EXODUS')
		//	okmenus=okmenus.replace('FINANCE,','')
		okmenus = okmenus.split(',')
	}

	if (!element.exodusmenuaccesskeys)
		element.exodusmenuaccesskeys = []

	var childnodes = element.childNodes

	nextchild:
	for (var i = element.childNodes.length - 1; i >= 0; i--) {
		var child = childnodes[i]

		//delete some menus
		if (okmenus && okmenus.join() != '' && child.id && child.id.slice(0, 5) == 'menu_') {
			if (!okmenus.exoduslocate(child.id.slice(5))) {
				//ignore failure due to too early key response
				try {
					child = element.removeChild(child)
					continue nextchild
				}
				catch (e) { }
			}
		}

		if (child.className == 'menu' || child.parentNode.className == 'menu') {
			if (child.tagName == 'DIV') {
				child.className = 'menu'
				child.style.display = 'none'
				menuhide(child)
			}
			if (child.tagName == 'A') {
				child.onmouseover = menuonmouseover
				//stops keys working properly
				//child.onclick = menuonmouseover
				child.onmouseout = menuonmouseout
				//do both ?!
				//addeventlistener(child,'mouseover','menuonmouseover')
				//addeventlistener(child,'mouseout','menuonmouseout')
				//child.onfocus=menuonmouseover
				//child.onblur=menuonmouseout
				child.className = 'menuitem'
				//child.style.paddingLeft = '0px'
				child.style.paddingLeft = '5px'

				if (typeof child.exodussubmenuoption == 'undefined') {

					//save an array of elements by accesskey in the menu
					var underlineelement
					if ((underlineelement = child.getElementsByTagName('U')).length) {
						var menuaccesskey = underlineelement[0].innerText.exodustrim().slice(0, 1).toUpperCase()
						var temp = element.exodusmenuaccesskeys[menuaccesskey]
						if (gusername == 'EXODUS' && temp)
							void exodus_begin(function () {
								return exoui_note('Duplicate menu access key ' + menuaccesskey.exodusquote() + ' for\r' + child.innerText + '\rand\r' + temp.innerText)
							}, 'duplicate menu access key')
							// alert('Duplicate menu access key ' + menuaccesskey.exodusquote() + ' for \r' + child.innerText + ' \rand \r' + temp.innerText)
						element.exodusmenuaccesskeys[menuaccesskey] = child
					}
				}

				//add submenu indicators
				if (!child.href && !child.onclick && typeof child.exodussubmenuoption == 'undefined') {
					child.exodussubmenuoption = true
					//child.style.width=child.parentNode.runtimeStyle.width
					//child.style.border='1px solid'
					if (element.id != 'menudiv')
						child.innerHTML += ' ...'

				}
				else {
					child.exodussubmenuoption = false
				}

			}
		}
	}

	element.menuok = true

}

var gnmenus = 0
var gmenutimeout = ''

function menufitviewport(submenu) {

	// After menuonmouseover positions the flyout, clamp its viewport top edge
	// between the menubar foot and the window foot (Media > Files etc.)
	var pad = 6
	var maxBottom = document.documentElement.clientHeight - pad
	var minTop = pad
	if (gexodus_menubar) {
		var barBottom = gexodus_menubar.getBoundingClientRect().bottom + 2
		if (barBottom > minTop)
			minTop = barBottom
	}

	var rect = submenu.getBoundingClientRect()
	var shift = Math.max(minTop, Math.min(rect.top, maxBottom - rect.height)) - rect.top

	if (shift)
		submenu.style.top = ((parseFloat(submenu.style.top) || 0) + shift) + 'px'
}

//menu_onclick=menuonclick
async function menu_onclick(event) {

	//disabled to get menu working better on mobile
	//onclick cannot call "onmouseover" event on mobile since
	// "mouseover" event is called after onclick if not focussed (strange but true)
	//and double calling would have the effect of opening and closingopen+close)
	//if (typeof Touch == 'undefined')
	//	menuonmouseover(event, 'click')

	//menudiv.style.display = menudiv.style.display == '' ? 'none' : ''
	return
}

function menuonmouseover(event, menuoption) {

	//also called from onkeydown for alt+M keyboard shortcut

	//quit if the menu is not loaded from the web site yet
	if (!gmenuloaded)
		return false

	//like esc if any menus are open
	if (menuoption == 'click') {
		menuoption = ''
		if (gnmenus) {
			gnmenus = 0
			menuclose()
			return exoduscancelevent(event)
		}
	}

	//indicate menuing and cancel any request to close menus
	var keyboarding = menuoption
	gnmenus = 1
	window.clearTimeout(gmenutimeout)

	//get the menuoption
	if (!menuoption) {
		menuoption = event.target
		if (!menuoption || typeof menuoption != 'object')
			return
	}
	//click detected on childnode
	//if (menuoption.id == 'menubutton_label')
	if (menuoption.id == 'menubutton_label' || menuoption.tagName == 'IMG' || menuoption.tagName == 'U')
		menuoption = menuoption.parentNode

	var menu = menuoption.parentNode

	//window.status=new Date()+' '+menu.clientLeft+' '+menu.clientWidth
	var leftoffset
	var topoffset
	if (menuoption.id && (menuoption.id == 'menubutton' || menuoption.tagName == 'BUTTON' || menuoption.id.slice(0, 5) == 'menu_')) {
		//	console.log("menu_onmouseover 5a " + menuoption)

		//  leftoffset=menuoption.offsetWidth+menuoption.offsetLeft-4
		//  topoffset=0
		//make the menu visible just beneath the menu button
		leftoffset = menuoption.offsetLeft + 1
		topoffset = menuoption.offsetHeight + menuoption.offsetTop - 1
	}
	else {
		//	console.log("menu_onmouseover 5b " + menuoption)

		//make the menu visible just to the right of the menu option
		leftoffset = menu.offsetWidth - menu.clientLeft - 8
		//topoffset = -menu.clientTop
		//topoffset = 0
		topoffset = menuoption.offsetTop + 5
	}

	menuchangeoption(menu, menuoption)

	//menu.focus()
	//menu.onkeydown=menuonkeydown

	//looks for a following menudiv to open
	//otherwise returns
	var submenu
	submenu = menuoption
	while (submenu = submenu.nextSibling) {
		if (submenu.tagName)
			break
	}
	if (!submenu || submenu.className != 'menu') {
		if (!submenu)
			submenu = event.target
		if (!submenu)
			return
	}

	//close any other submenus
	menuhide(menu)

	//open the submenu
	// submenu.style.left=window.event.srcElement.clientLeft+window.event.srcElement.clientWidth
	submenu.style.left = leftoffset + 'px'
	if (topoffset)
		submenu.style.top = topoffset + 'px'
	submenu.style.display = ''
	submenu.noWrap = true
	menufitviewport(submenu)
	//alert(submenu.outerHTML)
	//highlight first menu item if none highlighted
	if (keyboarding || submenu.id == 'menudiv') {
		if (!submenu.highlightedelement) {
			//look for nextsibling
			var subelements = submenu.childNodes
			for (var subelementn = 0; subelementn < subelements.length; subelementn++) {
				if (subelements[subelementn].tagName == 'A') {
					menuchangeoption(submenu, subelements[subelementn])
					break
				}
			}
		}

		//simple submenu.focus doesnt work as usual ... blanks the menudiv!
		menufocus(submenu)
		submenu.onkeydown = menuonkeydown
		//do both ?!
		//addeventlistener(submenu,'keydown','menuonkeydown')

	}

}

//sets a timeout to focus on an element
//OR executes the timeout function
var gmenuelement;
function menufocus(menu) {

	//if given an element save it global and set timeout callback
	if (menu) {
		gmenuelement = menu
		exodussettimeout('menufocus()', 1)
		return
	}

	//focus on the saved element (so that keyboard events get send to menu elements)
	//-1 to allow scripting focus without forming part of normal keyboard tab sequence
	gmenuelement.tabIndex = -1
	try { gmenuelement.focus() } catch (e) { }

}

function menuonmouseout(event) {
	event = getevent(event)

	gnmenus = 0
	gmenutimeout = exodussettimeout('menuclose()', 1000)
	//window.event.srcElement.style.color='black'
}

function menuclose() {

	if (gnmenus == 0) {
		menuhide()
		var xmenubutton = $$('menubutton')
		xmenubutton.style.background = ''
		xmenubutton.style.backgroundColor = ''
		xmenubutton.style.color = ''
		xmenubutton.tabIndex = 9999
	}

}

function menuonkeydown(event, menu, key) {

	event = getevent(event)

	//usually called as an event with no parameters
	//doesnt seem to work well except called as an event
	if (!menu || !key) {
		menu = event.target
		key = event.keyCode
	}
	//console.log('menuonkeydown '+key+' '+menu+' '+key)

	var menuoption = menu.highlightedelement
	if (!menuoption)
		return

	var horizontal = (menu.id == 'menudiv')

	//35 End 36 Home 37 Left 38 Up 39 Right 40 Down
	//65-90 A-Z
	var alphakey = (key >= 65 && key <= 90)
	var endkey = key == 35
	var homekey = key == 36
	var leftkey = key == 37
	var upkey = key == 38
	var rightkey = key == 39
	var downkey = key == 40
	var esckey = key == 27
	var enterkey = key == 13

	//wstatus(new Date()+' '+key)

	//esc ... close menu and return focus to where it was when menu was opened
	if (esckey || (!horizontal && leftkey)) {
		if (menu.parentNode.parentNode.highlightedelement) {
			menuonmouseover(event, menu.parentNode.parentNode.highlightedelement)
			//exodussettimeout('menuonkeydown(13)',10)
			//attempt to press Enter on it
			//menuonkeydown(menu.parentNode.parentNode,13)
			return exoduscancelevent(event)
		}
		else {
			gnmenus = 0
			menuclose()
			return exoduscancelevent(event)
		}
	}

	//menu access key?
	if (alphakey && menu.exodusmenuaccesskeys) {
		var newmenuoption = menu.exodusmenuaccesskeys[String.fromCharCode(key)]
		if (newmenuoption) {
			menuoption = newmenuoption
			//will be done below menuchangeoption(menu,menuoption)
			enterkey = true
			alphakey = false
		}
	}

	//enter key opens a menu or click a link (shift+enter to open a new window)
	if (enterkey || (horizontal && downkey) || (!horizontal && rightkey)) {
		if (enterkey && menuoption.href) {
			try {
				if (menuoption.onclick)
					menuoption.onclick()
				else {
					if (event && event.shiftKey)
						windowopen_sync(menuoption.href)
					else
						window.location.assign(menuoption.href)
				}
			}
			catch (e) { }
		}
		else if (enterkey && menuoption.onclick) {
			try {
				menuoption.click()
			}
			catch (e) { }
		}
		else {
			if (!menuoption.href && !menuoption.onclick)
				menuonmouseover(event, menuoption)
		}
		return exoduscancelevent(event)
	}

	//right/left/up/down arrows and home/end
	//or letter keys to search for that item
	if ((endkey || homekey) || (horizontal && (leftkey || rightkey)) || (!horizontal && (upkey || downkey)) || alphakey) {

		var newmenuoption

		//search forward from current element to the end
		//(or backwards to beginning if left/up key)
		if ((key >= 37 && key <= 40) || alphakey) {
			newmenuoption = menuoption
			while (newmenuoption && (newmenuoption = ((leftkey || upkey) ? newmenuoption.previousSibling : newmenuoption.nextSibling))) {
				if (newmenuoption.tagName == 'A') {

					//if cursor key then break at first available menu option
					if (!alphakey)
						break

					//or break if the first letter matches the alpha code entered
					if (newmenuoption.innerText.exodustrim().slice(0, 1).toUpperCase() == String.fromCharCode(key))
						break

				}
			}
		}

		//resume search from the beginning (or end if left/up key)
		//search from the beginning if pressed Home
		//search from the end if pressed End
		if (!newmenuoption) {
			newmenuoption = ((endkey || leftkey || upkey) ? menu.lastChild : menu.firstChild)
			while (newmenuoption) {
				if (newmenuoption.tagName == 'A') {

					//if cursor key then break at first available menu option
					if (!alphakey)
						break

					//ir break if the first letter matches matches the alpha code entered
					if (newmenuoption.innerText.exodustrim().slice(0, 1).toUpperCase() == String.fromCharCode(key))
						break

				}
				newmenuoption = ((endkey || leftkey || upkey) ? newmenuoption.previousSibling : newmenuoption.nextSibling)
			}
		}
	}

	//if found a new option then move highlight to it
	if (newmenuoption && newmenuoption.tagName == 'A' && newmenuoption != menuoption) {

		menuchangeoption(menu, newmenuoption)

		return exoduscancelevent(event)

	}

	return true

}

function menuchangeoption(menu, newmenuoption) {

	//skip if over underlined element (bubble will still highlight the parent)
	if (newmenuoption.tagName == 'U') return

	//clear old menu item to default colors
	var menuoption = menu.highlightedelement
	if (menuoption) {
		menuoption.style.background = ''
		menuoption.style.backgroundColor = ''
		menuoption.style.color = ''
	}

	if (!newmenuoption) return

	//highlight new menu item
	//newmenuoption.style.backgroundColor='#d0d0d0'
	newmenuoption.style.background = 'var(--exodus-menu-hover-bg-color)'
	newmenuoption.style.borderRadius = '8px'
	//newmenuoption.style.backgroundColor = 'highlight'
	//newmenuoption.style.color = 'highlighttext'
	//newmenuoption.style.backgroundColor = 'lightgrey'
	// static styling, moved to global.css, only highlight colouring is left
	// because of existing mechanisms that determine which item to highlight
	// is not portable to css
	menu.highlightedelement = newmenuoption

	//focus and onkeydown should remain on the menu div not the item
	if (!true) {
		//menu.focus()
		gmenuelement = menu
		exodussettimeout('menufocus()', 10)
	}
	menu.onkeydown = menuonkeydown
	//menu.addEventListener('keydown',menuonkeydown)

}

/*
 * Non-form pages (no dbform document_onkeydown): Alt+M/G/R + tools that used
 * menubuttonhtml accesskey (e.g. Print Alt+P → id_onclick).
 */
var gexodus_menubar_keydown_installed = false

function exodus_menubar_ensure_keydown() {

	if (gexodus_menubar_keydown_installed || typeof gdictfilename != 'undefined')
		return
	gexodus_menubar_keydown_installed = true
	addeventlistener(document, 'keydown', 'exodus_menubar_keydown')
}

async function exodus_menubar_keydown(event) {

	event = getevent(event)
	if (!event || !event.altKey || event.shiftKey || event.ctrlKey)
		return true
	var t = event.target
	if (t && (t.tagName == 'INPUT' || t.tagName == 'TEXTAREA' || t.tagName == 'SELECT' || t.isContentEditable))
		return true
	var keycode = event.keyCode ? event.keyCode : event.which

	if (keycode == 77 && $$('menubutton') && typeof menuonmouseover == 'function') {
		exoduscancelevent(event)
		window.scrollTo(0, 0)
		exodussettimeout('menuonmouseover(null,$$("menubutton"),13)', 1)
		return false
	}
	if (keycode == 71 && typeof exoduslogout_onclick == 'function') {
		exoduscancelevent(event)
		await exoduslogout_onclick()
		return false
	}
	if (keycode == 82 && typeof refreshcache_onclick == 'function') {
		exoduscancelevent(event)
		await refreshcache_onclick()
		return false
	}
	// accesskey button under bar → id_onclick (menubutton id ends in "button")
	if (keycode < 65 || keycode > 90)
		return true
	var bar = gexodus_menubar || document.getElementById('exodus_menu')
	if (!bar)
		return true
	var letter = String.fromCharCode(keycode)
	var ak = bar.querySelector('button[accesskey="' + letter + '"],button[accesskey="' + letter.toLowerCase() + '"]')
	if (!ak || !ak.parentNode)
		return true
	var mb = ak.parentNode.querySelector('.menubutton[id$="button"]')
	if (!mb || !mb.id)
		return true
	var fn = window[mb.id.slice(0, -6) + '_onclick']
	if (typeof fn != 'function')
		return true
	exoduscancelevent(event)
	await fn(event)
	return false
}

// Non-form menubar tool. id → await id_onclick. setTimeout(0) from formfunctions_onload
// if standard Menu/Refresh are not on the bar yet.
function exodus_menubar_add_button(id, imagesrc, name, title, accesskey) {

	if (!gexodus_menubar)
		add_exodus_menubar()
	var wrap = document.createElement('span')
	wrap.innerHTML = menubuttonhtml(id, imagesrc, name, title, accesskey)
	var trail = gexodus_menubar.querySelector('.exodus_menubar_trailing')
	if (trail)
		gexodus_menubar.insertBefore(wrap, trail)
	else
		gexodus_menubar.appendChild(wrap)
	if (typeof gdictfilename == 'undefined')
		addeventlistener(wrap, 'click', id + '_onclick')
	if (typeof adjust_bodymargin == 'function')
		adjust_bodymargin()
	exodus_menubar_ensure_keydown()
	return wrap
}

function menubuttonhtml(id, imagesrc, name, title, accesskey, align) {

	var tagname = 'span'
	//tagname='div'
	var tx = '<' + tagname

	if (id)
		tx += ' id="' + id + 'button' + '"'

	if (title)
		tx += ' title="' + title + '"'

	//tx += ' class=graphicbutton'
	//tx += ' onmousedown=this.style.borderStyle="inset"'
	//tx += ' onmouseup=this.style.borderStyle="outset"'
	//tx += ' onmouseout=this.style.borderStyle="outset"'

	//there is no float:center?!
	var style = ''
	// logout button should not have a border on the right like the other buttons
	if (id != 'exoduslogout' && align != 'center') {
		//var style = 'white-space:nowrap; padding-right: 7.5px;';
		style = 'white-space:nowrap; padding: 5px;';
	}

	if (align == 'center') {
		// Unbound form OK/Save/Close (+ extras) and confirm/decide — raised button face
		tx += ' class=graphicbutton'
	} else {
		// Bound form tools + Menu/Refresh/Logout — flat menubar chrome, not a button
		tx += ' class=menubutton'
		style += ''//';float:' + align
	}

	//style=''
	if (style)
		tx += ' style="' + style + '"'

	if (accesskey)
		tx += ' accesskey="' + accesskey + '"'

	//tx += ' exodusonclick=' + id + '_onclick(event)"'
	tx += ' exodusonclick="'
	tx += 'await '
	tx += id + '_onclick(event)"'

	tx += '>'

	//alert(id+' '+tx)
	// image: painted URL string, or monochrome { mask, color }
	if (imagesrc) {
		if (exodus_is_icon_spec(imagesrc))
			tx += exodus_icon_html(imagesrc)
		else
			tx += '<IMG src=' + imagesrc + '>'
	}

	//tx+='<br />'

	// Empty string label = icon-only (still keep title + accesskey). Omit the span so
	// CSS does not reserve space for an empty label.
	if (name !== '' && name != null) {
		tx += '<span id=' + id + 'button' + '_label>'
		tx += name
		tx += '</span>'
	}

	tx += '</' + tagname + '>'

	//create a tiny button to capture the accesskey to prevent it from being used by the browser
	//firefox access key is shift+alt unless reconfigured in about:config
	//http://kb.mozillazine.org/Ui.key.contentAccess
	//so ALSO implemented in onkeydown
	if (accesskey) {
		//tx += '<button xtabindex=-1 style="background-color:white; height:1px; width:1px; border-style:none; margin:0px ;padding:0px"'
		// hide access keys on screen
		// Same Gate A entry as visible menubutton: await inside exodusonclick (not raw markup).
		tx += '<button xtabindex=-1 style="display:none;"'
		tx += ' accesskey="' + accesskey + '"'
		tx += ' exodusonclick="await ' + id + '_onclick(event)"'
		tx += '></button>'
	}

	return tx

}

function setgraphicbutton(button, labeltext, src) {

	if (!button)
		return
	if (labeltext) {
		var label = $$(button.id + '_label')
		if (label)
			label.innerHTML = labeltext.replace(/^\s+/, '')
	}
	if (src == null || typeof src == 'undefined')
		return

	var icon = button.querySelector('.exodus-icon')
	var img = button.getElementsByTagName('IMG')[0]

	if (exodus_is_icon_spec(src)) {
		if (icon) {
			exodus_icon_apply(icon, src)
		} else if (img) {
			var span = document.createElement('span')
			span.innerHTML = exodus_icon_html(src)
			span = span.firstChild
			img.parentNode.replaceChild(span, img)
		} else {
			button.insertAdjacentHTML('afterbegin', exodus_icon_html(src))
		}
	} else {
		// painted multicolour URL (New/Edit/Delete)
		if (img) {
			img.src = src
		} else if (icon) {
			var im = document.createElement('IMG')
			im.src = src
			icon.parentNode.replaceChild(im, icon)
		}
	}
}

async function refreshcache_onclick() {
	if (clearcache())
		await exoui_note('All EXODUS data cached in this window has been cleared\rand will be retrieved from the server again as and when required.')
	// \r\rN.B. EXODUS forms and scripts will remain cached and may\rbe updated when you close and reopen all browser\rwindows - depending on the cache settings in your browser.')
	else
		await exoui_note('Cannot clear cache.')
	return true
}

function exoduscancelevent(event) {

	//should error maybe
	event = getevent(event)
	if (!event) {
		//console.log('exoduscancelevent QUITTING - no event!')
		return false
	}

	event.cancelBubble = true
	event.returnValue = false

	if (event.stopPropagation) {
		//console.log('event.stopPropagation()')
		event.stopPropagation()
	}

	if (event.preventDefault) {
		//console.log('event.preventDefault()')
		event.preventDefault()
	}

	event.cancel = true

	return false

}

//not really a great idea to use global when async functions could be running
//used in getrecn and setreadonly at the moment
var gevent

function getevent(event) {

	// if not passed event then try window.event (legacy)
	if (!event) {
		if (window.event)
			event = window.event
		else
			event = gevent
	}

	//if not given, and cannot get event then return empty object (with no target property)
	//perhaps should throw an error
	if (!event)
		return {}

	// if no target then try srcElement (legacy)
	if (!event.target && event.srcElement)
		event.target = event.srcElement

	//if cannot find target then return empty object with target property of
	//so if passed an element then element.target is itself. usually the caller is looking for .target
	if (!event.target) {
		var origevent = event
		event = {}
		event.target = origevent
	}

	//defeat Safari 3 bug
	if (event.target && event.target.nodeType == 3)
		event.target = event.target.parentNode;

	//return event (with .target)
	gevent = event
	return event

}

function exodusformpaneof(tablex) {

	// Return the visible shell around a form table when exoduswrapformpanes() has wrapped it.
	var parent = tablex && tablex.parentNode
	if (!parent || !parent.className)
		return tablex
	if ((' ' + parent.className + ' ').indexOf(' exodusformpane ') >= 0)
		return parent
	return tablex
}

function exodusform_is_inside_exodusform(tablex) {

	var el = tablex && tablex.parentNode
	while (el) {
		if (el.tagName == 'TABLE' && el.className && (' ' + el.className + ' ').indexOf(' exodusform ') >= 0)
			return true
		el = el.parentNode
	}
	return false
}

// Nodes that may sit between sibling forms without ending a pane run.
// HTML comments used to break wrap/coalesce (bookings multi-section HTM).
function exodus_is_formpane_run_sep(node) {
	if (!node)
		return false
	if (node.nodeType == 8) // Comment
		return true
	if (node.nodeType == 3 && !String(node.nodeValue).replace(/\s/g, ''))
		return true
	if (node.nodeType == 1 && node.tagName == 'BR')
		return true
	return false
}

function exoduscoalesceformpanes() {

	// Merge sibling .exodusformpane shells (only br/ws/comment between) into one rounded frame.
	// A plain <span></span> between panes keeps separate shells (schedules.htm).
	var panes = document.getElementsByClassName('exodusformpane')
	var parents = []
	for (var panen = 0; panen < panes.length; panen++) {
		var parentx = panes[panen].parentNode
		if (parentx && parents.indexOf(parentx) < 0)
			parents.push(parentx)
	}
	for (var parentn = 0; parentn < parents.length; parentn++) {
		var parentx = parents[parentn]
		var node = parentx.firstChild
		var runpanes = []
		var runseps = []
		var seps = []

		function flushrun() {
			if (runpanes.length < 2)
				return
			var dest = runpanes[0]
			for (var runn = 1; runn < runpanes.length; runn++) {
				for (var sepn = 0; sepn < runseps[runn].length; sepn++)
					dest.appendChild(runseps[runn][sepn])
				while (runpanes[runn].firstChild)
					dest.appendChild(runpanes[runn].firstChild)
				parentx.removeChild(runpanes[runn])
			}
		}

		function resetrun() {
			runpanes = []
			runseps = []
		}

		while (node) {
			var next = node.nextSibling
			if (node.nodeType == 1 && node.className && (' ' + node.className + ' ').indexOf(' exodusformpane ') >= 0) {
				runpanes.push(node)
				runseps.push(seps)
				seps = []
			}
			else if (runpanes.length && exodus_is_formpane_run_sep(node)) {
				seps.push(node)
			}
			else {
				flushrun()
				resetrun()
				seps = []
			}
			node = next
		}
		flushrun()
	}
}

function exodus_is_wrappable_exodusform(tablex) {

	// Top-level TABLE.exodusform not already inside a pane (or nested in another form).
	if (!tablex || !tablex.className || (' ' + tablex.className + ' ').indexOf(' exodusform ') < 0)
		return false
	if (exodusform_is_inside_exodusform(tablex))
		return false
	var parent = tablex.parentNode
	if (!parent)
		return false
	if (parent.className && (' ' + parent.className + ' ').indexOf(' exodusformpane ') >= 0)
		return false
	return true
}

function exodus_reveal_form_panes() {

	// Allow painting after wrap+coalesce (pairs with global.css html:not(.exodus-panes-ready)).
	try {
		document.documentElement.classList.add('exodus-panes-ready')
	} catch (e) { }
}

/*
 * Extreme-wide form auto-class (temp/wide-form-crush).
 *
 * Doc (AIM + method of narrow vs wide):
 *   service/www/exodus/doc/FORM-UI-WIDE-NARROW.md
 * Also: FORM-UI-TYPES.md (field categories), FORM-UI-PHILOSOPHY.md (pane model).
 *
 * wantWide = (skeleton > soft_ceiling) OR (record && free-text crushed).
 * Skeleton: free-text at empty floor 6ch, table unlimited max-content width.
 * Soft ceiling = current zoom (100vw − 2rem). Remeasure each decide (KISS).
 *
 * Free-text soft max 30ch ONLY when .exodusform-wide (form_table_set_wide).
 * REGRESSION GUARD: never leave style.maxWidth=30ch on narrow forms — that
 * looks like “all text locked to 30 characters”. Always apply 100% when !wide.
 * Nested host tables must NOT use width:max-content (global.css) or free-text
 * cannot fold under the soft ceiling (repeated user-facing regression).
 */
var gform_wide_layout_resize_wired = false
var gform_wide_crush_slack_px = 32
var gform_wide_exomax_px_cache = null
var gform_wide_last_geom_snap = ''

function form_table_is_wide(table) {
	if (!table)
		return false
	if (table.classList)
		return table.classList.contains('exodusform-wide')
	return (' ' + (table.className || '') + ' ').indexOf(' exodusform-wide ') >= 0
}

function form_table_set_wide_class(table, wide) {
	if (!table)
		return
	if (table.classList) {
		if (wide)
			table.classList.add('exodusform-wide')
		else
			table.classList.remove('exodusform-wide')
		return
	}
	var has = form_table_is_wide(table)
	if (wide && !has)
		table.className = (table.className ? table.className + ' ' : '') + 'exodusform-wide'
	else if (!wide && has)
		table.className = (' ' + table.className + ' ').replace(/ exodusform-wide /g, ' ').replace(/^\s+|\s+$/g, '')
}

function form_table_apply_freetext_wide_max(table, wide) {
	if (!table || !table.querySelectorAll)
		return
	// Never paint 30ch unless the table is actually wide (stale flag / call order)
	if (wide && !form_table_is_wide(table))
		wide = false
	var spans = table.querySelectorAll('SPAN[exomaxwidth]')
	for (var i = 0; i < spans.length; i++) {
		var sp = spans[i]
		var mx = sp.getAttribute('exomaxwidth')
		if (!mx)
			continue
		// narrow: 100% of cell (fold under soft ceiling). wide: soft max e.g. 30ch
		sp.style.maxWidth = wide ? mx : '100%'
	}
}

function form_table_set_wide(table, wide) {
	if (!table)
		return
	form_table_set_wide_class(table, wide)
	// Always sync free-text maxWidth to class (clears stale 30ch when leaving wide)
	form_table_apply_freetext_wide_max(table, !!wide)
}

function form_freetext_exomaxwidth_px(span) {
	if (!span || !span.getAttribute)
		return 0
	var raw = span.getAttribute('exomaxwidth')
	if (!raw)
		return 0
	if (!gform_wide_exomax_px_cache)
		gform_wide_exomax_px_cache = {}
	if (gform_wide_exomax_px_cache[raw] != null)
		return gform_wide_exomax_px_cache[raw]
	var prev = span.style.maxWidth
	var px = 0
	try {
		span.style.maxWidth = raw
		px = parseFloat(getComputedStyle(span).maxWidth) || 0
	} catch (e) { }
	span.style.maxWidth = prev
	gform_wide_exomax_px_cache[raw] = px
	return px
}

function form_soft_ceiling_px() {
	var rem = 16
	try {
		rem = parseFloat(getComputedStyle(document.documentElement).fontSize) || 16
	} catch (e) { }
	return Math.max(0, (window.innerWidth || 0) - 2 * rem)
}

function form_record_is_displayed() {
	try {
		if (typeof gkey != 'undefined' && gkey !== null && String(gkey) !== '')
			return true
	} catch (e) { }
	return false
}

// Skeleton first (floor); crush only adds wide when skeleton fits.
function form_table_wants_wide(table, ceiling) {
	if (form_table_skeleton_wants_wide(table, ceiling))
		return true
	if (form_record_is_displayed() && form_table_crush_wants_wide(table, ceiling))
		return true
	return false
}

function form_wide_layout_tables() {
	var forms = document.querySelectorAll('.exodusformpane > TABLE.exodusform')
	if (forms.length)
		return forms
	return document.querySelectorAll('TABLE.exodusform')
}

function form_wide_layout_geom_snap(forms, ceiling) {
	var parts = [String(Math.round(ceiling)), form_record_is_displayed() ? '1' : '0']
	for (var i = 0; i < forms.length; i++) {
		var t = forms[i]
		if (t.offsetParent === null && t.offsetWidth === 0 && t.offsetHeight === 0)
			parts.push('x')
		else
			parts.push(String(Math.round(t.getBoundingClientRect().width || t.offsetWidth || 0)))
	}
	return parts.join(',')
}

function form_save_span_box(sp) {
	return {
		el: sp,
		maxWidth: sp.style.maxWidth,
		minWidth: sp.style.minWidth,
		width: sp.style.width,
		overflow: sp.style.overflow,
		whiteSpace: sp.style.whiteSpace,
		display: sp.style.display
	}
}

function form_restore_span_boxes(saved) {
	for (var sn = 0; sn < saved.length; sn++) {
		var sv = saved[sn]
		sv.el.style.maxWidth = sv.maxWidth
		sv.el.style.minWidth = sv.minWidth
		sv.el.style.width = sv.width
		sv.el.style.overflow = sv.overflow
		sv.el.style.whiteSpace = sv.whiteSpace
		sv.el.style.display = sv.display
	}
}

// Unlimited preferred width: free-text @ 6ch (empty paint floor), no wide-class toggle.
function form_table_skeleton_wants_wide(table, ceiling) {
	if (!table || !(ceiling > 0))
		return false
	var spans = table.querySelectorAll('SPAN[contenteditable], SPAN[exoalign="T"]')
	var saved = []
	var sn
	for (sn = 0; sn < spans.length; sn++) {
		var sp = spans[sn]
		saved.push({
			el: sp,
			maxWidth: sp.style.maxWidth,
			minWidth: sp.style.minWidth,
			width: sp.style.width,
			overflow: sp.style.overflow,
			whiteSpace: sp.style.whiteSpace
		})
		sp.style.maxWidth = '6ch'
		sp.style.minWidth = '6ch'
		sp.style.width = '6ch'
		sp.style.overflow = 'hidden'
		sp.style.whiteSpace = 'nowrap'
	}
	var prevMax = table.style.maxWidth
	var prevWidth = table.style.width
	var skeleton = 0
	try {
		table.style.maxWidth = 'none'
		table.style.width = 'max-content'
		skeleton = table.scrollWidth || table.offsetWidth || 0
	} finally {
		table.style.maxWidth = prevMax
		table.style.width = prevWidth
		for (sn = 0; sn < saved.length; sn++) {
			var sv = saved[sn]
			sv.el.style.maxWidth = sv.maxWidth
			sv.el.style.minWidth = sv.minWidth
			sv.el.style.width = sv.width
			sv.el.style.overflow = sv.overflow
			sv.el.style.whiteSpace = sv.whiteSpace
		}
	}
	return skeleton > ceiling - 4
}

// Free-text crushed under soft ceiling while narrow?
function form_table_crush_wants_wide(table, ceiling) {
	if (!table || !(ceiling > 0))
		return false
	var hadWide = form_table_is_wide(table)
	var prevMax = table.style.maxWidth
	var prevWidth = table.style.width
	var wantWide = false
	var prefSaved = null
	try {
		form_table_set_wide_class(table, false)
		table.style.maxWidth = ''
		table.style.width = ''
		var tableW = table.getBoundingClientRect().width || table.offsetWidth || 0
		if (tableW >= ceiling - 4) {
			var spans = table.querySelectorAll('SPAN[exomaxwidth]')
			var candidates = []
			var sn
			for (sn = 0; sn < spans.length; sn++) {
				var span = spans[sn]
				var text = (span.innerText || span.textContent || '').replace(/\s+/g, ' ').replace(/^\s+|\s+$/g, '')
				if (!text || text.length < 2)
					continue
				var actual = span.clientWidth || 0
				if (!(actual > 0))
					continue
				var maxPx = form_freetext_exomaxwidth_px(span)
				if (!(maxPx > 0))
					continue
				if (actual >= maxPx - gform_wide_crush_slack_px)
					continue
				candidates.push({ el: span, actual: actual })
			}
			if (candidates.length) {
				prefSaved = []
				for (sn = 0; sn < candidates.length; sn++) {
					var sp = candidates[sn].el
					prefSaved.push(form_save_span_box(sp))
					sp.style.whiteSpace = 'nowrap'
					sp.style.display = 'inline-block'
					sp.style.width = 'max-content'
					sp.style.maxWidth = 'none'
					sp.style.minWidth = '0'
					sp.style.overflow = 'visible'
				}
				for (sn = 0; sn < candidates.length; sn++) {
					var c = candidates[sn]
					var pref = c.el.scrollWidth || c.el.offsetWidth || 0
					if (pref > c.actual + gform_wide_crush_slack_px) {
						wantWide = true
						break
					}
				}
			}
		}
	} finally {
		if (prefSaved)
			form_restore_span_boxes(prefSaved)
		table.style.maxWidth = prevMax
		table.style.width = prevWidth
		form_table_set_wide_class(table, hadWide)
	}
	return wantWide
}

function form_update_wide_layout(fromResize) {

	var ceiling = form_soft_ceiling_px()
	if (!(ceiling > 0)) {
		form_wide_layout_wire_resize()
		return
	}

	var forms = form_wide_layout_tables()
	var snap = form_wide_layout_geom_snap(forms, ceiling)
	if (fromResize && snap === gform_wide_last_geom_snap) {
		form_wide_layout_wire_resize()
		return
	}

	gform_wide_exomax_px_cache = {}

	for (var i = 0; i < forms.length; i++) {
		var table = forms[i]
		if (table.offsetParent === null && table.offsetWidth === 0 && table.offsetHeight === 0)
			continue
		var hadWide = form_table_is_wide(table)
		var wantWide = form_table_wants_wide(table, ceiling)
		if (wantWide !== hadWide) {
			console.log((hadWide ? 'wide' : 'narrow') + ' → ' + (wantWide ? 'wide' : 'narrow'))
			form_table_set_wide(table, wantWide)
		} else if (wantWide) {
			form_table_apply_freetext_wide_max(table, true)
		} else {
			// Stay narrow: clear any leftover 30ch soft max (fold under cell 100%)
			form_table_apply_freetext_wide_max(table, false)
		}
	}

	gform_wide_last_geom_snap = form_wide_layout_geom_snap(forms, ceiling)
	form_wide_layout_wire_resize()
}

function form_wide_layout_wire_resize() {
	if (gform_wide_layout_resize_wired || typeof window == 'undefined' || !window.addEventListener)
		return
	gform_wide_layout_resize_wired = true
	var t = null
	window.addEventListener('resize', function () {
		if (t)
			window.clearTimeout(t)
		t = window.setTimeout(function () {
			t = null
			form_update_wide_layout(true)
		}, 100)
	})
}

function exoduswrapformpanes() {

	// One pane per sibling *run* of top-level forms (same rules as coalesce:
	// only br/ws/comment between). Build each shell in a single step.
	// Pre-authored .exodusformpane shells are left alone; coalesce still merges
	// those if they sit with only br/ws/comment between them.
	// Caller reveals via exodus_reveal_form_panes() after this returns.
	var tables = document.getElementsByTagName('TABLE')
	var candidates = []
	var tablen
	for (tablen = 0; tablen < tables.length; tablen++) {
		if (exodus_is_wrappable_exodusform(tables[tablen]))
			candidates.push(tables[tablen])
	}

	var parents = []
	var ci
	for (ci = 0; ci < candidates.length; ci++) {
		var p = candidates[ci].parentNode
		if (p && parents.indexOf(p) < 0)
			parents.push(p)
	}

	for (var parentn = 0; parentn < parents.length; parentn++) {
		var parentx = parents[parentn]
		var node = parentx.firstChild
		var runforms = []
		var runseps = []
		var seps = []

		function flushrun() {
			if (!runforms.length)
				return
			var pane = document.createElement('div')
			pane.className = 'exodusformpane'
			var first = runforms[0]
			if (first.style && first.style.display) {
				pane.style.display = first.style.display
				first.style.display = ''
			}
			parentx.insertBefore(pane, first)
			for (var runn = 0; runn < runforms.length; runn++) {
				if (runn > 0) {
					for (var sepn = 0; sepn < runseps[runn].length; sepn++)
						pane.appendChild(runseps[runn][sepn])
				}
				pane.appendChild(runforms[runn])
			}
			runforms = []
			runseps = []
		}

		while (node) {
			var next = node.nextSibling
			if (node.nodeType == 1 && node.tagName == 'TABLE'
				&& candidates.indexOf(node) >= 0) {
				runforms.push(node)
				runseps.push(seps)
				seps = []
			}
			else if (runforms.length && exodus_is_formpane_run_sep(node)) {
				seps.push(node)
			}
			else {
				flushrun()
				seps = []
			}
			node = next
		}
		flushrun()
	}

	exoduscoalesceformpanes()
	exodusclear_embeddedtable_hostborders()
	exodus_mark_form_edge_rows()
}

/*
 * Pane owns outer edges: mark first/last *visible* content (display != none).
 * Runs after formfunctions_onload (postinit showhide already applied). Only
 * direct child forms of .exodusformpane.
 *
 * T/B: first/last non-spacer row. Multi-form pane: only the last form gets
 * edge-bottom so the rule between forms stays.
 * L/R: first/last visible cell per row (hidden trailers e.g. schedules heading
 * button/rating cells must not leave a false right grid edge on the last
 * visible side-by-side fieldstrip).
 *
 * Spacers: bare <tr></tr> (joblist Format) or cells with no element children
 * and no text (costestimateprint trailing <tr><td colspan="2"></td></tr>).
 * Empty-cell spacers are display:none so they do not draw a bottom strip.
 */
function exodus_mark_form_edge_rows() {

	var panes = document.getElementsByClassName('exodusformpane')
	for (var panen = 0; panen < panes.length; panen++) {
		var forms = []
		var kids = panes[panen].children
		for (var kidn = 0; kidn < kids.length; kidn++) {
			var kid = kids[kidn]
			if (kid.tagName == 'TABLE' && kid.className
				&& (' ' + kid.className + ' ').indexOf(' exodusform ') >= 0)
				forms.push(kid)
		}
		for (var fi = 0; fi < forms.length; fi++) {
			var rows = forms[fi].rows
			var firstRow = null
			var lastRow = null
			for (var rown = 0; rows && rown < rows.length; rown++) {
				var tr = rows[rown]
				tr.classList.remove('exodus-form-edge-top')
				tr.classList.remove('exodus-form-edge-bottom')
				// L/R classes on cells every pass (row may be re-shown later)
				if (tr.cells) {
					for (var cci = 0; cci < tr.cells.length; cci++) {
						tr.cells[cci].classList.remove('exodus-form-edge-left')
						tr.cells[cci].classList.remove('exodus-form-edge-right')
					}
				}
				if (tr.style.display == 'none')
					continue
				// Empty spacer: no cells, or only empty cells (no kids, no text).
				var spacer = !tr.cells || !tr.cells.length
				if (!spacer) {
					spacer = true
					for (var ci = 0; ci < tr.cells.length; ci++) {
						var cell = tr.cells[ci]
						if (cell.children && cell.children.length) {
							spacer = false
							break
						}
						var tx = (cell.textContent || '').replace(/\u00a0/g, ' ').replace(/\s+/g, '')
						if (tx) {
							spacer = false
							break
						}
					}
				}
				if (spacer) {
					// Hide empty-cell trailers so they do not paint a false bottom band.
					if (tr.cells && tr.cells.length)
						tr.style.display = 'none'
					continue
				}
				if (!firstRow)
					firstRow = tr
				lastRow = tr
				// First/last visible cell in this row (not structural first/last)
				var firstCell = null
				var lastCell = null
				for (var cjn = 0; cjn < tr.cells.length; cjn++) {
					var c = tr.cells[cjn]
					if (c.style.display == 'none')
						continue
					// computed style for id=ratingsection showhide etc.
					try {
						if (window.getComputedStyle && getComputedStyle(c).display == 'none')
							continue
					} catch (e) { }
					if (!firstCell)
						firstCell = c
					lastCell = c
				}
				if (firstCell)
					firstCell.classList.add('exodus-form-edge-left')
				if (lastCell)
					lastCell.classList.add('exodus-form-edge-right')
			}
			if (firstRow)
				firstRow.classList.add('exodus-form-edge-top')
			// last form in pane only — intermediate forms keep bottom as separator
			if (fi == forms.length - 1 && lastRow)
				lastRow.classList.add('exodus-form-edge-bottom')
		}
	}
}

function exodusclear_embeddedtable_hostborders() {

	// Static exotable / embedded group tables: strip inline borders on the host row/cell
	var tables = document.getElementsByTagName('TABLE')
	for (var tablen = 0; tablen < tables.length; tablen++) {
		var tablex = tables[tablen]
		var isgroup = Number(tablex.getAttribute('exogroupno'))
		var isexotable = tablex.className && (' ' + tablex.className + ' ').indexOf(' exotable ') >= 0
		if (!isgroup && !isexotable)
			continue
		var hostcell = tablex.parentNode
		// Outer grid prompt cells may be th; data cells are td
		if (!hostcell || (hostcell.tagName != 'TD' && hostcell.tagName != 'TH'))
			continue
		if ((' ' + hostcell.className + ' ').indexOf(' exodusembeddedtable ') < 0)
			hostcell.className += (hostcell.className ? ' ' : '') + 'exodusembeddedtable'
		hostcell.style.removeProperty('border')
		var hostrow = hostcell.parentNode
		if (hostrow && hostrow.tagName == 'TR') {
			hostrow.style.removeProperty('border')
			hostrow.style.removeProperty('border-top')
			hostrow.style.removeProperty('border-bottom')
		}
	}
}

// sequential DOM scan by sourceIndex / walk (not document.all index)
function nextelement(element) {

	//return first child
	if (element.childNodes.length)
		return element.firstChild

	//otherwise return next sibling
	if (element.nextSibling)
		return element.nextSibling

	//otherwise return next aunt
	while (true) {
		element = element.parentNode
		if (!element.tagName)
			return false
		if (element.nextSibling)
			return element.nextSibling
	}

}

function id2classname(element) {

	//scan all elements adding id to classname as exodus_idname
	if (!element)
		element = document.body
	while (element = nextelement(element)) {
		if (element.id) {
			if (element.className) {
				if (element.className.indexOf('exodusid_') < 0) {
					element.className += ' exodusid_' + element.id
				}
			}
			else
				element.className = 'exodusid_' + element.id
		}
	}

}

function getmaxwindow_sync() {

	//maximum size is the size of the parent window
	var parentwindow = window.opener
	//popups within popups are limited by the original window size not the immediate parent
	while (parentwindow && parentwindow.opener)
		parentwindow = parentwindow.opener
	//if .opener not supported
	if (!parentwindow) {
		parentwindow = window.parent
		while (parentwindow && parentwindow.parent && parentwindow != parentwindow.parent)
			parentwindow = parentwindow.parent
	}
	if (!parentwindow)
		systemerror('cannot find window.opener or window.parent', 'getmaxwindow_sync')

	var max = {}
	try {
		// some environments throw permission denied when uploading files
		max.width = parentwindow.outerWidth
		max.height = parentwindow.outerHeight
		//max.height=parentwindow.innerHeight
		max.left = parentwindow.screenLeft || parentwindow.screenX
		max.top = parentwindow.screenTop || parentwindow.screenY
		//max.top=max.top+(parentwindow.outerHeight-parentwindow.innerHeight)
		//alert('getmaxwindow_sync() max='+max.width+' '+max.height+' '+max.left+' '+max.top)
	} catch (e) {
	}
	if (!max.width) {
		max.width = window.screen.availWidth
		max.height = window.screen.availHeight
		max.left = 0
		max.top = 0
	}
	max.style = ';DialogHeight:' + max.height + 'px; DialogWidth:' + max.width + 'px;,'
	max.style += ',width=' + max.width
	max.style += ',height=' + max.height
	max.style += ',left=' + max.left//window.screenX
	max.style += ',top=' + max.top//window.screenY

	return max
}


// gblockevents: while a Gate A flight is airborne, starteventhandler must not
// start a *second* flight (classic case: document_onfocus still validating the
// previous field when the same click's document_onclick would also run).
//
// It is *not* a license to cancel browser default actions for the control that
// just received focus. document_onfocus is entered on focus, *before* the click
// of that gesture; preventDefault on that click yields non-native SELECT
// behaviour (focus only on first click; open on second). Competing clicks on
// other targets stay cancelled. Modal work uses #uiblockerdiv separately.
var gblockevents

// Gate A diagnostics (stuck keyboard / mediadiary-style freezes):
// always-on ring of block/unblock + callers; heartbeat detects orphan depth
// (no flight/confirm/colors/calendar) and systemerror + auto-reset to 0.
// Console: exodus_gblockevents_dump() / exodus_gblockevents_force0()
var gblockevents_hist = []
var gblockevents_hist_max = 48
var gblockevents_nonzero_since = 0
// Continuous raw flight: only (no modal_dialog / db_send / confirm / …).
// Open→search for minutes then READU must not inherit dialog age as "long flight".
var gblockevents_raw_flight_since = 0
var gblockevents_skipped_n = 0
var gblockevents_heartbeat_id = 0
var gblockevents_stuck_reported = false
// Orphan depth (no known holder): alert + force clear after this.
var gblockevents_orphan_ms = 8000
// Continuous raw flight this long: systemerror dump only (do not force-clear).
var gblockevents_flight_warn_ms = 120000

// ---------------------------------------------------------------------------
// Browser chrome vs app events (modal / form open)
//
// LEFT TO THE BROWSER (do not preventDefault):
//   • Ctrl/Cmd + / - / 0          zoom in/out/reset (keydown capture below)
//   • Ctrl/Cmd + wheel            zoom (modalblock_onwheel returns early)
//   • Ctrl/Cmd + C when allowed   copy (confirm/colour startevent true)
//   • Alt+Left / Alt+Right        history back/forward when form is not wide
//                                 (wide forms: horizontal viewport pan like Alt+Up/Down)
//
// TAKEN BY APP while modal (gmodalblockdepth / gblockevents):
//   • plain wheel                 page scroll blocked; confirm body may scroll
//                                 (decide list: option step via gdecide_onwheel)
//   • touchmove outside scroll    blocked
//   • form keydown/click path     blocked (startevent swallow) except above
//
// PRODUCT-SPECIFIC (popup owns the gesture):
//   • colours Ctrl+wheel          continuum resolution N (colors.js — blocks zoom
//                                 over that popup by design)
//   • decide type-to-filter etc.  on #exodusconfirmdiv only
//   • decide plain wheel          radio: check+focus next/prev; multi: focus only
//
// Browser zoom keys: early *sync capture* keydown — not Gate A, not
// document_onkeydown. Stops propagation without preventDefault so the browser
// still acts. Install once from client init.
// ---------------------------------------------------------------------------
var gexodus_browser_chrome_keydown_installed = false

function exodus_is_browser_chrome_keydown(event) {
	if (!event || event.type != 'keydown')
		return false
	// Ctrl or Cmd (Mac); not Alt
	if (!(event.ctrlKey || event.metaKey) || event.altKey)
		return false
	var kc = event.keyCode ? event.keyCode : event.which
	// Zoom in: + / = / numpad+
	if (kc == 187 || kc == 61 || kc == 107)
		return true
	// Zoom out: - / numpad-
	if (kc == 189 || kc == 173 || kc == 109)
		return true
	// Zoom reset: 0 / numpad0
	if (kc == 48 || kc == 96)
		return true
	return false
}

function exodus_browser_chrome_keydown_capture(event) {
	if (!exodus_is_browser_chrome_keydown(event))
		return
	// Do not preventDefault — browser zoom (etc.) must still run.
	if (event.stopPropagation)
		event.stopPropagation()
	if (event.stopImmediatePropagation)
		event.stopImmediatePropagation()
	// Decide locks px width; reflow after zoom (resize/visualViewport also refit)
	var conf = $$('exodusconfirmdiv')
	if (conf && conf.classList && conf.classList.contains('exodusconfirm_decide'))
		window.setTimeout(function () { exodusconfirm_fit_decide_popup(true) }, 0)
}

function exodus_ensure_browser_chrome_keydown() {
	if (gexodus_browser_chrome_keydown_installed)
		return
	if (!document.addEventListener)
		return
	gexodus_browser_chrome_keydown_installed = true
	document.addEventListener('keydown', exodus_browser_chrome_keydown_capture, true)
}

function form_blockevents_stack_snippet() {
	try {
		var s = (new Error()).stack
		if (!s)
			return ''
		var lines = s.split('\n')
		var out = []
		for (var i = 0; i < lines.length && out.length < 5; i++) {
			var L = lines[i]
			if (!L)
				continue
			if (L.indexOf('form_blockevents') >= 0)
				continue
			if (/^\s*Error\b/.test(L))
				continue
			out.push(L.replace(/^\s+at\s+/, '').replace(/^\s+/, ''))
		}
		return out.join(' <- ')
	} catch (e) {
		return ''
	}
}

function form_blockevents_hist_push(kind, depth, callername, callinfo) {
	gblockevents_hist.push({
		t: Date.now(),
		kind: kind,
		depth: depth,
		caller: callername || '',
		info: callinfo == null ? '' : String(callinfo),
		stack: form_blockevents_stack_snippet()
	})
	if (gblockevents_hist.length > gblockevents_hist_max)
		gblockevents_hist.shift()
}

// Known long-lived holders of gblockevents (not orphans).
// Order matters: real wait UI before raw flight — schedule Book line keeps Gate A
// open for the whole exoui_showmodaldialog session; lazy db.send keeps it open
// for the whole XHR (Wait/Cancel). Those are not stuck flights.
// Confirm/decide also sit inside a flight; classify them before raw flight so
// a long decide does not systemerror as "long flight".
function exodus_gblockevents_holder() {
	// Parent awaiting exoui_showmodaldialog close (gpendingDialogResolve set)
	try {
		if (typeof gpendingDialogResolve != 'undefined' && gpendingDialogResolve)
			return 'modal_dialog'
	} catch (e0) { }
	// Child window still open (real window, not lazy Wait/Cancel stub)
	try {
		if (typeof gchildwin != 'undefined' && gchildwin && !gchildwin.lazy) {
			if (gchildwin.closed === false)
				return 'modal_child'
			if (gchildwin.actual && !gchildwin.actual.closed)
				return 'modal_child'
		}
	} catch (e1) { }
	// Lazy db.send modal wait (Wait/Cancel on uiblocker). Overwrites gchildwin
	// with {lazy:true,xhttp}; Gate A stays airborne until XHR ends (up to timeout).
	// modaldepth often 2 (flight + dbsend). Not a stuck flight.
	try {
		if (typeof gchildwin != 'undefined' && gchildwin && gchildwin.lazy && gchildwin.xhttp)
			return 'db_send'
	} catch (e1b) { }
	// In-DOM confirm / decide while Gate A awaits resolvePendingConfirm
	try {
		if (document.getElementById('exodusconfirmdiv'))
			return 'exodusconfirmdiv'
	} catch (e) { }
	try {
		if (typeof colors_popup != 'undefined' && colors_popup && colors_popup._showing)
			return 'colors_popup'
	} catch (e2) { }
	try {
		if (typeof calendar_checkInDatePicker != 'undefined'
			&& calendar_checkInDatePicker && calendar_checkInDatePicker._showing)
			return 'calendar'
	} catch (e3) { }
	// Raw flight only if no known wait UI (true hang / never lands)
	if (typeof g_exodus_flow != 'undefined' && g_exodus_flow)
		return 'flight:' + (g_exodus_flow.location || g_exodus_flow.n)
	return null
}

function exodus_gblockevents_dump() {
	var now = Date.now()
	var age = gblockevents_nonzero_since ? (now - gblockevents_nonzero_since) : 0
	var rawAge = gblockevents_raw_flight_since ? (now - gblockevents_raw_flight_since) : 0
	var lines = []
	lines.push(
		'gblockevents=' + (gblockevents || 0)
		+ ' nonzero_ms=' + age
		+ ' raw_flight_ms=' + rawAge
		+ ' skipped_events=' + gblockevents_skipped_n
		+ ' flow=' + (typeof g_exodus_flow != 'undefined' && g_exodus_flow
			? ('#' + g_exodus_flow.n + ' ' + g_exodus_flow.location) : 'null')
		+ ' holder=' + (exodus_gblockevents_holder() || 'none')
		+ ' modaldepth=' + (typeof gmodalblockdepth != 'undefined' ? gmodalblockdepth : '?')
	)
	for (var i = 0; i < gblockevents_hist.length; i++) {
		var r = gblockevents_hist[i]
		lines.push(
			(now - r.t) + 'ms ago ' + r.kind + ' depth=' + r.depth
			+ ' caller=' + (r.caller || '?')
			+ ' info=' + r.info
			+ (r.stack ? ' stack=[' + r.stack + ']' : '')
		)
	}
	var text = lines.join('\n')
	if (typeof console != 'undefined' && console.log)
		console.log(text)
	return text
}

// Manual unlock for console after a freeze (also used by heartbeat).
function exodus_gblockevents_force0(reason) {
	var was = gblockevents || 0
	gblockevents = 0
	gblockevents_nonzero_since = 0
	gblockevents_raw_flight_since = 0
	gblockevents_skipped_n = 0
	gblockevents_stuck_reported = false
	form_blockevents_hist_push('force0', 0, 'force0', reason || ('was=' + was))
	if (typeof console != 'undefined' && console.log)
		console.log('exodus_gblockevents_force0 was=' + was + ' ' + (reason || ''))
	return was
}

function exodus_gblockevents_heartbeat() {
	if (!gblockevents) {
		gblockevents_stuck_reported = false
		gblockevents_raw_flight_since = 0
		return
	}
	var now = Date.now()
	var age = gblockevents_nonzero_since ? (now - gblockevents_nonzero_since) : 0
	var holder = exodus_gblockevents_holder()
	// Track continuous *raw* flight only. Long Open→search (modal_dialog) then
	// READU must not use total nonzero_ms (~6 min) as the long-flight clock.
	if (holder && holder.indexOf('flight:') == 0) {
		if (!gblockevents_raw_flight_since)
			gblockevents_raw_flight_since = now
	} else {
		gblockevents_raw_flight_since = 0
		// Legitimate hold: allow a later true hang to report again
		if (holder)
			gblockevents_stuck_reported = false
	}
	if (holder) {
		// modal_dialog / modal_child / db_send / confirm / colors / calendar:
		// legitimate hold — no warn. flight: only continuous raw flight age.
		var rawAge = gblockevents_raw_flight_since
			? (now - gblockevents_raw_flight_since) : 0
		if (holder.indexOf('flight:') == 0
			&& rawAge >= gblockevents_flight_warn_ms
			&& !gblockevents_stuck_reported) {
			gblockevents_stuck_reported = true
			systemerror(
				'gblockevents long flight',
				'Gate A raw flight ' + rawAge + 'ms under ' + holder
				+ ' (nonzero_ms=' + age + '; no auto-reset — dump for bug report):\n'
				+ exodus_gblockevents_dump()
			)
		}
		return
	}
	// Orphan: depth without flight/confirm/colors/calendar → stuck keys.
	if (age < gblockevents_orphan_ms || gblockevents_stuck_reported)
		return
	gblockevents_stuck_reported = true
	var dump = exodus_gblockevents_dump()
	var was = exodus_gblockevents_force0('orphan heartbeat was=' + gblockevents + ' age_ms=' + age)
	// force0 cleared depth; re-log was in message
	systemerror(
		'gblockevents orphan auto-reset',
		'Gate A stuck at depth ' + was + ' for ' + age + 'ms with no known holder'
		+ ' (flight/confirm/colors/calendar). Reset to 0 so keyboard works.'
		+ ' Paste this dump when reporting:\n' + dump
	)
}

function form_blockevents_ensure_heartbeat() {
	if (gblockevents_heartbeat_id)
		return
	if (typeof window == 'undefined' || !window.setInterval)
		return
	gblockevents_heartbeat_id = window.setInterval(exodus_gblockevents_heartbeat, 2000)
}

function form_blockevents(truefalse, callinfo) {

	var callername = ''
	if (form_blockevents.caller) {
		callername = form_blockevents.caller.name
	}

	if (!gblockevents)
		gblockevents = 0

	if (truefalse) {
		++gblockevents
		if (gblockevents == 1) {
			gblockevents_nonzero_since = Date.now()
			gblockevents_raw_flight_since = 0
			gblockevents_skipped_n = 0
			gblockevents_stuck_reported = false
		}
		logevent('-------->' + gblockevents + '		 block ' + callername + ' ' + callinfo)
		form_blockevents_hist_push('block', gblockevents, callername, callinfo)

	} else {
		--gblockevents
		logevent('		 ' + gblockevents + '<--------unblock ' + callername + ' ' + callinfo)
		if (gblockevents < 0)
			gblockevents = 0
		form_blockevents_hist_push('unblock', gblockevents, callername, callinfo)
		if (gblockevents == 0) {
			gblockevents_nonzero_since = 0
			gblockevents_raw_flight_since = 0
			gblockevents_skipped_n = 0
			gblockevents_stuck_reported = false
		}
	}

	form_blockevents_ensure_heartbeat()

}

// True when the event is the pointer activation of a native listbox that should
// keep browser-default open/toggle behaviour while Gate A is already airborne.
// Does not start a flight; only skips preventDefault.
function exodus_native_select_activation(event) {

	if (!event || !event.target || !event.target.tagName)
		return false
	var typ = event.type
	if (typ != 'click' && typ != 'mousedown' && typ != 'mouseup')
		return false
	var t = event.target
	if (t.tagName == 'OPTION' && t.parentNode)
		t = t.parentNode
	if (t.tagName != 'SELECT')
		return false
	// By click time the select should already hold focus from this gesture.
	var active = document.activeElement
	return !active || active == t || (t.contains && t.contains(active))

}

function starteventhandler(eventfunctionname, functionx) {

	//event handler function
	//2. creates a generator (suspendable/resumable function)
	//   of the right type for the event
	//   (knows event generator function from closure)
	//   and passes the event into it
	//3. calls the generator function to start executing
	//4. the generator function will either return (with event return value?)
	//   or suspend and wait for some window or xmlhttp to resume it
	//5. any window/xmlhttp that wishes the function to resume
	//   can call geventhandler .next(data) where data is the value to be used
	//   as the expression to the right of the yield statement

	// Updated to support bulk conversion to async/await.
	// If the target functionx is an async function, we start it as a promise-based flow
	// at the new geventhandler marker, using the same blocking.
	const isAsyncTarget = functionx && functionx.constructor && functionx.constructor.name === 'AsyncFunction';

	return function exodus_anon_sync_event_handler(event) {

		event = getevent(event)
		//YIELD//console.log('\n--- Event '+event.type+' ---')

		var eventdescription = event.type + ' ' + event.target.tagName + ' id:' + event.target.id + ' gblockevents:' + gblockevents

		//prevent events like onfocus being followed by onclick while
		//onfocus is yielding to async events like window.open etc
		//var uiblockerdiv=$$('uiblockerdiv')
		//if (gblockevents||uiblockerdiv) {
		if (gblockevents) {

			// Always deliver unload (TODO: own gcurrentevent path).
			if (event.type == 'unload' || event.type == 'beforeunload') {
				// fall through to unblocked path below? currently cancelled like others
			}

			// ---------------------------------------------------------------
			// Popup isolation while gblockevents (modal / exclusive UI).
			//
			// Popups own their DOM events (confirm/decide, colours, calendar).
			// Form path only asks: is a popup open? Then swallow form keys.
			// Contract when a *startevent helper is present:
			//   null  — that popup is not open
			//   true  — allow browser default (e.g. copy); no form logic
			//   false — swallow (exoduscancelevent)
			// Calendar: keys on its div; form uses form_closepopups for Esc.
			// ---------------------------------------------------------------
			if (typeof exodusconfirm_startevent == 'function') {
				var confEv = exodusconfirm_startevent(event)
				if (confEv === true)
					return true
				if (confEv === false)
					return exoduscancelevent(event)
			}

			if (typeof colors_popup_startevent == 'function') {
				var colorEv = colors_popup_startevent(event)
				if (colorEv === true)
					return true
				if (colorEv === false)
					return exoduscancelevent(event)
			}

			// Let native <select> complete open/toggle; do not start a new flight.
			if (exodus_native_select_activation(event)) {
				logevent('native SELECT activation while blocked: ' + eventdescription)
				return true
			}

			// Plain arrows while a keydown flight is still airborne (key-repeat):
			// do not preventDefault — otherwise caret movement is swallowed and
			// fields feel unresponsive. Form field-to-field nav waits for idle.
			if (event.type == 'keydown' && !event.altKey && !event.ctrlKey && !event.metaKey) {
				var blockedkey = event.keyCode ? event.keyCode : event.which
				if (blockedkey == 37 || blockedkey == 38 || blockedkey == 39 || blockedkey == 40)
					return true
			}

			++gblockevents_skipped_n
			logevent('!!!SKIPPING event!!! ' + eventdescription + ' because gblockevents is set, and not keydown related to exodusconfirmdiv')

			return exoduscancelevent(event)

		}//end of event blocking

		//events are not blocked - create a new event handler

		if (isAsyncTarget) {
			// Async DOM handlers → Gate A only (public API: exodus_begin).
			return exodus_begin(functionx, eventdescription + ' (async) in starteventhandler', event)
		}

		// Sync handler (e.g. onbeforeunload text): run to completion, no gate.
		var result = functionx(event)
		if (exodusisgeneratoriterator(result)) {
			systemerror(
				'starteventhandler',
				'function* handlers are no longer supported (' + eventdescription
				+ '). Convert to async function.'
			)
			return exoduscancelevent(event)
		}
		if (result)
			event.returnValue = result
		else
			result = false
		return result
	}
}

// geventhandler was the global generator owner; kept null for any leftover reads.
var geventhandler = null
var geventn = 0

// Confirm/decide leaf: button/key/click handlers resolve a Promise (Gate A or B).
var gpendingConfirmResolve
// Owner of gpendingConfirmResolve: 'A' (Gate A business), 'B' (Gate B wait/cancel), or null.
// Force-close paths must pass expectedOwner so A and B cannot cross-wire.
var gpendingConfirmOwner
var gexodusconfirmdefaultbutton

// For the child window / showmodaldialog leaf (next after confirm).
var gpendingDialogResolve
// Child-window dialogs are always Gate A business (never wait/cancel).
var gpendingDialogOwner

// --- async/generator bridge helpers (asyncjs migration) ---

function exodusisasyncfunction(fn) {
	return fn && fn.constructor && fn.constructor.name === 'AsyncFunction'
}

function exodusisgeneratoriterator(value) {
	return value && typeof value.next === 'function' && typeof value.throw === 'function'
		&& value.constructor && value.constructor.name === 'Generator'
}

function exodusispromise(value) {
	return value && typeof value.then === 'function'
}

// Gate A — exclusive main event flow (one plane). All business async
// commencements enter here via exodus_begin. Nested await inside the flight is fine.
//
// Model today: at most ONE active flight (g_exodus_flow). A separate wait list
// (g_exodus_flow_queue) may hold jobs that start only after that flight lands.
// The active job is not counted in the wait list.
//
// g_exodus_flow_queue_max = wait-list capacity only:
//   0 = no queuing — second start while airborne is systemerror (debug signal)
//   1 = at most one deferred takeoff, drained after land
//   N = deeper FIFO
//
// Parallel starts: avoid at the source. Nested await in the current flight is
// fine; a second exodus_begin while airborne is not. Callers outside Gate A
// (capture keydown, timers, etc.) must check g_exodus_flow / use
// exodus_begin_if_idle (optional) or exodus_begin_when_idle (required after land).
// Gate A does not silently ignore conflicts — systemerror is intentional so races
// surface until the initiating call site is fixed.
//
// Not implemented (bridge too far for now): a pool where "size" means active +
// waiting together, and size > 1 could mean several flights airborne at once.
// That needs safe multi-active rules; today xhttp/session largely serializes or
// rejects parallel dbio (db.requesting). Until then: exclusive active + optional
// wait list.
//
// Gate B (wait/cancel) is a separate concurrent stack — see exodus_begin_waitcancel.
var g_exodus_flow = null
var g_exodus_flow_queue = []
// FUTURE: set to 1+ to allow deferred takeoffs while a flight is airborne.
var g_exodus_flow_queue_max = 0
var g_exodus_flight_n = 0

function exodus_flight_log(msg) {
	// Quiet by default (stage 5). Enable: glogflights=true or ?logflights=1
	if (glogflights && typeof console != 'undefined' && console.log)
		console.log('[exodus flight] ' + msg)
	if (glogevents || glogflights)
		logevent('[exodus flight] ' + msg)
}

// Visible failure when required work cannot start (prefer this over silent no-op).
function exodus_flight_skip_error(location, reason) {
	var msg = 'Action not started: "' + (location || 'unknown') + '". ' + reason
		+ '\n\nThis is a Gate A conflict (exclusive async). Fix: await inside the'
		+ ' current flight, or schedule with exodus_begin_when_idle after land.'
	exodus_flight_log('SKIP ERROR "' + location + '": ' + reason)
	if (typeof console != 'undefined' && console.error)
		console.error('[exodus flight] ' + msg)
	systemerror('exodus_begin', msg)
}

// Public API #1 of 3 — exclusive Gate A business async.
// Returns Promise of { value, done: true }, or null if skipped (busy + full wait list).
// Skip is a hard visible error — required work must not vanish.
function exodus_begin(asyncHandler, location, event) {
	location = location || 'unknown'
	if (g_exodus_flow) {
		// Only the wait list is size-limited; airborne is tracked in g_exodus_flow.
		if (g_exodus_flow_queue.length >= g_exodus_flow_queue_max) {
			exodus_flight_skip_error(
				location,
				'Already busy with flight #' + g_exodus_flow.n
				+ ' "' + g_exodus_flow.location + '" (queue_max='
				+ g_exodus_flow_queue_max + ').'
			)
			return Promise.resolve(null)
		}
		exodus_flight_log(
			'QUEUED "' + location + '" (airborne #' + g_exodus_flow.n
			+ ' "' + g_exodus_flow.location + '", queue='
			+ (g_exodus_flow_queue.length + 1) + '/' + g_exodus_flow_queue_max + ')'
		)
		return new Promise(function (resolve, reject) {
			g_exodus_flow_queue.push({
				asyncHandler: asyncHandler,
				location: location,
				event: event,
				resolve: resolve,
				reject: reject
			})
		})
	}
	return exodus_begin_run(asyncHandler, location, event)
}

// Schedule required work after the current flight lands. Not a fourth gate —
// only retries until exodus_begin can take off. systemerror if still busy after
// max_wait_ms (default 30s). Prefer over one-shot setTimeout→exodus_begin.
function exodus_begin_when_idle(asyncHandler, location, options) {
	options = options || {}
	var delay_ms = typeof options.delay_ms == 'number' ? options.delay_ms : 1
	var max_wait_ms = typeof options.max_wait_ms == 'number' ? options.max_wait_ms : 30000
	var retry_ms = typeof options.retry_ms == 'number' ? options.retry_ms : 10
	location = location || 'when_idle'
	var t0 = Date.now()

	function try_start() {
		if (g_exodus_flow) {
			if (Date.now() - t0 > max_wait_ms) {
				exodus_flight_skip_error(
					location,
					'Still busy after ' + max_wait_ms + 'ms with flight #'
					+ g_exodus_flow.n + ' "' + g_exodus_flow.location + '".'
				)
				return
			}
			window.setTimeout(try_start, retry_ms)
			return
		}
		// Race: another takeoff between check and begin — begin will systemerror.
		void exodus_begin(asyncHandler, location)
	}

	window.setTimeout(try_start, delay_ms)
}

async function exodus_begin_run(asyncHandler, location, event) {
	var n = ++g_exodus_flight_n
	g_exodus_flow = { n: n, location: location }
	exodus_flight_log('TAKEOFF #' + n + ' "' + location + '"')

	form_blockevents(true, location)
	// Keep form_blockevents for re-entry (strict one plane), but skip #uiblockerdiv +
	// overflow:hidden for high-frequency chrome that must not thrash page scroll:
	//   focus/activate — full-viewport overlay mid-click closes native <select>
	//   background     — exodus_begin_if_idle (expression2 every 250ms, keepalive, relock)
	//   keydown/keypress/keyup — arrow/caret; key-repeat must not lock overflow
	//   input/change — every keystroke / SELECT change is not a user-wait dialog
	// Real dialogs/db waits still call blockmodalui themselves.
	// Second takeoff while airborne still systemerrors (queue_max 0) — unchanged.
	var etype = event && event.type
	var modalblock = !(etype == 'focus' || etype == 'focusin' || etype == 'activate'
		|| etype == 'background'
		|| etype == 'keydown' || etype == 'keypress' || etype == 'keyup'
		|| etype == 'input' || etype == 'change')
	if (modalblock)
		blockmodalui_sync()

	try {
		var result = await asyncHandler(event)
		exodus_flight_log('LANDING #' + n + ' "' + location + '" ok')
		return { value: result, done: true }
	} catch (e) {
		exodus_flight_log(
			'LANDING #' + n + ' "' + location + '" error: '
			+ (e && (e.message || e.description || e))
		)
		throw e
	} finally {
		form_blockevents(false, location)
		if (modalblock)
			unblockmodalui_sync()
		g_exodus_flow = null
		exodus_begin_drain()
	}
}

function exodus_begin_drain() {
	if (g_exodus_flow || !g_exodus_flow_queue.length)
		return
	var job = g_exodus_flow_queue.shift()
	exodus_flight_log(
		'DEQUEUE "' + job.location + '" (remaining queue='
		+ g_exodus_flow_queue.length + '/' + g_exodus_flow_queue_max + ')'
	)
	exodus_begin_run(job.asyncHandler, job.location, job.event).then(job.resolve, job.reject)
}

// Deprecated alias for exodus_begin (pre-gate name). Prefer exodus_begin.
function startAsyncFlow(asyncHandler, location, event) {
	return exodus_begin(asyncHandler, location, event)
}

// Optional background work (expression2, keepalive, relock): Gate A only when idle.
// Skip if busy — form/lock state may be wrong later; missing a tick is fine.
// Public API #2 of 3 (see block above exodus_begin_waitcancel).
// Passes type 'background' so exodus_begin_run does not mount #uiblockerdiv /
// overflow:hidden (expression2 ticks every 250ms were thrashing page scrollbars).
function exodus_begin_if_idle(asyncHandler, location) {
	location = location || 'background'
	if (g_exodus_flow) {
		exodus_flight_log(
			'SKIP "' + location + '" (A#' + g_exodus_flow.n
			+ ' "' + g_exodus_flow.location + '" airborne)'
		)
		return Promise.resolve(null)
	}
	if (typeof db != 'undefined' && db.requesting) {
		exodus_flight_log('SKIP "' + location + '" (db.requesting)')
		return Promise.resolve(null)
	}
	return exodus_begin(asyncHandler, location, { type: 'background' })
}

// Internal bridge from *_sync / HTML attribute handlers — not a fourth gate.
// Async → exodus_begin; else sync (generators rejected stage 6).
function exodusinvokesynctarget(target, args, location) {
	args = args || []
	if (exodusisasyncfunction(target)) {
		void exodus_begin(function () { return target.apply(null, args) }, location)
		return
	}
	var result = target.apply(null, args)
	if (exodusisgeneratoriterator(result)) {
		systemerror('exodusinvokesynctarget', 'function* removed (stage 6) at ' + location)
		return
	}
	if (exodusispromise(result)) {
		void exodus_begin(function () { return result }, location)
		return result
	}
	return result
}

// Invoke from a legacy *_sync() bridge that returns a value to its caller.
function exodusinvokesynctargetreturn(target, args, location) {
	args = args || []
	if (exodusisasyncfunction(target))
		return exodus_begin(function () { return target.apply(null, args) }, location)
	var result = target.apply(null, args)
	if (exodusisgeneratoriterator(result)) {
		systemerror('exodusinvokesynctargetreturn', 'function* removed (stage 6) at ' + location)
		return false
	}
	if (exodusispromise(result))
		return exodus_begin(function () { return result }, location)
	return result
}

// Normalise a call result inside an async function (await promise only).
async function exodusawaitresult(result, location) {
	if (!result) return result
	if (exodusispromise(result))
		return await result
	if (exodusisgeneratoriterator(result)) {
		systemerror('exodusawaitresult', 'function* removed (stage 6) at ' + location)
		return false
	}
	return result
}

// Fire-and-forget for unload etc. — no UI blocking.
function exodusfireandforget(result, location) {
	if (!result) return
	if (exodusispromise(result)) {
		void result.catch(function (e) {
			console.log('exodusfireandforget ' + location + ': ' + (e.description || e.message || e))
		})
		return
	}
	if (exodusisgeneratoriterator(result))
		systemerror('exodusfireandforget', 'function* removed (stage 6) at ' + location)
}

// Historical name: async → Gate A. Generators no longer accepted (stage 6).
function exodusneweventhandler(eventhandler, location) {
	++geventn
	logevent(' ')
	logevent('=== NEW EVENT HANDLER ' + geventn + ' for ' + location + '===')
	if (exodusisasyncfunction(eventhandler))
		return exodus_begin(eventhandler, location)
	if (exodusisgeneratoriterator(eventhandler)) {
		systemerror(
			'exodusneweventhandler',
			'function* / generators removed (stage 6) at ' + location
			+ '. Convert to async function.'
		)
		return { done: true, value: false }
	}
	// Sync result already produced by a pre-invoked non-async handler.
	return { done: true, value: eventhandler }
}

function addeventlistener(element, eventname, functionx) {

	if (!element)
		systemerror('addeventlistener ' + eventname + ' ' + functionx, 'element is missing')

	//logevent('addeventlistener:'+(element.id||element.name||element.tagName||window)+','+eventname+'->'+functionx)

	if (typeof functionx == 'string') {
		//element.setAttribute('on'+eventname,functionx+'(event)')
		//return
		functionxname = functionx
		functionx = window[functionx]
		if (!functionx)
			systemerror('error: in addeventlistener ' + functionxname + ' function does not exist')
		functionx = starteventhandler(eventname, functionx)
	}

	//normally using bubbling style but focus event doesnt bubble so we use capture style for that
	var capture = eventname == 'focus'

	if (element.addEventListener)
		element.addEventListener(eventname, functionx, capture)
	else if (element.attachEvent) // legacy
		//if cant attachEvent then possibly it isnt a DOM element
		element.attachEvent('on' + eventname, functionx)
	//else
	//run function in the environment of the element
	//element.attachEvent('on'+eventname,function(){functionx.call(element)})

	return true
}

/*
function exodusgetattribute(element,attributename) {

var attribute=element.getAttribute(attributename)
if (!attribute)
return attribute
if (typeof attribute=='string') {

if (attribute=='true')
return true
else if (attribute=='false')
return false
return attribute
}
return attribute
}
*/

//used anywhere yet?
function SetAttribute(element, attName, attValue) {

	if (attValue == null || attValue.length == 0)
		element.removeAttribute(attName, 0);			// 0 : Case Insensitive
	else
		element.setAttribute(attName, attValue, 0);	// 0 : Case Insensitive
}
function GetAttribute(element, attName, valueIfNull) {

	var oAtt = element.attributes[attName];

	if (oAtt == null || !oAtt.specified)
		return valueIfNull ? valueIfNull : '';

	var oValue = element.getAttribute(attName, 2);

	if (oValue == null)
		oValue = oAtt.nodeValue;

	return (oValue == null ? valueIfNull : oValue);
}

function exodusremoveelementsbyid(id) {
	while (true) {
		var tt = document.getElementById(id)
		if (!tt)
			break
		tt.parentNode.removeChild(tt)
	}
}

function exodusremovenode(element) {
	element.parentNode.removeChild(element)
}

function HTMLEncode(text) {
	if (!text)
		return ''
	//&amp first
	return text.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
}

function HTMLDecode(text) {
	if (!text)
		return ''
	//&amp; last
	return text.replace(/&gt;/g, '>').replace(/&lt;/g, '<').replace(/&amp;/g, '&')
}

function exodusint2date(exodusdate) {
	//problem unless automatic daylight saving time change is switched off
	//both the following return the same date!!!
	//zzz need to find a more reliable algorithm
	//alert(new Date(1967,11,31+14361))
	//alert(new Date(1967,11,31+14362))
	//return new Date(1967,11,31+(+exodusdate))

	//var date=new Date(0)
	//date.setUTCFullYear(1967)
	//date.setUTCMonth(11)
	//date.setUTCDate(31+(+exodusdate))
	//return date

	return new Date(Date.UTC(1967, 11, 31 + parseInt(exodusdate, 10)))

}

// Thin timeout wrapper. Prefer await inside the current Gate A flight.
// Preferred deferral of async work (stage 3):
//   exodus_begin_when_idle(myfunc, 'label', { delay_ms: ms })
//   or exodussettimeout(myAsyncFn, ms)  — AsyncFunction → when_idle then begin
// String 'await …' / 'yield* …' is legacy (eval via new Function); do not add more.
function exodussettimeout(command, milliseconds) {
	if (glogsettimeout)
		console.log('exodussetimeout(' + command + ')')
	if (typeof command == 'function') {
		if (exodusisasyncfunction(command)) {
			var label = 'timeout ' + (command.name || 'fn')
			// Must not one-shot begin while still airborne (silent miss → when_idle).
			return window.setTimeout(function () {
				exodus_begin_when_idle(command, label, { delay_ms: 0 })
			}, milliseconds)
		}
		return window.setTimeout(command, milliseconds)
	}
	if (typeof command == 'string' && (command.match(gyieldregex) || command.match(/await /))) {
		exodus_flight_log('LEGACY STRING TIMEOUT "' + command + '"')
		command = command.replace(gyieldregex, '').replace(/await /g, '').replace(/"/g, "'")
		return window.setTimeout(function () {
			exodus_begin_when_idle(function () {
				return exodustimeout_async_run(command)
			}, 'timeout ' + command, { delay_ms: 0 })
		}, milliseconds)
	}
	return window.setTimeout(command, milliseconds)
}

// LEGACY: evaluate a string expression under Gate A (via exodussettimeout string path).
async function exodustimeout_async_run(command) {
	var fn = new Function('return ' + command)
	return await fn()
}

// Interval wrapper. Prefer function callbacks; AsyncFunction → exodus_begin_if_idle each tick.
function exodussetinterval(command, milliseconds) {
	if (typeof command == 'function') {
		if (exodusisasyncfunction(command)) {
			var label = 'interval ' + (command.name || 'fn')
			return window.setInterval(function () {
				void exodus_begin_if_idle(command, label)
			}, milliseconds)
		}
		return window.setInterval(command, milliseconds)
	}
	if (typeof command == 'string' && (command.match(gyieldregex) || command.match(/await /))) {
		exodus_flight_log('LEGACY STRING INTERVAL "' + command + '"')
		command = command.replace(gyieldregex, '').replace(/await /g, '').replace(/"/g, "'")
		return window.setInterval(function () {
			void exodusinterval_async_sync(command)
		}, milliseconds)
	}
	return window.setInterval(command, milliseconds)
}

// LEGACY: string expression under Gate A when idle (relock/keepalive style).
async function exodusinterval_async_sync(command) {
	if (g_exodus_flow || gblockevents) {
		exodus_flight_log('SKIP interval "' + command + '" (busy)')
		return
	}
	if (typeof db != 'undefined' && db.requesting) {
		exodus_flight_log('SKIP interval "' + command + '" (db.requesting)')
		return
	}
	await exodus_begin(async function () {
		var fn = new Function('return ' + command)
		return await fn()
	}, 'interval ' + command)
}

// Guard: SYSTEM_ERROR report must not re-enter systemerror if the side request fails.
var gexodus_reporting_system_error = false

function systemerror(functionname, e) {
	if (typeof functionname == 'undefined')
		functionname = 'undefined'
	// User/client cancelled or aborted a server request (db.send abort, xhttp .5
	// "Error: Client cancelled request in EXODUS xhttp.php …"). Not a system failure —
	// typeahead supersede, unload, Gate A race. Match exact 'Cancelled' or the xhttp text.
	var fns = String(functionname)
	if (fns == 'Cancelled' || fns.indexOf('Client cancelled') >= 0)
		return
	if (typeof e == 'undefined')
		e = ''
	var msg = e.toString()
	if (msg == 'Cancelled' || msg.indexOf('Client cancelled') >= 0)
		return
	//if (e.name)
	//	msg+='\n'+e.name
	//if (e.message)
	//	msg+='\nError '+e.message
	if (e.fileName)
		msg += '\nFilename ' + e.fileName
	if (!e.stack && e.lineNumber)
		msg += ' Line No. ' + e.lineNumber
	if (!e.stack && e.columnNumber)
		msg += ' Column No. ' + e.columnNumber
	if (e.stack)
		msg += '\n' + e.stack
	else {
		var stack = new Error().stack
		if (stack)
			msg += '\n\n' + stack.toString()//.exodusconvert('\n\r','||')
		else {
			var caller
			if (arguments && arguments.callee)
				caller = arguments.callee
			else if (systemerror && systemerror.caller)
				caller = systemerror.caller
			if (caller)
				msg += '\n'
			while (caller) {
				msg += '\n' + caller.toString().split('\n')[0]
				caller = caller.caller
			}
		}
	}
	// Full technical + stack (+ page URL). Always console.log — whether or not the
	// WUI alert hides it. Side-request data for xhttp SYSTEM_ERROR (hijacked in PHP;
	// never depends on listen being up).
	var technical = 'System Error: ' + functionname + '\n' + msg
	try {
		var pageurl = ''
		if (typeof location != 'undefined')
			pageurl = location.href || ((location.pathname || '') + (location.search || ''))
		if (pageurl)
			technical += '\n\nPage: ' + pageurl
	} catch (eurl) { }
	try {
		console.log('EXODUS systemerror:\n' + technical)
	} catch (e2) { }

	// Report on a side dblink (not main db). Awaitable if callers use await systemerror().
	// Send OK → friendly "support informed"; fail → alert full technical text.
	return (async function systemerror_report_and_alert() {
		var reported = false
		if (!gexodus_reporting_system_error && !gonunload && typeof exodusdblink == 'function') {
			gexodus_reporting_system_error = true
			try {
				var reportdb = new exodusdblink()
				reportdb.request = 'EXECUTE\rGENERAL\rSYSTEM_ERROR'
				// data = full technical text; xhttp.php hijacks this request (not listen)
				reported = !!(await reportdb.send(technical))
			} catch (e4) {
				reported = false
			} finally {
				gexodus_reporting_system_error = false
			}
		}
		if (!gonunload) {
			var usermsg = reported ? gexodus_system_error_user_msg : technical
			try {
				usermsg = String(usermsg).replace(/\r\n/g, '\n')
			} catch (e3) { }
			alert(usermsg)
		}
		if (gstepping || (gusername == 'EXODUS'))
			debugger
	})()
}

// Implementation of $$. Class-based (exodusid_*) so one logical name can have many nodes
// (rows/repeats). single → element, multi → HTMLCollection, miss → getElementById (one only).
function $class(elementid, element) {
	var temp
	if (element)
		temp = element.getElementsByClassName('exodusid_' + elementid)
	else
		temp = document.getElementsByClassName('exodusid_' + elementid)
	// one → bare element (callers use .value / .focus etc. without [0])
	if (temp.length == 1)
		return temp[0]
	// many → collection (callers index or iterate)
	else if (temp.length > 1)
		return temp

	// fallback: real id attribute (always at most one; ignores element scope)
	temp = document.getElementById(elementid)
	if (temp)
		return temp

	return
}

function setdisabledandhidden(element, truefalse) {
	if (!element)
		return
	if (truefalse) {
		element.disabled = true//this seems to have the effect of setting attribute disabled to "" in modern browsers!
		element.setAttribute('disabled', 'disabled')
		element.style.display = 'none'
	}
	else {
		element.disabled = false
		if (element.removeAttribute)
			element.removeAttribute('disabled')
		element.style.display = ''
	}
}


//cross browser
async function getcurrentstyle(element) {
	if (window.getComputedStyle)
		return document.defaultView.getComputedStyle(element, null)
	if (element.currentStyle)
		return element.currentStyle
}

// Firefox innerText historically differed from WebKit (BR → newline); use walk when needed
//need BR to show as \n
function exodus_getinnertext(element) {
	if (element.tagName == 'BR')
		return '\n'
	if (typeof element.nodeValue == 'string')
		return element.nodeValue
	var nodes = element.childNodes
	if (!nodes)
		return ''
	var text = ''
	for (var noden = 0; noden < nodes.length; ++noden) {
		//for (var noden=0;noden<nodes.length;++noden) {
		//alert(nodes[node].tagName+' '+nodes[node].textContent)
		text += exodus_getinnertext(nodes[noden])//recursive
		//alert(text)
	}
	return text
}

function exodusconfirm_scrollpane() {
	var div = $$('exodusconfirmdiv')
	return div && (div.querySelector('.exodusconfirm_body') || div)
}

// Scroll a table row into view below a sticky thead (scrollIntoView nearest
// can leave the focused radio/row under sticky colheads on wheel/Up/Home).
// +4px: radio/checkbox focus ring is 3px box-shadow; flush under thead clips it.
function exodus_scroll_row_below_sticky_thead(tr, scrollpane, thead) {
	if (!tr || !scrollpane)
		return
	if (!thead) {
		var table = getancestor(tr, 'table')
		thead = table && table.tHead
	}
	var headH = thead ? thead.offsetHeight : 0
	var paneRect = scrollpane.getBoundingClientRect()
	var trRect = tr.getBoundingClientRect()
	var topLimit = paneRect.top + headH + (headH ? 4 : 0)
	if (trRect.top < topLimit)
		scrollpane.scrollTop -= (topLimit - trRect.top)
	else if (trRect.bottom > paneRect.bottom)
		scrollpane.scrollTop += (trRect.bottom - paneRect.bottom)
}

function exodusconfirm_footerwrap(content) {
	// Buttons only — left edge shared via one-row two-col shell (icon | everything).
	return content
}

/*
 * Monochrome icons: black SVG + CSS mask tint (--exodus-icon-green/red/orange/…).
 * Multicolour icons (New/Edit/Delete) stay as normal <img src>. See global.css .exodus-icon.
 *
 * IMPORTANT: mask-image URLs must be applied on the element (or resolved absolute).
 * url() inside a custom property used from global.css is resolved against the CSS
 * file path and often 404s → solid coloured square.
 */
function exodus_icon_abs_url(maskFile) {
	var rel = (typeof gimagetheme != 'undefined' ? gimagetheme : '') + maskFile
	try {
		// Resolve like <img src> — against the document URL
		var a = document.createElement('a')
		a.href = rel
		return a.href
	} catch (e) {
		return rel
	}
}

function exodus_icon_mask_style(maskFile) {
	var abs = exodus_icon_abs_url(maskFile)
	// Set mask on the element style so the URL resolves against the document
	return '-webkit-mask-image:url(\'' + abs + '\');mask-image:url(\'' + abs + '\')'
}

function exodus_icon_html(maskFileOrSpec, colorName, extraAttrs) {
	var mask = maskFileOrSpec
	var color = colorName || 'darkgrey'
	if (exodus_is_icon_spec(maskFileOrSpec)) {
		mask = maskFileOrSpec.mask
		color = maskFileOrSpec.color || 'darkgrey'
	}
	var attrs = extraAttrs || ''
	// extraAttrs may include style=; merge mask into style if present
	var style = exodus_icon_mask_style(mask)
	var m = attrs.match(/\bstyle="([^"]*)"/)
	if (m) {
		attrs = attrs.replace(/\bstyle="([^"]*)"/, 'style="' + m[1] + ';' + style + '"')
	} else {
		attrs += ' style="' + style + '"'
	}
	return '<span class="exodus-icon exodus-icon-' + color + '"'
		+ attrs
		+ ' aria-hidden="true"></span>'
}

function exodus_icon_apply(el, spec) {
	if (!el || !exodus_is_icon_spec(spec))
		return
	var id = el.id
	// Keep framework id→class map (exodusid_*) so $$() still finds multi-id groups
	// (e.g. all sortbutton_N). Do not wipe className wholesale.
	var keep = ''
	if (el.className) {
		var parts = String(el.className).split(/\s+/)
		for (var pi = 0; pi < parts.length; pi++) {
			if (parts[pi].indexOf('exodusid_') === 0)
				keep += (keep ? ' ' : '') + parts[pi]
		}
	}
	el.className = (keep ? keep + ' ' : '') + 'exodus-icon exodus-icon-' + (spec.color || 'darkgrey')
	if (id)
		el.id = id
	var abs = exodus_icon_abs_url(spec.mask)
	el.style.webkitMaskImage = 'url(\'' + abs + '\')'
	el.style.maskImage = 'url(\'' + abs + '\')'
	// clear any old custom-prop approach
	el.style.removeProperty('--exodus-icon-mask')
}

// Create or update an icon host: monochrome {mask,color} → .exodus-icon; string → <img>
function exodus_create_icon_element(specOrUrl) {
	if (exodus_is_icon_spec(specOrUrl)) {
		var tmp = document.createElement('span')
		tmp.innerHTML = exodus_icon_html(specOrUrl)
		return tmp.firstChild
	}
	var img = document.createElement('img')
	img.src = specOrUrl
	return img
}

function exodus_set_icon_element(el, specOrUrl) {
	if (!el)
		return null
	if (exodus_is_icon_spec(specOrUrl)) {
		if (el.classList && el.classList.contains('exodus-icon')) {
			exodus_icon_apply(el, specOrUrl)
			return el
		}
		var span = exodus_create_icon_element(specOrUrl)
		if (el.id)
			span.id = el.id
		// keep common attributes used on field chrome / static toolbar buttons
		;['title', 'isexoduspopup', 'isexoduslink', 'exodusonclick', 'exodustype',
			'accesskey', 'exogroupno', 'style', 'class'].forEach(function (n) {
			if (n == 'class' || n == 'style')
				return
			var v = el.getAttribute && el.getAttribute(n)
			if (v)
				span.setAttribute(n, v)
		})
		if (el.getAttribute && el.getAttribute('style')) {
			// keep cursor etc. without wiping mask
			var st = el.getAttribute('style')
			if (st.indexOf('cursor') >= 0)
				span.style.cursor = el.style.cursor || 'pointer'
			if (el.style.verticalAlign)
				span.style.verticalAlign = el.style.verticalAlign
		}
		if (el.parentNode)
			el.parentNode.replaceChild(span, el)
		return span
	}
	// painted URL
	if (el.tagName == 'IMG') {
		el.src = specOrUrl
		return el
	}
	var img = document.createElement('img')
	if (el.id)
		img.id = el.id
	img.src = specOrUrl
	if (el.parentNode)
		el.parentNode.replaceChild(img, el)
	return img
}

// Decide Select: green check (mask)
function exodusconfirm_ok_image() {
	return exodus_icon_html(exodus_icon_spec('confirm-ok.svg', 'green'))
}
// Decide Cancel: red X mask
function exodusconfirm_cancel_image() {
	return exodus_icon_html(exodus_icon_spec('record-close.svg', 'red'))
}
// Confirm Yes/OK (positive) — Save label uses menubar tray icon (record-save.svg), else check
function exodusconfirm_yes_image(buttontext) {
	var plain = String(buttontext == null ? '' : buttontext).replace(/<[^>]*>/g, '')
	plain = plain.replace(/\s+/g, ' ').trim()
	if (plain.toLowerCase() == 'save')
		return exodus_icon_html(exodus_icon_spec('record-save.svg', 'green'))
	return exodusconfirm_ok_image()
}
// Confirm No: orange X if Cancel also shown, else red X
function exodusconfirm_no_image(hasCancelButton) {
	return exodus_icon_html(exodus_icon_spec('record-close.svg', hasCancelButton ? 'orange' : 'red'))
}
// Confirm Cancel (Esc) — red U-turn
function exodusconfirm_back_image() {
	return exodus_icon_html(exodus_icon_spec('confirm-back.svg', 'red'))
}

function exodusconfirm_focusable_elements() {

	var confirm=$$('exodusconfirmdiv')
	if (!confirm)
		return []
	// Document order: text input (if any), then footer action controls
	var nodes=confirm.querySelectorAll(
		'#exodusconfirmdiv_textinput,'
		+ ' .exodusconfirm_footer .graphicbutton[tabindex]'
	)
	var list=[]
	for (var i=0;i<nodes.length;++i) {
		var el=nodes[i]
		if (el.disabled||el.getAttribute('tabindex')=='-1')
			continue
		// Fixed dialog children often have offsetParent null — use size instead
		if (!(el.offsetWidth || el.offsetHeight || el.getClientRects().length))
			continue
		list[list.length]=el
	}
	return list
}

function exodusconfirm_focus_endpoint(first) {

	var list=exodusconfirm_focusable_elements()
	if (!list.length)
		return false
	client_focuson(first?list[0]:list[list.length-1])
	return true
}

// ---------------------------------------------------------------------------
// Plain confirm key ownership (OK / Yes-No / text input — not decide lists)
//
// ONE path: capture-phase document keydown while the shell is open.
// Independent of gblockevents / starteventhandler / document_onkeydown.
// Decide lists keep decide_document_onkeydown on the div + Esc via startevent.
// ---------------------------------------------------------------------------
var gexodusconfirm_plain_keydown_capture = false

function exodusconfirm_install_plain_keydown() {
	if (gexodusconfirm_plain_keydown_capture)
		return
	document.addEventListener('keydown', exodusconfirm_plain_keydown, true)
	gexodusconfirm_plain_keydown_capture = true
}

function exodusconfirm_uninstall_plain_keydown() {
	if (!gexodusconfirm_plain_keydown_capture)
		return
	document.removeEventListener('keydown', exodusconfirm_plain_keydown, true)
	gexodusconfirm_plain_keydown_capture = false
}

// Capture-phase: handle keys for plain confirm before form/Gate A sees them.
// AGENTS.md HIGH PRIORITY: typing needs capture true AND startevent true (both).
function exodusconfirm_plain_keydown(event) {

	var conf = document.getElementById('exodusconfirmdiv')
	if (!conf || (conf.classList && conf.classList.contains('exodusconfirm_decide')))
		return

	var r = exodusconfirm_keymap(event)
	// true = typing/copy: leave default action; do not stopPropagation (target needs key)
	if (r === null || r === true)
		return
	// WRONG: also preventDefault when r === true — kills character insertion
	// if (event.preventDefault) event.preventDefault()
	// Handled or swallow — do not let form path see this key
	if (event.stopImmediatePropagation)
		event.stopImmediatePropagation()
	if (event.stopPropagation)
		event.stopPropagation()
	if (event.preventDefault)
		event.preventDefault()
}

// Isolation for form path while confirm/decide is open (gblockevents / document_onkeydown).
// Plain confirm keys are already handled in capture (above); this only:
//   - swallows form shortcuts while any confirm is up
//   - Esc-cancels decide when focus is outside the popup
// Return null = no confirm; true = allow browser; false = swallow form path.
function exodusconfirm_startevent(event) {

	var confirmdiv = document.getElementById('exodusconfirmdiv')
	if (!confirmdiv)
		return null

	if (event.type == 'copy')
		return true

	// Decide: keys on the popup div; form path only swallows (Esc if focus outside).
	if (document.getElementById('decide_table1')
		|| (confirmdiv.classList && confirmdiv.classList.contains('exodusconfirm_decide'))) {
		if (event.type != 'keydown')
			return false
		if (event.ctrlKey && (event.which == 67 || event.keyCode == 67))
			return true
		var dkey = event.keyCode ? event.keyCode : event.which
		if (dkey == 27) {
			window.setTimeout(function () {
				resolvePendingConfirm('', 'exodusconfirm_startevent Esc decide')
			}, 1)
			return false
		}
		return false
	}

	// Plain confirm while gblockevents: form path only swallows — capture owns Esc/Enter.
	if (event.type != 'keydown')
		return false
	if (event.ctrlKey && (event.which == 67 || event.keyCode == 67))
		return true
	// WRONG (broke all typing in exoui_input — material code prompt etc.):
	// return false
	// RIGHT: when focus is the text field, allow browser (capture already left key un-cancelled).
	var textel = document.getElementById('exodusconfirmdiv_textinput')
	if (textel && document.activeElement === textel)
		return true
	return false
}

// Shared key map for plain confirm (OK/Yes/No/text). Used by capture handler.
// null = not our event type / not open; true = allow browser (type/copy);
// false = action taken or swallow.
function exodusconfirm_keymap(event) {

	if (!document.getElementById('exodusconfirmdiv'))
		return null
	if (event.type != 'keydown')
		return false

	// Ctrl+C copy of message text
	if (event.ctrlKey && (event.which == 67 || event.keyCode == 67))
		return true

	var keycode = event.keyCode ? event.keyCode : event.which
	var keyletter = String.fromCharCode(keycode).toUpperCase()
	var textel = document.getElementById('exodusconfirmdiv_textinput')
	var istextinput = !!textel
	var active = document.activeElement
	var focusedConfirmBtn = null
	if (active) {
		if (active.id == 'positivebutton' || active.id == 'negativebutton' || active.id == 'cancelbutton')
			focusedConfirmBtn = active
		else if (active.closest) {
			var wrap = active.closest('#positivebutton, #negativebutton, #cancelbutton')
			if (wrap)
				focusedConfirmBtn = wrap
		}
	}

	// Tab: cycle text field and buttons (works even when nothing focused yet)
	if (keycode == 9) {
		exodusconfirm_focus_cycle(!!event.shiftKey)
		return false
	}

	// Arrows: same cycle as Tab, unless caret is in the confirm text field
	if ((keycode == 37 || keycode == 38 || keycode == 39 || keycode == 40)
		&& !(istextinput && active && active.id == 'exodusconfirmdiv_textinput')) {
		exodusconfirm_focus_cycle(keycode == 37 || keycode == 38)
		return false
	}

	// Text field focused (no modifiers): type freely. Enter/F9=OK, Esc=Cancel.
	// Pair with startevent text-field return true (both required for typing).
	if (istextinput && active && active.id == 'exodusconfirmdiv_textinput'
		&& !event.altKey && !event.ctrlKey && !event.metaKey) {
		if (keycode == 13 || keycode == 120) {
			window.setTimeout(exodus_confirm_function1_sync, 1)
			return false
		}
		if (keycode == 27) {
			window.setTimeout(exodus_confirm_function3_sync, 1)
			return false
		}
		// Tab already handled above
		// WRONG: fall through to bare O/C access letters below
		return true
	}

	// Space/Enter: footer button focused, or sole OK button (OK-only note/invalid).
	// WRONG: bare Enter always positive on multi-button (silent OK with no highlight).
	if (keycode == 13 || keycode == 32) {
		if (!focusedConfirmBtn
			&& document.getElementById('positivebutton')
			&& !document.getElementById('negativebutton')
			&& !document.getElementById('cancelbutton'))
			focusedConfirmBtn = document.getElementById('positivebutton')
		if (focusedConfirmBtn) {
			if (focusedConfirmBtn.id == 'negativebutton')
				window.setTimeout(exodus_confirm_function2_sync, 1)
			else if (focusedConfirmBtn.id == 'cancelbutton')
				window.setTimeout(exodus_confirm_function3_sync, 1)
			else
				window.setTimeout(exodus_confirm_function1_sync, 1)
			return false
		}
	}

	// Access letters — CHANGE LOG (do not flip-flop; AGENTS HIGH PRIORITY):
	// 1) Bare O/C/Y/N always → broke typing in text-input confirms (material code etc.).
	// 2) Alt+letter only always → pure button OK/Cancel needed Alt+C (wrong for no-input).
	// 3) Split: pure button popup → bare letter (and Alt+ still ok);
	//           text-input confirm → Alt+letter only (bare letters type; text field path
	//           above already returns true when focus is the input).
	// WRONG: var accessLetter = gexodusconfirmletters && keyletter  // always bare
	// WRONG: var accessLetter = event.altKey && gexodusconfirmletters && keyletter  // always Alt
	var accessLetter = gexodusconfirmletters && keyletter
		&& (!istextinput || event.altKey)

	// F9 / first-button letter: always positive (OK/Yes) — not bare Enter
	if (keycode == 120
		|| (accessLetter && keyletter == gexodusconfirmletters[1])) {
		window.setTimeout(exodus_confirm_function1_sync, 1)
		return false
	}

	// Bare Enter/Space with nothing focused: swallow, do not invent a button press

	// CANCEL: Esc always; letter per accessLetter rules above
	if (keycode == 27 || (accessLetter && keyletter == gexodusconfirmletters[3])) {
		window.setTimeout(exodus_confirm_function3_sync, 1)
		return false
	}

	// NEGATIVE: F8; letter per accessLetter rules above
	if (keycode == 119 || (accessLetter && keyletter == gexodusconfirmletters[2])) {
		window.setTimeout(exodus_confirm_function2_sync, 1)
		return false
	}

	// Home/End
	if (keycode == 36 || keycode == 35) {
		exodusconfirm_focus_endpoint(keycode == 36)
		return false
	}

	return false
}

// document_onkeydown belt: swallow form keys while confirm open (capture already acted).
function exodusconfirm_document_keydown(event) {
	return exodusconfirm_startevent(event)
}

// Tab / Shift+Tab within the open confirm (not decide lists — those have their own handler).
// If nothing in the dialog is focused yet, Tab focuses the first control (first button).
function exodusconfirm_focus_cycle(reverse) {

	var list=exodusconfirm_focusable_elements()
	if (!list.length)
		return false
	var active=document.activeElement
	var i=-1
	var j
	for (j=0;j<list.length;++j) {
		if (list[j]===active||(list[j].contains&&list[j].contains(active))) {
			i=j
			break
		}
	}
	var next
	if (i<0)
		// No prefocus: Tab → first button; Shift+Tab → last button
		next=reverse?list[list.length-1]:list[0]
	else if (reverse)
		next=list[(i-1+list.length)%list.length]
	else
		next=list[(i+1)%list.length]
	// focus immediately so successive Tabs work before client_focuson timeout
	try {
		next.focus()
	} catch (e) {}
	client_focuson(next)
	return true
}

// Explicit default only: 1 / 2 / 3. Empty, 0, or omitted → no prefocus.
// Button numbers: 1=positive (OK/Yes), 2=negative (No), 3=cancel (Cancel).
// OK/Cancel dialogs have no negative: default 2 means Cancel (Windows MB_OKCANCEL).
function exodusconfirm_has_default_button(defaultbuttonn) {
	var n=Number(defaultbuttonn)
	return n===1||n===2||n===3
}

function exodusconfirm_default_button_element(defaultbuttonn) {

	if (!exodusconfirm_has_default_button(defaultbuttonn))
		return null
	var defn=Number(defaultbuttonn)
	if (defn==2)
		return $$('negativebutton') || $$('cancelbutton') || null
	if (defn==3)
		return $$('cancelbutton')||null
	return $$('positivebutton')||null
}

var gexodusconfirm_scrollhint_resize

function exodusconfirm_update_scroll_hints() {
	var scrollpane = exodusconfirm_scrollpane()
	var wrap = $$('exodusconfirm_scrollhint_wrap')
	if (!scrollpane || !wrap)
		return

	var canDown = scrollpane.scrollHeight > scrollpane.clientHeight + 1
		&& scrollpane.scrollTop + scrollpane.clientHeight < scrollpane.scrollHeight - 2

	// Type-filter may hide every option (display:none); no triangle when nothing to scroll to
	var tbody = $$('decide_table1body1')
	if (tbody && canDown) {
		var anyVisible = false
		var rows = tbody.getElementsByTagName('tr')
		for (var i = 0; i < rows.length; i++) {
			if (rows[i].style.display == 'none')
				continue
			anyVisible = true
			break
		}
		if (!anyVisible)
			canDown = false
	}

	wrap.classList.toggle('can_scroll_down', canDown)
	wrap.setAttribute('aria-hidden', canDown ? 'false' : 'true')
}

function exodusconfirm_fit_decide_popup(force) {

	var div=$$('exodusconfirmdiv')
	var table=$$('decide_table1')
	if (!div||!table||!div.classList.contains('exodusconfirm_decide'))
		return
	if (div.getAttribute('exodusconfirm_fitted')&&!force)
		return

	// Drop previous pixel lock so the table can measure at natural size after zoom
	div.style.width=''
	div.style.maxHeight=''

	var vw=window.innerWidth||document.documentElement.clientWidth||0
	var vh=window.innerHeight||document.documentElement.clientHeight||0
	// visualViewport tracks pinch/keyboard zoom better than layout viewport alone
	try {
		if (window.visualViewport) {
			if (window.visualViewport.width)
				vw=window.visualViewport.width
			if (window.visualViewport.height)
				vh=window.visualViewport.height
		}
	} catch (e) { }

	var maxw=Math.max(vw-40, 120)
	var maxh=Math.max(vh-40, 120)
	var iconcol=div.querySelector('.exodusconfirm_iconcol')
	var iconw=iconcol ? iconcol.offsetWidth : 0
	// scrollWidth after clearing width = natural content width
	var want=Math.max(table.scrollWidth, table.offsetWidth)+iconw+24
	var footer=div.querySelector('.exodusconfirm_footer')
	if (footer)
		want=Math.max(want, footer.scrollWidth+iconw+24)
	div.style.width=Math.min(want, maxw)+'px'
	div.style.maxHeight=maxh+'px'
	div.setAttribute('exodusconfirm_fitted','1')
	exodusconfirm_update_scroll_hints()
}

function exodusconfirm_bind_scroll_hints() {
	exodusconfirm_unbind_scroll_hints()

	var scrollpane = exodusconfirm_scrollpane()
	if (!scrollpane || !$$('exodusconfirm_scrollhint_wrap'))
		return

	// fit_decide_popup already refreshes the ▼ hint
	exodusconfirm_fit_decide_popup()
	scrollpane.addEventListener('scroll', exodusconfirm_update_scroll_hints, { passive: true })
	gexodusconfirm_scrollhint_resize=function() {
		exodusconfirm_fit_decide_popup(true)
	}
	window.addEventListener('resize', gexodusconfirm_scrollhint_resize, { passive: true })
	// Zoom often updates visualViewport without (or before) window.resize
	try {
		if (window.visualViewport)
			window.visualViewport.addEventListener('resize', gexodusconfirm_scrollhint_resize, { passive: true })
	} catch (e) { }
}

function exodusconfirm_unbind_scroll_hints() {
	if (gexodusconfirm_scrollhint_resize) {
		window.removeEventListener('resize', gexodusconfirm_scrollhint_resize)
		try {
			if (window.visualViewport)
				window.visualViewport.removeEventListener('resize', gexodusconfirm_scrollhint_resize)
		} catch (e) { }
		gexodusconfirm_scrollhint_resize = null
	}
}

// Popup is temporary chrome: remember where the user was, restore after close.
// Raw .focus / setSelectionRange only — no form validate, no client_focuson chain.
function exodusconfirm_capture_invoker() {

	var ae = document.activeElement
	if (!ae || ae === document.body || ae === document.documentElement)
		return null
	// already inside a confirm (nested / re-entry) — leave alone
	try {
		if (ae.closest && ae.closest('#exodusconfirmdiv'))
			return null
	} catch (e) { }
	var saved = { el: ae, start: null, end: null }
	try {
		if (typeof ae.selectionStart == 'number') {
			saved.start = ae.selectionStart
			saved.end = ae.selectionEnd
		}
	} catch (e2) { }
	return saved
}

function exodusconfirm_release_invoker(saved) {

	if (!saved || !saved.el)
		return
	// Drop pending client_focuson into confirm chrome that is about to vanish
	if (gclient_focuson_element) {
		try {
			if (!document.body.contains(gclient_focuson_element)
				|| (gclient_focuson_element.closest
					&& gclient_focuson_element.closest('#exodusconfirmdiv')))
				gclient_focuson_element = undefined
		} catch (e) {
			gclient_focuson_element = undefined
		}
	}
	var el = saved.el
	var start = saved.start
	var end = saved.end
	// Same 1ms delay as client_focuson so we run after any pending focus-into-button
	window.setTimeout(function () {
		try {
			if (!el || !el.focus || !document.body.contains(el) || el.disabled)
				return
			if (el.closest && el.closest('#exodusconfirmdiv'))
				return
			try {
				el.focus({ preventScroll: true })
			} catch (e0) {
				el.focus()
			}
			if (typeof start == 'number' && typeof el.setSelectionRange == 'function') {
				try {
					el.setSelectionRange(start, end)
				} catch (e2) { }
			}
		} catch (e3) { }
	}, 1)
}

async function exodusconfirm2(questionx, defaultbuttonn, positivebuttonx, negativebuttonx, cancelbuttonx, text, texthidden, imagesrc, default_icons) {

	//performs "in-window" questions, selections and inputs
	//replaces (or called by)
	//exoui_confirm: questions (yes/no/cancel) and one line inputs
	//exoui_decide/exoui_decide2: selections
	// default_icons: role icons on Yes/No/Cancel-style buttons (default true; false for multi-choice labels)

	// Capture before any focus into the popup shell
	var invoker_focus = exodusconfirm_capture_invoker()

	if (typeof default_icons == 'undefined')
		default_icons = true

	var decide_args//holds popup list args if any

	if (typeof questionx == 'object') {

		//[question, data, cols, returncoln, defaultreply, many, inverted]
		decide_args = questionx
		questionx = decide_args[0]
		var decide_returnmany = decide_args[5]

		//question
		if (!questionx)
			questionx = 'Which do you want?'
		questionx = questionx.replace(/[\|\r\n]/g, '<br />')
		//if (decide_returnmany)
		//	questionx += '&nbsp;&nbsp;<span style="font-size:66%">(A=All)</span>'

	} else
		var decide_args = undefined

	var nbuttons = 0
	var buttons = []

	gexodusconfirmletters = []

	//check buttons
	if (positivebuttonx)
		nbuttons++
	if (negativebuttonx)
		nbuttons++
	if (cancelbuttonx)
		nbuttons++
	if (nbuttons == 0) {
		positivebuttonx = '<u>Y</u>es'
		negativebuttonx = '<u>N</u>o'
		cancelbuttonx = '<u>C</u>ancel'
		nbuttons = 3
	}

	var istextinput = typeof text != 'undefined' && text != null

	//create a centralised div with the appropriate buttons or input box
	var div = document.createElement('div')
	// id name determines style, see global.css
	div.id = 'exodusconfirmdiv'
	div.classList.add('exodusconfirmdiv')
	if (istextinput)
		div.classList.add('exodusconfirm_textinput')
	if (decide_args)
		div.classList.add('exodusconfirm_decide')
	div.style.maxHeight = (window.innerHeight - 120) + 'px'

	//image

	var html = ''
	if (!imagesrc && (questionx.indexOf('!') + 1 || questionx.toLowerCase().indexOf('are you sure ') + 1 || questionx.toLowerCase().indexOf('warning') + 1))
		imagesrc = 'warning'
	else if (!imagesrc) {
		if (nbuttons == 1)
			imagesrc = 'info'
		else
			imagesrc = 'question1'
	}
	if (imagesrc) {
		// Painted dialog discs: _lm / _dm twins (DM brighter so info/question stay visible).
		var xpdm = (typeof gisdarktheme != 'undefined' && gisdarktheme) ? '_dm' : '_lm'
		if (imagesrc == 'critical') {
			imagesrc = 'dialog-critical' + xpdm + '.svg'
			div.classList.add('exodusconfirm_critical')
		}
		if (imagesrc == 'warning') {
			imagesrc = 'dialog-warning' + xpdm + '.svg'
			div.classList.add('exodusconfirm_warning')
		}
		if (imagesrc == 'info') {
			imagesrc = 'dialog-info' + xpdm + '.svg'
			div.classList.add('exodusconfirm_info')
		}
		if (imagesrc == 'question1') {
			imagesrc = 'dialog-question' + xpdm + '.svg'
			div.classList.add('exodusconfirm_question1')
		}
		if (!(imagesrc.indexOf('/') + 1 + imagesrc.indexOf('\\') + 1)) {
			imagesrc = gimagetheme + imagesrc
			if (window.location.href.toString().indexOf('index.html') >= 0)
				imagesrc = imagesrc.slice(3)
		}
		html += '<img src="' + imagesrc + '" alt="" xstyle="display: none" />'
	}
	var imagehtml = html

	//question

	var html = ''
	questionx = questionx.replace(/\r\n/g, '\r')
	questionx = questionx.replace(/\r\r/g, '<p />')
	questionx = questionx.replace(/\r/g, '<br />')
	questionx = questionx.replace(/\n/g, '<br />')
	questionx = questionx.replace(FMre, '<br />')
	questionx = questionx.replace(VMre, '<br />')
	questionx = questionx.replace(SMre, '<br />')
	questionx = questionx.replace(TMre, '<br />')
	questionx = questionx.replace(/\|/g, '<br />')
	var questionhtml = questionx

	//buttons
	var html = ''
	if (!decide_args) {
		var nbuttons = 0
		if (positivebuttonx)
			await addbutton('positive', 1, positivebuttonx, 'F9')
		if (negativebuttonx)
			await addbutton('negative', 2, negativebuttonx, 'F8')
		if (cancelbuttonx)
			await addbutton('cancel', 3, cancelbuttonx, 'Esc')

		//make sure no button is default unless specified
		if (!defaultbuttonn && !istextinput) {
			//xxtry{gbuttons[0].blur()}catch(e){}
		}
	}
	var buttonshtml = html
	html = ''

	async function addbutton(buttonid, buttonn, buttontext, buttonfunckey) {

		nbuttons++

		// Same chrome as decide Select/Cancel (graphicbutton: filled outset face).
		// Icons: 1 Yes=green check; 2 No=orange X if Cancel present else red X; 3 Cancel=red U-turn.
		html += '<span id="' + buttonid + 'button"'
		html += ' tabindex="0"'
		html += ' class="graphicbutton"'
		html += ' onclick="exodus_confirm_function' + buttonn + '_sync()"'

		// Hotkey letter: pure button = bare letter; text-input confirm = Alt+letter only.
		// (See exodusconfirm_keymap accessLetter CHANGE LOG — do not flip-flop.)
		var letter
		var marked = String(buttontext).match(/<[uU]>(.)<\/[uU]>/)
		if (!marked)
			marked = String(buttontext).match(/<[iI]>(.)<\/[iI]>/)
		if (marked) {
			letter = marked[1]
			// Normalise <i> hotkey markup to underline for display
			buttontext = String(buttontext).replace(/<[iI]>(.)<\/[iI]>/, '<u>$1</u>')
		} else {
			letter = String(buttontext).replace(/<[^>]*>/g, '').charAt(0)
			// Auto-underline first character when label is plain text (e.g. Before/After/Cancel)
			if (letter && String(buttontext).charAt(0).toUpperCase() == letter.toUpperCase())
				buttontext = '<u>' + String(buttontext).charAt(0) + '</u>' + String(buttontext).slice(1)
		}
		if (letter) {
			letter = letter.toUpperCase()
			html += ' exodusletter="' + letter + '"'
		}
		gexodusconfirmletters[buttonn] = letter

		// Tooltip matches keymap: bare letter when no text field; Alt+ when text-input.
		html += ' title="Press '
		if (letter) {
			if (istextinput)
				html += 'Alt+' + letter + ' or '
			else
				html += letter + ' or '
		}
		html += buttonfunckey
		if (buttonn == 1)
			html += ' or Ctrl+Enter'
		html += '"'

		//set the button number
		html += ' exodusbuttonnumber="' + nbuttons + '"'
		html += ' exodusyesnocancel="' + (buttonn % 3) + '"'

		var iconhtml = ''
		if (default_icons) {
			if (buttonn == 1)
				iconhtml = exodusconfirm_yes_image(buttontext)
			else if (buttonn == 2)
				iconhtml = exodusconfirm_no_image(!!cancelbuttonx)
			else if (buttonn == 3)
				iconhtml = exodusconfirm_back_image()
		}

		html += '>'
		if (iconhtml)
			html += iconhtml
		html += buttontext + '</span>'

	}//end of addbutton

	// One-row two-col shell: col1 = type icon, col2 = everything else
	// (question, option table / text input, scroll ▼, footer buttons).
	// Shared left edge for title / table / cont mark / buttons — no spacer hacks.
	var bodyinner = '\
					<div class="statementclass" id="question1">\
					'+ questionhtml + '\
					</div>'

	var footerhtml = ''

	if (decide_args) {
		bodyinner += '\
			<div class="exodusconfirm_decideblock">\
			<div id="decide_filter_status" class="decide_filter_status" style="display:none"></div>\
			<table id="decide_table1" xwidth=100% xclass="exodusform">\
				<thead onclick="decide_sorttable2_sync(event)" style="cursor: pointer">\
					<tr id="decide_table1head1row1">\
					</tr>\
				</thead>\
				<tbody id="decide_table1body1">\
				</tbody>\
			</table>\
			</div>'
		// Icon+label graphicbuttons (mask-tinted mono icons).
		// "Select" not "OK" — avoids confusion when an option is itself named Cancel.
		// Bare letters type-to-filter; Select/Cancel via Enter/Esc/F9 or Alt+S / Alt+C.
		footerhtml = exodusconfirm_footerwrap(
			'<span id="decide_okbutton" tabindex="0" class="graphicbutton"'
			+ ' title="Press Enter, Ctrl+Enter, F9 or Alt+S">'
			+ exodusconfirm_ok_image()
			+ '<span id="decide_okbutton_label"><u>S</u>elect</span>'
			+ '</span>'
			+ '<span id="decide_cancelbutton" tabindex="0" class="graphicbutton"'
			+ ' title="Press Esc or Alt+C (Esc clears type-filter first)">'
			+ exodusconfirm_cancel_image()
			+ '<span id="decide_cancelbutton_label"><u>C</u>ancel</span>'
			+ '</span>')
	} else if (istextinput) {
		// NB id 'exodusconfirmdiv_textinput' used in starteventhandler()
		bodyinner += '\
						<input id="exodusconfirmdiv_textinput" size="60" style="display: block;">'
		footerhtml = exodusconfirm_footerwrap('<span id="yesnocancelbuttons">'+ buttonshtml + '</span>')
	} else {
		footerhtml = exodusconfirm_footerwrap('<span id="yesnocancelbuttons">'+ buttonshtml + '</span>')
	}

	var scrollhinthtml = ''
	if (decide_args)
		scrollhinthtml = '\
			<div class="exodusconfirm_scrollhint_wrap" id="exodusconfirm_scrollhint_wrap" aria-hidden="true">\
				<div class="exodusconfirm_scrollhint" id="exodusconfirm_scrollhint">&#9660;</div>\
			</div>'

	// Div shell (not table): option rows are the only <tr>s, so click hit-testing is local.
	var html = '\
		<div class="exodusconfirm_layout">\
			<div class="exodusconfirm_iconcol">'+ imagehtml + '</div>\
			<div class="exodusconfirm_promptcol">\
				<div class="exodusconfirm_promptstack">\
					<div class="exodusconfirm_body">'+ bodyinner + '</div>\
					'+ scrollhinthtml + '\
					<div class="exodusconfirm_footer">'+ footerhtml + '</div>\
				</div>\
			</div>\
		</div>'

	//finally create the div body
	div.innerHTML = html

	//insert and centralise the div after it has autosized itself
	document.body.insertBefore(div, null)

	if (istextinput) {
		var textinput = $$('exodusconfirmdiv_textinput')
		// $$ may return a NodeList if multiple matches — use the real input element
		if (textinput && !textinput.tagName && textinput.length)
			textinput = textinput[0]
		// type=password before value; focus later (after blockmodal) — early focus
		// flashes then loses to pending form client_focuson / browser type-change.
		if (texthidden)
			textinput.type = 'password'
		textinput.value = text == null ? '' : String(text)
		textinput.autocomplete = texthidden ? 'new-password' : 'off'
		// Enter in the field = OK (including empty string — historical confirm.htm behaviour).
		// Esc = Cancel. Document-level handler also covers this while gblockevents is set.
		textinput.onkeydown = function exodusconfirm_textinput_onkeydown(event) {
			event = getevent(event)
			var keycode = event.keyCode ? event.keyCode : event.which
			if (keycode == 13) {
				window.setTimeout(exodus_confirm_function1_sync, 1)
				return exoduscancelevent(event)
			}
			if (keycode == 27) {
				window.setTimeout(exodus_confirm_function3_sync, 1)
				return exoduscancelevent(event)
			}
			return true
		}
		// Focus applied after shell is fully open (see below). Do not focus here.
	}

	//build rows of decide popup
	if (decide_args) {

		//build rows, or if only one option, then obtain the response
		/////////////////////////////////////////////////////////////
		var response = await decide_onload(decide_args)

		//return only option
		// quit if false ie failed to load eg no options
		// (decide_fail_no_options may already have removed the shell before invalid)
		if (typeof response != 'undefined') {
			if (div && div.parentNode)
				exodusremovenode(div)
			exodusconfirm_release_invoker(invoker_focus)
			return response
		}

		exodusconfirm_bind_scroll_hints()

	}

	// centers exodusconfirmdiv popups via global.css (dynamic centering on resize)

	//if case too much to fit vertically on the screen, use scrollbars on the body only
	//for messages show the bottom of the message; footer buttons stay visible
	//for popup lists, show the top of the list
	var scrollpane = exodusconfirm_scrollpane()
	if (!decide_args)
		scrollpane.scrollTop = scrollpane.scrollHeight

	//div.onkeydown=function exodusconfirm_onkeydown(event) {
	//	exoduscancelevent(event)
	//}

	//YIELD RIGHT HERE!
	//1. hang here until something like a button click function calls geventhandler .next(response)
	//2. keyword "yield" must stay out of /2/ build strings (legacy)
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Phase 1.2: convert the confirm/decide UI leaf to Promise-driven.
	// The various click/key handlers now resolve this promise (via resolvePendingConfirm).
	// fromPromise feeds the value to the existing generator machinery exactly as before.
	// All higher wrappers (exoui_confirm, decide*, yesno, input, filepopup etc.) and
	var confirmResolve
	var confirmPromise = new Promise((resolve) => {
		confirmResolve = resolve
	})
	gpendingConfirmResolve = confirmResolve
	// Gate B wait/cancel owns this confirm when B is open; otherwise Gate A business.
	gpendingConfirmOwner = (g_exodus_waitcancel || gprocessing_waitcancel_active) ? 'B' : 'A'
	gexodusconfirmdefaultbutton = defaultbuttonn || 1

	blockmodalui_sync()
	form_blockevents(true, 'exodusconfirm2')
	// Plain confirm: capture keydown owns Enter/Esc (not the form gblockevents maze).
	// Decide lists use decide_document_onkeydown on the div instead.
	if (!decide_args)
		exodusconfirm_install_plain_keydown()

	// Text-input confirm: caret in field (after modal lock).
	if (istextinput) {
		if (typeof gclient_focuson_element != 'undefined')
			gclient_focuson_element = undefined
		var exodusconfirm_park_text_focus = function () {
			try {
				var el = document.getElementById('exodusconfirmdiv_textinput')
				if (!el || !document.getElementById('exodusconfirmdiv'))
					return
				if (document.activeElement === el)
					return
				try {
					el.focus({ preventScroll: true })
				} catch (e0) {
					el.focus()
				}
				if (el.value && el.type != 'password' && typeof el.select == 'function')
					el.select()
			} catch (e) { }
		}
		exodusconfirm_park_text_focus()
		window.setTimeout(exodusconfirm_park_text_focus, 1)
		window.setTimeout(exodusconfirm_park_text_focus, 50)
	} else if (!decide_args) {
		// Default button, or sole OK (note/invalid) so focus matches Enter behaviour
		var defbtn = exodusconfirm_default_button_element(defaultbuttonn)
		if (!defbtn && nbuttons == 1)
			defbtn = document.getElementById('positivebutton')
		if (defbtn)
			client_focuson(defbtn)
	}

	var response
	try {
		response = await confirmPromise
	} finally {
		if (!decide_args)
			exodusconfirm_uninstall_plain_keydown()
		gpendingConfirmResolve = null
		gpendingConfirmOwner = null
		gexodusconfirmdefaultbutton = null
		form_blockevents(false, 'exodusconfirm2')
		unblockmodalui_sync()
		exodusconfirm_unbind_scroll_hints()
		exodusremovenode(div)
		exodusconfirm_release_invoker(invoker_focus)
	}

	// Text input: OK → string (incl. ''); Cancel → false.
	// WRONG: return '' or 0 on Cancel — callers cannot tell empty OK from cancel.
	if (istextinput) {
		if (typeof response == 'string')
			return response
		return false
	}

	//no response treated same as cancel button (0) or '' if popup list
	if (!response) {
		if (decide_args)
			response = ''
		else
			response = 0
	}

	return response
}

//return 1 - Positive Button i.e. 'Ok' with optional text input
// DOM/HTML entry points — *_sync (resolve confirm leaf; not Gate A takeoff)
function exodus_confirm_function1_sync(event) {
	var el = $$('exodusconfirmdiv_textinput')
	if (el && !el.tagName && el.length)
		el = el[0]
	// Always pass a string when the text field is present so empty OK is '' not 1/0/false
	if (el && el.tagName)
		return exodus_confirm_function(String(el.value), event)
	return exodus_confirm_function(1, event)
}

//return 2
function exodus_confirm_function2_sync(event) {
	return exodus_confirm_function(2, event)
}

//return 0
function exodus_confirm_function3_sync(event) {
	return exodus_confirm_function(0, event)
}

// Click on modal blocker outside confirm: last footer action (Cancel/No), not OK/Select.
// Matches Esc-ish "leave" rather than affirming the default/first button.
function exodus_confirm_outside_click_sync(event) {
	if ($$('decide_cancelbutton'))
		return resolvePendingConfirm('', 'exodus_confirm_outside_click_sync')
	if ($$('cancelbutton'))
		return exodus_confirm_function3_sync(event)
	if ($$('negativebutton'))
		return exodus_confirm_function2_sync(event)
	// Only one action button (e.g. OK alone)
	return exodus_confirm_function1_sync(event)
}

// expectedOwner: optional 'A' | 'B' — when set, only resolve if gpendingConfirmOwner matches
// (force-close paths). User button/key paths omit it and always answer the open confirm.
function resolvePendingConfirm(value, source, expectedOwner) {
	if (gpendingConfirmResolve) {
		if (expectedOwner && gpendingConfirmOwner && gpendingConfirmOwner !== expectedOwner) {
			exodus_flight_log(
				'CONFIRM resolve blocked (want ' + expectedOwner
				+ ' have ' + gpendingConfirmOwner + ' from ' + source + ')'
			)
			return false
		}
		var resolver = gpendingConfirmResolve
		var owner = gpendingConfirmOwner
		gpendingConfirmResolve = null
		gpendingConfirmOwner = null
		logevent(
			'resolvePendingConfirm value=' + value
			+ ' owner=' + (owner || '?') + ' from ' + source
		)
		resolver(value)
		return true
	}
	// No pending confirm — ignore duplicate clicks after the dialog already resolved.
	return false
}

// expectedOwner: optional 'A' — child-window dialogs are always Gate A.
function resolvePendingDialog(value, source, expectedOwner) {
	if (gpendingDialogResolve) {
		if (expectedOwner && gpendingDialogOwner && gpendingDialogOwner !== expectedOwner) {
			exodus_flight_log(
				'DIALOG resolve blocked (want ' + expectedOwner
				+ ' have ' + gpendingDialogOwner + ' from ' + source + ')'
			)
			return false
		}
		var resolver = gpendingDialogResolve
		var owner = gpendingDialogOwner
		gpendingDialogResolve = null
		gpendingDialogOwner = null
		logevent(
			'resolvePendingDialog value=' + value
			+ ' owner=' + (owner || '?') + ' from ' + source
		)
		resolver(value)
		return true
	}
	return false
}

function exodus_confirm_function(buttonno, event) {

	console.log('exodus_confirm_function buttonno:' + buttonno)
	event = getevent(event)
	exoduscancelevent(event)
	//exodus_resume(buttonno, 'exodus_confirm_function')
	resolvePendingConfirm(buttonno, 'exodus_confirm_function')
}

//backpage when popup is up should remove the popup and NOT backpage
//especially important on mobile where no escape or backspace key exists
window.addEventListener('popstate', cancel_backpage_event, false);
function cancel_backpage_event(event) {
	// The popstate event is fired each time when the current history entry changes.
	// there must be some history present (see pushState below)

	//execute the normal action if no popup is present
	if (!document.getElementById('exodusconfirmdiv')) {
		history.back()
		return
	}

	//remove the popup and its controlling generator/coroutine
	//exodusremovenode(exodusconfirmdiv)
	//geventhandler = false
	//exodus_resume(false, 'cancel_backpage_event')
	resolvePendingConfirm(false, 'cancel_backpage_event')

	// History entry so Back/popstate dismisses the popup (not navigate away) — mobile has no Esc.
	// Keep search+hash: pathname alone drops ?MODE=… (e.g. mediadiary.htm?MODE=CERTIFICATEFILE).
	history.pushState(null, null, window.location.pathname + window.location.search + window.location.hash);

	return
}

// Decide shell is mounted before rows are built; remove it before exoui_invalid
// so the pale list popup does not sit behind the invalid dialog.
async function decide_fail_no_options() {
	gdecide_onwheel = null
	gdecide_select_all = false
	gdecide_ctx_menu = false
	if (gdecide_ctx_timer) {
		window.clearTimeout(gdecide_ctx_timer)
		gdecide_ctx_timer = null
	}
	var shell = $$('exodusconfirmdiv')
	if (shell)
		exodusremovenode(shell)
	return await exoui_invalid('No records found.')
}

// --- Decide list select-all / copy (same idea as form typeahead) ---

function decide_is_open() {
	return !!(document.getElementById('decide_table1'))
}

function decide_selection_in_list() {
	var table = document.getElementById('decide_table1')
	if (!table)
		return false
	var sel = window.getSelection && window.getSelection()
	if (!sel || sel.isCollapsed || !sel.rangeCount)
		return false
	return !!(table.contains(sel.anchorNode) || table.contains(sel.focusNode))
}

function decide_select_all_list() {
	var table = document.getElementById('decide_table1')
	if (!table)
		return false
	if (window.getSelection && document.createRange) {
		var sel = window.getSelection()
		var range = document.createRange()
		range.selectNodeContents(table)
		sel.removeAllRanges()
		sel.addRange(range)
	}
	gdecide_select_all = true
	return true
}

// Visible option rows only (respect type-filter); all columns including scrolled-off.
function decide_list_tsv() {
	var table = document.getElementById('decide_table1')
	if (!table)
		return ''
	var lines = []
	var ths = table.querySelectorAll('thead th')
	if (ths.length) {
		var h = []
		for (var i = 0; i < ths.length; i++)
			h.push(String(ths[i].textContent || '').replace(/\t/g, ' ').replace(/\r?\n/g, ' '))
		lines.push(h.join('\t'))
	}
	var trs = table.querySelectorAll('tbody tr[decide_row]')
	for (var r = 0; r < trs.length; r++) {
		var tr = trs[r]
		// Type-filter uses display:none on non-matches
		if (tr.style && tr.style.display == 'none')
			continue
		var tds = tr.cells
		var cells = []
		for (var c = 0; c < tds.length; c++)
			cells.push(String(tds[c].textContent || '').replace(/\t/g, ' ').replace(/\r?\n/g, ' '))
		lines.push(cells.join('\t'))
	}
	return lines.join('\n')
}

function decide_copy_text_sync(event, text) {
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
	if (typeof exoduscancelevent == 'function')
		return exoduscancelevent(event) || true
	if (event.preventDefault)
		event.preventDefault()
	return true
}

// Used by dbform document_oncopy and decide capture: null = not us; true = browser; false = handled.
function decide_oncopy(event) {
	if (!decide_is_open())
		return null
	if (gdecide_select_all) {
		if (decide_copy_text_sync(event, decide_list_tsv()))
			return false
	}
	if (decide_selection_in_list())
		return true
	return null
}

function decide_contextmenu_watch(event) {
	if (!decide_is_open())
		return
	event = getevent(event)
	var t = event.target
	var conf = document.getElementById('exodusconfirmdiv')
	if (!conf || !t || !(conf.contains(t)))
		return
	gdecide_ctx_menu = true
	if (gdecide_ctx_timer)
		window.clearTimeout(gdecide_ctx_timer)
	gdecide_ctx_timer = window.setTimeout(function () {
		gdecide_ctx_menu = false
		gdecide_ctx_timer = null
	}, 5000)
}

function decide_on_selectionchange() {
	if (!gdecide_ctx_menu || !decide_is_open())
		return
	var sel = window.getSelection && window.getSelection()
	if (!sel || !sel.rangeCount || sel.isCollapsed)
		return
	if (decide_selection_in_list())
		return
	decide_select_all_list()
}

;(function decide_copy_select_install() {
	if (typeof document == 'undefined' || !document.addEventListener)
		return
	document.addEventListener('contextmenu', decide_contextmenu_watch, true)
	document.addEventListener('selectionchange', decide_on_selectionchange)
	document.addEventListener('copy', function (event) {
		var r = decide_oncopy(event)
		// false = we put TSV on clipboard (preventDefault already done)
		// true/null = leave alone (browser or other handlers)
		if (r === false && event.stopPropagation)
			event.stopPropagation()
	}, true)
})()

async function decide_onload(decide_args) {

	//var question = decide_args[0]
	var data = decide_args[1]
	var cols = decide_args[2]
	var decide_returncolid = decide_args[3]
	var defaultreply = decide_args[4]//must be an array with method exoduslocate
	var decide_returnmany = decide_args[5]
	var decide_inverted = decide_args[6]

	//data
	//[[,,,],[,,,],[,,,]] or
	//col1.1 vm col1.2 fm col2.1 vm col2.2 etc
	if (!data && !cols)
		data = [['Yes'], ['No']]
	if (!data)
		return await decide_fail_no_options()
	if (typeof data == 'string') {
		data = data.split(data.indexOf(fm) + 1 ? fm : ':')
	}
	for (ii = 0; ii < data.length; ii++) {
		if (typeof data[ii] == 'string') {
			data[ii] = data[ii].split(data[ii].indexOf(vm) + 1 ? vm : ';')
		}
		else if (typeof data[ii] == 'number')
			data[ii] = [data[ii]]
	}

	//quit if no data (drop decide shell first — it is already in the DOM)
	if (!data.length)
		return await decide_fail_no_options()

	//columns
	//[[dictid,title],etc. or
	//colid vm coltitle fm ... etc one per column
	if (!cols) {
		cols = []
		// Empty first row → no options (not "column N missing")
		if (!data[0] || !data[0].length)
			return await decide_fail_no_options()
		for (var ii = 0; ii < data[0].length; ii++)
			cols[ii] = [ii, '']
	}
	if (typeof cols == 'string') {
		if (cols.indexOf(fm) < 0 && cols.indexOf(vm) < 0) {
			cols = cols.replace(/:/g, fm)
			cols = cols.replace(/;/g, vm)
		}
		cols = cols.split(fm)
	}
	for (ii = 0; ii < cols.length; ii++) {
		if (typeof cols[ii] == 'string')
			cols[ii] = cols[ii].split(vm)
	}
	var ncols = cols.length
	if (!ncols)
		return await decide_fail_no_options()

	// Zero options before column checks — empty multi-hit / pending vehicles used to
	// fall through to "popup column N not in popup data" (e.g. inverted cols with
	// length-0 arrays, or row data with cols but no real options).
	var noptions
	if (decide_inverted) {
		var firstcolid = cols[0][0]
		noptions = (data[firstcolid] && data[firstcolid].length) ? data[firstcolid].length : 0
	} else {
		noptions = data.length
	}
	if (!noptions)
		return await decide_fail_no_options()

	//decide_returncolid
	if (typeof decide_returncolid == 'undefined')
		decide_returncolid = ''

	//solve ptcy pop account list without ledger present
	//convert numerical column number to colid
	if (typeof decide_returncolid == 'number') {
		var tt = cols[decide_returncolid]
		//dont convert if column ids are numeric since presumably the returncolid is an id not a columnno
		if (tt && tt[0] && !exodusnum(tt[0]))
			decide_returncolid = tt[0]
	}

	//defaultreply

	//returnmany
	//returnmany=

	//build the table

	//build the column headings row
	var firstselection

	//get a handle on the existing heading row
	var oRow = $$('decide_table1head1row1')

	//add two columns for the check box or radio button column to the column heading
	var oCell = document.createElement('th')
	oCell.colSpan = 2
	oCell.align = 'center'
	if (decide_returnmany)
		var tt = '<button'
			+ ' title="Alt+A: select/deselect all visible (filtered) rows"'
			//+ ' onclick="decide_all_onclick_sync()"'
			+ ' style="font-size:80%" class="exodusbutton"'
			+ '><u>A</u>ll</button>'
	else
		var tt = '&nbsp;'
	oCell.innerHTML = tt
	oRow.appendChild(oCell)

	if (decide_returnmany)
		oCell.getElementsByTagName('button')[0].onclick = decide_all_onclick_sync

	//add a column to show the order of selections
	if (decide_returnmany) {
		var oCell = document.createElement('th')
		oCell.align = 'center'
		if (decide_returnmany)
			oRow.appendChild(oCell)
	}

	//add other column headings
	for (ii = 0; ii < ncols; ii++) {
		var oCell = document.createElement('th')
		oCell.style.fontWeight = 'bold'
		var title = cols[ii][1]
		if (typeof title == 'undefined') {
			title = cols[ii][0]
			if (!title)
				title = ''
			if (parseInt(title))
				title = ''
			title = title.exoduscapitalise().replace(/_/gi, ' ')
		}
		oCell.innerHTML = title
		oCell.className = 'decide_datacol'
		// Sort only when the column has a visible title (untitled data cols: no icon, no sort)
		if (String(title || '').replace(/^\s+|\s+$/g, '')
			&& typeof exodus_create_icon_element == 'function'
			&& typeof exodus_sortimage == 'function') {
			var sorticon = exodus_create_icon_element(
				(typeof gsortimage != 'undefined' && gsortimage) ? gsortimage : exodus_sortimage())
			if (typeof exodus_apply_sort_icon == 'function')
				sorticon = exodus_apply_sort_icon(sorticon, '') || sorticon
			sorticon.id = 'sortbutton_decide_' + ii
			sorticon.title = 'Sort by ' + String(title)
			oCell.appendChild(sorticon)
		}
		oRow.appendChild(oCell)

		if (!cols[ii][3] && cols[ii][2] && (cols[ii][2] == 'DATE' || cols[ii][2] == 'TIME'))
			cols[ii][3] = 'right'

		//check colid is in data
		//if ((!inverted&&typeof (data[0][cols[ii][0]])=='undefined')||(inverted&&typeof (data[cols[ii][0]][0])=='undefined'))
		if ((!decide_inverted && typeof (data[0][cols[ii][0]]) == 'undefined') || (decide_inverted && typeof (data[cols[ii][0]]) == 'undefined')) {
			//  alert(cols[ii])
			alert('popup column "' + cols[ii][0] + '" not in popup data in decide_onload')
			return false
		}

	}

	//column heading row built

	//build the table rows
	var starttime = new Date
	var maxsecs = 10

	var table1body1x = $$('decide_table1body1')

	var optionno = 0

	//rows
	var nrows = decide_inverted ? data[cols[0][0]].length : data.length
	for (rown = 0; rown < nrows; rown++) {

		//break if too many rows
		if ((new Date() - starttime) > (maxsecs * 1000)) {
			if (!(confirm('A large popup window is taking time to prepare.\r\rClick [OK] to wait or [Cancel] to see part.')))
				break
			starttime = new Date
		}

		if (decide_returncolid === '') {
			returnvalue = rown + 1
		}
		else {
			returnvalue = decide_inverted ? data[decide_returncolid][rown] : data[rown][decide_returncolid]
			if (typeof returnvalue == 'object')
				returnvalue = returnvalue.text
		}

		//hide checkboxes on rows with nothing to return
		var visibilityhidden = ''
		if (decide_returncolid) {
			if (decide_returnmany)
				//visibilityhidden = returnvalue ? '' : 'onclick="decide_checkbox_select()"'
				visibilityhidden = ''
			else
				visibilityhidden = returnvalue ? '' : 'style="visibility:hidden"'
		}

		//create a new row
		var oRow = document.createElement('tr')
		oRow.setAttribute('decide_row', rown)

		table1body1x.appendChild(oRow)

		//add line number column
		var oCell = document.createElement('td')
		oRow.appendChild(oCell)
		//if (decide_returnmany)

		oCell.id = 'decide_optionno'
		oCell.name = 'decide_optionno'
		oCell.setAttribute('name', 'decide_optionno')
		if (visibilityhidden) {
			if (decide_returnmany)
				optionno++
			oCell.style.visibility = 'hidden'
		}
		else {
			optionno++
			oCell.setAttribute('decide_optionno', optionno)
			oCell.innerHTML = optionno + "."
		}

		//add a check box or radio button column
		var oCellx = document.createElement('td')
		oRow.appendChild(oCellx)

		var event_handler = undefined
		if (decide_returnmany) {
			oCellx.innerHTML = '<input ' + visibilityhidden + ' id="decide_selection" name="decide_selection" type="checkbox" />'
			event_handler = decide_checkbox_select
		}
		else {
			var checked = ''
			if (!firstselection) {
				if (!visibilityhidden) {
					checked = 'checked=true'
					firstselection = oCellx
				}
			}
			//oCellx.innerHTML = '<input ' + checked + ' ' + visibilityhidden + ' id=decide_selection name=decide_selection type=radio onmousedown="decide_radio_onmousedown_sync(this)" />'
			oCellx.innerHTML = '<input ' + checked + ' ' + visibilityhidden + ' id="decide_selection" name="decide_selection" type="radio" />'
			event_handler = decide_radio_select
		}

		var oCellxInput = oCellx.getElementsByTagName('input')[0]
		if (event_handler) {
			// Checkbox: mouseup so row format/check state updates on release, not press.
			// Radio: mousedown for immediate select+OK.
			if (decide_returnmany)
				oCellxInput.onmouseup = event_handler
			else
				oCellxInput.onmousedown = event_handler
		}
		oCellxInput.setAttribute('decide_optionno', optionno)

		if (defaultreply && defaultreply.exoduslocate(returnvalue))
			oCellx.firstChild.checked = true

		oCellx.firstChild.setAttribute('decide_returnvalue', returnvalue)

		var singlereturnvalue
		if (returnvalue) {
			if (typeof singlereturnvalue == 'undefined')
				singlereturnvalue = returnvalue
			else
				singlereturnvalue = false
		}

		//add a column to show the order of selections
		if (decide_returnmany) {
			var oCell = document.createElement('td')
			oCell.setAttribute('name', 'decide_rank')
			oCell.align = 'center'

			//set defaultreply's row, to rank 1 (GB)
			if (typeof defaultreply[0] == 'number') {
				if (rown == defaultreply[0] - 1)
					oCell.innerHTML = 1
			}

			oRow.appendChild(oCell)
		}

		//add the data columns for this row
		for (coln = 0; coln < ncols; coln++) {
			var oCell = document.createElement('td')
			oRow.appendChild(oCell)

			var colinfo = cols[coln]

			if (decide_inverted) {
				value = data[colinfo[0]][rown]
			}
			else {
				value = data[rown][colinfo[0]]
			}
			if (typeof value == 'undefined')
				value = ''
			else if (typeof value == 'object')
				value = value.text

			//date conversion
			if (colinfo[2] == 'DATE' && exodusnum(value))
				value = DATE(value)

			//time conversion
			if (colinfo[2] == 'TIME' && exodusnum(value))
				value = TIME(value)

			//oCell.innerHTML=value
			//use innerText otherwise things like <> in the data do not show
			//oCell.innerText = value
			//oCell.innerHTML = value.exodusconvert(vm+sm+tm,'   ')
			value = value.replace(ALLFMre, '<br/>')
			oCell.innerHTML = value

			//align
			if (colinfo[3])
				oCell.align = colinfo[3]
			else
				oCell.align = 'left'
			oCell.className = 'decide_datacol'

			if (defaultreply) {
				if (typeof decide_returncolid === '') {
					if (defaultreply.exoduslocate(rown + 1))
						oCellx.firstChild.checked = true
				}
				else {
					//returncolid was being treated as onscreen col preventing proper defaulting
					//if (value && coln === returncolid) {
					if (value && (colinfo[0] == decide_returncolid || Number(colinfo[0]) == decide_returncolid)) {
						if (defaultreply.exoduslocate(value))
							oCellx.firstChild.checked = true
					}
				}
			}

		}

		//table rows built

		//table built

	}

	// Footer actions: only close the popup (popup-local; form path is blocked).
	var okbutton = $$('decide_okbutton')
	okbutton.onclick = function (event) {
		exoduscancelevent(getevent(event))
		decide_close(decide_getreturnvalues())
		return false
	}
	var cancelbutton = $$('decide_cancelbutton')
	cancelbutton.onclick = function (event) {
		exoduscancelevent(getevent(event))
		decide_close('')
		return false
	}

	//autoselect only one option
	if (singlereturnvalue) {
		if (decide_returnmany)
			singlereturnvalue = [singlereturnvalue]
		return singlereturnvalue
	} else if (typeof singlereturnvalue == 'undefined') {
		return await decide_fail_no_options()
	}

	//exodussettimeout('exodusautofitwindow()', 10)

	var selections = document.getElementsByName('decide_selection')

	// Entry check/rank snapshot — F8/Del restore (not pure clear-all)
	var decide_entry_checked = []
	var decide_entry_ranks = []
	if (decide_returnmany) {
		var ranks0 = document.getElementsByName('decide_rank')
		for (var si0 = 0; si0 < selections.length; si0++) {
			decide_entry_checked[si0] = !!selections[si0].checked
			decide_entry_ranks[si0] = ranks0[si0] ? String(ranks0[si0].innerText || '') : ''
		}
	}

	//focus on the first checked item or the first rown
	// Tab cycles options (as one stop) -> Select -> Cancel -> same option (see decide_document_onkeydown)
	var decide_last_option_element = null
	// Must init before return — handlers below are hoisted but var assignment after return never runs
	var decide_filter_text = ''
	for (var ii = 0; ii < selections.length; ++ii)
		if (selections[ii].checked)
			break
	if (ii >= selections.length)
		ii = 0
	//selections[ii].focus()
	decide_last_option_element = selections[ii]
	client_focuson(selections[ii])

	// History entry so Back/popstate dismisses the popup (not navigate away) — mobile has no Esc.
	// See cancel_backpage_event. Keep search+hash: pathname alone drops ?MODE=… etc.
	if (typeof ghistorypushed == 'undefined') {
		ghistorypushed = true
		history.pushState(null, null, window.location.pathname + window.location.search + window.location.hash);
	}

	addeventlistener(exodusconfirmdiv, 'keydown', decide_document_onkeydown)
	addeventlistener(exodusconfirmdiv, 'keyup', decide_document_onkeyup)
	addeventlistener(exodusconfirmdiv, 'click', decide_document_onclick)
	addeventlistener(exodusconfirmdiv, 'dblclick', decide_document_ondblclick)
	addeventlistener(exodusconfirmdiv, 'mouseover', decide_document_onmouseover)
	addeventlistener(exodusconfirmdiv, 'mouseout', decide_document_onmouseout)
	// Real pointer move only — not scroll-under-cursor (see decide_hover_locked)
	addeventlistener(exodusconfirmdiv, 'mousemove', decide_document_onmousemove)

	// Wheel over the option table: step like Up/Down (modalblock_onwheel calls this).
	gdecide_onwheel = function decide_onwheel(event) {
		if (!event || event.ctrlKey || event.metaKey)
			return false
		var table = $$('decide_table1')
		if (!table)
			return false
		var t = event.target
		if (!(t === table || (table.contains && table.contains(t))))
			return false
		var delta = event.deltaY
		if (!delta && event.detail)
			delta = event.detail
		if (!delta)
			return false
		var direction = delta > 0 ? 1 : -1
		// Radio: check + focus; multi-select: focus only. No wrap (arrows wrap).
		return decide_move_option(direction, 1, !decide_returnmany, false)
	}

	//returning undefined indicates that we need to yield and wait for decide_ok_onclick_sync etc to resume
	//returning false indicates some problem
	//returning anything else indicates that there is only one option
	return undefined

	//remainder of functions is event handlers

	// Alt+A / All: toggle only visible (type-filtered) rows; leave hidden alone.
	function decide_all_onclick_sync(event) {

		selections = document.getElementsByName('decide_selection')
		var ranks = document.getElementsByName('decide_rank')
		var ii

		// If every visible row is checked → uncheck them; else check all visible
		var allVisibleChecked = true
		var anyVisible = false
		for (ii = 0; ii < selections.length; ii++) {
			if (decide_selection_row_hidden(selections[ii]))
				continue
			anyVisible = true
			if (!selections[ii].checked) {
				allVisibleChecked = false
				break
			}
		}
		if (!anyVisible)
			return
		var checking = !allVisibleChecked

		if (!checking) {
			for (ii = 0; ii < selections.length; ii++) {
				if (decide_selection_row_hidden(selections[ii]))
					continue
				if (selections[ii].checked)
					decide_checkbox_select(event, selections[ii], false, ii)
			}
			return
		}

		// Append ranks after any already-selected (including filtered-out) rows
		var lastrank = 0
		for (ii = 0; ii < ranks.length; ++ii) {
			var rank = Number(ranks[ii].innerText)
			if (rank > lastrank)
				lastrank = rank
		}
		for (ii = 0; ii < selections.length; ii++) {
			if (decide_selection_row_hidden(selections[ii]))
				continue
			if (!selections[ii].checked)
				lastrank = decide_checkbox_select(event, selections[ii], true, ii, lastrank)
		}
	}

	// F8/Del: restore checked+rank as when the popup opened (not wipe to empty).
	function decide_restore_entry_state() {
		var sels = document.getElementsByName('decide_selection')
		var ranks = document.getElementsByName('decide_rank')
		for (var ii = 0; ii < sels.length; ii++) {
			var want = !!decide_entry_checked[ii]
			sels[ii].checked = want
			var rowtag = getancestor(sels[ii], 'tr')
			if (rowtag)
				rowtag.style.fontWeight = want ? 'bold' : 'normal'
			if (ranks[ii])
				ranks[ii].innerText = decide_entry_ranks[ii] || ''
		}
	}

	function decide_checkbox_selectone(element, checking, checkrown, ranks, lastrank) {

		element.checked = checking

		var rowtag = getancestor(element, 'tr')
		if (rowtag)
			rowtag.style.fontWeight = checking ? 'bold' : 'normal'

		if (checking) {
			lastrank++
			ranks[checkrown].innerText = lastrank
		} else
			ranks[checkrown].innerText = ''

		return lastrank
	}

	function decide_checkbox_select(event, element, checking, checkrown, lastrank) {

		event = getevent(event)
		if (!element)
			element = event.target

		// dont handle bubbled click on checkbox; mouseup on the input already did the job
		if (event.type == 'click' && event.target.type == 'checkbox')
			return false

		var doingall = true
		if (typeof checkrown == 'undefined') {
			if (typeof checking == 'undefined')
				checking = !element.checked
			doingall = false
		}

		if (typeof checking == 'undefined')
			checking = !element.checked

		//find current rown and level
		if (typeof checkrown == 'undefined') {
			var indentregex = /^[\. -]*/
			var selection2 = document.getElementsByName('decide_selection')
			var level
			for (ii = 0; ii < selection2.length; ii++) {
				if (selection2[ii] == element) {
					checkrown = ii
					level = selection2[ii].parentNode.nextSibling.nextSibling.innerText.match(indentregex)
					break
				}
			}
		}

		var ranks = document.getElementsByName('decide_rank')

		//checking=determine lastrank unless provided
		if (checking) {
			//count selections
			if (!doingall) {
				lastrank = 0
				var checkupto = doingall ? checkrown : ranks.length
				for (var ii = 0; ii < checkupto; ++ii) {
					var rank = Number(ranks[ii].innerText)
					if (rank)
						lastrank++
				}
			}
		}

		//unchecking=reduce higher ranks
		else {
			var removerank = Number(ranks[checkrown].innerText)
			for (var ii = 0; ii < ranks.length; ++ii) {
				var rank = Number(ranks[ii].innerText)
				if (rank > removerank)
					ranks[ii].innerText = rank - 1
			}
		}

		lastrank = decide_checkbox_selectone(element, checking, checkrown, ranks, lastrank)

		//check/uncheck any following lower level rows
		if (!doingall) {
			for (ii = checkrown + 1; ii < selection2.length; ii++) {
				//quit once get back to the same level
				if (selection2[ii].parentNode.nextSibling.nextSibling.innerText.match(indentregex) <= level)
					break
				lastrank = decide_checkbox_selectone(selection2[ii], checking, ii, ranks, lastrank)
				//lastrank=decide_checkbox_select(event,selection2[ii],checking,ii,lastrank)
			}
		}

		//necessary to prevent unchecking by something unknown higher up
		//event.cancelBubble=true
		exoduscancelevent(event)

		if (doingall)
			return lastrank
		else
			return false
	}

	function decide_radio_select(event, element) {
		event = getevent(event)
		if (!element)
			element = event.target
		element.checked = true
		decide_last_option_element = element
		decide_close(element.getAttribute('decide_returnvalue'))
		return exoduscancelevent(event)
	}

	function decide_document_onmouseover(event) {
		return decide_document_onmouse(event, 'over')
	}

	function decide_document_onmouseout(event) {
		return decide_document_onmouse(event, 'out')
	}

	// After keyboard/wheel row move, ignore mouseover/out until a real mousemove
	// (scroll-under-cursor would otherwise steal or strip the highlight).
	var decide_hover_locked = false

	function decide_document_onmousemove() {
		decide_hover_locked = false
	}

	// One row highlight: mouseover steals after unlock; arrows/wheel set + lock.
	function decide_set_row_hover(tr, fromkeys) {
		var tbody = $$('decide_table1body1')
		if (tbody) {
			var hovered = tbody.querySelectorAll('tr.decide_row_hover')
			for (var hi = 0; hi < hovered.length; hi++)
				hovered[hi].classList.remove('decide_row_hover')
		}
		if (tr)
			tr.classList.add('decide_row_hover')
		if (fromkeys)
			decide_hover_locked = true
	}

	function decide_document_onmouse(event, mode) {

		event = getevent(event)
		exoduscancelevent(event)

		if (decide_hover_locked)
			return

		var trtag = getancestor(event.target, 'tr')
		if (!trtag)
			return false
		if (!trtag.getAttribute('decide_row'))
			return false

		var element = trtag.getElementsByTagName('input')[0]
		if (!element)
			return

		// Highlight row in popup when hovering over it (steals keyboard highlight)
		if (mode == 'over')
			decide_set_row_hover(trtag)
		else
			trtag.classList.remove('decide_row_hover')

		return
	}

	// Option-row click only (tbody rows carry decide_row). Footer uses its own handlers.
	function decide_document_onclick(event, forceCheck) {
		event = getevent(event)
		// Drag-selected list text → leave for Ctrl+C / right-click Copy
		if (decide_selection_in_list())
			return
		if (event.target && event.target.type == 'checkbox' && typeof forceCheck !== 'boolean')
			return exoduscancelevent(event)

		var trtag = getancestor(event.target, 'tr')
		if (!trtag || trtag.getAttribute('decide_row') == null)
			return

		var element = trtag.getElementsByTagName('input')[0]
		if (!element)
			return

		gdecide_select_all = false
		if (decide_returnmany) {
			if (typeof forceCheck === 'boolean')
				decide_checkbox_select(event, element, forceCheck)
			else
				decide_checkbox_select(event, element)
		}
		else
			decide_radio_select(event, element)

		decide_last_option_element = element
		client_focuson(element)
		return exoduscancelevent(event)
	}

	function decide_document_ondblclick(event) {
		decide_document_onclick(event, true)
		decide_close(decide_getreturnvalues())
		return exoduscancelevent(event)
	}

	function decide_getreturnvalues() {

		//NB returns row numbers 1 based not 0 based!
		//0 means cancelled

		var returnvalues = []
		var selection2 = document.getElementsByName('decide_selection')
		var ranks = document.getElementsByName('decide_rank')
		for (ii = 0; ii < selection2.length; ii++) {
			if (selection2[ii].checked) {
				var returnvalue = selection2[ii].getAttribute('decide_returnvalue')
				if (!decide_returnmany) {
					returnvalues = returnvalue
					break
				}
				if (returnvalue) {
					var rank = Number(ranks[ii].innerText) - 1
					returnvalues[rank] = returnvalue
				}
			}
		}

		// Nothing checked: last focused / first visible option
		if (!decide_returnmany && (returnvalues === '' || returnvalues == null
			|| (typeof returnvalues == 'object' && !returnvalues.length))) {
			var fallback = decide_last_option_element
			if (!fallback || decide_selection_row_hidden(fallback)) {
				fallback = null
				for (ii = 0; ii < selection2.length; ii++) {
					if (!decide_selection_row_hidden(selection2[ii])) {
						fallback = selection2[ii]
						break
					}
				}
			}
			if (fallback) {
				fallback.checked = true
				returnvalues = fallback.getAttribute('decide_returnvalue')
			}
		}

		if (returnvalues == null || returnvalues === ''
			|| (typeof returnvalues == 'object' && !returnvalues.length))
			returnvalues = ''
		else if (typeof returnvalues == 'object' && returnvalues.exodustrim)
			returnvalues = returnvalues.exodustrim('')

		return returnvalues
	}

	// Only exit from the decide popup
	function decide_close(value) {
		gdecide_onwheel = null
		gdecide_select_all = false
		gdecide_ctx_menu = false
		if (gdecide_ctx_timer) {
			window.clearTimeout(gdecide_ctx_timer)
			gdecide_ctx_timer = null
		}
		resolvePendingConfirm(value, 'decide_close')
	}

	// direction ±1, steps (1 or 10), selectRadio: check for single-select (arrows).
	// wrap: true = Up/Down cycle ends (default); false = wheel/PgUp/PgDn stop at ends.
	function decide_move_option(direction, steps, selectRadio, wrap) {
		var selections = document.getElementsByName('decide_selection')
		if (!selections || !selections.length)
			return false
		if (typeof wrap == 'undefined' || wrap == null)
			wrap = true

		// Origin: highlighted row first (mouse hover or prior key steal), else focus/check.
		// So Up/Down from a hover on row 5 moves from 5, not from the checked radio on row 1.
		var element = null
		var hovertr0 = document.querySelector('#decide_table1body1 tr.decide_row_hover')
		if (hovertr0) {
			var hoverinp = hovertr0.getElementsByTagName('input')[0]
			if (hoverinp && hoverinp.name == 'decide_selection' && !decide_selection_row_hidden(hoverinp))
				element = hoverinp
		}
		if (!element) {
			var active = document.activeElement
			if (active && active.name == 'decide_selection' && !decide_selection_row_hidden(active))
				element = active
			else if (decide_last_option_element
				&& decide_last_option_element.name == 'decide_selection'
				&& !decide_selection_row_hidden(decide_last_option_element))
				element = decide_last_option_element
			else {
				for (var ci = 0; ci < selections.length; ci++) {
					if (selections[ci].checked && !decide_selection_row_hidden(selections[ci])) {
						element = selections[ci]
						break
					}
				}
				if (!element) {
					for (var vi = 0; vi < selections.length; vi++) {
						if (!decide_selection_row_hidden(selections[vi])) {
							element = selections[vi]
							break
						}
					}
				}
			}
		}
		if (!element)
			return false

		var n = -1
		for (var si = 0; si < selections.length; si++) {
			if (selections[si] == element) {
				n = si
				break
			}
		}
		if (n < 0)
			return false

		var startn = n
		var stepped = 0
		var guard = 0
		var lastVisible = null
		var newelement = element
		while (guard++ <= selections.length) {
			n += direction
			if (n < 0) {
				if (!wrap)
					break
				n = selections.length - 1
			}
			if (n >= selections.length) {
				if (!wrap)
					break
				n = 0
			}
			if (n == startn && stepped == 0 && guard > 1)
				break
			newelement = selections[n]
			if (!newelement || decide_selection_row_hidden(newelement))
				continue
			if (newelement == element && stepped == 0)
				continue
			stepped++
			lastVisible = newelement
			if (stepped >= steps)
				break
		}
		if (!stepped || !lastVisible)
			return false
		newelement = lastVisible

		var newoptionno = newelement.getAttribute('decide_optionno')
		var scrollpane = exodusconfirm_scrollpane()
		var newn = -1
		for (si = 0; si < selections.length; ++si) {
			if (selections[si] == newelement) {
				newn = si
				break
			}
		}
		if (scrollpane) {
			if (newn == 0 || newoptionno == 1)
				scrollpane.scrollTop = 0
			else if (newn == (selections.length - 1))
				scrollpane.scrollTop = scrollpane.scrollHeight
			else {
				try {
					var tr = getancestor(newelement, 'tr')
					if (tr)
						exodus_scroll_row_below_sticky_thead(tr, scrollpane)
				} catch (e) { }
			}
		}

		decide_last_option_element = newelement
		try {
			newelement.focus()
			if (newelement.select)
				newelement.select()
		} catch (e2) {
			client_focuson(newelement)
		}

		// Arrows/wheel steal highlight + lock until real mousemove
		var hovertr = getancestor(newelement, 'tr')
		if (hovertr)
			decide_set_row_hover(hovertr, true)

		// Single-select: move the radio with focus (arrow/wheel). Multi: focus only.
		if (selectRadio && !decide_returnmany)
			newelement.checked = true

		return true
	}

	function decide_ok_onclick_sync() {
		decide_close(decide_getreturnvalues())
	}

	function decide_cancel_onclick_sync() {
		decide_close('')
	}

	//purely to suppress any automatic checkbox ticking by the browser
	//so we can control it in onkeydown
	function decide_document_onkeyup(event) {
		return exoduscancelevent(event)
	}
	function decide_document_onmouseup(event) {
		return exoduscancelevent(event)
	}

	function decide_focus_option_endpoint(first) {

		var selection2=document.getElementsByName('decide_selection')
		if (!selection2.length)
			return false
		var newelement
		var idx
		if (first) {
			for (idx=0;idx<selection2.length;++idx) {
				if (!decide_selection_row_hidden(selection2[idx])) {
					newelement=selection2[idx]
					break
				}
			}
		}
		else {
			for (idx=selection2.length-1;idx>=0;--idx) {
				if (!decide_selection_row_hidden(selection2[idx])) {
					newelement=selection2[idx]
					break
				}
			}
		}
		if (!newelement)
			return false
		idx=Array.prototype.indexOf.call(selection2,newelement)
		var scrollpane=exodusconfirm_scrollpane()
		if (scrollpane) {
			if (idx==0||newelement.getAttribute('decide_optionno')==1)
				scrollpane.scrollTop=0
			else if (idx==selection2.length-1)
				scrollpane.scrollTop=scrollpane.scrollHeight
			else {
				var trEnd = getancestor(newelement, 'tr')
				if (trEnd)
					exodus_scroll_row_below_sticky_thead(trEnd, scrollpane)
			}
		}
		decide_last_option_element = newelement
		newelement.focus()
		newelement.select()
		// Home/End: same highlight steal + lock as arrows
		var hovertr = getancestor(newelement, 'tr')
		if (hovertr)
			decide_set_row_hover(hovertr, true)
		return true
	}

	// Return focus to the option the user last worked on (or checked / first).
	// Options act as one Tab stop; arrows still move within the list.
	function decide_focus_return_option() {

		var selection2 = document.getElementsByName('decide_selection')
		if (!selection2.length)
			return false

		var el = decide_last_option_element
		if (!(el && el.name == 'decide_selection' && !decide_selection_row_hidden(el)
			&& document.body.contains(el))) {
			el = null
			var i
			for (i = 0; i < selection2.length; ++i) {
				if (selection2[i].checked && !decide_selection_row_hidden(selection2[i])) {
					el = selection2[i]
					break
				}
			}
			if (!el) {
				for (i = 0; i < selection2.length; ++i) {
					if (!decide_selection_row_hidden(selection2[i])) {
						el = selection2[i]
						break
					}
				}
			}
		}
		if (!el)
			return false

		decide_last_option_element = el
		client_focuson(el)
		return true
	}

	function decide_all_button() {
		var headrow = $$('decide_table1head1row1')
		if (!headrow)
			return null
		var buttons = headrow.getElementsByTagName('button')
		return buttons.length ? buttons[0] : null
	}

	// Type-to-filter: decide_filter_text is declared above (before return undefined).

	// True if option is not pickable / not shown (visibility or type-filter display:none).
	function decide_selection_row_hidden(sel) {
		if (!sel || sel.style.visibility == 'hidden')
			return true
		var row = typeof getancestor == 'function' ? getancestor(sel, 'TR') : null
		return !!(row && row.style && row.style.display == 'none')
	}

	// Hide non-matches with display:none only — keep original option numbers.
	function decide_apply_filter() {
		var filter = (decide_filter_text || '').toLowerCase()
		var tbody = $$('decide_table1body1')
		if (!tbody)
			return
		var rows = tbody.getElementsByTagName('tr')
		var vis = 0
		var firstVis = null
		var i
		for (i = 0; i < rows.length; ++i) {
			var row = rows[i]
			var sel = null
			var inputs = row.getElementsByTagName('input')
			for (var ii = 0; ii < inputs.length; ++ii) {
				if (inputs[ii].name == 'decide_selection') {
					sel = inputs[ii]
					break
				}
			}
			// Permanently non-returnable rows (hidden radio) stay out of the list
			if (sel && sel.style.visibility == 'hidden') {
				row.style.display = filter ? 'none' : ''
				continue
			}
			var match = !filter
				|| (row.innerText || row.textContent || '').toLowerCase().indexOf(filter) >= 0
			row.style.display = match ? '' : 'none'
			if (!match || !sel)
				continue
			vis++
			if (!firstVis)
				firstVis = sel
		}
		var st = $$('decide_filter_status')
		if (st) {
			if (filter) {
				st.style.display = ''
				st.textContent = 'Filter: ' + decide_filter_text
					+ (vis ? ' — ' + vis + ' match' + (vis == 1 ? '' : 'es') : ' — no matches')
				st.className = 'decide_filter_status '
					+ (vis ? 'decide_filter_ok' : 'decide_filter_empty')
			} else {
				st.style.display = 'none'
				st.textContent = ''
				st.className = 'decide_filter_status'
			}
		}
		if (firstVis) {
			var cur = document.activeElement
			var needFocus = !cur || cur.name != 'decide_selection' || decide_selection_row_hidden(cur)
			if (needFocus) {
				decide_last_option_element = firstVis
				try {
					client_focuson(firstVis)
				} catch (e) { }
				if (!decide_returnmany)
					firstVis.checked = true
			}
		} else if (filter) {
			// No matches: park on Select so keydown still reaches the dialog
			var okb = $$('decide_okbutton')
			if (okb)
				try { client_focuson(okb) } catch (e) { }
		}
		// Refit + refresh ▼ (fit calls update_scroll_hints)
		exodusconfirm_fit_decide_popup(true)
	}

	function decide_document_onkeydown(event) {

		event = getevent(event)

		if (typeof decide_returnmany == 'undefined')
			decide_returnmany = ''

		var keycode = event.keyCode

		//console.log('decide_document_onkeydown ' + keycode)

		// Ctrl/Cmd+A: select whole decide list (incl. scrolled), not the page
		if ((event.ctrlKey || event.metaKey) && !event.altKey && keycode == 65) {
			if (decide_select_all_list())
				return exoduscancelevent(event)
		}

		// Tab: list (one stop) -> Select -> Cancel -> list (Shift reverses).
		// Multi-select also includes the All button before the list.
		// Arrows still move among options; Tab leaves the list like a radio group.
		if (keycode == 9) {
			var okb = $$('decide_okbutton')
			var canb = $$('decide_cancelbutton')
			var allb = decide_all_button()
			var target = event.target
			if (target && target.name == 'decide_selection')
				decide_last_option_element = target

			var stops = []
			if (allb)
				stops.push(allb)
			stops.push('option')
			if (okb)
				stops.push(okb)
			if (canb)
				stops.push(canb)

			var current = -1
			if (allb && (target === allb || allb.contains(target)))
				current = stops.indexOf(allb)
			else if (target && target.name == 'decide_selection')
				current = stops.indexOf('option')
			else if (okb && (target === okb || okb.contains(target)))
				current = stops.indexOf(okb)
			else if (canb && (target === canb || canb.contains(target)))
				current = stops.indexOf(canb)
			else
				// body/header click target etc. — treat as list
				current = stops.indexOf('option')

			var next = event.shiftKey ? current - 1 : current + 1
			if (next < 0)
				next = stops.length - 1
			if (next >= stops.length)
				next = 0

			var dest = stops[next]
			if (dest === 'option')
				decide_focus_return_option()
			else
				client_focuson(dest)

			return exoduscancelevent(event)
		}

		// Arrows on footer buttons (Select / Cancel / All): cycle among those only.
		// Left/Up = previous, Right/Down = next. Options keep using arrows for the list.
		if (keycode == 37 || keycode == 38 || keycode == 39 || keycode == 40) {
			var okbA = $$('decide_okbutton')
			var canbA = $$('decide_cancelbutton')
			var allbA = decide_all_button()
			var targetA = event.target
			var footerBtns = []
			if (allbA)
				footerBtns.push(allbA)
			if (okbA)
				footerBtns.push(okbA)
			if (canbA)
				footerBtns.push(canbA)
			var fi = -1
			for (var fii = 0; fii < footerBtns.length; ++fii) {
				if (targetA === footerBtns[fii]
					|| (footerBtns[fii].contains && footerBtns[fii].contains(targetA))) {
					fi = fii
					break
				}
			}
			if (fi >= 0 && footerBtns.length > 1) {
				var reverseA = (keycode == 37 || keycode == 38)
				var nexti = reverseA
					? (fi - 1 + footerBtns.length) % footerBtns.length
					: (fi + 1) % footerBtns.length
				var destA = footerBtns[nexti]
				try { destA.focus() } catch (e) {}
				client_focuson(destA)
				return exoduscancelevent(event)
			}
			// else fall through: arrows on options / body still move the list
		}

		// Esc: clear type-filter first, then Cancel
		if (keycode == 27) {
			if (decide_filter_text) {
				decide_filter_text = ''
				decide_apply_filter()
				return exoduscancelevent(event)
			}
			decide_cancel_onclick_sync()
			return exoduscancelevent(event)
		}

		// Alt+C = Cancel (bare C is free for type-filter)
		if (keycode == 67 && event.altKey && !event.ctrlKey && !event.metaKey) {
			decide_cancel_onclick_sync()
			return exoduscancelevent(event)
		}

		// F9 / Ctrl+Enter / Alt+S = Select (bare S is free for type-filter)
		if (keycode == 120
			|| (keycode == 13 && event.ctrlKey)
			|| (keycode == 83 && event.altKey && !event.ctrlKey && !event.metaKey)) {
			decide_ok_onclick_sync()
			return exoduscancelevent(event)
		}

		// Alt+A = All (multi-select only; bare A is free for type-filter)
		if (keycode == 65 && event.altKey && !event.ctrlKey && !event.metaKey) {
			if (decide_returnmany)
				decide_all_onclick_sync(event)
			return exoduscancelevent(event)
		}

		// Home/End: first/last option (like Ctrl+PgUp/Ctrl+PgDn focus)
		if (keycode == 36) {
			decide_focus_option_endpoint(true)
			return exoduscancelevent(event)
		}
		if (keycode == 35) {
			decide_focus_option_endpoint(false)
			return exoduscancelevent(event)
		}

		var selections = document.getElementsByName('decide_selection')
		var options = document.getElementsByName('decide_optionno')

		var element = event.target

		// Space/Enter on footer (or All) buttons — do not treat as option shortcuts
		if (keycode == 13 || keycode == 32) {
			var okb2 = $$('decide_okbutton')
			var canb2 = $$('decide_cancelbutton')
			var allb2 = decide_all_button()
			if (okb2 && (element === okb2 || okb2.contains(element))) {
				decide_ok_onclick_sync()
				return exoduscancelevent(event)
			}
			if (canb2 && (element === canb2 || canb2.contains(element))) {
				decide_cancel_onclick_sync()
				return exoduscancelevent(event)
			}
			if (allb2 && (element === allb2 || allb2.contains(element))) {
				decide_all_onclick_sync(event)
				return exoduscancelevent(event)
			}
		}

		//ctrl+enter and f9 is ok ... so is space if not !returnmany
		if (keycode == 120 || (keycode == 13 && event.ctrlKey) || (keycode == 32 && !decide_returnmany)) {
			decide_ok_onclick_sync()
			return exoduscancelevent(event)
		}

		// Backspace: always shorten type-filter first (even when no matches /
		// focus is on filter status). Must run before list navigation.
		if (keycode == 8 && decide_filter_text) {
			decide_filter_text = decide_filter_text.slice(0, -1)
			decide_apply_filter()
			return exoduscancelevent(event)
		}

		// multi Backspace: reverse Enter — up one (wrap) then toggle. Not clear-all (Del/F8).
		if (decide_returnmany && keycode == 8) {
			if (!decide_move_option(-1, 1, false, true))
				return exoduscancelevent(event)
			if (decide_last_option_element)
				decide_checkbox_select(event, decide_last_option_element)
			return exoduscancelevent(event)
		}

		// Type-to-filter: letters always; digits/space/- etc only once filter is active.
		// First character cannot be 1-9 (those select option 1-9 when filter empty).
		// Once no matches (red filter title), block further insert keys —
		// Backspace/Esc above still shorten or clear.
		if (!event.ctrlKey && !event.altKey && !event.metaKey) {
			var ch = ''
			if (event.key && event.key.length == 1)
				ch = event.key
			if (ch) {
				var isDigit = ch >= '0' && ch <= '9'
				var isLetter = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
				var isExtra = ch == ' ' || ch == '-' || ch == '.' || ch == '/' || ch == '_' || ch == '*' || ch == '#'
				if (isLetter || (decide_filter_text && (isDigit || isExtra))) {
					var stEmpty = $$('decide_filter_status')
					if (stEmpty && stEmpty.classList
						&& stEmpty.classList.contains('decide_filter_empty'))
						return exoduscancelevent(event)
					decide_filter_text += ch
					decide_apply_filter()
					return exoduscancelevent(event)
				}
			}
		}

		// digits 1-9 select options 1-9 (only when filter empty — else digits extend filter above)
		if (!decide_filter_text && keycode >= 49 && keycode <= 57 && !event.altKey && !event.ctrlKey) {

			var optionn = keycode - 48
			if (optionn == 0)
				optionn = 10
			for (var rown = 0; rown < options.length; ++rown) {
				if (options[rown].getAttribute('decide_optionno') == optionn
					&& !decide_selection_row_hidden(selections[rown])) {
					if (!decide_returnmany || selections.length == 1) {
						//decide_ok_onclick_sync()
						decide_last_option_element = selections[rown]
						decide_radio_select(event, selections[rown])
						break
					}
					//selections[rown].checked = !selections[rown].checked
					//break;
					var element = selections[rown]
					decide_last_option_element = element
					decide_checkbox_select(event, element)
					client_focuson(element)
					break
				}
			}
			return exoduscancelevent(event)
		}

		// multi: Space toggles checkbox only (stay on row). Enter toggles + move down (Shift+Enter up).
		if (decide_returnmany && (keycode == 32 || keycode == 13)) {
			decide_checkbox_select(event)
			if (keycode == 32)
				return exoduscancelevent(event)
			if (event.shiftKey)
				keycode = 38//fake up
			else
				keycode = 40//fake down
		}

		// Index into selections[] (not table rowIndex — thead shifts rowIndex).
		var n = -1
		var si
		for (si = 0; si < selections.length; ++si) {
			if (selections[si] == element) {
				n = si
				break
			}
		}
		if (n < 0) {
			// event target may be a cell/label in the row
			var tr = element
			while (tr && tr.tagName != 'TR')
				tr = tr.parentNode
			if (tr) {
				var tin = tr.getElementsByTagName('input')
				for (var ti = 0; ti < tin.length; ++ti) {
					if (tin[ti].name == 'decide_selection') {
						for (si = 0; si < selections.length; ++si) {
							if (selections[si] == tin[ti]) {
								n = si
								element = tin[ti]
								break
							}
						}
						break
					}
				}
			}
		}
		if (n < 0)
			return

		//pgup 33/pgdn 34/down 40/up 38 keys (Tab handled above; multi Backspace above)
		// Step by *visible* rows only (display:none filter skips); PgUp/PgDn = 10 visible.
		// Wheel uses the same decide_move_option path (radio check; multi focus only).
		if (keycode == 33 || keycode == 34 || keycode == 40 || keycode == 38) {

			var direction
			if (keycode == 34 || keycode == 40) direction = 1
			if (keycode == 33 || keycode == 38) direction = -1

			var steps = 1
			var toEnd = false
			if (keycode == 34 || keycode == 33) {
				if (event.ctrlKey)
					toEnd = true
				else
					steps = 10
			}

			// Ctrl+PgUp/Dn: first/last visible (not via decide_move_option steps)
			if (toEnd) {
				var lastVisible = null
				if (direction > 0) {
					for (var j = 0; j < selections.length; j++) {
						if (!decide_selection_row_hidden(selections[j]))
							lastVisible = selections[j]
					}
				} else {
					for (var j2 = 0; j2 < selections.length; j2++) {
						if (!decide_selection_row_hidden(selections[j2])) {
							lastVisible = selections[j2]
							break
						}
					}
				}
				if (!lastVisible || lastVisible == element)
					return exoduscancelevent(event)
				decide_last_option_element = lastVisible
				try {
					lastVisible.focus()
					if (lastVisible.select)
						lastVisible.select()
				} catch (e3) {
					client_focuson(lastVisible)
				}
				// PgUp/Dn: scan without changing radio (historical)
				return exoduscancelevent(event)
			}

			// Up/Down (and faked from Enter multi): radio checks; multi focus only; wrap ends.
			// PgUp/PgDn: no wrap (wheel same).
			var selectRadio = !decide_returnmany
			var wrap = !(keycode == 33 || keycode == 34)
			if (!decide_move_option(direction, steps, selectRadio, wrap))
				return exoduscancelevent(event)

			return exoduscancelevent(event)

		}

		//all following refers to many selections
		if (!decide_returnmany) {
			if (keycode == 13) {
				decide_ok_onclick_sync()
				return exoduscancelevent(event)
			}
			return
		}

		// Del or F8: restore entry checks (not clear-all)
		if (keycode == 46 || keycode == 119) {
			decide_restore_entry_state()
			return exoduscancelevent(event)
		}

	}

} //of decide_onload()

var gsorttable2offset = 1//decide
//var gsorttable2offset=0//decide2
function decide_sorttable2_sync(event) {

	//locate the current element
	event = getevent(event)

	// Click may be on the sort icon (or its mask child) — resolve to TH
	var th = event.target
	while (th && th.tagName != 'TH' && th != document.body)
		th = th.parentNode
	if (!th || th.tagName != 'TH')
		return (0)
	// Only titled data columns (untitled: no sortbutton_ — skip All / rank / blank heads)
	if (!th.classList || !th.classList.contains('decide_datacol'))
		return (0)
	if (!th.querySelector('[id^="sortbutton_"]'))
		return (0)

	var tableelement = th.parentElement.parentElement.parentElement
	var tablerows = tableelement.tBodies[0].getElementsByTagName('tr')
	var coln = th.cellIndex + gsorttable2offset

	// Toggle: first click ascending, second reverse (same as before)
	var reverse = th.getAttribute('sorttable2_issorted')
	th.setAttribute('sorttable2_issorted', reverse ? '' : 1)

	// Chevrons: active col down→up cycle; others unsorted (exotable sortbutton_*)
	var headrow = th.parentElement
	if (headrow && typeof exodus_apply_sort_icon == 'function') {
		var dths = headrow.querySelectorAll('th.decide_datacol')
		for (var hi = 0; hi < dths.length; hi++) {
			var icon = dths[hi].querySelector('[id^="sortbutton_"]')
			if (!icon)
				continue
			if (dths[hi] === th) {
				// reverse was previous state: was sorted → now up; was unset → down
				exodus_apply_sort_icon(icon, reverse ? 'up' : 'down')
			} else {
				dths[hi].removeAttribute('sorttable2_issorted')
				exodus_apply_sort_icon(icon, '')
			}
		}
	}

	var nrows = tablerows.length
	fromrown = 0
	uptorown = nrows - 1

	if (gdateformat == 'M/d/yyyy')
		var dateformat = [2, 0, 1]
	else if (gdateformat == 'yyyy/M/d')
		var dateformat = [0, 1, 2]
	else //gdateformat = 'd/M/yyyy'
		var dateformat = [2, 1, 0]
	var yy = dateformat[0] + 1
	var mm = dateformat[1] + 1
	var dd = dateformat[2] + 1

	var sortdata = []
	var dateregex = / ?(\d{4}|\d{1,2})\/ ?(\d{1,2})\/(\d{4}|\d{2})/
	var periodregex = / ?(\d{1,2})\/(\d{4})/g
	for (var ii = fromrown; ii <= uptorown; ++ii) {
		var cell = tablerows[ii].cells[coln]
		if (!cell)
			continue
		var value = (cell.textContent || cell.innerText || "").toUpperCase()
		var match
		while (match = value.match(dateregex)) {
			//convert dates like n/n/yy or n/n/yyyy to sortable yyyy|mm|dd format
			value = value.replace(dateregex, ('0000' + match[yy]).slice(-4) + '|' + ('00' + match[mm]).slice(-2) + '|' + ('00' + match[dd]).slice(-2))
			//console.log(value)
		}

		value = value.replace(periodregex, '$2|$1')

		//natural sort
		value = value.replace(/[-+]?[1234567890.,]+/g, function (x) { if (x.slice(0, 1) == '-') { y = '-'; x = x.slice(1) } else y = ''; return y + ('0000000000000000000000' + x).slice(-20) })
		//value+=('000000000000'+ii).slice(-10)//stable sort
		//cell.setAttribute('sortvalue',value)
		sortdata.push([value, ii])
	}

	sortdata.sort(function (a, b) { if (a[0] < b[0]) return -1; if (a[0] > b[0]) return 1; return a[1] - b[1] })
	if (reverse)
		sortdata.reverse()

	//get an array of the tablerows
	var oldrows = []
	for (var oldrown = 0; oldrown < tablerows.length; oldrown++)
		oldrows[oldrown] = tablerows[oldrown]

	//reorder table rows
	for (var newrown = 0; newrown < sortdata.length; ++newrown) {
		var oldrown = sortdata[newrown][1]
		if (newrown != oldrown)
			tablerows[newrown].swapNode(oldrows[oldrown])
	}

}//decide_sorttable2_sync

//these functions should be removed after a while if never called
function login() {
	if (gusername && gusername == 'EXODUS')
		alert(login.caller.name + ' called login')
}
function log() {
	//if (gusername&&gusername=='EXODUS')
	//	alert(log.caller.name+' called log')
}
function logout() {
	if (gusername && gusername == 'EXODUS')
		alert(logout.caller.name + ' called logout')
}

function setdateformat() {

	gfirstdayofweek = exodusgetcookie2('fd')
	if (!gfirstdayofweek) gfirstdayofweek = 1
	gfirstdayofweek = Number(gfirstdayofweek)

	var dateformat = exodusgetcookie2('df')
	//international and default
	if (!dateformat || dateformat.slice(0, 2) == '31') {
		gdatedaypos = 0
		gdatemonthpos = 1
		gdateyearpos = 2
		gdateformat = 'd/M/yyyy'
		//american
	} else if (dateformat.slice(0, 2) == '01') {
		gdatedaypos = 1
		gdatemonthpos = 0
		gdateyearpos = 2
		gdateformat = 'M/d/yyyy'
	}
	//modern sortable date
	else {
		gdatedaypos = 2
		gdatemonthpos = 1
		gdateyearpos = 0
		gdateformat = 'yyyy/M/d'
	}
	//console.log('setdateformat() '+gfirstdayofweek)
	return
}

function DATE(mode, value, params) {

	//gmsg='e.g. 31, 31/1, 31/1/01 or 31/1/2001'

	//can handle an array of values
	if (typeof value == 'object')
		return exodusconvarray(DATE, mode, value, params)

	//not == which would disallow 0 which is 31/12/1967
	if (value === '')
		return ''

	if (typeof value == 'undefined') {
		value = mode
		mode = 'OCONV'
	}

	var result
	var result2

	//input conversion
	if (mode == 'ICONV') {

		//four digits are assumed to be in ddmm or mmdd format
		if (value.match(/^\d{4}$/))
			value = value.substr(0, 2) + '/' + value.substr(2, 2)

		//five digits are assumed to be already in internal format
		if (value.match(/^\d{5}$/))
			return value

		//six/eight digits are assumed to be in DDMMYY/MMDDYY/DDMMYYYY/MMDDYYYY format
		if (value.match(/^(\d{6})|(\d{8})$/))
			value = value.substr(0, 2) + '/' + value.substr(2, 2) + '/' + value.substr(4)

		//assume in the format 31x1x2000
		value = value.replace(/\W/g, ' ').split(' ').slice(0, 3)

		//check day is one or two digits and force to three values (blanks if necessary)
		if (value.length == 1) {
			value[gdatedaypos] = value[0]
			value[gdatemonthpos] = ''
			value[gdateyearpos] = ''
		}
		else if (value.length == 2)
			value[2] = ''

		//default params
		if (typeof params == 'undefined') params = ''

		//option to not iconv (re
		//if (params=='NOICONV')
		//{
		// if (value.length==1)
		// return value[0]
		//}

		var fromto = ''
		if (params.indexOf('=') + 1) {
			params = params.split('=')
			fromto = params[0]
			var otherdateid = params[1]
			//otherdate = await gds.getx(otherdateid,grecn)
			//otherdate = getvalue(otherdateid,grecn)
			//direct access to gds.data to get the internal date (getvalue gets the external format)
			//should NOT be accessing gds directly but cannot use yielding .getx from non-yielding oconv/iconv
			//TODO doesnt handle other dates in rows very well
			var otherdate = gds.data[otherdateid]
			if (!otherdate)
				otherdate = gds.data['group' + ggroupno][grecn][otherdateid]
			otherdate = otherdate.text
			if (typeof otherdate == 'object')
				otherdate = otherdate[0]

			//params=otherdate.exodusoconv('[DATE]').exodusfield('/',2,2)
			//get month/year
			params = otherdate.exodusoconv('[DATE,MONTH]') + '/' + otherdate.exodusoconv('[DATE,YEAR]')

		}

		//split the params
		params = params.replace(/\//g, ' ').split(' ')

		//default to month parameter
		//if (!value[1]&&!!params[0]) value[1]=params[0]
		if (!value[gdatemonthpos] && !!params[0])
			value[gdatemonthpos] = params[0]

		//otherwise default to current month
		if (!value[gdatemonthpos])
			value[gdatemonthpos] = new Date().getMonth() + 1

		//convert month names to month numbers
		var monthno = parseInt(value[gdatemonthpos], 10)
		if (isNaN(monthno)) {
			var mthname = value[gdatemonthpos].toUpperCase()
			var mthnamelen = mthname.length
			for (var ii = 0; ii < 12; ++ii) {
				if (gmonthnames[ii].slice(0, mthnamelen).toUpperCase() == mthname) {
					monthno = ii + 1
					value[gdatemonthpos] = monthno
					break
				}
			}
		}

		//default to year parameter
		if (!value[gdateyearpos] && !!params[1])
			value[gdateyearpos] = params[1]

		//otherwise default to current year
		if (value[gdateyearpos])
			value[gdateyearpos] = ADDCENT(value[gdateyearpos])
		else
			value[gdateyearpos] = new Date().getFullYear()

		//check integers
		if (isNaN(parseInt(value[gdatedaypos], 10)) || isNaN(monthno) || isNaN(parseInt(value[gdateyearpos], 10))) {
			gmsg = invaliddatemsg()
			return null
		}

		//check day is one or two digits
		if (!value[gdatedaypos].match(/^\d{1,2}$/)) {
			gmsg = invaliddatemsg()
			return null
		}

		/* convert to ms date
		//try to convert to internal date format
		result=new Date(value[2],value[1]-1,value[0])
		if (isNaN(result)) {

		gmsg=invaliddatemsg()
		return null
		}
		*/

		//convert dates from MS base to base zero=31/12/67
		result = (Date.UTC(value[gdateyearpos], value[gdatemonthpos] - 1, value[gdatedaypos]) - Date.UTC(1967, 11, 31)) / 24 / 60 / 60 / 1000

		//prevent 6 digit years ie ge 15 oct 2241
		//causes problem with -10000
		//if (result.toString().length > 5) {
		//	gmsg = invaliddatemsg()
		//	return null
		//}

		//limit 1/1/1900 to 31/12/2099
		if (result < -24835 || result > 48213) {
			gmsg = invaliddatemsg()
			return null
		}

		//check that the oconv matches the iconv
		//the above algorithm allows dom <=99 and is calculated into next months
		result2 = DATE('OCONV', result).split('/')
		if (parseInt(result2[gdatedaypos], 10) != parseInt(value[gdatedaypos], 10) || parseInt(result2[gdatemonthpos], 10) != parseInt(value[gdatemonthpos], 10)) {
			gmsg = invaliddatemsg()
			return null
		}

		//check versus/amend other date

		if (fromto) {

			var otherdate0 = otherdate

			//<= >= and !== allow for date 0 31/12/1967

			//prevent todate less than fromdate
			if (fromto == 'TO' && result >= otherdate)
				otherdate = result
			if (fromto == 'FROM' && result <= otherdate)
				otherdate = result

			//if change fromdate and todate blank or same as from goldvalue
			if (fromto == 'UPTO' && (otherdate == '' || otherdate == goldvalue))
				otherdate = result

			//update the otherdate
			if (otherdate !== otherdate0) {

				// Sync DATE iconv cannot await. Peer setx after current flight lands.
				// when_idle retries; systemerror if still busy (never silent drop).
				;(function (id, recn, val) {
					exodus_begin_when_idle(function () {
						return gds.setx(id, recn, val)
					}, 'date-fromto setx ' + id, { delay_ms: 1 })
				})(otherdateid, grecn, otherdate)
			}

		}
	}

	//output conversion
	else {

		//  if (value=='"') alert(value)
		//if not digits then return unconverted
		//allow for negative dates before 1968
		if (typeof value == 'string' && !value.match(/^-?\d*$/))
			return value
		//  if (value=='"') alert('x'+value)

		/* convert to ms date
		value=new Date(value)
		if (isNaN(value)) {

		gmsg=invaliddatemsg()
		return null
		}
		//zzz should format it with params?
		result=value.getDate()+'/'+(value.getMonth()+1)+'/'+value.getFullYear()
		*/

		//convert from 1=1/1/67 to text DD/MM/YYYY format
		//result=new Date(1967,11,31+parseInt(value,10))
		result = new Date
		result.setTime(Date.UTC(1967, 11, 31 + parseInt(value, 10)))

		switch (params) {
			case 'DOW': {

				result = ((value - 1) % 7) + 1
				break
			}
			case 'DAYNAME': {

				result = ((value - 1) % 7) + 1
				result = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday'][result - 1]
				break
			}
			case 'DAYNAME3': {

				result = ((value - 1) % 7) + 1
				result = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'][result - 1]
				break
			}
			case 'DOM': {

				result = result.getUTCDate()
				break
			}
			case 'MONTH': {

				result = result.getUTCMonth() + 1
				break
			}
			case 'DOM2': {

				result = ('0' + result.getUTCDate()).slice(-2)
				break
			}
			case 'MONTH2': {

				result = ('0' + (result.getUTCMonth() + 1)).slice(-2)
				break
			}
			case 'YEAR': {

				result = result.getUTCFullYear()
				break
			}
			case 'YEAR2': {

				result = parseInt(result.getUTCFullYear().toString().slice(-2), 10)
				break
			}
			case 'PERIOD': {

				//returns the current month/current year eg 1/2000
				result = (result.getUTCMonth() + 1) + '/' + result.getUTCFullYear()
				break
			}
			case 'PERIOD2': {

				//returns the current month/current year eg 1/00
				result = (result.getUTCMonth() + 1) + '/' + result.getUTCFullYear().toString().slice(-2)
				break
			}
			case 'YEARPERIOD': {

				//returns the current current year.current month eg 2001.01
				result = result.getUTCFullYear() + '.' + ('00' + (result.getUTCMonth() + 1)).slice(-2)
				break
			}
			case 'MINIMAL': {

				result2 = []
				result2[gdatedaypos] = result.getUTCDate()
				result2[gdatemonthpos] = result.getUTCMonth() + 1
				result2[gdateyearpos] = result.getUTCFullYear()
				result = result2

				//trim same year and month
				var curryear = (new Date().getFullYear()).toString()
				if (result[gdateyearpos] == curryear) {
					result[gdateyearpos] = ''
					var currmonth = (new Date().getMonth() + 1).toString()
					if (result[gdatemonthpos] == currmonth) {
						result[gdatemonthpos] = ''
					}
				}
				result = result.exodusjoin('/').exodustrim('/')

				//add day of week
				var dow = ((value - 1) % 7) + 1
				result = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'][dow - 1] + ' ' + result

				break
			}
			case 'D': {
				//alert('"'+result+'"')
				//result = result.getUTCDate()// + '/' + gmonthnames[result.getUTCMonth() + 1] + '/' + result.getUTCFullYear().toString()
				result = result.getUTCDate() + ' ' + gmonthnames[result.getUTCMonth()].substr(0, 3).toUpperCase() + ' ' + result.getUTCFullYear();
				break;
			}
			default: {

				result2 = []
				result2[gdatedaypos] = result.getUTCDate()
				result2[gdatemonthpos] = result.getUTCMonth() + 1
				result2[gdateyearpos] = result.getUTCFullYear()
				result = result2.join('/')
			}
		}

	}

	return result

}

function TIME(mode, value, params) {

	gmsg = 'e.g. 12:45, 1245 12.75, 00:00, 23:59, 11:59:59, 24:00, 25:00 etc.'

	var result

	//can handle an array of values
	if (typeof value == 'object')
		return exodusconvarray(TIME, mode, value, params)
	/*
	//can handle an array of values
	if (typeof(value)=='object') {

	result=[]
	for (var i=0;i<value.length;i++) {

	result[i]=TIME(mode,value[i],params)

	//if any conversion fails return complete failure
	if (result[i]==null) return null

	}
	return result
	}
	*/

	//blank in .. blank out
	if (value == '')
		return ''

	if (typeof value == 'undefined') {
		value = mode
		mode = 'OCONV'
	}

	if (mode == 'OCONV') {

		//times by themselves for now are considered to be "local time" of data entry person tz could be added info
		//DATE_TIME uses gtz
		//value=Number(value)+gtz[0]

		var secs = value % 60
		value -= secs
		var mins = (value / 60) % 60
		value -= 60 * mins
		//var hours=(value/60/60)%24
		//allow 24:00 and 25:00 etc
		var hours = (value / 60 / 60)
		result = ('0' + hours).slice(-2) + ':' + ('0' + mins).slice(-2)
		//		if (params.indexOf('S'))
		//		 result+= ':' + ('0' + secs).slice(-2)

		//result+=':'+('0'+secs).slice(-2)
	}
	else {

		//format must be HH:MM or HH:MM:SS (":" may also be space or missing)
		var temp = value
		if (temp.match(/^\d{4}$/))
			temp = temp.substr(0, 2) + ':' + temp.substr(2, 2)
		if (temp.match(/^\d{6}$/))
			temp = temp.substr(0, 2) + ':' + temp.substr(2, 2) + ':' + temp.substr(4, 2)
		temp = temp.exodusconvert('. ', '::').split(':')
		if (!temp[1]) temp[1] = '00'
		if (!temp.join(':').match(/(^\d{1,2}:\d{1,2}$)|(^\d{1,2}:\d{1,2}:\d{1,2}$)/)) return null

		//if (!temp[1]) temp[1]=0
		if (!temp[2]) temp[2] = 0
		temp[0] = +temp[0]
		temp[1] = +temp[1]
		temp[2] = +temp[2]
		//if (temp[0]>23) return null
		//allow up to two days eg 47:59
		if (temp[0] > 47) return null
		if (temp[1] > 59) return null
		if (temp[2] > 59) return null

		result = temp[0] * 60 * 60 + temp[1] * 60 + temp[2]
		//times by themselves for now are considered to be "local time" of data entry person tz could be added info
		//DATE_TIME uses gtz
		//result-=gtz[0]

	}

	return result

}

function DATE_TIME(mode, value, params) {
	if (value == '') return ''
	if (mode == 'OCONV') {
		if (!params) params = ''
		params = (params + '!').split('!')
		value = value.toString().split('.')
		var datebit = Number(value[0])
		var timebit
		if (value.length < 1)
			timebit = ''
		else {
			timebit = Number(value[1]) + gtz[0]
			//assume -86400<gtz[0]<86400
			if (timebit < 0) {
				datebit -= 1
				timebit += 86400
			}
			else if (timebit > 86400) {
				datebit -= 1
				timebit += 86400
			}
		}
		return datebit.exodusoconv('[DATE,' + params[0] + ']') + ' ' + timebit.exodusoconv('[TIME,' + params[1] + ']')
	}
	else {
		//iconv not implemented yet
		return value
	}

}

function PERIOD_OF_TIME(mode, value, params) {

	if (value == '')
		return ''

	var result

	//can handle an array of values
	if (typeof value == 'object')
		return exodusconvarray(PERIOD_OF_TIME, mode, value, params)
	/*
	//can handle an array of values
	if (typeof(value)=='object') {

	result=[]
	for (var i=0;i<value.length;i++) {

	result[i]=PERIOD_OF_TIME(mode,value[i],params)

	//if any conversion fails return complete failure
	if (result[i]==null) return null

	}
	return result
	}
	*/

	if (mode == 'ICONV') {

		//if already numeric then simply return it
		if (exodusnum(value)) return value

		//allow slash, dash, space and comma as well as ":" for separator
		value = value.exodusconvert('/- ,', '::::')

		value = value.split(':')
		if (value.length < 2) value[1] = 0

		hours = value[0]
		mins = value[1]

		//check numeric otherwise return undefined
		if (!(exodusnum(hours) || !exodusnum(mins))) {
			return null
		}

		result = +hours + (+mins) / 60

	}
	else {

		if (!(exodusnum(value))) {
			//out=''
			//status()=2
			return null
		}

		//nothing in nothing out
		if (value == '') return ''

		value = value.toString()
		value = value.split('.')
		if (value[0] == '') value[0] = '0'
		if (value.length < 2) value[1] = '0'

		var result = value[0] + ':'
		temp = value[1]
		if (temp) temp = parseFloat('.' + temp)
		temp = exodusround(temp * 60, 0)
		temp = '00' + temp
		result += temp.slice(temp.length - 2)

	}

	return result

}

var gclient_focuson_element
function client_focuson(element) {
	gclient_focuson_element = element
	window.setTimeout(client_focuson2, 1)
}

function client_focuson2(element) {
	if (gclient_focuson_element && gclient_focuson_element.focus) {
		gclient_focuson_element.focus()
		gclient_focuson_element = undefined
	}
}

function logevent(msg) {
	if (!console)
		return
	if (glogevents)
		console.log(msg)
}

// Dev dump: more than outerHTML — box + key computed styles + matched rules.
// Usage (console):
//   exodus_dump_styles()                    // $0 if set, else document.activeElement
//   exodus_dump_styles('#INVOICE_TO')
//   exodus_dump_styles(el, ['display','width','padding'])
//   exodus_dump_styles('table[exogroupno="4"] tbody tr:first-child > *')
// Returns the object; console.logs JSON; copy() when available.
function exodus_dump_styles(sel, props) {
	var roots
	if (sel == null || sel === '') {
		// $0 = last selected node in DevTools (when present)
		var pick = (typeof $0 != 'undefined' && $0) || document.activeElement
		roots = pick ? [pick] : []
	}
	else if (typeof sel == 'string')
		roots = Array.prototype.slice.call(document.querySelectorAll(sel))
	else if (sel && sel.nodeType == 1)
		roots = [sel]
	else if (sel && sel.length != null)
		roots = Array.prototype.slice.call(sel)
	else
		roots = []

	// Default props: enough for layout/chrome bugs without dumping every longhand
	var want = props
	if (!want || !want.length) {
		want = [
			'display', 'visibility', 'position', 'box-sizing',
			'width', 'min-width', 'max-width', 'height', 'min-height', 'max-height',
			'padding', 'padding-left', 'padding-right', 'padding-top', 'padding-bottom',
			'margin', 'margin-left', 'margin-right', 'margin-top', 'margin-bottom',
			'border', 'border-width', 'border-left-width', 'border-right-width',
			'overflow', 'overflow-x', 'overflow-y',
			'flex', 'flex-shrink', 'flex-grow', 'align-items', 'vertical-align',
			'font', 'font-size', 'line-height', 'white-space', 'color', 'background-color'
		]
	}

	function sheet_label(ss, i) {
		if (ss.href)
			return ss.href
		if (ss.ownerNode && ss.ownerNode.id)
			return '#' + ss.ownerNode.id
		return 'inline#' + i
	}

	function matched_rules(el) {
		var hits = []
		for (var si = 0; si < document.styleSheets.length; si++) {
			var ss = document.styleSheets[si]
			var rules
			try {
				rules = ss.cssRules
			}
			catch (e) {
				hits.push({ sheet: sheet_label(ss, si), error: String(e) })
				continue
			}
			if (!rules)
				continue
			for (var ri = 0; ri < rules.length; ri++) {
				var r = rules[ri]
				if (!r.selectorText)
					continue
				try {
					if (el.matches(r.selectorText))
						hits.push({
							sheet: sheet_label(ss, si),
							sel: r.selectorText,
							css: r.style ? r.style.cssText : r.cssText
						})
				}
				catch (e2) {
					// :has() / complex selectors that matches() rejects — skip
				}
			}
		}
		return hits
	}

	function dump_one_node(el) {
		var cs = window.getComputedStyle(el)
		var styles = {}
		for (var i = 0; i < want.length; i++) {
			var p = want[i]
			styles[p] = cs.getPropertyValue(p) || cs[p]
		}
		var r = el.getBoundingClientRect()
		return {
			tag: el.tagName,
			id: el.id || '',
			name: el.getAttribute('name') || el.getAttribute('exoname') || '',
			className: el.className || '',
			html: (el.outerHTML || '').slice(0, 500),
			box: {
				x: r.x, y: r.y, w: r.width, h: r.height,
				top: r.top, left: r.left, bottom: r.bottom, right: r.right
			},
			offset: {
				w: el.offsetWidth, h: el.offsetHeight,
				offsetParent: el.offsetParent && (el.offsetParent.id || el.offsetParent.tagName)
			},
			inline: el.getAttribute('style') || '',
			computed: styles,
			matched: matched_rules(el)
		}
	}

	var out = {
		url: location.href,
		n: roots.length,
		nodes: roots.map(dump_one_node)
	}
	var text = JSON.stringify(out, null, 2)
	if (typeof console != 'undefined' && console.log)
		console.log(out)
	if (typeof copy == 'function') {
		try {
			copy(text)
		}
		catch (e) {
		}
	}
	return out
}

//end of client.js
