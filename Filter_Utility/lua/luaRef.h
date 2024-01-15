
#ifndef LuaRef_H_
#define LuaRef_H_

#include "luaApi.h"
#include "luaMath.h"
#include "luaVal.h"
#include "geo/geo.h"
#include "geo/array.h"

namespace precitec {
namespace filter {
namespace lua {

template<typename List>
struct ListElement
{
	List* list;
	int index;

	ListElement(List& list, int index)
	: list(&list)
	, index(index)
	{}
};

/*
General purpose library for list elements pointing at members of a Geo::DoubleArray.
Almost all operations other than the setter transform the reference into a value and call the value functions.
*/
template<typename List, typename Value>
class LuaRef
{
public:
	LUA_OBJECT_HEADER(ListElement<List>)
	using Val = LuaVal<List, Value>;

	/// Declarations to be defined by the relevant structure
	static typename Val::Type toVal(const Type &ref);
	static void fromVal(const Type &ref, const typename Val::Type &val);
	///

private:
	static int binaryPredicate(lua_State *luaState, std::function<bool(double, double)> predicate)
	{
		const Type &a = lua_checkData<LuaRef>(luaState, 1);			// arg1: a
		const Type &b = lua_checkData<LuaRef>(luaState, 2);			// arg2: b

		const bool result = Val::binaryPredicate(toVal(a), toVal(b), predicate);
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
		const Type &a = lua_checkData<LuaRef>(luaState, 1);			// arg1: a
		const Type &b = lua_checkData<LuaRef>(luaState, 2);			// arg2: b

		const typename Val::Type& val = Val::binaryOperation(toVal(a), toVal(b), operation);
		lua_createObject<Val>(luaState, val);
		return 1;
	}

	template<BinaryOperation<double> Op>
	static int binaryOperation(lua_State *luaState)
	{
		return binaryOperation(luaState, Op);
	}

	static int unaryOperation(lua_State *luaState, std::function<double(double)> operation)
	{
		const Type &ref = lua_checkData<LuaRef>(luaState, 1);		// arg1: ref

		const typename Val::Type& val = Val::unaryOperation(toVal(ref), operation);
		lua_createObject<Val>(luaState, val);
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
		const double scale = luaL_checknumber(luaState, 2); 								// arg2: scale
		return unaryOperation(luaState, [scale](double x) -> double { return x * scale; });	// arg1
	}

	static int shift(lua_State *luaState)
	{
		lua_checkArgNums(luaState, {2});
		// arg1 is taken care of in lua_unaryOperation
		const double shift = luaL_checknumber(luaState, 2); 								// arg2: shift
		return unaryOperation(luaState, [shift](double x) -> double { return x + shift; });	// arg1
	}

public:
	static void registerMetatable(lua_State *luaState)
	{
		lua_newtable(luaState);
		lua_setglobal(luaState, LUA_NAME);

		luaL_newmetatable(luaState, LUA_LIBRARY);

		lua_pushvalue(luaState, -1);
		lua_setfield(luaState, -2, "__index");

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

#endif /*LuaRef_H_*/