#include "luaCanvas.h"
#include "luaScalar.h"
#include "luaConstants.h"
#include "overlay/color.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "module/moduleLogger.h"

namespace precitec{
namespace filter{
namespace lua{

namespace {

class PointPaintCall : public LuaPaintCall
{
private:
	int x, y;
	image::Color color;
public:
	PointPaintCall(image::LayerType layer, int x, int y, const image::Color& color)
	: LuaPaintCall(layer)
	, x(x)
	, y(y)
	, color(color)
	{}
protected:
	virtual void paint(image::OverlayLayer &overlayLayer, const interface::Trafo &trafo) const override
	{
		overlayLayer.add<image::OverlayPoint>(trafo(geo2d::Point(x, y)), color);
	};
};

class LinePaintCall : public LuaPaintCall
{
private:
	int x0, y0, x1, y1;
	image::Color color;
public:
	LinePaintCall(image::LayerType layer, int x0, int y0, int x1, int y1, const image::Color& color)
	: LuaPaintCall(layer)
	, x0(x0)
	, y0(y0)
	, x1(x1)
	, y1(y1)
	, color(color)
	{}
protected:
	virtual void paint(image::OverlayLayer &overlayLayer, const interface::Trafo &trafo) const override
	{
		overlayLayer.add<image::OverlayLine>(trafo(geo2d::Point(x0, y0)), trafo(geo2d::Point(x1, y1)), color);
	};
};

class CrossPaintCall : public LuaPaintCall
{
private:
	int x, y, r;
	image::Color color;
public:
	CrossPaintCall(image::LayerType layer, int x, int y, int r, const image::Color& color)
	: LuaPaintCall(layer)
	, x(x)
	, y(y)
	, r(r)
	, color(color)
	{}
protected:
	virtual void paint(image::OverlayLayer &overlayLayer, const interface::Trafo &trafo) const override
	{
		overlayLayer.add<image::OverlayCross>(trafo(geo2d::Point(x, y)), r, color);
	};
};

class RectPaintCall : public LuaPaintCall
{
private:
	int x, y, w, h;
	image::Color color;
public:
	RectPaintCall(image::LayerType layer, int x, int y, int w, int h, const image::Color& color)
	: LuaPaintCall(layer)
	, x(x)
	, y(y)
	, w(w)
	, h(h)
	, color(color)
	{}
protected:
	virtual void paint(image::OverlayLayer &overlayLayer, const interface::Trafo &trafo) const override
	{
		overlayLayer.add<image::OverlayRectangle>(trafo(geo2d::Rect(x, y, w, h)), color);
	};
};

class TextPaintCall : public LuaPaintCall
{
private:
	std::string text;
	int x, y, size;
	image::Color color;
public:
	TextPaintCall(image::LayerType layer, const std::string& text, int x, int y, int size, const image::Color& color)
	: LuaPaintCall(layer)
	, text(text)
	, x(x)
	, y(y)
	, size(size)
	, color(color)
	{}
protected:
	virtual void paint(image::OverlayLayer &overlayLayer, const interface::Trafo &trafo) const override
	{
		overlayLayer.add<image::OverlayText>(text, image::Font(size), trafo(geo2d::Point(x, y)), color);
	};
};

class CirclePaintCall : public LuaPaintCall
{
private:
	int x, y, r;
	image::Color color;
public:
	CirclePaintCall(image::LayerType layer, int x, int y, int r, const image::Color& color)
	: LuaPaintCall(layer)
	, x(x)
	, y(y)
	, r(r)
	, color(color)
	{}
protected:
	virtual void paint(image::OverlayLayer &overlayLayer, const interface::Trafo &trafo) const override
	{
		overlayLayer.add<image::OverlayCircle>(trafo(geo2d::Point(x, y)), r, color);
	};
};

}

LuaPaintCall::LuaPaintCall(image::LayerType layer)
: _layer(layer)
{}

void LuaPaintCall::paint(image::OverlayCanvas &canvas, const interface::Trafo &trafo) const
{
	paint(canvas.getLayer(_layer), trafo);
}

LuaCanvas::Type *lua_createCanvas(lua_State *luaState, LuaPaintCallQueue& queue)
{
	return lua_createData<LuaCanvas>(luaState, &queue);
}

namespace helper{

int checkNatural(lua_State *luaState, int arg)
{
	int natural = 0;

	int outScalarType = lua_type(luaState, arg);
	if(outScalarType == LUA_TNUMBER)
	{
		natural = luaL_checkinteger(luaState, arg);
	}
	else if(outScalarType == LUA_TUSERDATA)
	{
		LuaScalar::Type *scalar = lua_checkObject<LuaScalar>(luaState, arg);
		luaL_argcheck(luaState, scalar->ref().size() > 0, arg, "can't operate with an empty scalar");

		natural = (int)std::get<eData>(scalar->ref()[0]);
	}
	else
	{
		luaL_argcheck(luaState, false, arg, "expected object of type 'int' or 'scalar'");
	}

	luaL_argcheck(luaState, natural >= 0, arg, "can't operate with negative numbers");
	return 0.0;
}

image::LayerType checkLayer(lua_State *luaState, int arg)
{
	image::LayerType layer = (image::LayerType)luaL_checkinteger(luaState, arg);
	luaL_argcheck(luaState, image::LayerType::eLayerMin <= layer && layer <= image::LayerType::eLayerMax, arg, "layer out of range");
	return layer;
}

image::Color checkColor(lua_State *luaState, int arg)
{
	return image::Color(luaL_checkinteger(luaState, arg));
}
} /// namespace helper

template<>
const char* LuaCanvas::LUA_NAME = "Canvas";

template<>
const char* LuaCanvas::LUA_LIBRARY = "Filter.canvas";

namespace canvas{

int drawPoint(lua_State *luaState)
{
	lua_checkArgNums(luaState, {5});
	LuaCanvas::Type& canvas = lua_checkData<LuaCanvas>(luaState, 1);	// arg1: canvas
	const image::LayerType layer = helper::checkLayer(luaState, 2);		// arg2: layer
	const int x = helper::checkNatural(luaState, 3);					// arg3: x
	const int y = helper::checkNatural(luaState, 4);					// arg4: y
	const image::Color color = helper::checkColor(luaState, 5);			// arg5: color
	canvas->emplace_front(std::make_unique<PointPaintCall>(layer, x, y, color));
	return 0;
}

int drawLine(lua_State *luaState)
{
	lua_checkArgNums(luaState, {7});
	LuaCanvas::Type& canvas = lua_checkData<LuaCanvas>(luaState, 1);	// arg1: canvas
	const image::LayerType layer = helper::checkLayer(luaState, 2);		// arg2: layer
	const int x0 = helper::checkNatural(luaState, 3);					// arg3: x0
	const int y0 = helper::checkNatural(luaState, 4);					// arg4: y0
	const int x1 = helper::checkNatural(luaState, 5);					// arg5: x1
	const int y1 = helper::checkNatural(luaState, 6);					// arg6: y1
	const image::Color color = helper::checkColor(luaState, 7);			// arg7: color
	canvas->emplace_front(std::make_unique<LinePaintCall>(layer, x0, y0, x1, y1, color));
	return 0;
}

int drawCross(lua_State *luaState)
{
	lua_checkArgNums(luaState, {6});
	LuaCanvas::Type& canvas = lua_checkData<LuaCanvas>(luaState, 1);	// arg1: canvas
	const image::LayerType layer = helper::checkLayer(luaState, 2);		// arg2: layer
	const int x = helper::checkNatural(luaState, 3);					// arg3: x
	const int y = helper::checkNatural(luaState, 4);					// arg4: y
	const int r = helper::checkNatural(luaState, 5);					// arg5: r
	const image::Color color = helper::checkColor(luaState, 6);			// arg6: color
	canvas->emplace_front(std::make_unique<CrossPaintCall>(layer, x, y, r, color));
	return 0;
}

int drawRect(lua_State *luaState)
{
	lua_checkArgNums(luaState, {7});
	LuaCanvas::Type& canvas = lua_checkData<LuaCanvas>(luaState, 1);	// arg1: canvas
	const image::LayerType layer = helper::checkLayer(luaState, 2);		// arg2: layer
	const int x = helper::checkNatural(luaState, 3);					// arg3: x
	const int y = helper::checkNatural(luaState, 4);					// arg4: y
	const int w = helper::checkNatural(luaState, 5);					// arg5: w
	const int h = helper::checkNatural(luaState, 6);					// arg6: h
	const image::Color color = helper::checkColor(luaState, 7);			// arg7: color
	canvas->emplace_front(std::make_unique<RectPaintCall>(layer, x, y, w, h, color));
	return 0;
}

int drawText(lua_State *luaState)
{
	lua_checkArgNums(luaState, {7});
	LuaCanvas::Type& canvas = lua_checkData<LuaCanvas>(luaState, 1);	// arg1: canvas
	const image::LayerType layer = helper::checkLayer(luaState, 2);		// arg2: layer
	const int x = helper::checkNatural(luaState, 3);					// arg3: x
	const int y = helper::checkNatural(luaState, 4);					// arg4: y
	const char* text = luaL_checkstring(luaState, 5);					// arg5: text
	const int size = luaL_checkinteger(luaState, 6);					// arg6: size
	const image::Color color = helper::checkColor(luaState, 7);			// arg7: color
	canvas->emplace_front(std::make_unique<TextPaintCall>(layer, text, x, y, size, color));
	return 0;
}

int drawCircle(lua_State *luaState)
{
	lua_checkArgNums(luaState, {6});
	LuaCanvas::Type& canvas = lua_checkData<LuaCanvas>(luaState, 1);	// arg1: canvas
	const image::LayerType layer = helper::checkLayer(luaState, 2);		// arg2: layer
	const int x = helper::checkNatural(luaState, 3);					// arg3: x
	const int y = helper::checkNatural(luaState, 4);					// arg4: y
	const int r = helper::checkNatural(luaState, 5);					// arg5: y
	const image::Color color = helper::checkColor(luaState, 6);			// arg6: color
	canvas->emplace_front(std::make_unique<CirclePaintCall>(layer, x, y, r, color));
	return 0;
}
}

template<>
void LuaCanvas::registerMethods(lua_State *luaState)
{
	lua_registerMethod(luaState, "point", canvas::drawPoint);
	lua_registerMethod(luaState, "line", canvas::drawLine);
	lua_registerMethod(luaState, "cross", canvas::drawCross);
	lua_registerMethod(luaState, "rect", canvas::drawRect);
	lua_registerMethod(luaState, "text", canvas::drawText);
	lua_registerMethod(luaState, "circle", canvas::drawCircle);

	LuaIntegerConstants::registerConstants(luaState, "_Colors", {
		{"red",		image::Color::Red().toARGB()},
		{"green",	image::Color::Green().toARGB()},
		{"blue",	image::Color::Blue().toARGB()},
		{"white",	image::Color::White().toARGB()},
		{"black",	image::Color::Black().toARGB()},
		{"yellow",	image::Color::Yellow().toARGB()},
		{"cyan",	image::Color::Cyan().toARGB()},
		{"orange",	image::Color::Orange().toARGB()},
		{"magenta",	image::Color::Magenta().toARGB()}
	});

	LuaIntegerConstants::registerConstants(luaState, "_Layers", {
		{"line",			image::LayerType::eLayerLine},
		{"contour",			image::LayerType::eLayerContour},
		{"position",		image::LayerType::eLayerPosition},
		{"text",			image::LayerType::eLayerText},
		{"gridtransp",		image::LayerType::eLayerGridTransp},
		{"linetransp",		image::LayerType::eLayerLineTransp},
		{"contourtransp",	image::LayerType::eLayerContourTransp},
		{"positiontransp",	image::LayerType::eLayerPositionTransp},
		{"texttransp",		image::LayerType::eLayerTextTransp},
		{"infobox",			image::LayerType::eLayerInfoBox},
		{"image",			image::LayerType::eLayerImage}
	});
}

} // namespace lua
} // namespace filter
} // namespace precitec