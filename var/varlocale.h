#include <boost/locale.hpp>

namespace exo {

//// exodus uses one locale per thread instead of global
//inline thread_local std::locale thread_boost_locale1;
//
//inline void init_boost_locale1() {
//	if (thread_boost_locale1.name() != "*") {
//		boost::locale::generator generator1;
//		thread_boost_locale1 = generator1("");
//	}
//}

}  // namespace exo
