
#ifndef LuaGeo_H_
#define LuaGeo_H_

#include "luaApi.h"
#include "luaMath.h"
#include "luaContext.h"
#include "luaData.h"
#include "luaRef.h"
#include "luaVal.h"
#include "geo/geo.h"
#include "geo/array.h"

namespace precitec {
namespace filter {
namespace lua {

/*
General purpose library for Geo list objects that have context, rank and analysis result members.
Since we want to be able to modify the list via the index operator we support reference objects.
Operations instead create value objects. All reference objects can be translated into value objects.
*/
template<typename List, typename Value>
class LuaGeo
{
public:
	LUA_OBJECT_HEADER(interface::TGeo<List>)
	using Ref = LuaRef<List, Value>;
	using Val = LuaVal<List, Value>;

	/// Declarations to be defined by the relevant structure
	static int newList(lua_State *luaState);
	static int toString(lua_State *luaState);
	static Type *createCompatible(lua_State *luaState, Type *a, Type *b);
	static int scaleExtended(lua_State *luaState);
	static int shiftExtended(lua_State *luaState);
	///

	static typename Val::Type checkRefOrVal(lua_State *luaState, int arg)
	{
		void *userdata = luaL_testudata(luaState, arg, Val::LUA_LIBRARY);
		if(userdata != nullptr)
		{
			return *reinterpret_cast<typename Val::Type *>(userdata);
		}

		userdata = luaL_testudata(luaState, arg, Ref::LUA_LIBRARY);
		luaL_argcheck(luaState, userdata != nullptr, arg, lua_sprintf("expected '%s' reference or value!", LUA_NAME).c_str());
		return Ref::toVal(*reinterpret_cast<typename Ref::Type *>(userdata));
	}

private:
	static int getLen(lua_State *luaState)
	{
		Type *list = lua_checkObject<LuaGeo>(luaState, 1);		// arg1: list
		lua_pushnumber(luaState, list->ref().size());
		return 1;
	}

	static int getAt(lua_State *luaState)
	{
		Type *list = lua_checkObject<LuaGeo>(luaState, 1);		// arg1: list

		luaL_getmetatable(luaState, LUA_LIBRARY);
		lua_pushvalue(luaState, 2);
		lua_rawget(luaState, -2);

		/* This index function is used both for methods and array access [],
		   so we need to check whether there is a function for the given key. */
		if (lua_isnil(luaState, -1))
		{
			luaL_argcheck(luaState, lua_type(luaState, 2) == LUA_TNUMBER, 2, lua_sprintf("method '%s' does not exist", lua_tostring(luaState, 2)).c_str());

			const int index = luaL_checkinteger(luaState, 2);	// arg2: index
			luaL_argcheck(luaState, 1 <= index && index <= (int)list->ref().size(), 2, "index out of range");

			lua_createData<Ref>(luaState, typename Ref::Type(list->ref(), index - 1));
		};
		return 1;
	};

	static int setAt(lua_State *luaState)
	{
		Type *list = lua_checkObject<LuaGeo>(luaState, 1);				// arg1: list
		const int index = luaL_checkinteger(luaState, 2);				// arg2: index
		luaL_argcheck(luaState, 1 <= index && index <= (int)list->ref().size(), 2, "index out of range");
		const typename Val::Type& val = checkRefOrVal(luaState, 3);		// arg3: value

		Ref::fromVal(typename Ref::Type(list->ref(), index - 1), val);
		return 0;
	}

	static int result(lua_State *luaState)
	{
		lua_checkArgNums(luaState, {1, 2});
		const int n = lua_gettop(luaState);
		Type *list = lua_checkObject<LuaGeo>(luaState, 1); 	// arg1: list
		if(n == 1)
		{
			lua_pushinteger(luaState, list->analysisResult());
			return 1;
		}

		const int result = luaL_checkinteger(luaState, 2); // arg2: result
		luaL_argcheck(luaState, 0 <= result && result < interface::ResultType::EndOfResultTypes, 2, "invalid result type");
		list->setAnalysisResult((interface::ResultType)result);
		return 0;
	}

	static int score(lua_State *luaState)
	{
		lua_checkArgNums(luaState, {1, 2});
		const int n = lua_gettop(luaState);
		Type *list = lua_checkObject<LuaGeo>(luaState, 1); 	// arg1: list
		if(n == 1)
		{
			lua_pushnumber(luaState, list->rank());
			return 1;
		}

		const double score = luaL_checknumber(luaState, 2); // arg2: score
		luaL_argcheck(luaState, 0.0 <= score && score <= 1.0, 2, "score outside range [0..1]");
		list->rank(score);
		return 0;
	}

