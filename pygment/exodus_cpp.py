# /root/exodus_lexers/exodus_cpp.py
from pygments.lexers.c_cpp import CppLexer
from pygments.token import Keyword, Name, Operator
import re

class ExodusCppLexer(CppLexer):
    name = 'ExodusCpp'
    aliases = ['exoduscpp', 'exodus_cpp']
    filenames = ['*.cpp', '*.cxx', '*.cc']
    mimetypes = ['text/x-exoduscpp']

#    'ExoProgram', '_ExoProgram',
#    'ExoProgram',

    EXTRA_FUNCTIONS = {
        'abs', 'append', 'appender',
        'assigned', 'at', 'atan', 'attach', 'backtrace', 'begin', 'begintrans',
        'breakoff', 'breakon', 'c_str',
        # 'call',
        'chr', 'clearcache', 'clearfile',
        'clearselect', 'clone', 'close', 'committrans', 'connect', 'contains',
        'convert', 'converter', 'cos', 'count', 'createString', 'createfile',
        'createindex', 'createstring', 'crop', 'cropper', 'cut', 'cutter', 'data',
        'date', 'dbcopy', 'dbcreate', 'dbcursorexists', 'dbdelete', 'dblist', 'debug',
        'defaulter', 'deletec', 'deletefile', 'deleteindex', 'deletelist',
        'deleterecord', 'detach', 'dim', 'dim_iter', 'disconnect', 'disconnectall',
        'dump', 'echo', 'empty', 'end', 'ends', 'eof', 'errput', 'errputl',
        'exo_backtrace', 'exo_savestack', 'exodus_main', 'exp', 'extract', 'f',
        'fcase', 'fcaser', 'fcount', 'field', 'field2', 'fieldstore', 'fieldstorer',
        'first', 'firster', 'floor', 'flushindex', 'format',
        'from_codepage',
        'from_u32tring', 'getexecpath', 'gethostname', 'getlist', 'getprecision',
        'getprocessn', 'getprompt', 'getxlocale', 'hash', 'hasinput', 'hasnext',
        'iconv', 'iconv_D', 'iconv_HEX', 'iconv_MT', 'iconv_TX', 'index', 'indexn',
        'indexr', 'initrnd', 'input', 'inputn', 'insert', 'inserter', 'insertrecord',
        'integer', 'into', 'invert', 'inverter', 'isnum', 'isterminal', 'join',
        'keypressed', 'last', 'laster', 'lasterror', 'lcase', 'lcaser', 'len', 'let',
        'listfiles', 'listindex', 'load', 'localeAwareCompare', 'listed', 'locate', 'locateby',
        'locatebyusing', 'locateusing', 'lock', 'loge', 'loglasterror', 'logput',
        'logputl', 'lower', 'lowerer', 'match', 'mod', 'move', 'multivalued', 'mv',
        'normalize', 'normalizer', 'num', 'numberinwords', 'oconv', 'oconv_D',
        'oconv_HEX', 'oconv_LRC', 'oconv_MD', 'oconv_MR', 'oconv_MS', 'oconv_MT',
        'oconv_T', 'oconv_TX',
        'open', 'operator', 'or_default', 'ord',
        'osbread', 'osbwrite', 'osclose', 'oscopy', 'oscwd', 'osdir', 'osfile',
        'osflush', 'osgetenv', 'osinfo', 'oslist', 'oslistd', 'oslistf', 'osmkdir',
        'osmove', 'osopen', 'osopenx', 'ospid', 'osread', 'osremove', 'osrename',
        'osrmdir', 'ossetenv', 'osshell', 'osshellread', 'osshellwrite', 'ossleep',
        'ostempdir', 'ostempfile', 'ostid', 'ostime', 'ostimestamp', 'oswait',
        'oswrite', 'output', 'outputl', 'outputt', 'parse', 'parser', 'paste',
        'pasteall', 'paster', 'pasterall', 'pop', 'popper', 'prefix', 'prefixer',
        'print', 'printl', 'println', 'printt', 'printx', 'put', 'pwr', 'quote',
        'quoter', 'raise', 'raiser', 'range', 'read', 'readc', 'readf', 'readnext',
        'reccount', 'remove', 'remover', 'renamefile', 'replace', 'replacer',
        'reverse', 'reverse_range', 'reverser', 'rex', 'rnd', 'rollbacktrans', 'round',
        'savelist', 'search', 'select', 'selectkeys', 'selectx', 'setlasterror',
        'setprecision', 'setprompt', 'setxlocale', 'randomize', 'randomizer', 'sin',
        'sort', 'sorter', 'space', 'split', 'sqlexec', 'sqrt', 'squote', 'squoter',
        'starts', 'statustrans', 'str', 'substr', 'substr2', 'substr3', 'substrer',
        'sum', 'sumall', 'swap', 'tan', 'tcase', 'tcaser', 'textchr', 'textchrname',
        'textconvert', 'textconverter', 'textlen', 'textord', 'textwidth', 'time',
        'toBool', 'toChar', 'toDouble', 'toInt', 'toInt64', 'toString',
        'to_codepage', 'to_u32string', 'to_wstring', 'trim', 'trimboth', 'trimfirst',
        'trimlast', 'trimmer', 'trimmerboth', 'trimmerfirst', 'trimmerlast', 'ucase',
        'ucaser', 'unassigned', 'unique', 'uniquer', 'unlock', 'unlockall', 'unquote',
        'unquoter', 'update', 'updatekey', 'updater', 'updaterecord',
        # 'from',
        # 'on',
        #        'to',
        # 'var',
        #        'var_base', 'var_iter', 'var_mid', 'var_proxy1', 'var_proxy2', 'var_proxy3',
        #        'varint_t',
        'version', 'write', 'writec', 'writef', 'xlate',
        # 'with'
        #        'dim', '_dim',
        'cols',
        # 'dim', 'getelementref', 'init',
        'join',
        # 'operator',
        'osread', 'oswrite', 'read', 'redim', 'reverser', 'rows', 'randomizer', 'sort',
        'sorter', 'split', 'splitter', 'write',

        'AT',
        'abort', 'abortall',
        'amountunit', 'at', 'calculate', 'chain', 'clearselect', 'debug', 'decide',
        'deletelist', 'deleterecord', 'elapsedtimetext', 'esctoexit', 'execute',
        'exoprog_date', 'exoprog_number', 'formlist', 'fsmsg', 'getcursor',
        'getdatetime', 'getlist', 'hasnext', 'iconv', 'invertarray', 'libinfo',
        'lockrecord', 'logoff', 'makelist', 'note', 'oconv', 'otherdatasetusers',
        'otherdatausers', 'otherusers', 'perform', 'popselect', 'pushselect',
        'readnext', 'savelist', 'select', 'selectkeys', 'setcursor', 'sortarray',
        'stop', 'timedate2', 'unlockrecord', 'xlate'
    } # EXODUS_FUNCTIONS  .nf

    EXTRA_TYPES = {
         'dim', 'var', '_var', 'rex', '_rex', 'let', 'in', 'out', 'io', 'qqqqqqq'
        'programexit', 'programinit',
        'libraryinit', 'libraryexit',
         'commoninit', 'commonexit',
        'dictinit', 'dictexit'
    }  # EXODUS_TYPES

    EXTRA_FLOW_CONTROL = {             # EXODUS_FLOWCONTROL  .kr reserved
        'stop', 'abort', 'abortall',
        'return', 'continue', 'break', 'throw', 'goto', 'co_return', 'co_yield', 'co_await'} #added c++ to get same color

    EXTRA_DECLARATIONS = {'func', 'subr', 'function', 'subroutine'}  # EXODUS_DECLARATIONS  .kd declaration

    EXTRA_OPERATORS = {
        'not', 'and', 'or', 'eq', 'ne', 'lt',
        'gt', 'le', 'ge', 'on', 'from', 'with', 'to'
    } # EXODUS_OPERATORS    normally a .k become purple .ow operator word

    EXTRA_KEYWORDS = {'call', 'gosub'}  # EXODUS_KEYWORDS  .k ? style

    EXTRA_LITERALS = {
# Some literal abbreviations.          Fixed in ASCII/Unicode.
'NL', 'BS', 'DQ', 'SQ',
'_NL', '_BS', '_DQ', '_SQ',
'NL_', 'BS_', 'DQ_', 'SQ_',

# Exodus literal abbreviations.        Fixed in Exodus.
'RM', 'FM', 'VM', 'SM', 'TM', 'STM',
'_RM', '_FM', '_VM', '_SM', '_TM', '_STM',
'RM_', 'FM_', 'VM_', 'SM_', 'TM_', 'STM_',
'_ALL_FMS',
'_VISIBLE_RM', '_VISIBLE_FM', '_VISIBLE_VM', '_VISIBLE_SM', '_VISIBLE_TM', '_VISIBLE_STM',
'VISIBLE_RM_', 'VISIBLE_FM_', 'VISIBLE_VM_', 'VISIBLE_SM_', 'VISIBLE_TM_', 'VISIBLE_STM_',
'_VISIBLE_FMS',

# CPU, OS, compiler and c++ info.      Fixed in Exodus build/release.

# Fixed in compiler and Exodus build options.
# e.g. "clang" "19" "24"
'_COMPILER', '_COMPILER_VERSION', '_CPP_STANDARD',

# Fixed in OS.
# e.g. "Ubuntu" "24.04"
'_OS_NAME', '_OS_VERSION',

# Fixed in CPU type: "x86" "x64".
'PLATFORM', '_PLATFORM',

# Fixed in OS type.
# Posix: "/"   Windows: "\"
'OSSLASH', '_OSSLASH', 'OSSLASH_',
# Posix: "\n"  Windows: "\r\n",
'EOL', '_EOL',
    }

    EXTRA_CONSTANTS = {
#unorganised atm
'DATEFMT', 'BASEFMT', 'TZ',
'EW', 'PW', 'VW', 'XW', 'MW', 'HW', 'AW',
'FILE', 'ID', 'RECORD', 'DICT', 'MV', 'ANS', 'PSEUDO', 'DATA', 'CURSOR',
'DEFINITIONS', 'SECURITY', 'SYSTEM', 'SESSION', 'TIMESTAMP',
'USERNAME', 'STATUS', 'LISTACTIVE',
'SENTENCE', 'EXECPATH', 'COMMAND', 'OPTIONS',
'USER0', 'USER1', 'USER2', 'USER3', 'USER4',
'RECUR0', 'RECUR1', 'RECUR2', 'RECUR3', 'RECUR4',
'APPLICATION',
'TERMINAL', 'LEVEL', 'THREADNO',
'CRTHIGH', 'CRTWIDE', 'LPTRHIGH', 'LPTRWIDE',
'STATION', 'PRIVILEGE', 'FILES', 'TCLSTACK', 'INTCONST', 'STATUS', 'COL1', 'COL2', 'PRIORITYINT', 'FILEERRORMODE', 'FILEERROR', 'RECCOUNT',
}  # EXODUS_CONSTANTS

