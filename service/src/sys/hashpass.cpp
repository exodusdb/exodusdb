#include <exodus/library.h>
libraryinit()

function main(in password) {

	var encryptkey = 1234567;

	var hashedpassword = password;

	//pass1
	while (true) {
		///BREAK;
		if (not(hashedpassword ne ""))
			break;
		encryptkey = (encryptkey.mod(390001)) * (hashedpassword[1]).seq() + 1;
		hashedpassword.cutter(1);
	}  //loop;

	//pass2
	while (true) {
		hashedpassword ^= chr(65 + (encryptkey.mod(50)));
		encryptkey = (encryptkey / 50).floor();
		///BREAK;
		if (not encryptkey)
			break;
	}  //loop;

	return hashedpassword;
}

libraryexit()
