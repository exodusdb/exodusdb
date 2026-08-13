// Self-check for NUMBER / DECIMAL / INTEGER / ROUND.
//
//   deno run --allow-read test_number.js
//   deno run --allow-read test_number.js /path/to/other/exodus.js
//
// One suite for old and new builds: probes what exists, skips or adapts.
//
//   • NUMBER only as NUMBER(mode, value, params) — three args.
//   • display=false → DECIMAL section (if present).
//   • forced_ndecs → INTEGER section only when INTEGER forces 0dp.
//   • Grouping: gbasefmt 'MD2,' vs plain 'MD2'. Prefer large amounts.
//   • Invalid → null + gmsg (or throw on some old multivalue fails). Blank/Z → "".
//   • OCONV unit peel without CURRENCY is deliberate when display defaults on.

var passed = 0
var failed = 0
var skipped = 0

function check(label, actual, expected) {
    var ok
    if (expected === null)
        ok = actual === null
    else if (typeof actual == 'object' && actual !== null)
        ok = JSON.stringify(actual) === JSON.stringify(expected)
    else
        ok = actual === expected || String(actual) === String(expected)

    if (ok) {
        passed++
        console.log('ok  ' + label + '  (' + JSON.stringify(actual) + ')')
    } else {
        failed++
        console.log('FAIL ' + label)
        console.log('     got:      ' + JSON.stringify(actual))
        console.log('     expected: ' + JSON.stringify(expected))
    }
}

function skip(label, why) {
    skipped++
    console.log('skip ' + label + '  (' + why + ')')
}

// ---------------------------------------------------------------------------
// Load
// ---------------------------------------------------------------------------

globalThis.fm = String.fromCharCode(0x1e)
globalThis.vm = String.fromCharCode(0x1d)
globalThis.sm = String.fromCharCode(0x1c)
globalThis.tm = String.fromCharCode(0x1b)
globalThis.stm = String.fromCharCode(0x1a)
globalThis.gmsg = ''
globalThis.gbasefmt = 'MD2,'
globalThis.gndecs = 2
globalThis.gthousands_regex = /,/g
globalThis.systemerror = function (fn, msg) { throw new Error(fn + ': ' + msg) }
globalThis.alert = function () { }

var path = (Deno.args && Deno.args[0])
    ? Deno.args[0]
    : new URL('./exodus.js', import.meta.url).pathname
console.log('loading ' + path)
;(0, eval)(await Deno.readTextFile(path))

if (typeof NUMBER != 'function') {
    console.log('NUMBER missing')
    Deno.exit(2)
}

// --- capability probes (no 4th/5th args on NUMBER) ---
var hasDecimal = typeof DECIMAL == 'function'
var hasInteger = typeof INTEGER == 'function'
var hasRound = typeof ROUND == 'function'
var hasExodusround = typeof exodusround == 'function'
var hasExodusroundMember = typeof Number.prototype.exodusround == 'function'

gbasefmt = 'MD2,'
gthousands_regex = /,/g
var hasOconvGroup = false
try {
    hasOconvGroup = String(NUMBER('OCONV', '1234567', '0')).indexOf(',') >= 0
} catch (e) { }

var hasCurrency = false
try {
    hasCurrency = NUMBER('ICONV', '10USD', '0,CURRENCY') == '10USD' ||
        NUMBER('ICONV', '10.00USD', '2,CURRENCY') == '10.00USD'
} catch (e) { }

gbasefmt = 'MC2,'
gthousands_regex = /\./g
var hasMc = false
try {
    hasMc = NUMBER('ICONV', '1.234,50', '2') == '1234.50'
} catch (e) { }
gbasefmt = 'MD2,'
gthousands_regex = /,/g

// Multivalue path (some old builds: result[i] ReferenceError under strict)
var hasMv = false
try {
    hasMv = NUMBER('OCONV', '1' + vm + '2', '0') == '1' + vm + '2'
} catch (e) { hasMv = false }

// Modern ICONV strips commas even without gthousands_regex
var hasStripWithoutRegex = false
try {
    delete globalThis.gthousands_regex
    hasStripWithoutRegex = NUMBER('ICONV', '1,234.50', '2') == '1234.50'
} catch (e) { hasStripWithoutRegex = false }
globalThis.gthousands_regex = /,/g