#    EXTRA_CONSTANTS = {'true', 'false'}  # EXODUS_CONSTANTS
#    EXTRA_PSEUDO = {'system', 'env'}  # EXODUS_PSEUDO
    _exodus_rules = [
        (re.compile(r'\b(' + '|'.join(EXTRA_OPERATORS) + r')\b'), Operator.Word),
        (re.compile(r'\b(' + '|'.join(EXTRA_FUNCTIONS) + r')\b'), Name.Function),
        (re.compile(r'\b(' + '|'.join(EXTRA_DECLARATIONS) + r')\b'), Keyword.Declaration),
        (re.compile(r'\b(' + '|'.join(EXTRA_FLOW_CONTROL) + r')\b'), Keyword.Reserved),
        (re.compile(r'\b(' + '|'.join(EXTRA_TYPES) + r')\b'), Keyword.Type),
        (re.compile(r'\b(' + '|'.join(EXTRA_KEYWORDS) + r')\b'), Keyword),
        (re.compile(r'\b(' + '|'.join(EXTRA_LITERALS) + r')\b'), Name.Builtin), #.nb is like true/false i.e literals
        (re.compile(r'\b(' + '|'.join(EXTRA_CONSTANTS) + r')\b'), Keyword.Constant),

#        (re.compile(r'\b(' + '|'.join(EXTRA_PSEUDO) + r')\b'), Keyword.Pseudo),
    ]

    def get_tokens_unprocessed(self, text, stack=('root',)):
        for index, token, value in super().get_tokens_unprocessed(text, stack):
            matched = False
            for pattern, token_type in self._exodus_rules:
                if pattern.fullmatch(value):
                    yield index, token_type, value
                    matched = True
                    break
            if not matched:
                yield index, token, value
