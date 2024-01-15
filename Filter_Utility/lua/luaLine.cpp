#include "luaLine.h"
#include "luaMath.h"
#include "luaContext.h"
#include "luaScalar.h"
#include "event/resultType.h"
#include "module/moduleLogger.h"


namespace precitec {
namespace filter {
namespace lua {

LuaLine::Type *lua_createLine(lua_State *luaState, const interface::ImageContext* context, interface::ResultType result, double score, size_t count, size_t size, double value, int rank)
{
	if(!context)
	{
		luaL_error(luaState, "Error allocating new line, invalid image context");
	}
	LuaLine::Type **linePtr = lua_allocateObject<LuaLine>(luaState);
	*linePtr = new LuaLine::Type(*context, geo2d::VecDoublearray(count, geo2d::Doublearray(size, value, rank)), result, score);
	return *linePtr;
}

LuaLine::Type *lua_createLine(lua_State *luaState, const interface::ImageContext* context, size_t size, double value, int rank)
{
	return lua_createLine(luaState, context, interface::ResultType::AnalysisOK, interface::Limit, 1, size, value, rank);
}

LuaLine::Type lua_makeEmptyLine(const interface::ImageContext& defaultContext)
{
	// Default to one empty line for lua output
	return LuaLine::Type(defaultContext, geo2d::VecDoublearray(1, geo2d::Doublearray()), interface::AnalysisOK, interface::NotPresent);
}

void lua_retreiveLine(lua_State* luaState, const char* name, LuaLine::Type& result, bool& hasError)
{
	lua_getglobal(luaState, name);
	int outLineType = lua_type(luaState, -1);
	if(outLineType != LUA_TNONE && outLineType != LUA_TNIL)
	{
		void *userdata = luaL_testudata(luaState, -1, LuaLine::LUA_LIBRARY);
		if (userdata != NULL)
		{
			// TODO: Implement move semantics
			result = **reinterpret_cast<LuaLine::Type **>(userdata);
		}
		else
		{
			hasError = true;
			result.setAnalysisResult(interface::LuaError);
			wmLog(LogType::eError, "LUA error: %s is not a line!\n", name);
		}
		lua_pop(luaState,1);
	}
}

namespace helper
{
int getIndex(lua_State *luaState, LuaLine::Val::Type& val, const char* library)
{
	luaL_getmetatable(luaState, library);
	lua_pushvalue(luaState, 2);
	lua_rawget(luaState, -2);

	/* This index function is used both for methods and array access [],
		so we need to check whether there is a function for the given key. */
	if (lua_isnil(luaState, -1))
	{
		luaL_argcheck(luaState, lua_type(luaState, 2) == LUA_TNUMBER, 2, lua_sprintf("method '%s' does not exist", lua_tostring(luaState, 2)).c_str());

		const int index = luaL_checkinteger(luaState, 2);   // arg2: index
		luaL_argcheck(luaState, 1 <= index && index <= (int)val.size(), 2, "index out of range");

		lua_createData<LuaScalar::Ref>(luaState, typename LuaScalar::Ref::Type(val, index - 1));
	};
	return 1;
};

int setIndex(lua_State *luaState, LuaLine::Val::Type& val)
{
	const int index = luaL_checkinteger(luaState, 2);				                // arg2: index
	luaL_argcheck(luaState, 1 <= index && index <= (int)val.size(), 2, "index out of range");
	const LuaScalar::Val::Type& newVal = LuaScalar::checkRefOrVal(luaState, 3);     // arg3: value

	val.getData()[index - 1] = std::get<eData>(newVal);
	val.getRank()[index - 1] = std::get<eRank>(newVal);
	return 0;
}

void lineValScalarValOperation(LuaLine::Val::Type &resultVal, const LuaLine::Val::Type &lineVal, const LuaScalar::Val::Type& scalarVal, std::function<double(double, double)> operation)
{
	for(int i = 0; i < (int)resultVal.size(); i++)
	{
		std::get<eData>(resultVal[i]) = operation(std::get<eData>(lineVal[i]), std::get<eData>(scalarVal));
		std::get<eRank>(resultVal[i]) = std::min(std::get<eRank>(lineVal[i]), std::get<eRank>(scalarVal));
	}
}

int lineScalarOperation(lua_State *luaState, std::function<double(double, double)> operation)
{
	LuaLine::Type *line = lua_checkObject<LuaLine>(luaState, 1);		    // arg1: line
	luaL_argcheck(luaState, line->ref().size() > 0, 1, "can't operate on an empty parameter");
	LuaScalar::Type *scalar = lua_checkObject<LuaScalar>(luaState, 2);		// arg2: scalar
	luaL_argcheck(luaState, scalar->ref().size() > 0, 2, "can't operate on an empty parameter");

	// If scalar has just one value we apply it to all line values
	const interface::ResultType result = std::max(line->analysisResult(), scalar->analysisResult());
	const double score = std::min(line->rank(), scalar->rank());
	const int size = (int)line->ref()[0].size();
	if(scalar->ref().size() == 1)
	{
		const int count = (int)line->ref().size();
		LuaLine::Type *output = lua_createLine(luaState, &line->context(), result, score, count, size, 0.0, filter::eRankMin);
		for(int i = 0; i < count; i++)
		{
			assert((int)line->ref()[i].size() == size);
			lineValScalarValOperation(output->ref().at(i), line->ref()[i], scalar->ref()[0], operation);
		}
	}
	else
	{
		const int count = (int)std::min(line->ref().size(), scalar->ref().size());
		LuaLine::Type *output = lua_createLine(luaState, &line->context(), result, score, count, size, 0.0, filter::eRankMin);
		for(int i = 0; i < count; i++)
		{
			assert((int)line->ref()[i].size() == size);
			lineValScalarValOperation(output->ref().at(i), line->ref()[i], scalar->ref()[i], operation);
		}
	}
	return 1;
}
} /// namespace helper

// Line data specialisations

template<> const char* LuaLine::LUA_NAME = "Line";
template<> const char* LuaLine::LUA_LIBRARY = "Filter.line";

template<>
int LuaLine::newList(lua_State *luaState)
{
	lua_checkArgNums(luaState, {1, 4, 6});
	const int n = lua_gettop(luaState);
	if (n == 1)
	{
		Type *line = lua_checkObject<LuaLine>(luaState, 1);							// arg1: line

		lua_createObject<LuaLine>(luaState, *line);
		return 1;
	}
	else if (n == 4)
	{
		const LuaContext::Type& context = lua_checkData<LuaContext>(luaState, 1);	// arg1: context
		double size = luaL_checkinteger(luaState, 2);								// arg2: size
		luaL_argcheck(luaState, size >= 0, 2, "Error creating new line, invalid line size");
		double value = luaL_checknumber(luaState, 3);								// arg4: value
		double rank = luaL_checkinteger(luaState, 4);								// arg5: rank
		luaL_argcheck(luaState, rank >= 0, 4, "Error creating new line, invalid rank");

		lua_createLine(luaState, context, size, value, rank);
		return 1;
	}
	else if (n == 6)
	{
		const LuaContext::Type& context = lua_checkData<LuaContext>(luaState, 1);	// arg1: context
		double score = luaL_checknumber(luaState, 2);								// arg2: score
		luaL_argcheck(luaState, score >= 0, 2, "Error creating new line, invalid score");
		int count = luaL_checkinteger(luaState, 3);									// arg3: count
		luaL_argcheck(luaState, count >= 0, 3, "Error creating new line, invalid count");
		double size = luaL_checkinteger(luaState, 4);								// arg5: size
		luaL_argcheck(luaState, size >= 0, 4, "Error creating new line, invalid line size");
		double value = luaL_checknumber(luaState, 5);								// arg6: value
		int rank = luaL_checkinteger(luaState, 6);									// arg7: rank
		luaL_argcheck(luaState, rank >= 0, 6, "Error creating new line, invalid rank");

		lua_createLine(luaState, context, interface::AnalysisOK, score, count, size, value, rank);
		return 1;
	}

	luaL_error(luaState, "Error creating new line, invalid number of arguments");
	return 0;
}

template<>
int LuaLine::toString(lua_State *luaState)
{
	LuaLine::Type *line = lua_checkObject<LuaLine>(luaState, 1);
	if (line->ref().size() > 0)
	{
		lua_pushfstring(luaState, "Line[%d:%d]", line->ref().size(), line->ref()[0].size());
		return 1;
	}
	lua_pushstring(luaState, "Line[]");
	return 1;
}

template<>
LuaLine::Type *LuaLine::createCompatible(lua_State *luaState, Type *a, Type *b)
{
	LuaContext::Type context = lua_compareContext(luaState, a->context(), b->context());
	const interface::ResultType result = std::max(a->analysisResult(), b->analysisResult());
	const double score = std::min(a->rank(), b->rank());
	const int count = (int)std::min(a->ref().size(), b->ref().size());
	// We assume here that all lines have the same size
	const int size = (int)std::min(a->ref()[0].size(), b->ref()[0].size());
	return lua_createLine(luaState, context, result, score, count, size, 0.0, filter::eRankMin);
}

template<>
int LuaLine::scaleExtended(lua_State *luaState)
{
	return helper::lineScalarOperation(luaState, &lua_doubleMul);
}

template<>
int LuaLine::shiftExtended(lua_State *luaState)
{
	return helper::lineScalarOperation(luaState, &lua_doubleAdd);
}

template<>
void LuaLine::registerMethods(lua_State *luaState)
{
}

template<> const char* LuaLine::Ref::LUA_NAME = "LineRef";
template<> const char* LuaLine::Ref::LUA_LIBRARY = "Filter.lineRef";

template<>
LuaLine::Val::Type LuaLine::Ref::toVal(const LuaLine::Ref::Type &ref)
{
	return Val::Type(ref.list->at(ref.index));
}

template<>
void LuaLine::Ref::fromVal(const LuaLine::Ref::Type &ref, const LuaLine::Val::Type &val)
{
	ref.list->at(ref.index) = val;
}

namespace ref
{
int len(lua_State *luaState)
{
	LuaLine::Ref::Type &ref = lua_checkData<LuaLine::Ref>(luaState, 1); // arg1: ref
	lua_pushnumber(luaState, ref.list->at(ref.index).size());
	return 1;
}

int getIndex(lua_State *luaState)
{
	LuaLine::Ref::Type &ref = lua_checkData<LuaLine::Ref>(luaState, 1); // arg1: ref
	LuaLine::Val::Type &val = ref.list->at(ref.index);

	return helper::getIndex(luaState, val, LuaLine::Ref::LUA_LIBRARY);
};

int setIndex(lua_State *luaState)
{
	LuaLine::Ref::Type &ref = lua_checkData<LuaLine::Ref>(luaState, 1); // arg1: ref
	LuaLine::Val::Type &val = ref.list->at(ref.index);

	return helper::setIndex(luaState, val);
}
} // namespace ref

template<>
void LuaLine::Ref::registerMethods(lua_State *luaState)
{
	lua_registerMethod(luaState, "__len", ref::len);
	lua_registerMethod(luaState, "__index", ref::getIndex);
	lua_registerMethod(luaState, "__newindex", ref::setIndex);
}

template<> const char* LuaLine::Val::LUA_NAME = "LineVal";
template<> const char* LuaLine::Val::LUA_LIBRARY = "Filter.lineVal";

template<>
int LuaLine::Val::newVal(lua_State *luaState)
{
	lua_checkArgNums(luaState, {1, 2});
	const int n = lua_gettop(luaState);
	if (n == 1)
	{
		LuaLine::Val::Type *val = lua_checkObject<LuaLine::Val>(luaState, 1);   // arg1: val

		lua_createObject<LuaLine::Val>(luaState, *val);
		return 1;
	}
	else if (n == 2)
	{
		double size = luaL_checkinteger(luaState, 1);							// arg1: size
		luaL_argcheck(luaState, size >= 0, 1, "Error creating new line, invalid line size");
		const double value = luaL_checknumber(luaState, 2);						// arg2: value
		const double rank = luaL_checkinteger(luaState, 3);						// arg3: rank
		luaL_argcheck(luaState, rank >= 0, 3, "Error creating new line value, invalid element rank");

		lua_createObject<LuaLine::Val>(luaState, LuaLine::Val::Type(size, value, rank));
		return 1;
	}

	luaL_error(luaState, "Error creating new line value, invalid number of arguments");
	return 0;
}

template<>
bool LuaLine::Val::binaryPredicate(const LuaLine::Val::Type &a, const LuaLine::Val::Type &b, std::function<bool(double, double)> predicate)
{
	bool result = true;

	const int size = (int)std::min(a.size(), b.size());
	for(int i = 0; i < size; i++)
	{
		const bool p = predicate(a.getData()[i], b.getData()[i]);
		result = (result && p);
	}

	return result;
}

template<>
LuaLine::Val::Type LuaLine::Val::binaryOperation(const LuaLine::Val::Type &a, const LuaLine::Val::Type &b, std::function<double(double, double)> operation)
{
	const int size = (int)std::min(a.size(), b.size());
	LuaLine::Val::Type result(size, 0.0, filter::eRankMin);
	for(int i = 0; i < size; i++)
	{
		result.getData()[i] = operation(a.getData()[i], b.getData()[i]);
		result.getRank()[i] = std::min(a.getRank()[i], b.getRank()[i]);
	}
	return result;
}

template<>
LuaLine::Val::Type LuaLine::Val::unaryOperation(const LuaLine::Val::Type &val, std::function<double(double)> operation)
{
	const int size = (int)val.size();
	LuaLine::Val::Type result = val;
	for(int i = 0; i < size; i++)
	{
		result.getData()[i] = operation(result.getData()[i]);
	}
	return result;
}

namespace val
{
int len(lua_State *luaState)
{
	LuaLine::Val::Type *val = lua_checkObject<LuaLine::Val>(luaState, 1); // arg1: val
	lua_pushnumber(luaState, val->size());
	return 1;
}

int getIndex(lua_State *luaState)
{
	LuaLine::Val::Type *val = lua_checkObject<LuaLine::Val>(luaState, 1); // arg1: val
	return helper::getIndex(luaState, *val, LuaLine::Ref::LUA_LIBRARY);
};

int setIndex(lua_State *luaState)
{
	LuaLine::Val::Type *val = lua_checkObject<LuaLine::Val>(luaState, 1); // arg1: val
	return helper::setIndex(luaState, *val);
}
} // namespace val

template<>
void LuaLine::Val::registerMethods(lua_State *luaState)
{
	lua_registerMethod(luaState, "__len", val::len);
	lua_registerMethod(luaState, "__index", val::getIndex);
	lua_registerMethod(luaState, "__newindex", val::setIndex);
}

} // namespace lua
} // namespace filter
} // namespace precitec