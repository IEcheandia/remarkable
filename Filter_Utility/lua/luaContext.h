
#ifndef LuaContext_H_
#define LuaContext_H_

#include "luaApi.h"
#include "luaSingleton.h"
#include "common/geoContext.h"

namespace precitec {
namespace filter {
namespace lua {

using LuaContext = LuaSingleton<const interface::ImageContext*>;

LuaContext::Type *lua_createContext(lua_State *luaState, const interface::ImageContext& context);

LuaContext::Type lua_compareContext(lua_State *luaState, const interface::ImageContext& a, const interface::ImageContext& b);

} // namespace lua
} // namespace filter
} // namespace precitec

#endif /*LuaContext_H_*/