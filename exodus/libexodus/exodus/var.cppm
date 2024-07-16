//module;
//export module exo;
//export import :var;
//export import :dim;

module;

#include <exodus/var.h>
#include <exodus/dim.h>

export module var;

namespace exo {
    export using exo::var;
    export using exo::dim;
}
