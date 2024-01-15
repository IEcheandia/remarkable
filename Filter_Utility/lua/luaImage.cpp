#include "luaImage.h"
#include "luaMath.h"
#include "luaContext.h"
#include "luaScalar.h"
#include "module/moduleLogger.h"

#include <functional>

namespace precitec {
namespace filter {
namespace lua {

LuaImage::Type *lua_createImage(lua_State *luaState, const interface::ImageContext* context, interface::ResultType resultType, int width, int height, byte value)
{
	if(!context)
	{
		luaL_error(luaState, "Error allocating new image, invalid image context");
	}

	image::BImage imageData(image::BImage::SizeType(width, height));
	imageData.fill(value);

	LuaImage::Type **imagePtr = lua_allocateObject<LuaImage>(luaState);
	*imagePtr = new LuaImage::Type(*context, imageData, resultType);
	return *imagePtr;
}

LuaImage::Type lua_makeEmptyImage(const interface::ImageContext& defaultContext)
{
	return LuaImage::Type(defaultContext, image::BImage(), interface::AnalysisOK);
}

void lua_retreiveImage(lua_State* luaState, const char* name, LuaImage::Type& result, bool& hasError)
{
	lua_getglobal(luaState, name);
	int outImageType = lua_type(luaState, -1);
	if(outImageType != LUA_TNONE && outImageType != LUA_TNIL)
	{
		void *userdata = luaL_testudata(luaState, -1, LuaImage::LUA_LIBRARY);
		if (userdata != NULL)
		{
			// TODO: Implement move semantics
			result = **reinterpret_cast<LuaImage::Type **>(userdata);
		}
		else
		{
			hasError = true;
			result.setAnalysisResult(interface::LuaError);
			wmLog(LogType::eError, "LUA error: %s is not an image!\n", name);
		}
		lua_pop(luaState, 1);
	}
}

namespace helper{

int binaryOperation(lua_State *luaState, std::function<int(int a, int b)> operation)
{
	LuaImage::Type *a = lua_checkObject<LuaImage>(luaState, 1);		// arg1: a
	luaL_argcheck(luaState, a->data().size().area() > 0, 1, "can't operate on an empty parameter");
	LuaImage::Type *b = lua_checkObject<LuaImage>(luaState, 2);		// arg2: b
	luaL_argcheck(luaState, b->data().size().area() > 0, 2, "can't operate on an empty parameter");

	LuaContext::Type context = lua_compareContext(luaState, a->context(), b->context());
	const interface::ResultType result = std::max(a->analysisResult(), b->analysisResult());
	const int width = (int)std::min(a->data().width(), b->data().width());
	const int height = (int)std::min(a->data().height(), b->data().height());
	LuaImage::Type *image = lua_createImage(luaState, context, result, width, height, 0);

	for(int x = 0; x < width; x++)
	{
		for(int y = 0; y < height; y++)
		{
			const int value = operation((int)a->data().getValue(x, y), (int)b->data().getValue(x, y));
			image->data().setValue(x, y, (byte)std::clamp(value, 0, 0xFF));
		}
	}
	return 1;
}

int unaryOperation(lua_State *luaState, std::function<int(int a)> operation)
{
	LuaImage::Type *image = lua_checkObject<LuaImage>(luaState, 1);	// arg1: image

	// Clone and apply unary operator
	LuaImage::Type *output = lua_createObject<LuaImage>(luaState, *image);
	const int width = (int)output->data().width();
	const int height = (int)output->data().height();
	for(int x = 0; x < width; x++)
	{
		for(int y = 0; y < height; y++)
		{
			const int value = (int)output->data().getValue(x, y);
			output->data().setValue(x, y, (byte)operation(value));
		}
	}
	return 1;
}
} /// namespace helper

template<>
const char* LuaImage::LUA_NAME = "Image";

template<>
const char* LuaImage::LUA_LIBRARY = "Filter.image";

template<>
int LuaImage::newContent(lua_State *luaState)
{
	lua_checkArgNums(luaState, {1, 4});
	const int n = lua_gettop(luaState);
	if (n == 1)
	{
		LuaImage::Type *image = lua_checkObject<LuaImage>(luaState, 1);				// arg1: image

		lua_createObject<LuaImage>(luaState, *image);
		return 1;
	}
	else if (n == 4)
	{
		const LuaContext::Type& context = lua_checkData<LuaContext>(luaState, 1);	// arg1: context
		int width = luaL_checkinteger(luaState, 2);			    					// arg2: width
		luaL_argcheck(luaState, width >= 0, 2, "Error creating new image, negative width");
		int height = luaL_checkinteger(luaState, 3);								// arg3: height
		luaL_argcheck(luaState, height >= 0, 3, "Error creating new image, negative height");
		int value = luaL_checkinteger(luaState, 4);									// arg4: value
		luaL_argcheck(luaState, 0 <= value && value <= 0xFF, 4, "Error creating new image, value must be in [0..255]");

		lua_createImage(luaState, context, interface::AnalysisOK, width, height, (byte)value);
		return 1;
	}

	luaL_error(luaState, "Error creating new image, invalid number of arguments");
	return 0;
}

template<>
int LuaImage::toString(lua_State *luaState)
{
	LuaImage::Type *image = lua_checkObject<LuaImage>(luaState, 1);	// arg1: image
	lua_pushfstring(luaState, "Image{%d:%d}", image->data().width(), image->data().height());
	return 1;
}

namespace image{

int width(lua_State *luaState)
{
	lua_checkArgNums(luaState, {1});
	LuaImage::Type *image = lua_checkObject<LuaImage>(luaState, 1);	// arg1: image
	lua_pushnumber(luaState, image->data().width());
	return 1;
}

int height(lua_State *luaState)
{
	lua_checkArgNums(luaState, {1});
	LuaImage::Type *image = lua_checkObject<LuaImage>(luaState, 1);	// arg1: image
	lua_pushnumber(luaState, image->data().height());
	return 1;
}

int set(lua_State *luaState)
{
	lua_checkArgNums(luaState, {4});
	LuaImage::Type *image = lua_checkObject<LuaImage>(luaState, 1);	// arg1: image

	int x = luaL_checkinteger(luaState, 2);
	luaL_argcheck(luaState, 1 <= x && x <= image->data().width(), 2, "x out of range");

	int y = luaL_checkinteger(luaState, 3);
	luaL_argcheck(luaState, 1 <= y && y <= image->data().height(), 3, "y out of range");

	int value = (int)luaL_checknumber(luaState, 4);
	luaL_argcheck(luaState, 0 <= value && value <= 0xFF, 4, "Error creating new image, value must be in [0..255]");

	image->data().setValue(x-1, y-1, (byte)value);
	return 0;
}

int get(lua_State *luaState)
{
	lua_checkArgNums(luaState, {3});
	LuaImage::Type *image = lua_checkObject<LuaImage>(luaState, 1);	// arg1: image

	int x = luaL_checkinteger(luaState, 2);
	luaL_argcheck(luaState, 1 <= x && x <= image->data().width(), 2, "x out of range");

	int y = luaL_checkinteger(luaState, 3);
	luaL_argcheck(luaState, 1 <= y && y <= image->data().height(), 3, "y out of range");

	lua_pushnumber(luaState, image->data().getValue(x-1, y-1));
	return 1;
}

template<BinaryOperation<int> Op>
int binaryOperation(lua_State *luaState)
{
	return helper::binaryOperation(luaState, Op);
}

int scale(lua_State *luaState)
{
	lua_checkArgNums(luaState, {2});
	double scale = 0.0;

	const int argType = lua_type(luaState, 2);
	if(argType == LUA_TNUMBER)
	{
		scale = luaL_checknumber(luaState, 2);								// arg2: scale
	}
	else
	{
		LuaScalar::Type *scalar = lua_checkObject<LuaScalar>(luaState, 2);	// arg2: scalar
		luaL_argcheck(luaState, scalar->ref().size() > 0, 2, "can't operate with an empty scalar");
		scale = std::get<eData>(scalar->ref()[0]);
	}

	return helper::unaryOperation(luaState, [scale](int x) -> int { return (int)(x * scale); });
}

int shift(lua_State *luaState)
{
	lua_checkArgNums(luaState, {2});
	double shift = 0.0;

	const int argType = lua_type(luaState, 2);
	if(argType == LUA_TNUMBER)
	{
		shift = luaL_checknumber(luaState, 2);								// arg2: shift
	}
	else
	{
		LuaScalar::Type *scalar = lua_checkObject<LuaScalar>(luaState, 2);	// arg2: scalar
		luaL_argcheck(luaState, scalar->ref().size() > 0, 2, "can't operate with an empty scalar");
		shift = std::get<eData>(scalar->ref()[0]);
	}

	return helper::unaryOperation(luaState, [shift](int x) -> int { return x + (int)shift; });
}
} // namespace image

template<>
void LuaImage::registerMethods(lua_State *luaState)
{
	lua_registerMethod(luaState, "width", image::width);
	lua_registerMethod(luaState, "height", image::height);
	lua_registerMethod(luaState, "get", image::get);
	lua_registerMethod(luaState, "set", image::set);
	lua_registerMethod(luaState, "__add", image::binaryOperation<lua_intAdd>);
	lua_registerMethod(luaState, "__mul", image::binaryOperation<lua_intMul>);
	lua_registerMethod(luaState, "__sub", image::binaryOperation<lua_intSub>);
	lua_registerMethod(luaState, "__div", image::binaryOperation<lua_intDiv>);
	lua_registerMethod(luaState, "__mod", image::binaryOperation<lua_intMod>);
	lua_registerMethod(luaState, "__pow", image::binaryOperation<lua_intPow>);
	lua_registerMethod(luaState, "scale", image::scale);
	lua_registerMethod(luaState, "shift", image::shift);
}

} // namespace lua
} // namespace filter
} // namespace precitec