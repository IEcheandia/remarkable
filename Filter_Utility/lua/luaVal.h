
#ifndef LuaVal_H_
#define LuaVal_H_

#include "luaApi.h"
#include "luaData.h"
#include "luaMath.h"
#include "geo/geo.h"
#include "geo/array.h"

namespace precitec {
namespace filter {
namespace lua {

/*
General purpose library for list element values of a Geo::DoubleArray.
Mainly defined accessors and math operations.
*/
template<typename List, typename Value>
class LuaVal
{
public:
	LUA_OBJECT_HEADER(Value)

	/// Declarations to be defined by the relevant structure
	static int newVal(lua_State *luaState);
	static bool binaryPredicate(const Type &a, const Type &b, std::function<bool(double, double)> predicate);
	static Type binaryOperation(const Type &a, const Type &b, std::function<double(double, double)> operation);
	static Type unaryOperation(const Type &val, std::function<double(double)> operation);
	///

private:
	static int binaryPredicate(lua_State *luaState, std::function<bool(double, double)> predicate)
	{
		Type *a = lua_checkObject<LuaVal>(luaState, 1);		// arg1: a
		Type *b = lua_checkObject<LuaVal>(luaState, 2);		// arg2: b

		const bool result = binaryPredicate(*a, *b, predicate);
		lua_pushboolean(luaState, result);
		return 1;
	}

	template<BinaryPredicate<double> P>
	static int binaryPredicate(lua_State *luaState)
	{
		return binaryPredicate(luaState, P);
	}

	static int binaryOperation(lua_State *luaState, std::function<double(double, double)> operation)
	{
		Type *a = lua_checkObject<LuaVal>(luaState, 1);		// arg1: a
		Type *b = lua_checkObject<LuaVal>(luaState, 2);		// arg2: b

		const Type& result = binaryOperation(*a, *b, operation);
		lua_createObject<LuaVal>(luaState, result);
		return 1;
	}

	template<BinaryOperation<double> Op>
	static int binaryOperation(lua_State *luaState)
	{
		return binaryOperation(luaState, Op);
	}

	static int unaryOperation(lua_State *luaState, std::function<double(double)> operation)
	{
		Type *val = lua_checkObject<LuaVal>(luaState, 1);	// arg1: val

		const Type& result = unaryOperation(*val, operation);
		lua_createObject<LuaVal>(luaState, result);
		return 1;
	}

	template<UnaryOperation<double> Op>
	static int unaryOperation(lua_State *luaState)
	{
		return unaryOperation(luaState, Op);
	}

	static int scale(lua_State *luaState)
	{
		lua_checkArgNums(luaState, {2});
		// arg1 is taken care of in lua_unaryOperation
		const double scale = luaL_checknumber(luaState, 2); // arg2: scale
		return unaryOperation(luaState, [scale](double x) -> double { return x * scale; });
	}

	static int shift(lua_State *luaState)
	{
		lua_checkArgNums(luaState, {2});
		// arg1 is taken care of in lua_unaryOperation
		const double shift = luaL_checknumber(luaState, 2); // arg2: shift
		return unaryOperation(luaState, [shift](double x) -> double { return x + shift; });
	}

public:
	static void registerMetatable(lua_State *luaState)
	{
		lua_newtable(luaState);
		lua_pushcfunction(luaState, newVal);
		lua_setfield(luaState, -2, "new");
		lua_setglobal(luaState, LUA_NAME);

		luaL_newmetatable(luaState, LUA_LIBRARY);
		lua_registerMethod(luaState, "__gc", lua_gcObject<LuaVal>);

		lua_registerMethod(luaState, "__eq", binaryPredicate<lua_doubleEq>);
		lua_registerMethod(luaState, "__lt", binaryPredicate<lua_doubleLt>);
		lua_registerMethod(luaState, "__le", binaryPredicate<lua_doubleLe>);
		lua_registerMethod(luaState, "__add", binaryOperation<lua_doubleAdd>);
		lua_registerMethod(luaState, "__mul", binaryOperation<lua_doubleMul>);
		lua_registerMethod(luaState, "__sub", binaryOperation<lua_doubleSub>);
		lua_registerMethod(luaState, "__div", binaryOperation<lua_doubleDiv>);
		lua_registerMethod(luaState, "__mod", binaryOperation<lua_doubleMod>);
		lua_registerMethod(luaState, "__pow", binaryOperation<lua_doublePow>);
		lua_registerMethod(luaState, "__unm", unaryOperation<lua_doubleUnm>);
		lua_registerMethod(luaState, "scale", scale);
		lua_registerMethod(luaState, "shift", shift);

		registerMethods(luaState);
		lua_pop(luaState, 1);
	}
};

} // namespace lua
} // namespace filter
} // namespace precitec

#endif /*LuaVal_H_*/