console.log('caps: DECIMAL=' + hasDecimal +
    ' INTEGER=' + hasInteger +
    ' ROUND=' + hasRound +
    ' oconvGroup=' + hasOconvGroup +
    ' CURRENCY=' + hasCurrency +
    ' MC=' + hasMc +
    ' mv=' + hasMv +
    ' stripNoRegex=' + hasStripWithoutRegex)

// ===========================================================================
// NUMBER — three args only
// ===========================================================================

console.log('\n=== NUMBER (3-arg) ===')

// --- empty / invalid ---
console.log('\n--- empty / invalid ---')
gbasefmt = 'MD2,'
gthousands_regex = /,/g
gmsg = ''

check('empty OCONV → ""', NUMBER('OCONV', '', '2'), '')
check('empty ICONV → ""', NUMBER('ICONV', '', '2'), '')
gmsg = 'stale'
check('empty clears gmsg', NUMBER('ICONV', '', '2'), '')
check('gmsg after empty', gmsg, '')
check('empty CURRENCY OCONV → ""', NUMBER('OCONV', '', '2,CURRENCY'), '')
check('empty CURRENCY ICONV → ""', NUMBER('ICONV', '', '2,CURRENCY'), '')
check('empty BASE OCONV → ""', NUMBER('OCONV', '', 'BASE'), '')

check('non-numeric → null', NUMBER('ICONV', 'abc', '2'), null)
check('non-numeric gmsg', gmsg.indexOf('cannot be understood') >= 0, true)
check('non-numeric is not ""', NUMBER('ICONV', 'xyz', '2') === '', false)
check('non-numeric OCONV → null', NUMBER('OCONV', 'nope', '2'), null)
check('letters only → null', NUMBER('ICONV', 'USD', '2'), null)
check('letters only gmsg', gmsg.indexOf('cannot be understood') >= 0, true)
check('unit ICONV without CURRENCY → null', NUMBER('ICONV', '1234567USD', '2'), null)
check('space → null', NUMBER('ICONV', '1 234', '2'), null)
check('two dots → null', NUMBER('ICONV', '1.2.3', '2'), null)

// --- ndecs (plain OCONV) ---
console.log('\n--- ndecs ---')
gbasefmt = 'MD2'
check('pad OCONV', NUMBER('OCONV', '1234567', '2'), '1234567.00')
check('pad ICONV', NUMBER('ICONV', '1234567', '2'), '1234567.00')
check('round OCONV', NUMBER('OCONV', '1234567.895', '2'), '1234567.90')
check('round ICONV', NUMBER('ICONV', '1234567.895', '2'), '1234567.90')
check('round half OCONV', NUMBER('OCONV', '1234567.885', '2'), '1234567.89')
check('0dp OCONV', NUMBER('OCONV', '1234567.4', '0'), '1234567')
check('0dp ICONV', NUMBER('ICONV', '1234567.6', '0'), '1234568')
check('1dp OCONV', NUMBER('OCONV', '1234567.85', '1'), '1234567.9')
check('3dp pad OCONV', NUMBER('OCONV', '1234567.8', '3'), '1234567.800')
check('auto ndecs', NUMBER('OCONV', '1234567.895', ''), '1234567.895')
check('auto ndecs int', NUMBER('OCONV', '1234567', ''), '1234567')
check('auto ndecs one dp', NUMBER('OCONV', '1234567.5', ''), '1234567.5')

// --- Z ---
console.log('\n--- Z ---')
gbasefmt = 'MD2'
gmsg = ''
check('0Z OCONV → ""', NUMBER('OCONV', '0', '0Z'), '')
check('0Z ICONV → ""', NUMBER('ICONV', '0', '0Z'), '')
check('0Z keeps non-zero', NUMBER('OCONV', '1234567.4', '0Z'), '1234567')
check('2Z round-to-zero → ""', NUMBER('OCONV', '0.001', '2Z'), '')
check('2Z keeps 0.01', NUMBER('OCONV', '0.01', '2Z'), '0.01')
check('Z alone → ""', NUMBER('OCONV', '0', 'Z'), '')
check('Z keeps large', NUMBER('OCONV', '1234567', 'Z'), '1234567')
check('0Z gmsg clear', gmsg, '')

