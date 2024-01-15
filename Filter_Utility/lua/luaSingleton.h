
#ifndef LuaSingleton_H_
#define LuaSingleton_H_

#include "luaApi.h"
#include "luaData.h"
#include "luaMath.h"
#include <common/frame.h>

namespace precitec {
namespace filter {
namespace lua {

/*
General purpose library for singleton objects that are created in C++ only and
are accessible in LUA via global variables.
*/
template<typename Target>
class LuaSingleton
{
public:
	LUA_OBJECT_HEADER(Target)

	static void registerMetatable(lua_State *luaState)
	{
		lua_newtable(luaState);
		lua_setglobal(luaState, LUA_NAME);

		luaL_newmetatable(luaState, LUA_LIBRARY);

		lua_pushvalue(luaState, -1);
		lua_setfield(luaState, -2, "__index");

		registerMethods(luaState);
		lua_pop(luaState, 1);
	}
};

} // namespace lua
} // namespace filter
} // namespace precitec

#endif /*LuaSingleton_H_*/