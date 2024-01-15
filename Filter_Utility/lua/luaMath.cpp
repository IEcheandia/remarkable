#include "luaMath.h"
#include "luaConstants.h"
#include <limits>
#include <cmath>

namespace precitec {
namespace filter {
namespace lua {

double lua_doubleAdd(double a, double b) { return a + b; }
double lua_doubleMul(double a, double b) { return a * b; }
double lua_doubleSub(double a, double b) { return a - b; }
double lua_doubleDiv(double a, double b) { return (b != 0.0) ? (a / b) : std::numeric_limits<double>::infinity(); }
double lua_doubleMod(double a, double b) { return std::fmod(a, b); }
double lua_doublePow(double a, double b) { return std::pow(a, b); }
double lua_doubleUnm(double x) { return -x; }
bool lua_doubleEq(double a, double b) { return a == b; } // TODO: Consider equality threshold
bool lua_doubleLt(double a, double b) { return a < b; }
bool lua_doubleLe(double a, double b) { return a <= b; }

int lua_intAdd(int a, int b) { return a + b; }
int lua_intMul(int a, int b) { return a * b; }
int lua_intSub(int a, int b) { return a - b; }
int lua_intDiv(int a, int b) { return (b != 0.0) ? (a / b) : 0; }
int lua_intMod(int a, int b) { return (b != 0.0) ? (a % b) : 0; }
int lua_intPow(int a, int b) { return std::pow(a, b); }

void lua_registerMathConstants(lua_State* luaState)
{
	lua::LuaNumberConstants::registerConstants(luaState, "_Math", {
		{"pi",			3.14159265358979323846},
		{"sqrt2", 		1.41421356237309504880},
		{"e",			2.71828182845904523536},
		{"ln2",			0.69314718055994530941}
	});
}

} // namespace lua
} // namespace filter
} // namespace precitec