	static int context(lua_State *luaState)
	{
		lua_checkArgNums(luaState, {1});
		Type *list = lua_checkObject<LuaGeo>(luaState, 1); // arg1: list

		// Allocating new userdata every time we retreive context is a bit of a waste, but
		// Light userdata is not an option since it doesn't support metatables/typesafety.
		lua_createContext(luaState, list->context());
		return 1;
	}

	static int binaryPredicate(lua_State *luaState, std::function<bool(double, double)> predicate)
	{
		Type *a = lua_checkObject<LuaGeo>(luaState, 1);	// arg1: a
		luaL_argcheck(luaState, a->ref().size() > 0, 1, "can't operate on an empty parameter");
		Type *b = lua_checkObject<LuaGeo>(luaState, 2);	// arg2: b
		luaL_argcheck(luaState, b->ref().size() > 0, 2, "can't operate on an empty parameter");

		const int len = (int)std::min(a->ref().size(), b->ref().size());
		for(int i = 0; i < len; i++)
		{
			if(!Val::binaryPredicate(Ref::toVal(typename Ref::Type(a->ref(), i)), Ref::toVal(typename Ref::Type(b->ref(), i)), predicate))
			{
				lua_pushboolean(luaState, false);
				return 1;
			}
		}
		lua_pushboolean(luaState, true);
		return 1;
	}

	template<BinaryPredicate<double> P>
	static int binaryPredicate(lua_State *luaState)
	{
		return binaryPredicate(luaState, P);
	}

	static int binaryOperation(lua_State *luaState, std::function<double(double, double)> operation)
	{
		Type *a = lua_checkObject<LuaGeo>(luaState, 1);	// arg1: a
		luaL_argcheck(luaState, a->ref().size() > 0, 1, "can't operate on an empty parameter");
		Type *b = lua_checkObject<LuaGeo>(luaState, 2);	// arg2: b
		luaL_argcheck(luaState, b->ref().size() > 0, 2, "can't operate on an empty parameter");

		Type *output = createCompatible(luaState, a, b);
		const int len = output->ref().size();
		for(int i = 0; i < len; i++)
		{
			const typename Val::Type& val = Val::binaryOperation(Ref::toVal(typename Ref::Type(a->ref(), i)), Ref::toVal(typename Ref::Type(b->ref(), i)), operation);
			Ref::fromVal(typename Ref::Type(output->ref(), i), val);
		}
		return 1;
	}

	template<BinaryOperation<double> Op>
	static int binaryOperation(lua_State *luaState)
	{
		return binaryOperation(luaState, Op);
	}

	static int unaryOperation(lua_State *luaState, std::function<double(double)> operation)
	{
		Type *list = lua_checkObject<LuaGeo>(luaState, 1);	// arg1: list

		// Clone and apply unary operator
		Type *output = lua_createObject<LuaGeo>(luaState, *list);
		const int len = (int)output->ref().size();
		for(int i = 0; i < len; i++)
		{
			const typename Ref::Type ref(output->ref(), i);
			const typename Val::Type &val = Val::unaryOperation(Ref::toVal(ref), operation);
			Ref::fromVal(ref, val);
		}
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
		const int argType = lua_type(luaState, 2);
		if(argType == LUA_TNUMBER)
		{
			const double scale = luaL_checknumber(luaState, 2); 								// arg2: scale
			return unaryOperation(luaState, [scale](double x) -> double { return x * scale; });	// arg1
		}
		return scaleExtended(luaState);
	}

	static int shift(lua_State *luaState)
	{
		lua_checkArgNums(luaState, {2});
		const int argType = lua_type(luaState, 2);
		if(argType == LUA_TNUMBER)
		{
			const double shift = luaL_checknumber(luaState, 2); 								// arg2: shift
			return unaryOperation(luaState, [shift](double x) -> double { return x + shift; });	// arg1
		}
		return shiftExtended(luaState);
	}

public:
	static void registerMetatable(lua_State *luaState)
	{
		Ref::registerMetatable(luaState);
		Val::registerMetatable(luaState);

		lua_newtable(luaState);
		lua_pushcfunction(luaState, newList);
		lua_setfield(luaState, -2, "new");
		lua_setglobal(luaState, LUA_NAME);

		luaL_newmetatable(luaState, LUA_LIBRARY);
		lua_registerMethod(luaState, "__gc", lua_gcObject<LuaGeo>);

		lua_registerMethod(luaState, "__tostring", toString);
		lua_registerMethod(luaState, "__len", getLen);
		lua_registerMethod(luaState, "__index", getAt);
		lua_registerMethod(luaState, "__newindex", setAt);
		lua_registerMethod(luaState, "result", result);
		lua_registerMethod(luaState, "score", score);
		lua_registerMethod(luaState, "context", context);
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

#endif /*LuaGeo_H_*/