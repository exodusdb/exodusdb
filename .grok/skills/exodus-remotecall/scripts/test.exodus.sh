#!/bin/bash
[ "$INSTALL_SH" ] || [ $0 = bash ] || {
	export INSTALL_SH="$(echo ${0##*/}.$*.$(date +%H%M%S).log | tr ' :@' '.__')"
	exec bash "$0" "$@" |& tee -a "${INSTALL_SH}"
	exit "${PIPESTATUS[0]}"
}
set -euxo pipefail
PS4='+ [`date +%H:%M:%S` test.exodus.sh:${LINENO} ${SECONDS}s] '
: $0 "$@"

function main() {
:
: "./test.exodus.sh <fqdn> [https_port=443] [dataset=test] [username=exodus] [passfile=$HOME/.ssh/\${dataset}|exodus]"
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
: "e.g. ./test.exodus.sh c2comms.hosts.neosys.com 44325 c2comms"
:
: "Never load the password value into a bash variable where it might be logged."
:
: Command line
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
	fqdn="${1:?fqdn is required. e.g. test.hosts.neosys.com}"
	https_port="${2:-443}"
	dataset="${3:-test}"
	username="${4:-exodus}"
	passfile="${5:-${HOME}/.ssh/${dataset}}"
:
: Config
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
	test_id="${dataset}.${https_port}.$$"
	https="https://$fqdn:$https_port"
	api_url="${https}/3/exodus/scripts/xhttp.php"
:
	cookie_file=/tmp/$test_id.cookies.txt
:
	login_request="LOGIN|$username|\$pass|$dataset||ADAGENCY||newpass"
	login_request=${login_request//|/%0D}
	login_response_regex="<response>OK</response>"
:
	test_request="SELECT|CURRENCIES|BY CURRENCY_CODE|CURRENCY_CODE CURRENCY_NAME|XML|3"
	test_request=${test_request// /%20}
	test_request=${test_request//|/%0D}
	test_response_regex="/CURRENCY_NAME"
:
	data_raw="<root><token>${dataset}*${username}*</token><timeout></timeout><request>API_REQUEST</request><data></data></root>"
	login_data_raw="${data_raw/API_REQUEST/$login_request}"
	test_data_raw="${data_raw/API_REQUEST/$test_request}"
:
: curl options
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
		curl_opt=(--no-progress-meter)
#		curl_opt+=(-H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:151.0) Gecko/20100101 Firefox/151.0')
		curl_opt+=(-H 'Content-Type: text/plain;charset=UTF-8')
#		curl_opt+=(-H 'Origin: ${https}')
#		curl_opt+=(-H 'Referer: ${https}/3/exodus/users.htm')
:
: Validate pass file exists
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
	if ! [[ -s "$passfile" ]]; then
		passfile=~/.ssh/exodus
	fi
	if ! [[ -s "$passfile" ]]; then
		echo "ERROR: Password for $username required in $HOSTNAME $passfile (chmod 600)" >&2
		exit 1
	fi
	chmod 600 "$passfile"
:
: Create raw data template for login post
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
: Template with \$pass as placeholder - other values are non-secret.
	templatefile="/tmp/$test_id.template.txt"
	echo "$login_data_raw" > "$templatefile"
:
: Prepare a temp file to contain raw data to post
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
	rawdatafile=$(mktemp)
	chmod 600 "$rawdatafile"
	trap 'rm -f "$rawdatafile" 2>/dev/null || true' EXIT
:
: Use pass file and template file to create raw data file
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
:
	awk '
	  NR == FNR {
	    val = val (val ? "\n" : "") $0
	    next
	  }
	  {
	    gsub(/\$pass/, val)
	    print
	  }
	' "$passfile" "$templatefile" > "$rawdatafile"
:
	rm -f "$templatefile"
:
: Login
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
	outfile="/tmp/$test_id.out"
	curl ${api_url}                   \
		--cookie-jar $cookie_file     \
		--cookie $cookie_file         \
		"${curl_opt[@]}"              \
		--data-binary "@$rawdatafile" \
	| tee $outfile
:
:
: Validate login response
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
# <root><data>m=SUPPORT%2CFINANCE%2CMEDIAANALYSIS%2CMEDIA%2CJOBS%2CTIMESHEETS%2CHELP%26cc=1%26nc=4%26pd=7/2026%26bc=AED%26bf=MD20P%2C%26mk=UAE%26mc=AED%26tz=14400%26ms=1000000%26db=C2%20Comms%0A%26sp=1%26fc=%26ff=%26fs=%26df=31/01/90%26fd=1%26wn=%26ap=ADAGENCY</data><response>OK</response><result>1</result></root>
	if ! grep -q $login_response_regex $outfile; then
		: Error: Unexpected login response in $outfile
	fi
:
: Login done. Now you can use --cookie $cookie_file
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
: Clean up
:
	rm -f "$rawdatafile"
	trap - EXIT
:
: Test listen select
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
	curl ${api_url}                 \
		--cookie $cookie_file       \
		"${curl_opt[@]}"            \
		--data-raw "$test_data_raw" \
	| tee $outfile
:
:
: Validate response
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
	if ! grep -q $test_response_regex $outfile; then
		: Error: Unexpected response in $outfile
	fi
:
: Finished $0 "$@"
: ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
#<root><data>%3Crecords%3E%0A%3CRECORD%3E%0A%3CCURRENCY_CODE%3EAED%3C/CURRENCY_CODE%3E%0A%3CCURRENCY_NAME%3EUAE%20Dirhams%3C/CURRENCY_NAME%3E%0A%3C/RECORD%3E%0A%3CRECORD%3E%0A%3CCURRENCY_CODE%3EAS%3C/CURRENCY_CODE%3E%0A%3CCURRENCY_NAME%3EAustrian%20Schillings%3C/CURRENCY_NAME%3E%0A%3C/RECORD%3E%0A%3CRECORD%3E%0A%3CCURRENCY_CODE%3EASD%3C/CURRENCY_CODE%3E%0A%3CCURRENCY_NAME%3EAustralian%20Dollars%3C/CURRENCY_NAME%3E%0A%3C/RECORD%3E%0A%3CRECORD%3E%0A%3CCURRENCY_CODE%3EBAH%3C/CURRENCY_CODE%3E%0A%3CCURRENCY_NAME%3EBAHT%3C/CURRENCY_NAME%3E%0A%3C/RECORD%3E%0A%3CRECORD%3E%0A%3CCURRENCY_CODE%3EBD%3C/CURRENCY_CODE%3E%0A%3CCURRENCY_NAME%3EBahrain%20Dinars%3C/CURRENCY_NAME%3E%0A%3C/RECORD%3E%0A%3CRECORD%3E%0A%3CCURRENCY_CODE%3ECNY%3C/CURRENCY_CODE%3E%0A%3CCURRENCY_NAME%3ECHINESE%20YUAN%3C/CURRENCY_NAME%3E%0A%3C/RECORD%3E%0A%3CRECORD%3E%0A%3CCURRENCY_CODE%3ECYP%3C/CURRENCY_CODE%3E%0A%3CCURRENCY_NAME%3ECyprus%20Pounds%3C/CURRENCY_NAME%3E%0A%3C/RECORD%3E%0A%3CRECORD%3E%0A%3CCURRENCY_CODE%3EDFL%3C/CURRENCY_CODE%3E%0A%3CCURRENCY_NAME%3EDUTCH%20GUILDERS%3C/CURRENCY_NAME%3E%0A%3C/RECORD%3E%0A%3CRECORD%3E%0A%3CCURRENCY_CODE%3EDKK%3C/CURRENCY_CODE%3E%0A%3CCURRENCY_NAME%3EDanish%20kroner%3C/CURRENCY_NAME%3E%0A%3C/RECORD%3E%0A%3CRECORD%3E%0A%3CCURRENCY_CODE%3EDM%3C/CURRENCY_CODE%3E%0A%3CCURRENCY_NAME%3EDeutschmark%3C/CURRENCY_NAME%3E%0A%3C/RECORD%3E%0A%3C/records%3E</data><response>OK</response><result>1</result></root>
} # function main

main "$@"