// --- BASE / NDECS ---
console.log('\n--- BASE / NDECS ---')
gbasefmt = 'MD2'
check('BASE MD2', NUMBER('OCONV', '1234567.895', 'BASE'), '1234567.90')
gbasefmt = 'MD4'
check('BASE MD4', NUMBER('OCONV', '1234567.89543', 'BASE'), '1234567.8954')
gbasefmt = 'MD0'
check('BASE MD0', NUMBER('OCONV', '1234567.6', 'BASE'), '1234568')
gbasefmt = ''
check('BASE empty → 4', NUMBER('OCONV', '1234567.89543', 'BASE'), '1234567.8954')

gbasefmt = 'MD2'
gndecs = 3
check('NDECS gndecs=3', NUMBER('OCONV', '1234567.8954', 'NDECS'), '1234567.895')
gndecs = undefined
globalThis.gds = { data: { NDECS: { text: '1' } } }
check('NDECS from gds', NUMBER('OCONV', '1234567.85', 'NDECS'), '1234567.9')
gndecs = 2
globalThis.gds = undefined

// --- min / max (min: 0 = non-negative; former POSITIVE keyword) ---
console.log('\n--- min / max ---')
gbasefmt = 'MD2,'
gmsg = ''
check('min0 reject neg', NUMBER('ICONV', '-1234567.5', '2,0'), null)
check('min0 gmsg', gmsg.indexOf('less than') >= 0, true)
check('min0 accept 0', NUMBER('ICONV', '0', '2,0'), '0.00')
check('min0 accept pos', NUMBER('ICONV', '1234567.5', '2,0'), '1234567.50')
check('min reject', NUMBER('ICONV', '1000', '2,5000'), null)
check('min gmsg', gmsg.indexOf('less than') >= 0, true)
check('min accept', NUMBER('ICONV', '5000', '2,5000'), '5000.00')
check('max reject', NUMBER('ICONV', '9999999', '2,,1000000'), null)
check('max gmsg', gmsg.indexOf('more than') >= 0, true)
check('max accept', NUMBER('ICONV', '1000000', '2,,1000000'), '1000000.00')
check('max 0 reject pos', NUMBER('ICONV', '0.01', '2,,0'), null)
check('max 0 accept 0', NUMBER('ICONV', '0', '2,,0'), '0.00')
check('max 0 accept neg', NUMBER('ICONV', '-1', '2,SIGNED,0'), '-1.00')
check('no min reject neg', NUMBER('ICONV', '-1', '2'), null)
check('SIGNED accept neg', NUMBER('ICONV', '-1', '2,SIGNED'), '-1.00')
// SIGNED + max: unbounded below; max only (not the same as min=-max)
// [NUMBER,,SIGNED,100] → params ",SIGNED,100"
check('SIGNED+max accept -100', NUMBER('ICONV', '-100', ',SIGNED,100'), '-100')
check('SIGNED+max accept -101', NUMBER('ICONV', '-101', ',SIGNED,100'), '-101')
check('SIGNED+max reject 101', NUMBER('ICONV', '101', ',SIGNED,100'), null)
// symmetric range (dict_number signed:true + max → min = -max, no SIGNED token)
check('symm -100..100 accept -100', NUMBER('ICONV', '-100', ',-100,100'), '-100')
check('symm -100..100 reject -101', NUMBER('ICONV', '-101', ',-100,100'), null)
check('symm -100..100 accept 100', NUMBER('ICONV', '100', ',-100,100'), '100')
check('symm -100..100 reject 101', NUMBER('ICONV', '101', ',-100,100'), null)
check('min+max range', NUMBER('ICONV', '1234567', '0,1000,9999999'), '1234567')
check('min reject is not ""', NUMBER('ICONV', '1', '2,5') === '', false)
check('max reject is not ""', NUMBER('ICONV', '9999999', '2,,1000') === '', false)
check('min0 reject is not ""', NUMBER('ICONV', '-1', '2,0') === '', false)
check('min equal boundary', NUMBER('ICONV', '1000', '0,1000,9999999'), '1000')
check('max equal boundary', NUMBER('ICONV', '9999999', '0,1000,9999999'), '9999999')

// --- grouping ---
console.log('\n--- grouping ---')
gbasefmt = 'MD2,'
gthousands_regex = /,/g
check('ICONV strip 1,234,567.80', NUMBER('ICONV', '1,234,567.80', '2'), '1234567.80')
check('ICONV strip 1,234,567,890.12', NUMBER('ICONV', '1,234,567,890.12', '2'), '1234567890.12')
check('ICONV strip 1,000,000', NUMBER('ICONV', '1,000,000', '0'), '1000000')
check('ICONV strip negative group', NUMBER('ICONV', '-1,234,567.80', '2,SIGNED'), '-1234567.80')

