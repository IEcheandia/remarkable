
#ifndef LuaFrame_H_
#define LuaFrame_H_

#include "luaApi.h"
#include "luaMath.h"
#include "luaContext.h"
#include <common/frame.h>

namespace precitec {
namespace filter {
namespace lua {

/*
General purpose library for Frame objects that have context and analysis result members.
*/
template<typename Content>
class LuaFrame
{
public:
	LUA_OBJECT_HEADER(interface::Frame<Content>)

	/// Declarations to be defined by the relevant structure
	static int newContent(lua_State *luaState);
	static int toString(lua_State *luaState);
	///

private:
	static int result(lua_State *luaState)
	{
		lua_checkArgNums(luaState, {1, 2});
		const int n = lua_gettop(luaState);
		Type *content = lua_checkObject<LuaFrame>(luaState, 1);	// arg1: content
		if(n == 1)
		{
			lua_pushinteger(luaState, content->analysisResult());
			return 1;
		}

		const int result = luaL_checkinteger(luaState, 2); 		// arg2: result
		luaL_argcheck(luaState, 0 <= result && result < interface::ResultType::EndOfResultTypes, 2, "invalid result type");
		content->setAnalysisResult((interface::ResultType)result);
		return 0;
	}

	static int context(lua_State *luaState)
	{
		lua_checkArgNums(luaState, {1});
		Type *content = lua_checkObject<LuaFrame>(luaState, 1);	// arg1: content
		lua_createContext(luaState, content->context());
		return 1;
	}

public:
	static void registerMetatable(lua_State *luaState)
	{
		lua_newtable(luaState);
		lua_pushcfunction(luaState, newContent);
		lua_setfield(luaState, -2, "new");
		lua_setglobal(luaState, LUA_NAME);

		luaL_newmetatable(luaState, LUA_LIBRARY);

		lua_pushvalue(luaState, -1);
		lua_setfield(luaState, -2, "__index");

		lua_registerMethod(luaState, "__gc", lua_gcObject<LuaFrame>);

		lua_registerMethod(luaState, "__tostring", toString);
		lua_registerMethod(luaState, "result", result);
		lua_registerMethod(luaState, "context", context);

		registerMethods(luaState);
		lua_pop(luaState, 1);
	}
};

} // namespace lua
} // namespace filter
} // namespace precitec

#endif /*LuaFrame_H_*/