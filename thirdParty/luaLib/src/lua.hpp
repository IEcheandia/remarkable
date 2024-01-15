// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

extern "C" {
#include "../include/lua.h"
#include "../include/lualib.h"
#include "../include/lauxlib.h"
}
