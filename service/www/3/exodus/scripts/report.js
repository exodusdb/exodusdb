// Report HTM extras (linked, not embedded). Missing script → no feature.
// Logo → existing edit mode → exo_report_onedit(true|false).
//
// Collapsed column:
//   edit ON  — narrow stub + ±; body text invisible (not a white hole)
//   edit OFF — visibility:collapse on <col> so the column takes no space

(function () {
	'use strict'

	var STYLE_ID = 'exo-rpt-colfold-style'
	var TABLE_CLASS = 'exo-rpt-cols'
	var EDITING_CLASS = 'exo-rpt-editing'
	var COL_CLASS = 'exo-rpt-col'
	var COLLAPSED = 'exo-rpt-col-collapsed'
	var LABEL_CLASS = 'exo-rpt-foldlabel'
	var BTN_CLASS = 'exo-rpt-foldbtn'
	var GLYPH_CLASS = 'exo-rpt-foldglyph'
	var STUB = '1.35em'

	function ensureCss() {
		var style = document.getElementById(STYLE_ID)
		if (!style) {
			style = document.createElement('style')
			style.id = STYLE_ID
			;(document.head || document.documentElement).appendChild(style)
		}
		style.textContent = [
			'table.' + TABLE_CLASS + ' {',
			'  table-layout: fixed;',
			'}',
			'th.' + COLLAPSED + ',',
			'td.' + COLLAPSED + ' {',
			'  box-sizing: border-box;',
			'  overflow: hidden;',
			'  padding: 0;',
			/* prevent narrow col from wrapping cell text into a tall stack */
			'  white-space: nowrap;',
			'}',
			/* Edit on: hide glyphs; nowrap above keeps row height stable */
			'table.' + EDITING_CLASS + ' td.' + COLLAPSED + ',',
			'table.' + EDITING_CLASS + ' th.' + COLLAPSED + ' {',
			'  color: transparent;',
			'}',
			'table.' + EDITING_CLASS + ' td.' + COLLAPSED + ' *,',
			'table.' + EDITING_CLASS + ' th.' + COLLAPSED + ' * {',
			'  visibility: hidden;',
			'}',
			'th.' + COLLAPSED + ' .' + LABEL_CLASS + ' {',
			'  display: none;',
			'}',
			'th.' + TABLE_CLASS + '-ready {',
			'  position: relative;',
			'}',
			'.' + BTN_CLASS + ' {',
			'  position: absolute;',
			'  top: 0;',
			'  left: 50%;',
			'  transform: translateX(-50%);',
			'  box-sizing: border-box;',
			'  display: flex;',
			'  align-items: center;',
			'  justify-content: center;',
			'  width: 1.15em;',
			'  height: 1.15em;',
			'  padding: 0;',
			'  margin: 0;',
			'  background: #eee;',
			'  border: 1px solid #888;',
			'  border-radius: 2px;',
			'  cursor: pointer;',
			'  user-select: none;',
			'  z-index: 3;',
			'}',
			/* Drawn bars — same geometry for + and − (no font metrics) */
			'.' + GLYPH_CLASS + ' {',
			'  position: relative;',
			'  display: block;',
			'  width: 0.55em;',
			'  height: 0.55em;',
			'}',
			'.' + GLYPH_CLASS + '::before,',
			'.' + GLYPH_CLASS + '::after {',
			'  content: \"\";',
			'  position: absolute;',
			'  left: 50%;',
			'  top: 50%;',
			'  transform: translate(-50%, -50%);',
			'  background: #333;',
			'}',
			'.' + GLYPH_CLASS + '::before {',
			'  width: 100%;',
			'  height: 2px;',
			'}',
			'.' + GLYPH_CLASS + '--plus::after {',
			'  width: 2px;',
			'  height: 100%;',
			'}',
			'table.' + EDITING_CLASS + ' th.' + COLLAPSED + ' .' + BTN_CLASS + ',',
			'table.' + EDITING_CLASS + ' th.' + COLLAPSED + ' .' + BTN_CLASS + ' *,',
			'table.' + EDITING_CLASS + ' th.' + COLLAPSED + ' .' + BTN_CLASS + ' *::before,',
			'table.' + EDITING_CLASS + ' th.' + COLLAPSED + ' .' + BTN_CLASS + ' *::after {',
			'  visibility: visible;',
			'}',
			'table.' + EDITING_CLASS + ' th.' + COLLAPSED + ' .' + GLYPH_CLASS + '::before,',
			'table.' + EDITING_CLASS + ' th.' + COLLAPSED + ' .' + GLYPH_CLASS + '::after {',
			'  background: #333;',
			'}',
			'@media print {',
			'  .' + BTN_CLASS + ' { display: none; }',
			'  /* collapsed cols stay collapsed in print via col visibility */',
			'}'
		].join('\n')
	}

	function dataHeaderRow(table) {
		var thead = table.tHead
		if (!thead || !thead.rows.length)
			return null
		for (var r = thead.rows.length - 1; r >= 0; r--) {
			var row = thead.rows[r]
			var n = 0
			for (var c = 0; c < row.cells.length; c++) {
				if ((row.cells[c].colSpan || 1) === 1)
					n++
			}
			if (n > 1)
				return row
		}
		return thead.rows[thead.rows.length - 1]
	}

	function columnCount(table) {
		var row = dataHeaderRow(table)
		if (row)
			return row.cells.length
		if (table.tBodies[0] && table.tBodies[0].rows[0])
			return table.tBodies[0].rows[0].cells.length
		return 0
	}

	function ensureColgroup(table) {
		var ncols = columnCount(table)
		if (!ncols)
			return null
		var cg = table.getElementsByTagName('colgroup')[0]
		if (!cg) {
			cg = document.createElement('colgroup')
			table.insertBefore(cg, table.firstChild)
		}
		while (cg.children.length < ncols) {
			var col = document.createElement('col')
			col.className = COL_CLASS
			cg.appendChild(col)
		}
		return cg
	}

	function pinCellWidth(cell, width) {
		if (width) {
			cell.style.width = width
			cell.style.minWidth = width
			cell.style.maxWidth = width
		} else {
			cell.style.width = ''
			cell.style.minWidth = ''
			cell.style.maxWidth = ''
		}
	}

	// editOn: stub for ±. edit off: col visibility:collapse (no space).
	function layoutCollapsedCol(table, colIndex, editOn) {
		var cg = ensureColgroup(table)
		if (!cg || !cg.children[colIndex])
			return
		var col = cg.children[colIndex]
		if (!col.classList.contains(COLLAPSED)) {
			col.style.width = ''
			col.style.visibility = ''
			return
		}
		if (editOn) {
			col.style.visibility = ''
			col.style.width = STUB
			for (var r = 0; r < table.rows.length; r++) {
				var cell = table.rows[r].cells[colIndex]
				if (!cell || (cell.colSpan || 1) > 1)
					continue
				pinCellWidth(cell, STUB)
			}
		} else {
			col.style.visibility = 'collapse'
			col.style.width = '0'
			for (var r2 = 0; r2 < table.rows.length; r2++) {
				var cell2 = table.rows[r2].cells[colIndex]
				if (!cell2 || (cell2.colSpan || 1) > 1)
					continue
				pinCellWidth(cell2, '0')
			}
		}
	}

	function layoutAllCollapsed(table, editOn) {
		var cg = ensureColgroup(table)
		if (!cg)
			return
		for (var i = 0; i < cg.children.length; i++) {
			if (cg.children[i].classList.contains(COLLAPSED))
				layoutCollapsedCol(table, i, editOn)
		}
	}

	function thLabelText(th) {
		var label = th.querySelector('.' + LABEL_CLASS)
		var t = ''
		if (label)
			t = label.innerText || label.textContent || ''
		else
			t = th.innerText || th.textContent || ''
		return String(t).replace(/\s+/g, ' ').replace(/^\s+|\s+$/g, '')
	}

	function foldTitle(th, collapsed) {
		var name = thLabelText(th)
		if (collapsed)
			return name ? ('Expand ' + name) : 'Expand column'
		return name ? ('Collapse ' + name) : 'Collapse column'
	}

	function prepareTh(th) {
		var existing = th.querySelector('.' + BTN_CLASS)
		if (existing)
			return existing

		var label = th.querySelector('.' + LABEL_CLASS)
		if (!label) {
			label = document.createElement('span')
			label.className = LABEL_CLASS
			while (th.firstChild)
				label.appendChild(th.firstChild)
			th.appendChild(label)
		}

		th.classList.add(TABLE_CLASS + '-ready')

		var btn = document.createElement('span')
		btn.className = BTN_CLASS + ' noprint'
		btn.setAttribute('contenteditable', 'false')
		setFoldGlyph(btn, false)
		btn.title = foldTitle(th, false)
		btn.onmousedown = function (e) {
			e = e || window.event
			if (e.preventDefault) e.preventDefault()
			if (e.stopPropagation) e.stopPropagation()
			return false
		}
		btn.onclick = foldClick
		th.appendChild(btn)
		return btn
	}

	function setFoldGlyph(btn, collapsed) {
		var span = btn.firstChild
		if (!span || span.nodeType != 1 || span.tagName != 'SPAN') {
			btn.textContent = ''
			span = document.createElement('span')
			btn.appendChild(span)
		}
		span.className = GLYPH_CLASS + (collapsed ? '' : (' ' + GLYPH_CLASS + '--plus'))
		span.textContent = ''
	}

	function setCollapsed(table, colIndex, collapsed) {
		var cg = ensureColgroup(table)
		if (!cg || !cg.children[colIndex])
			return

		var col = cg.children[colIndex]
		var editOn = table.classList.contains(EDITING_CLASS)

		if (collapsed) {
			col.classList.add(COLLAPSED)
			for (var r = 0; r < table.rows.length; r++) {
				var cell = table.rows[r].cells[colIndex]
				if (!cell || (cell.colSpan || 1) > 1)
					continue
				cell.classList.add(COLLAPSED)
			}
		} else {
			col.classList.remove(COLLAPSED)
			col.style.width = ''
			col.style.visibility = ''
			for (var r2 = 0; r2 < table.rows.length; r2++) {
				var cell2 = table.rows[r2].cells[colIndex]
				if (!cell2 || (cell2.colSpan || 1) > 1)
					continue
				cell2.classList.remove(COLLAPSED)
				pinCellWidth(cell2, '')
			}
		}
		if (collapsed)
			layoutCollapsedCol(table, colIndex, editOn)
	}

	function foldClick(event) {
		event = event || window.event
		if (event.preventDefault) event.preventDefault()
		if (event.stopPropagation) event.stopPropagation()

		var btn = event.currentTarget || event.target
		while (btn && !(btn.classList && btn.classList.contains(BTN_CLASS)))
			btn = btn.parentNode
		if (!btn)
			return false

		var th = btn.parentNode
		while (th && th.tagName != 'TH')
			th = th.parentNode
		if (!th)
			return false

		var table = th
		while (table && table.tagName != 'TABLE')
			table = table.parentNode
		if (!table)
			return false

		var colIndex = th.cellIndex
		var collapsed = !th.classList.contains(COLLAPSED)
		setCollapsed(table, colIndex, collapsed)
		setFoldGlyph(btn, collapsed)
		btn.title = foldTitle(th, collapsed)
		return false
	}

	function armTable(table) {
		if (!table || table.tagName != 'TABLE')
			return
		table.classList.add(TABLE_CLASS)
		table.classList.add(EDITING_CLASS)
		ensureColgroup(table)
		layoutAllCollapsed(table, true)

		var row = dataHeaderRow(table)
		if (!row)
			return
		for (var c = 0; c < row.cells.length; c++) {
			var th = row.cells[c]
			if ((th.colSpan || 1) > 1)
				continue
			var btn = prepareTh(th)
			var collapsed = th.classList.contains(COLLAPSED)
			setFoldGlyph(btn, collapsed)
			btn.title = foldTitle(th, collapsed)
		}
	}

	function disarmTable(table) {
		var btns = table.querySelectorAll('.' + BTN_CLASS)
		for (var i = btns.length - 1; i >= 0; i--) {
			if (btns[i].parentNode)
				btns[i].parentNode.removeChild(btns[i])
		}
		table.classList.remove(EDITING_CLASS)
		// Collapsed columns take no space when not editing
		layoutAllCollapsed(table, false)
	}

	function eachReportTable(fn) {
		var tables = document.querySelectorAll('table.exotable, table')
		var seen = []
		for (var i = 0; i < tables.length; i++) {
			var t = tables[i]
			if (!t.tHead)
				continue
			if (seen.indexOf(t) >= 0)
				continue
			seen.push(t)
			fn(t)
		}
	}

	window.exo_report_onedit = function (on) {
		ensureCss()
		if (on)
			eachReportTable(armTable)
		else
			eachReportTable(disarmTable)
	}

	// ——— Column sort (was GETSORTJS poetry) ———
	// Hand cursor / click only after this file loads.
	function sorttable(event) {
		if (document.body.getAttribute('contenteditable'))
			return true
		event = event || window.event
		// do not assign event.target — getter-only in modern browsers
		var th = event.target || event.srcElement
		while (th && th.tagName != 'TH' && th !== document.body)
			th = th.parentNode
		if (!th || th.tagName != 'TH')
			return 0
		var table = th.parentElement.parentElement.parentElement
		if (!table || !table.tBodies || !table.tBodies[0])
			return 0
		var rows = table.tBodies[0].getElementsByTagName('tr')
		var coln = th.cellIndex
		var nrows = rows.length
		var fromrown = 0
		var uptorown = nrows - 1
		var rowchildNodes
		while ((uptorown < nrows) && rows[uptorown + 1] && (rowchildNodes = rows[uptorown + 1].cells)
			&& (rowchildNodes.length > coln) && (rowchildNodes[coln].tagName == 'TD'))
			uptorown++

		var dfmt = (typeof window.gdateformat != 'undefined' && window.gdateformat)
			? window.gdateformat : 'd/M/yyyy'
		var dateformat
		if (dfmt == 'M/d/yyyy')
			dateformat = [2, 0, 1]
		else if (dfmt == 'yyyy/M/d')
			dateformat = [0, 1, 2]
		else
			dateformat = [2, 1, 0]
		var yy = dateformat[0] + 1
		var mm = dateformat[1] + 1
		var dd = dateformat[2] + 1

		var dateregex = / ?(\d{1,2})\/ ?(\d{1,2})\/(\d{4}|\d{2})/
		var periodregex = / ?(\d{1,2})\/(\d{4})/g
		for (var ii = fromrown; ii <= uptorown; ++ii) {
			var cell = rows[ii].cells[coln]
			if (cell.getAttribute('sortvalue'))
				break
			var value = (cell.textContent || cell.innerText || '').toUpperCase()
			var match
			while (match = value.match(dateregex)) {
				value = value.replace(dateregex,
					('0000' + match[yy]).slice(-4) + '|'
					+ ('00' + match[mm]).slice(-2) + '|'
					+ ('00' + match[dd]).slice(-2))
			}
			value = value.replace(periodregex, '$2|$1')
			value = value.replace(/([-+]?[1234567890.,]+)([A-Z]{2,3})/g, '$2$1')
			value = value.replace(
				/[-+]?[1234567890.,]+/g,
				function (x) {
					x = x.replace(/,/g, '')
					var y
					if (x.slice(0, 1) == '-') {
						y = '-'
						x = (999999999999.999 + Number(x.replace(/,/g, ''))).toString()
					} else {
						y = ''
					}
					x = x.split('.')
					y += ('00000000000000000000' + x[0]).slice(-20)
					if (x[1])
						y += '.' + (x[1] + '0000000000').slice(0, 10)
					return y
				}
			)
			value += ('000000000000' + ii).slice(-10)
			cell.setAttribute('sortvalue', value)
		}
		QuickSort(rows, coln, fromrown, uptorown)
	}

	function QuickSort(rows, coln, min, max) {
		if (max <= min)
			return true
		var low = min
		var high = max
		var mid = rows[Math.floor((low + high) / 2)].cells[coln].getAttribute('sortvalue')
		do {
			while (rows[low].cells[coln].getAttribute('sortvalue') < mid)
				low++
			while (rows[high].cells[coln].getAttribute('sortvalue') > mid)
				high--
			if (low <= high) {
				rows[low].swapNode(rows[high])
				low++
				high--
			}
		} while (low <= high)
		if (high > min)
			QuickSort(rows, coln, min, high)
		if (low < max)
			QuickSort(rows, coln, low, max)
	}

	if (!document.swapNode) {
		Node.prototype.swapNode = function (node) {
			var p = node.parentNode
			var s = node.nextSibling
			this.parentNode.replaceChild(node, this)
			p.insertBefore(this, s)
			return this
		}
	}

	window.sorttable = sorttable

	function armSortableTheads() {
		var theads = document.querySelectorAll('table.exotable > thead')
		for (var i = 0; i < theads.length; i++) {
			var thead = theads[i]
			if (thead.getAttribute('data-exo-rpt-sort') == '1')
				continue
			thead.setAttribute('data-exo-rpt-sort', '1')
			thead.style.cursor = 'pointer'
			thead.addEventListener('click', sorttable)
		}
	}

	function whenDomReady(fn) {
		if (document.readyState == 'loading')
			document.addEventListener('DOMContentLoaded', fn)
		else
			fn()
	}
	whenDomReady(armSortableTheads)

	// ——— Break-row toggle + opener nwin link fix (was GETCSS poetry) ———
	// nlist may set window.togglendisplayed = nblocks after load
	if (typeof window.togglendisplayed == 'undefined')
		window.togglendisplayed = 0
	function toggle(t, mode) {
		if (typeof t == 'string') {
			if (document.getElementsByClassName)
				t = document.getElementsByClassName(t)
			else
				t = document.getElementsByName(t)
		}
		if (t && t.tagName)
			t = [t]
		if (!t || !t.length)
			return

		var display = 'none'
		for (var ii = t.length - 1; ii >= 0; ii--) {
			if (t[ii].style.display == '') {
				t[ii].style.display = 'none'
			} else {
				t[ii].style.display = ''
				display = ''
			}
		}

		if (!mode) {
			var toggleheading = false
			if (display == '') {
				window.togglendisplayed++
				if (window.togglendisplayed == 1)
					toggleheading = true
			} else {
				window.togglendisplayed--
				if (window.togglendisplayed == 0)
					toggleheading = true
			}
			if (toggleheading) {
				var rules = document.styleSheets[0].cssRules || document.styleSheets[0].rules
				var bheads = [rules[0], rules[1]]
				toggle(bheads, true)
			}
		}
	}
	window.toggle = toggle

	function fixOpenerNwinLinks() {
		if (!window.opener)
			return
		var links = document.getElementsByTagName('a')
		var vhtm
		for (var ii = 0; ii < links.length; ++ii) {
			var href = links[ii].href.toString()
			if (href.indexOf('nwin') < 0)
				continue
			href = href.toString().split("'")
			if (href[3] == 'V')
				href[3] = 'finance/vouchers.htm'
			if (!vhtm) {
				vhtm = window.opener.location.toString().split('/')
				vhtm.pop()
				if (href[3].indexOf('/') >= 0)
					vhtm.pop()
				vhtm = vhtm.join('/')
			}
			links[ii].href = vhtm + '/' + href[3] + '?key=' + href[1] + '&openreadonly=true'
			links[ii].target = '_blank'
		}
	}
	window.addEventListener('load', fixOpenerNwinLinks)

	// Mark loaded so GETSORTJS fallback chain skips (e.g. HTML-Complete
	// already injected xxx_files/report_….js ahead of the loader).
	window.__exo_rpt_ld = 1
})()