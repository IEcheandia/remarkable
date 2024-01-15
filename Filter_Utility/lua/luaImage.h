
#ifndef LuaImage_H_
#define LuaImage_H_

#include "luaApi.h"
#include "luaData.h"
#include "luaFrame.h"

namespace precitec {
namespace filter {
namespace lua {

using LuaImage = LuaFrame<image::BImage>;

LuaImage::Type *lua_createImage(lua_State *luaState, const interface::ImageContext* context, interface::ResultType result, int width, int height, byte value);

LuaImage::Type lua_makeEmptyImage(const interface::ImageContext& defaultContext);
void lua_retreiveImage(lua_State* luaState, const char* name, LuaImage::Type& result, bool& hasError);

} // namespace lua
} // namespace filter
} // namespace precitec

#endif /*LuaImage_H_*/