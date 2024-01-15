#include "luaScalar.h"
#include "luaMath.h"
#include "luaContext.h"
#include "event/resultType.h"
#include "module/moduleLogger.h"

#include <functional>

namespace precitec {
namespace filter {
namespace lua {

LuaScalar::Type *lua_createScalar(lua_State *luaState, const interface::ImageContext* context, interface::ResultType resultType, double score, size_t count, double value, int rank)
{
	if(!context)
	{
		luaL_error(luaState, "Error allocating new scalar, invalid image context");
	}
	LuaScalar::Type **scalarPtr = lua_allocateObject<LuaScalar>(luaState);
	*scalarPtr = new LuaScalar::Type(*context, geo2d::Doublearray(count, value, rank), resultType, score);
	return *scalarPtr;
}

LuaScalar::Type *lua_createScalar(lua_State *luaState, const interface::ImageContext* context, double value, int rank)
{
	return lua_createScalar(luaState, context, interface::ResultType::AnalysisOK, interface::Limit, 1, value, rank);
}

LuaScalar::Type lua_makeEmptyScalar(const interface::ImageContext& defaultContext)
{
	// Because we want to mimic the behaviour of standard numbers the lua output defaults to a non-empty scalar set to 0
	return LuaScalar::Type(defaultContext, geo2d::Doublearray(1, 0.0, filter::eRankMin), interface::AnalysisOK, interface::NotPresent);
}

void lua_retreiveScalar(lua_State* luaState, const char* name, LuaScalar::Type& result, bool& hasError)
{
	lua_getglobal(luaState, name);
	int outScalarType = lua_type(luaState, -1);
	if(outScalarType != LUA_TNONE && outScalarType != LUA_TNIL)
	{
		// For convenience we allow primitive numbers as scalar output.
		if(outScalarType == LUA_TNUMBER)
		{
			result = lua_makeEmptyScalar(result.context());
			std::get<eData>(result.ref()[0]) = luaL_checknumber(luaState, -1);
		}
		else
		{
			void *userdata = luaL_testudata(luaState, -1, LuaScalar::LUA_LIBRARY);
			if (userdata != NULL)
			{
				// TODO: Implement move semantics
				result = **reinterpret_cast<LuaScalar::Type **>(userdata);
			}
			else
			{
				hasError = true;
				result.setAnalysisResult(interface::LuaError);
				wmLog(LogType::eError, "LUA error: %s is not a scalar!\n", name);
			}
		}
		lua_pop(luaState,1);
	}
}

// Scalar data specialisations

template<> const char* LuaScalar::LUA_NAME = "Scalar";
template<> const char* LuaScalar::LUA_LIBRARY = "Filter.scalar";

template<>
int LuaScalar::newList(lua_State *luaState)
{
	lua_checkArgNums(luaState, {1, 3, 5});
	const int n = lua_gettop(luaState);
	if (n == 1)
	{
		LuaScalar::Type *scalar = lua_checkObject<LuaScalar>(luaState, 1);			// arg1: list

		lua_createObject<LuaScalar>(luaState, *scalar);
		return 1;
	}
	else if (n == 3)
	{
		const LuaContext::Type& context = lua_checkData<LuaContext>(luaState, 1);	// arg1: context
		const double value = luaL_checknumber(luaState, 2);							// arg2: value
		const double rank = luaL_checkinteger(luaState, 3);							// arg3: rank
		luaL_argcheck(luaState, rank >= 0, 3, "Error creating new scalar, invalid element rank");

		lua_createScalar(luaState, context, value, rank);
		return 1;
	}
	else if (n == 5)
	{
		const LuaContext::Type& context = lua_checkData<LuaContext>(luaState, 1);	// arg1: context
		const double score = luaL_checknumber(luaState, 2);							// arg2: score
		luaL_argcheck(luaState, score >= 0, 2, "Error creating new scalar, invalid score");
		const int count = luaL_checkinteger(luaState, 3);							// arg3: count
		luaL_argcheck(luaState, count >= 0, 3, "Error creating new scalar, invalid count");
		const double value = luaL_checknumber(luaState, 4);							// arg4: value
		const int rank = luaL_checkinteger(luaState, 5);							// arg5: rank
		luaL_argcheck(luaState, rank >= 0, 5, "Error creating new scalar, invalid element rank");

		lua_createScalar(luaState, context, interface::AnalysisOK, score, count, value, rank);
		return 1;
	}

	luaL_error(luaState, "Error creating new scalar, invalid number of arguments");
	return 0;
}

template<>
int LuaScalar::toString(lua_State *luaState)
{
	static const int MAX_DISPLAY_VALUES = 3;
	static const int FLOAT_PRECISION = 3;

	// Display the first N values
	LuaScalar::Type *scalar = lua_checkObject<LuaScalar>(luaState, 1);
	std::stringstream string;
	const int count = (int)scalar->ref().size();
	string << std::setprecision(FLOAT_PRECISION) << "Scalar[" << count << "](";
	for(int i = 0; i < std::min(count, MAX_DISPLAY_VALUES); i++)
	{
		if(i > 0)
		{
			string << ", ";
		}
		string << std::get<eData>(scalar->ref()[i]);
	}

	if(count > MAX_DISPLAY_VALUES)
	{
		string << ", ..";
	}
	string << ")";
	lua_pushstring(luaState, string.str().c_str());
	return 1;
}

template<>
LuaScalar::Type *LuaScalar::createCompatible(lua_State *luaState, Type *a, Type *b)
{
	LuaContext::Type context = lua_compareContext(luaState, a->context(), b->context());
	const interface::ResultType result = std::max(a->analysisResult(), b->analysisResult());
	const double score = std::min(a->rank(), b->rank());
	const size_t len = std::min(a->ref().size(), b->ref().size());
	return lua_createScalar(luaState, context, result, score, len, 0.0, filter::eRankMin);
}

template<>
int LuaScalar::scaleExtended(lua_State *luaState)
{
	luaL_argcheck(luaState, false, 2, "Scalars can only be scaled by numbers");
	return 0;
}

template<>
int LuaScalar::shiftExtended(lua_State *luaState)
{
	luaL_argcheck(luaState, false, 2, "Scalars can only be shifted by numbers");
	return 0;
}

template<>
void LuaScalar::registerMethods(lua_State *luaState)
{
}

template<> const char* LuaScalar::Ref::LUA_NAME = "ScalarRef";
template<> const char* LuaScalar::Ref::LUA_LIBRARY = "Filter.scalarRef";

template<>
LuaScalar::Val::Type LuaScalar::Ref::toVal(const LuaScalar::Ref::Type &ref)
{
	return Val::Type(
		ref.list->getData()[ref.index],
		ref.list->getRank()[ref.index]);
}

template<>
void LuaScalar::Ref::fromVal(const LuaScalar::Ref::Type &ref, const LuaScalar::Val::Type &val)
{
	ref.list->getData()[ref.index] = std::get<eData>(val);
	ref.list->getRank()[ref.index] = std::get<eRank>(val);
}

namespace ref
{
int value(lua_State *luaState)
{
	lua_checkArgNums(luaState, {1, 2});
	const int n = lua_gettop(luaState);
	LuaScalar::Ref::Type &ref = lua_checkData<LuaScalar::Ref>(luaState, 1);	// arg1: ref
	if(n == 1)
	{
		lua_pushnumber(luaState, std::get<eData>(LuaScalar::Ref::toVal(ref)));
		return 1;
	}

	ref.list->getData()[ref.index] = luaL_checknumber(luaState, 2);			// arg2: value
	return 0;
}

int rank(lua_State *luaState)
{
	lua_checkArgNums(luaState, {1, 2});
	const int n = lua_gettop(luaState);
	LuaScalar::Ref::Type &ref = lua_checkData<LuaScalar::Ref>(luaState, 1);	// arg1: ref
	if(n == 1)
	{
		lua_pushnumber(luaState, std::get<eRank>(LuaScalar::Ref::toVal(ref)));
		return 1;
	}

	ref.list->getRank()[ref.index] = luaL_checknumber(luaState, 2);			// arg2: value
	return 0;
}
} // namespace ref

template<>
void LuaScalar::Ref::registerMethods(lua_State *luaState)
{
	lua_registerMethod(luaState, "value", ref::value);
	lua_registerMethod(luaState, "rank", ref::rank);
}

template<> const char* LuaScalar::Val::LUA_NAME = "ScalarVal";
template<> const char* LuaScalar::Val::LUA_LIBRARY = "Filter.scalarVal";

namespace val
{
int value(lua_State *luaState)
{
	lua_checkArgNums(luaState, {1, 2});
	const int n = lua_gettop(luaState);
	LuaScalar::Val::Type *val = lua_checkObject<LuaScalar::Val>(luaState, 1);	// arg1: val
	if(n == 1)
	{
		lua_pushnumber(luaState, std::get<eData>(*val));
		return 1;
	}

	std::get<eData>(*val) = luaL_checknumber(luaState, 2);						// arg2: value
	return 0;
}

int rank(lua_State *luaState)
{
	lua_checkArgNums(luaState, {1, 2});
	const int n = lua_gettop(luaState);
	LuaScalar::Val::Type *val = lua_checkObject<LuaScalar::Val>(luaState, 1);	// arg1: val
	if(n == 1)
	{
		lua_pushnumber(luaState, std::get<eRank>(*val));
		return 1;
	}

	std::get<eRank>(*val) = luaL_checknumber(luaState, 2);						// arg2: value
	return 0;
}
} // namespace val

template<>
int LuaScalar::Val::newVal(lua_State *luaState)
{
	lua_checkArgNums(luaState, {1, 2});
	const int n = lua_gettop(luaState);
	if (n == 1)
	{
		LuaScalar::Val::Type *val = lua_checkObject<LuaScalar::Val>(luaState, 1);	// arg1: val

		lua_createObject<LuaScalar::Val>(luaState, *val);
		return 1;
	}
	else if (n == 2)
	{
		const double value = luaL_checknumber(luaState, 1);							// arg1: value
		const double rank = luaL_checkinteger(luaState, 2);							// arg2: rank
		luaL_argcheck(luaState, rank >= 0, 2, "Error creating new scalar value, invalid element rank");

		lua_createObject<LuaScalar::Val>(luaState, LuaScalar::Val::Type(value, rank));
		return 1;
	}

	luaL_error(luaState, "Error creating new scalar value, invalid number of arguments");
	return 0;
}

template<>
bool LuaScalar::Val::binaryPredicate(const LuaScalar::Val::Type &a, const LuaScalar::Val::Type &b, std::function<bool(double, double)> predicate)
{
	return predicate(std::get<eData>(a), std::get<eData>(b));
}

template<>
LuaScalar::Val::Type LuaScalar::Val::binaryOperation(const LuaScalar::Val::Type &a, const LuaScalar::Val::Type &b, std::function<double(double, double)> operation)
{
	const double value = operation(std::get<eData>(a), std::get<eData>(b));
	const int rank = std::min(std::get<eRank>(a), std::get<eRank>(b));
	return LuaScalar::Val::Type(value, rank);
}

template<>
LuaScalar::Val::Type LuaScalar::Val::unaryOperation(const LuaScalar::Val::Type &val, std::function<double(double)> operation)
{
	const double value = operation(std::get<eData>(val));
	const int rank = std::get<eRank>(val);
	return LuaScalar::Val::Type(value, rank);
}

template<>
void LuaScalar::Val::registerMethods(lua_State *luaState)
{
	lua_registerMethod(luaState, "value", val::value);
	lua_registerMethod(luaState, "rank", val::rank);
}

} // namespace lua
} // namespace filter
} // namespace precitec