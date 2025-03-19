#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {

    printl("test_variter says 'Hello World!'");

	{
		// INCREMENTING ITER (var_iter)

		var rec = "^aa^^cc^"_var;
		auto i = rec.begin();

		TRACE(*i)//
		assert(*i eq "");
		++i;
		TRACE(*i)//aa
		assert(*i eq "aa");
		++i;
		TRACE(*i)//
		assert(*i eq "");
		++i;
		TRACE(*i)//cc
		assert(*i eq "cc");
		++i;
		TRACE(*i)//
		assert(*i eq "");
		//++i; //throws
		//TRACE(*i);//

		//DECREMENTING ITER

		--i;
		TRACE(*i)//cc
		assert(*i eq "cc");
		--i;
		TRACE(*i)//
		assert(*i eq "");
		--i;
		TRACE(*i)//aa
		assert(*i eq "aa");
		--i;
		TRACE(*i)//
		assert(*i eq "");
		//--i; //throws
		//TRACE(*i);//
	}

	{
		// Check we can increase fields sizes
		var v = "1a^2b^3c"_var;
		var fn = 0;
		for (var l : v) {
			fn++;
			v(fn) = l ^ "x";
		}
		assert(v == "1ax^2bx^3cx"_var);
	}

	{
		// Check we can decrease fields size
		var v = "1a^2b^3c"_var;
		var fn = 0;
		for (var l : v) {
			fn++;
			v(fn) = l.cut(1);
		}
		assert(v == "a^b^c"_var);
	}

	{
		printl("Check we can remove fields with a manual backup of an iterator. INCLUDING field 1!");
		var v = "1^2^3^4^5^6^7^8^9"_var;
		var fn = 0;
		for (auto iter = v.begin(); iter != v.end(); ++iter) {
			fn++;
			if (*iter > 1 and *iter <= 7) {
				v.remover(fn);
				iter--; // backup on begin() is strange to variter. It creates an iter that == end() and ++ creates a begin() iter,
				fn--;
			}
		}
		TRACE(v)
		assert(v == "1^8^9"_var);
	}

	{
		printl("Check we can remove fields with complete manual control of an iterator 1.");
		var v = "1^2^3^4^5^6^7^8^9"_var;
		var fn = 0;
//		for (auto iter = v.begin(); iter != v.end();) {
		auto iter = v.begin();
		while(iter != v.end()) {
			fn++;
			TRACE(fn)
			TRACE(v)
			if (*iter < 3 or *iter > 8) {
				v.remover(fn);
				fn--;
				continue;
			}
			// Only bump the iterator when we do NOT delete a field
			// and when we have not deleted the last field!
			if (iter != v.end())
				++iter;
		}
		TRACE(v)
		assert(v == "3^4^5^6^7^8"_var);
	}

	{
		printl("Check we can remove fields with for and iterators (BEST WAY)");
		var v = "1^2^3^4^5^6^7^8^9"_var;
		var fn = 0;
		for (auto iter = v.begin(); iter != v.end(); ++iter) {
			fn++;
			TRACE(fn)
			TRACE(v)
			TRACE(*iter)
			if (*iter < 3 or *iter > 8) {
				v.remover(fn);
				// if on the beginning var_iter returns an iter with startpos = npos (i.e. an end iterator)
				// and the variter++ will convert it into a begin() iter. NICE!
				iter--;
				fn--;
				continue;
			}
		}
		TRACE(v)
		assert(v == "3^4^5^6^7^8"_var);
	}

	{
        printl("Check we ARE able to remove fields with range based for loop iterator over var");
        printl("ONLY IF you start manually accessing fields in the middle");

		var v = "1^2^3^4^5^6^7^8^9"_var;
		var fn = 0;
		var nfs = v.fcount(FM);

		for (var l : v) {
			fn++;

			// We must start processing all following elements as well since we cannot backup the iterator
			// when using "for var v : fields"
			while (l < 3 or l > 8) {
				v.remover(fn);
				l = v.f(fn);
				nfs--;
				if (fn > nfs)
					break;
			}

		}
		TRACE(v)
		assert(v == "3^4^5^6^7^8"_var);
//		assert(v == "2^4^5^6^7^9"_var);
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
