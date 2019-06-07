#include <boost/locale.hpp>

namespace exodus
{

// exodus uses one locale per thread instead of global
inline thread_local std::locale tls_boost_locale1;

inline void init_boost_locale1()
{
	if (tls_boost_locale1.name() != "*")
	{
		boost::locale::generator generator1;
		tls_boost_locale1 = generator1("");
	}
}

} // namespace exodus