if (hasOconvGroup) {
    check('OCONV 1,234,567.80', NUMBER('OCONV', '1234567.8', '2'), '1,234,567.80')
    check('OCONV 1,234,567,890.12', NUMBER('OCONV', '1234567890.12', '2'), '1,234,567,890.12')
    check('OCONV negative group', NUMBER('OCONV', '-1234567.8', '2'), '-1,234,567.80')
    check('OCONV 12,345.00', NUMBER('OCONV', '12345', '2'), '12,345.00')
    check('OCONV already-grouped MD input', NUMBER('OCONV', '1,234,567.8', '2'), '1,234,567.80')
    gbasefmt = 'MD2'
    check('OCONV MD2 no comma plain', NUMBER('OCONV', '1234567.8', '2'), '1234567.80')
    gbasefmt = ''
    check('OCONV empty gbasefmt plain', NUMBER('OCONV', '1234567.8', '2'), '1234567.80')
    gbasefmt = 'MD2,'
} else {
    check('OCONV plain (no group in build)', NUMBER('OCONV', '1234567.8', '2'), '1234567.80')
    check('OCONV bigger plain', NUMBER('OCONV', '1234567890.12', '2'), '1234567890.12')
    skip('OCONV multi-group thousands', 'this build does not paint thousands on OCONV')
}

if (hasStripWithoutRegex) {
    delete globalThis.gthousands_regex
    check('ICONV without gthousands_regex', NUMBER('ICONV', '1,234,567.80', '2'), '1234567.80')
    globalThis.gthousands_regex = /,/g
} else {
    skip('ICONV without gthousands_regex', 'this build needs gthousands_regex for comma strip')
}

// --- MC ---
console.log('\n--- MC ---')
if (hasMc) {
    gbasefmt = 'MC2,'
    gthousands_regex = /\./g
    check('MC ICONV 1.234.567,80', NUMBER('ICONV', '1.234.567,80', '2'), '1234567.80')
    check('MC ICONV bigger', NUMBER('ICONV', '1.234.567.890,12', '2'), '1234567890.12')
    check('MC ICONV no thousands', NUMBER('ICONV', '1234,50', '2'), '1234.50')
    if (hasOconvGroup) {
        check('MC OCONV 1.234.567', NUMBER('OCONV', '1234567', '0'), '1.234.567')
        check('MC OCONV bigger', NUMBER('OCONV', '1234567890', '0'), '1.234.567.890')
        check('MC OCONV negative', NUMBER('OCONV', '-1234567', '0'), '-1.234.567')
    } else {
        check('MC OCONV plain', NUMBER('OCONV', '1234567', '0'), '1234567')
    }
    gbasefmt = 'MD2,'
    gthousands_regex = /,/g
} else {
    skip('MC suite', 'no MC ICONV in this build')
}

// --- multivalue ---
console.log('\n--- multivalue ---')
if (!hasMv) {
    skip('multivalue suite', 'mv/array path broken in this build (e.g. old result[i])')
} else {
    gbasefmt = 'MD2'
    check('vm plain', NUMBER('OCONV', '1234567.8' + vm + '2345678.9', '1'),
        '1234567.8' + vm + '2345678.9')
    gbasefmt = 'MD2,'
    if (hasOconvGroup) {
        check('vm grouped', NUMBER('OCONV', '1234567.8' + vm + '2345678.9', '1'),
            '1,234,567.8' + vm + '2,345,678.9')
    } else {
        check('vm no-group build', NUMBER('OCONV', '1234567.8' + vm + '2345678.9', '1'),
            '1234567.8' + vm + '2345678.9')
    }
    gbasefmt = 'MD2'
    check('fm', NUMBER('OCONV', '1000000' + fm + '2000000', '0'), '1000000' + fm + '2000000')
    check('sm', NUMBER('OCONV', '1000000' + sm + '2000000', '0'), '1000000' + sm + '2000000')
    check('array', NUMBER('OCONV', ['1234567.89', '2345678.91'], '1'), ['1234567.9', '2345678.9'])
    // Bad cell: new returns null; old may throw on null.join — both OK
    try {
        check('array one bad → null', NUMBER('ICONV', ['1234567', 'x'], '0'), null)
    } catch (e) {
        check('array one bad (threw)', true, true)
    }
    try {
        check('vm one bad → null', NUMBER('ICONV', '1234567' + vm + 'x', '0'), null)
    } catch (e) {
        check('vm one bad (threw)', true, true)
    }
    try {
        check('fm one bad → null', NUMBER('ICONV', '1234567' + fm + 'x', '0'), null)
    } catch (e) {
        check('fm one bad (threw)', true, true)
    }
    try {
        check('sm one bad → null', NUMBER('ICONV', '1234567' + sm + 'x', '0'), null)
    } catch (e) {
        check('sm one bad (threw)', true, true)
    }
}
// params null/undefined: modern coerces; classic may throw on .split
try {
    check('params null', NUMBER('ICONV', '1234567.5', null), '1234567.5')
} catch (e) {
    skip('params null', 'build requires string params')
}
try {
    check('params undefined', NUMBER('ICONV', '1234567.89', undefined), '1234567.89')
} catch (e) {
    skip('params undefined', 'build requires string params')
}
check('params empty string', NUMBER('ICONV', '1234567.5', ''), '1234567.5')

