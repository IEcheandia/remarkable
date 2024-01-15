#include "luaConstants.h"

namespace precitec {
namespace filter {
namespace lua {

template<>
const char* LuaIntegerConstants::LUA_NAME = "IntegerConstants";

template<>
const char* LuaIntegerConstants::LUA_LIBRARY = "filter.integerConstants";

template<>
void LuaIntegerConstants::pushConstant(lua_State *luaState, const int& constant)
{
	lua_pushinteger(luaState, constant);
}

template<>
void LuaIntegerConstants::registerMethods(lua_State *luaState)
{
}

template<>
const char* LuaNumberConstants::LUA_NAME = "NumberConstants";

template<>
const char* LuaNumberConstants::LUA_LIBRARY = "filter.numberConstants";

template<>
void LuaNumberConstants::pushConstant(lua_State *luaState, const double& constant)
{
	lua_pushnumber(luaState, constant);
}

template<>
void LuaNumberConstants::registerMethods(lua_State *luaState)
{
}

} // namespace lua
} // namespace filter
} // namespace precitec