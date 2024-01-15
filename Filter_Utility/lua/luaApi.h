
#ifndef Lua_H_
#define Lua_H_

#include <string>
#include <unordered_set>

#ifdef __cplusplus
extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}
#endif //__cplusplus

namespace precitec {
namespace filter {
namespace lua {

// Compile LUA code to binary string
std::string lua_compile(const std::string& code);

// Create LUA state
lua_State* lua_create();

// Load LUA binaries into a state
void lua_load(lua_State* luaState, const std::string& luaBinaries, const std::string& name);

// Run LUA function that is currently on the stack
void lua_run(lua_State* luaState, bool& hasError);

// Register object method for a LUA object
void lua_registerMethod(lua_State* luaState, const char* name, lua_CFunction func);

// Shorthand for a limited size sprintf call
std::string lua_sprintf(const char* text, ...);

// Helper function to check the number or arguments in the current function and throw an error if not valid
void lua_checkArgNums(lua_State* luaState, const std::unordered_set<int>& validArgNums);

} // namespace lua
} // namespace filter
} // namespace precitec

#endif /*Lua_H_*/
