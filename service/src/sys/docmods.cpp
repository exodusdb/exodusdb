#include <exodus/library.h>
libraryinit()

function main(in letterheadopts, io tx) {
	//c sys in,io

	//html2pdf.exe messes up repeating headers on continuation pages
	//so turn thead into additional tbody
	if (not(letterheadopts.isnum())) {

		//swap 'THEAD>' with 'thead>' in tx
		//swap 'TBODY>' with 'body>' in tx
		//swap '<thead>' with '' in tx
		//swap '<tbody>' with '' in tx
		//swap '</thead>' with '' in tx
		//swap '</tbody>' with '' in tx

		tx.swapper("<THEAD", "<tbody");
		tx.swapper("</THEAD", "</tbody");
		tx.swapper("<thead", "<tbody");
		tx.swapper("</thead", "</tbody");

	}

	return 0;
}

libraryexit()
