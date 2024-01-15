
#ifndef LuaConstants_H_
#define LuaConstants_H_

#include "luaApi.h"
#include "luaData.h"

namespace precitec {
namespace filter {
namespace lua {

/*
General purpose object type for constants. Doesn´t allow assignment, read-only.
*/
template<typename Primitive>
class LuaConstants
{
public:
	LUA_OBJECT_HEADER(LuaDictionary<Primitive>)

	/// Declarations to be defined by the relevant structure
	static void pushConstant(lua_State *luaState, const Primitive& constant);
	///

	static void registerConstants(lua_State *luaState, const char* name, const Type& constants)
	{
		lua_createData<LuaConstants>(luaState, constants);
		lua_setglobal(luaState, name);
	}

private:
	static int getConstant(lua_State *luaState)
	{
		const Type &constants = lua_checkData<LuaConstants>(luaState, 1); 	// arg1: constants

		const char* key = luaL_checkstring(luaState, 2);					// arg2: key
		const auto it = constants.find(key);
		luaL_argcheck(luaState, it != constants.end(), 2, lua_sprintf("Constant '%s' does not exist", key).c_str());

		pushConstant(luaState, it->second);
		return 1;
	}

	static int setConstantError(lua_State *luaState)
	{
		luaL_error(luaState, "Setting a constant is not allowed");
		return 0;
	}

public:
	static void registerMetatable(lua_State *luaState)
	{
		lua_newtable(luaState);
		lua_setglobal(luaState, LUA_NAME);

		luaL_newmetatable(luaState, LUA_LIBRARY);
		lua_registerMethod(luaState, "__index", getConstant);
		lua_registerMethod(luaState, "__newindex", setConstantError);

		registerMethods(luaState);
		lua_pop(luaState, 1);
	}
};

// We define common constant types here as they are going to be shared
using LuaIntegerConstants = LuaConstants<int>;
using LuaNumberConstants = LuaConstants<double>;

} // namespace lua
} // namespace filter
} // namespace precitec

#endif /*LuaConstants_H_*/