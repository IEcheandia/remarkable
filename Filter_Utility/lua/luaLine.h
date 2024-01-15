
#ifndef LuaLine_H_
#define LuaLine_H_

#include "luaApi.h"
#include "luaGeo.h"
#include "geo/geo.h"
#include "geo/array.h"

namespace precitec {
namespace filter {
namespace lua {

using LuaLine = LuaGeo<geo2d::VecDoublearray, geo2d::Doublearray>;

LuaLine::Type *lua_createLine(lua_State *luaState, const interface::ImageContext* context, interface::ResultType result, double score, size_t count, size_t size, double value, int rank);
LuaLine::Type *lua_createLine(lua_State *luaState, const interface::ImageContext* context, size_t size, double value, int rank);

LuaLine::Type lua_makeEmptyLine(const interface::ImageContext& defaultContext);
void lua_retreiveLine(lua_State* luaState, const char* name, LuaLine::Type& result, bool& hasError);

} // namespace lua
} // namespace filter
} // namespace precitec

#endif /*LuaLine_H_*/