#include "luaContext.h"
#include "luaData.h"
#include "module/moduleLogger.h"

namespace precitec {
namespace filter {
namespace lua {

LuaContext::Type *lua_createContext(lua_State *luaState, const interface::ImageContext& context)
{
	return lua_createData<LuaContext>(luaState, &context);
}

LuaContext::Type lua_compareContext(lua_State *luaState, const interface::ImageContext& a, const interface::ImageContext& b)
{
	if(a != b)
	{
		luaL_error(luaState, "Can't handle objects with different context");
	}
	return &a;
}

template<>
const char* LuaContext::LUA_NAME = "Context";

template<>
const char* LuaContext::LUA_LIBRARY = "Filter.context";

template<>
void LuaContext::registerMethods(lua_State *luaState)
{
}

} // namespace lua
} // namespace filter
} // namespace precitec