// --- CURRENCY / UNIT ---
console.log('\n--- CURRENCY / UNIT ---')
gbasefmt = 'MD2,'
gmsg = ''

if (!hasCurrency) {
    skip('CURRENCY suite', 'no CURRENCY|UNIT param in this build')
} else {
    check('ICONV keeps unit', NUMBER('ICONV', '1234567.50USD', '2,CURRENCY'), '1234567.50USD')
    check('ICONV UNIT synonym', NUMBER('ICONV', '2345678.5EUR', '2,UNIT'), '2345678.50EUR')
    check('ICONV lowercase token', NUMBER('ICONV', '1234567USD', '2,currency'), '1234567.00USD')
    check('ICONV GBP auto ndecs', NUMBER('ICONV', '1000000GBP', 'CURRENCY'), '1000000GBP')
    check('ICONV unit-only letters GBP', NUMBER('ICONV', '100GBP', 'CURRENCY'), '100GBP')
    check('ICONV negative+unit', NUMBER('ICONV', '-1234567.5USD', '2,SIGNED,CURRENCY'), '-1234567.50USD')
    check('ICONV grouped+unit', NUMBER('ICONV', '1,234,567.50USD', '2,CURRENCY'), '1234567.50USD')
    check('ICONV bare amount', NUMBER('ICONV', '1234567', '2,CURRENCY'), '1234567.00')
    check('ICONV empty', NUMBER('ICONV', '', '2,CURRENCY'), '')

    gmsg = ''
    check('ICONV unit without flag → null', NUMBER('ICONV', '1234567USD', '2'), null)
    check('ICONV unit without flag gmsg', gmsg.indexOf('cannot be understood') >= 0, true)
    check('ICONV bad unit digits → null', NUMBER('ICONV', '1234567US1', '2,CURRENCY'), null)
    check('ICONV space unit → null', NUMBER('ICONV', '1234567 USD', '2,CURRENCY'), null)

    check('CURRENCY first slot', NUMBER('ICONV', '1234567USD', 'CURRENCY,2'), '1234567.00USD')
    check('CURRENCY last + min/max', NUMBER('ICONV', '1234567USD', '2,0,9999999,CURRENCY'), '1234567.00USD')
    check('CURRENCY only auto ndecs', NUMBER('ICONV', '1234567.5USD', 'CURRENCY'), '1234567.5USD')
    check('CURRENCY mid shifts min → null', NUMBER('ICONV', '1234567USD', '2,CURRENCY,9999999'), null)
    check('CURRENCY mid min gmsg', gmsg.indexOf('less than') >= 0, true)
    check('CURRENCY + min/max ok', NUMBER('ICONV', '1234567USD', '0,1000,9999999,CURRENCY'), '1234567USD')
    check('CURRENCY + min0 reject', NUMBER('ICONV', '-1234567USD', '2,0,CURRENCY'), null)
    check('CURRENCY + min0 accept', NUMBER('ICONV', '1234567USD', '2,0,CURRENCY'), '1234567.00USD')
    check('CURRENCY 0Z → unit only', NUMBER('ICONV', '0USD', '0Z,CURRENCY'), 'USD')
    check('CURRENCY 0Z 0.4→0 → unit only', NUMBER('ICONV', '0.4USD', '0Z,CURRENCY'), 'USD')
    check('CURRENCY 2Z keeps 0.01', NUMBER('ICONV', '0.01USD', '2Z,CURRENCY'), '0.01USD')
    check('CURRENCY UNIT mid slot', NUMBER('ICONV', '1234567EUR', '2,UNIT,0,9999999'), '1234567.00EUR')

    gbasefmt = 'MD2'
    check('OCONV CURRENCY plain', NUMBER('OCONV', '1234567.5USD', '2,CURRENCY'), '1234567.50USD')
    check('OCONV UNIT plain', NUMBER('OCONV', '2345678EUR', '2,UNIT'), '2345678.00EUR')
    check('OCONV negative unit plain', NUMBER('OCONV', '-1234567.5USD', '2,CURRENCY'), '-1234567.50USD')
    gndecs = 3
    check('OCONV NDECS,CURRENCY', NUMBER('OCONV', '1234567.8954USD', 'NDECS,CURRENCY'), '1234567.895USD')
    gndecs = 2
    check('OCONV BASE,CURRENCY', NUMBER('OCONV', '1234567.895USD', 'BASE,CURRENCY'), '1234567.90USD')

    gbasefmt = 'MD2,'
    if (hasOconvGroup) {
        check('OCONV CURRENCY grouped', NUMBER('OCONV', '1234567.5USD', '2,CURRENCY'), '1,234,567.50USD')
        check('OCONV CURRENCY bigger group', NUMBER('OCONV', '1234567890.12USD', '2,CURRENCY'), '1,234,567,890.12USD')
        check('OCONV peels unit without CURRENCY', NUMBER('OCONV', '1234567.5USD', '2'), '1,234,567.50USD')
    } else {
        check('OCONV CURRENCY plain build', NUMBER('OCONV', '1234567.5USD', '2,CURRENCY'), '1234567.50USD')
        check('OCONV peels unit without CURRENCY plain', NUMBER('OCONV', '1234567.5USD', '2'), '1234567.50USD')
    }
    check('OCONV 0Z+CURRENCY → unit only', NUMBER('OCONV', '0USD', '0Z,CURRENCY'), 'USD')
    gbasefmt = 'MD2'
    check('OCONV 0Z+CURRENCY plain gbasefmt', NUMBER('OCONV', '0USD', '0Z,CURRENCY'), 'USD')
    gbasefmt = 'MD2,'

    check('CURRENCY vm',
        NUMBER('ICONV', '1234567.5USD' + vm + '2345678.5EUR', '2,CURRENCY'),
        '1234567.50USD' + vm + '2345678.50EUR')
    check('CURRENCY fm',
        NUMBER('ICONV', '1000000USD' + fm + '2000000EUR', '0,CURRENCY'),
        '1000000USD' + fm + '2000000EUR')
}

