
#ifndef LuaData_H_
#define LuaData_H_

#include "luaApi.h"
#include <cstring>
#include <map>

// Helper functions and types for common userdata functionality

#define LUA_OBJECT_HEADER(T) \
public: using Type = T; \
static const char* LUA_NAME; \
static const char* LUA_LIBRARY; \
static void registerMethods(lua_State *luaState);

namespace precitec {
namespace filter {
namespace lua {

template<typename T> using LuaDictionary = std::map<std::string, T>;

template<typename Data>
static typename Data::Type& lua_checkData(lua_State *luaState, int arg)
{
	void *userdata = luaL_testudata(luaState, arg, Data::LUA_LIBRARY);
	luaL_argcheck(luaState, userdata != nullptr, arg, lua_sprintf("expected object of type '%s'", Data::LUA_NAME).c_str());
	return *reinterpret_cast<typename Data::Type *>(userdata);
}

template<typename Data>
static typename Data::Type *lua_createData(lua_State *luaState, const typename Data::Type& data)
{
	typename Data::Type *dataPtr = reinterpret_cast<typename Data::Type*>(lua_newuserdata(luaState, sizeof(typename Data::Type)));
	luaL_getmetatable(luaState, Data::LUA_LIBRARY);
	lua_setmetatable(luaState, -2);
	// lua_newuserdata returns uninitialized memory, we need to zero it in case type is a structure with copy/move constructors
	std::memset((void *)dataPtr, 0, sizeof(typename Data::Type));
	(*dataPtr) = data;
	return dataPtr;
}

template<typename Object>
typename Object::Type *lua_checkObject(lua_State *luaState, int arg)
{
	void *userdata = luaL_testudata(luaState, arg, Object::LUA_LIBRARY);
	luaL_argcheck(luaState, userdata != nullptr, arg, lua_sprintf("expected object of type '%s'!", Object::LUA_NAME).c_str());
	return *reinterpret_cast<typename Object::Type **>(userdata);
}

template<typename Object>
typename Object::Type **lua_allocateObject(lua_State *luaState)
{
	typename Object::Type **objectPtr = reinterpret_cast<typename Object::Type **>(lua_newuserdata(luaState, sizeof(typename Object::Type *)));
	luaL_getmetatable(luaState, Object::LUA_LIBRARY);
	lua_setmetatable(luaState, -2);
	return objectPtr;
}

template<typename Object>
typename Object::Type *lua_createObject(lua_State *luaState, const typename Object::Type& object)
{
	typename Object::Type **objectPtr = lua_allocateObject<Object>(luaState);
	*objectPtr = new typename Object::Type(object);
	return *objectPtr;
}

template<typename Object>
int lua_gcObject(lua_State *luaState)
{
	typename Object::Type **objectPtr = reinterpret_cast<typename Object::Type **>(lua_touserdata(luaState, 1));
	luaL_argcheck(luaState, *objectPtr != nullptr, 1, lua_sprintf("Error during GC, '%s' not found!", Object::LUA_NAME).c_str());
	delete *objectPtr;
	return 0;
}

}
}
}

#endif /*LuaData_H_*/