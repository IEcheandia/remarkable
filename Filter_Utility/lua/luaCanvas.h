
#ifndef LuaCanvas_H_
#define LuaCanvas_H_

#include "luaApi.h"
#include "luaSingleton.h"
#include "common/geoContext.h"
#include "overlay/layerType.h"
#include "overlay/overlayCanvas.h"

#include <forward_list>

namespace precitec {
namespace filter {
namespace lua {

// We use paintcalls as an interpretation layer between image::Overlay and lua
class LuaPaintCall
{
public:
	LuaPaintCall(image::LayerType layer);
	void paint(image::OverlayCanvas &canvas, const interface::Trafo &trafo) const;

protected:
	virtual void paint(image::OverlayLayer &overlayLayer, const interface::Trafo &trafo) const = 0;

private:
	image::LayerType _layer;
};

using LuaPaintCallQueue = std::forward_list<std::unique_ptr<LuaPaintCall>>;
using LuaCanvas = LuaSingleton<LuaPaintCallQueue*>;

LuaCanvas::Type *lua_createCanvas(lua_State *luaState, LuaPaintCallQueue& queue);

} // namespace lua
} // namespace filter
} // namespace precitec

#endif /*LuaCanvas_H_*/