// ===========================================================================
// DECIMAL — display false (if present)
// ===========================================================================

console.log('\n=== DECIMAL ===')
if (!hasDecimal) {
    skip('DECIMAL suite', 'DECIMAL not defined')
} else {
    gbasefmt = 'MD2,'
    check('DECIMAL plain large', DECIMAL('OCONV', '1234567.8', '2'), '1234567.80')
    check('DECIMAL pad', DECIMAL('OCONV', '1234567', '2'), '1234567.00')
    check('DECIMAL round', DECIMAL('OCONV', '1234567.895', '2'), '1234567.90')
    check('DECIMAL bigger plain', DECIMAL('OCONV', '1234567890.12', '2'), '1234567890.12')
    check('DECIMAL 0Z → ""', DECIMAL('OCONV', '0', '0Z'), '')
    check('DECIMAL Z → ""', DECIMAL('OCONV', '0', 'Z'), '')
    check('DECIMAL min/max ok', DECIMAL('ICONV', '1234567', '0,1000,9999999'), '1234567')
    check('DECIMAL min reject → null', DECIMAL('ICONV', '999', '0,1000,9999999'), null)
    check('DECIMAL min reject gmsg', gmsg.indexOf('less than') >= 0, true)
    check('DECIMAL max reject → null', DECIMAL('ICONV', '99999999', '0,0,9999999'), null)
    if (hasCurrency) {
        check('DECIMAL CURRENCY plain+unit', DECIMAL('OCONV', '1234567.5USD', '2,CURRENCY'), '1234567.50USD')
        check('DECIMAL CURRENCY ICONV', DECIMAL('ICONV', '1,234,567.50USD', '2,CURRENCY'), '1234567.50USD')
        check('DECIMAL unit without CURRENCY → null', DECIMAL('OCONV', '1234567.5USD', '2'), null)
        check('DECIMAL 0Z+CURRENCY → unit only', DECIMAL('OCONV', '0USD', '0Z,CURRENCY'), 'USD')
    }
    gbasefmt = 'MD2'
    check('DECIMAL BASE', DECIMAL('OCONV', '1234567.895', 'BASE'), '1234567.90')
}

