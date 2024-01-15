/***
*	@file
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			FH
*	@date			2023
*	@brief			Lua scripting in editor
*/

#include "luaApi.h"
#include "module/moduleLogger.h"

namespace precitec {
namespace filter {
namespace lua {

int lua_codeWriter(lua_State *L, const void *p, size_t size, void *u)
{
	std::string* code = (std::string*)u;
	unsigned char* start = (unsigned char*)p;
	unsigned char* end = (unsigned char*)start+size;
	try {
		code->insert(code->end(), start, end);
		return 0;
	} catch (...) {
		return 1;
	}
}

const char* lua_codeReader (lua_State *L, void *u, size_t *size) {
	std::string* code = (std::string*)u;
	*size = code->size();
	return code->c_str();
}

/// Custom print function based on lbaselib.c
std::string lua_toString(lua_State *luaState)
{
	int n = lua_gettop(luaState);
	int i;
	lua_getglobal(luaState, "tostring");
	std::string out = "";
	for (i=1; i<=n; i++)
	{
		const char *s;
		lua_pushvalue(luaState, -1);
		lua_pushvalue(luaState, i);
		lua_call(luaState, 1, 1);
		s = lua_tostring(luaState, -1);
		if (s == NULL)
		{
			return "";
		}
		if (i>1)
		{
			out += "\t";
		}
		out += s;
		lua_pop(luaState, 1);
	}
	return out;
}

int lua_customPrint(lua_State *luaState)
{
	const std::string string = lua_toString(luaState);
	precitec::wmLog(precitec::LogType::eInfo, "LUA Print: %s\n", string.c_str());
	return 0;
}

int lua_fail(lua_State *luaState)
{
	const std::string string = lua_toString(luaState);
	precitec::wmLog(precitec::LogType::eDebug, "Received LUA error from user script\n");
	luaL_error(luaState, string.c_str());
	return 0;
}

std::string lua_compile(const std::string& code)
{
	lua_State* luaState = luaL_newstate();
	int res = luaL_loadstring(luaState, code.c_str());
	if (res != LUA_OK)
	{
		lua_close(luaState);
		wmLog(LogType::eError, "Loading LUA code failed!\n");
		return "";
	}

	std::string luaBinary = "";
	res = lua_dump(luaState, lua_codeWriter, (void*)&luaBinary, 0);
	if (res != LUA_OK)
	{
		lua_close(luaState);
		wmLog(LogType::eError, "Compiling LUA code failed!\n");
		return "";
	}

	lua_close(luaState);
	wmLog(LogType::eInfo, "LUA compilation complete!\n");
	return luaBinary;
}

lua_State* lua_create()
{
	lua_State* luaState = luaL_newstate();
	luaL_openlibs(luaState);
	return luaState;
}

void lua_load(lua_State* luaState, const std::string& luaBinaries, const std::string& name)
{
	lua_register(luaState, "print", lua_customPrint);
	lua_register(luaState, "fail", lua_fail);

	lua_load(luaState, lua_codeReader, (void*)&luaBinaries, name.c_str(), NULL);
}

void lua_run(lua_State* luaState, bool& hasError)
{
	const int res = lua_pcall(luaState, 0, LUA_MULTRET, 0);
	if (res != LUA_OK)
	{
		const char *luaError = lua_tostring(luaState, -1);
		hasError = true;

		std::stringstream log;
		log << "LUA runtime error: " << luaError << std::endl;
		wmLog(eError, log.str());
	}
}

void lua_registerMethod(lua_State* luaState, const char* name, lua_CFunction func)
{
	lua_pushcfunction(luaState, func);
	lua_setfield(luaState, -2, name);
}

std::string lua_sprintf(const char* text, ...)
{
	static const int MAX_BUF_SIZE = 128;
	char buf[MAX_BUF_SIZE];

	va_list arglist;
	va_start( arglist, text );
	vsnprintf(buf, MAX_BUF_SIZE, text, arglist);
	va_end( arglist );

	return std::string(buf);
}

void lua_checkArgNums(lua_State* luaState, const std::unordered_set<int>& validArgNums)
{
	if (validArgNums.count(lua_gettop(luaState)) == 0)
	{
		luaL_error(luaState, "Wrong number of arguments");
	}
}

} // namespace lua
} // namespace filter
} // namespace precitec
