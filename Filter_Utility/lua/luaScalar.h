
#ifndef LuaScalar_H_
#define LuaScalar_H_

#include "luaApi.h"
#include "luaGeo.h"
#include "geo/geo.h"
#include "geo/array.h"

namespace precitec {
namespace filter {
namespace lua {

using LuaScalar = LuaGeo<geo2d::Doublearray, std::tuple<double, int>>;

LuaScalar::Type *lua_createScalar(lua_State *luaState, const interface::ImageContext* context, interface::ResultType result, double score, size_t count, double value, int rank);
LuaScalar::Type *lua_createScalar(lua_State *luaState, const interface::ImageContext* context, double value, int rank);

LuaScalar::Type lua_makeEmptyScalar(const interface::ImageContext& defaultContext);
void lua_retreiveScalar(lua_State* luaState, const char* name, LuaScalar::Type& result, bool& hasError);

} // namespace lua
} // namespace filter
} // namespace precitec

#endif /*LuaScalar_H_*/