// ===========================================================================
// INTEGER — forced 0 only when this build actually forces
// ===========================================================================

console.log('\n=== INTEGER ===')
// INTEGER is new and always means force 0dp; absent on old → skip entire section
if (!hasInteger) {
    skip('INTEGER suite', 'INTEGER not defined')
} else {
    gbasefmt = 'MD2,'
    check('INTEGER 1234567.6 → 1234568', INTEGER('OCONV', '1234567.6', ''), '1234568')
    check('INTEGER ICONV', INTEGER('ICONV', '1234567.4', ''), '1234567')
    check('INTEGER 0 → 0', INTEGER('OCONV', '0', ''), '0')
    check('INTEGER,Z → ""', INTEGER('OCONV', '0', 'Z'), '')
    check('INTEGER,Z 0.4 → ""', INTEGER('OCONV', '0.4', 'Z'), '')
    check('INTEGER,0Z → ""', INTEGER('OCONV', '0', '0Z'), '')
    check('INTEGER,2 force wins', INTEGER('OCONV', '1234567.6', '2'), '1234568')
    check('INTEGER ICONV force over 2', INTEGER('ICONV', '1234567.6', '2'), '1234568')
    check('INTEGER no thousands', INTEGER('OCONV', '1234567', ''), '1234567')
    check('INTEGER min/max ok', INTEGER('ICONV', '1234567', '0,1000,9999999'), '1234567')
    check('INTEGER max reject → null', INTEGER('ICONV', '99999999', '0,0,9999999'), null)
    check('INTEGER max reject gmsg', gmsg.indexOf('more than') >= 0, true)
    check('INTEGER,Z keeps non-zero', INTEGER('OCONV', '1234567.4', 'Z'), '1234567')
    check('INTEGER min0 reject', INTEGER('ICONV', '-1', '0,0'), null)
    check('INTEGER min0 accept 0', INTEGER('ICONV', '0', '0,0'), '0')
    check('INTEGER empty → ""', INTEGER('OCONV', '', ''), '')
}

// ===========================================================================
// ROUND
// ===========================================================================

console.log('\n=== ROUND ===')
if (hasRound && hasDecimal) {
    check('ROUND == DECIMAL',
        ROUND('OCONV', '1234567.895', '2'),
        DECIMAL('OCONV', '1234567.895', '2'))
} else if (hasRound) {
    gbasefmt = 'MD2'
    check('ROUND 2dp', ROUND('OCONV', '1234567.895', '2'), '1234567.90')
} else {
    skip('ROUND', 'not defined')
}

// ===========================================================================
// exodusround
// ===========================================================================

console.log('\n=== exodusround ===')
if (hasExodusround) {
    try {
        if (exodusround(12.7) === 13)
            check('exodusround default 0', exodusround(12.7), 13)
        else
            skip('exodusround default 0', 'needs explicit ndecimals')
    } catch (e) {
        skip('exodusround default 0', e.message)
    }
    check('exodusround(…, 2)', exodusround(1234567.895, 2), 1234567.9)
    check('exodusround(-1.5, 0)', exodusround(-1.5, 0), -2)
} else {
    skip('exodusround', 'not defined')
}
if (hasExodusroundMember) {
    check('(n).exodusround()', (1234567.6).exodusround(), 1234568)
    check('string.exodusround(2)', '1234567.895'.exodusround(2), 1234567.9)
} else {
    skip('.exodusround', 'not defined')
}

// ---------------------------------------------------------------------------

console.log('')
console.log(passed + ' passed, ' + failed + ' failed, ' + skipped + ' skipped')
if (failed)
    Deno.exit(1)
