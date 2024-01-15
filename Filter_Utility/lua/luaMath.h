
#ifndef LuaMath_H_
#define LuaMath_H_

#include "luaApi.h"
#include <functional>

namespace precitec {
namespace filter {
namespace lua {

template<typename T> using BinaryPredicate = bool (*)(T, T);
template<typename T> using BinaryOperation = T (*)(T, T);
template<typename T> using UnaryOperation = T (*)(T);

double lua_doubleAdd(double a, double b);
double lua_doubleMul(double a, double b);
double lua_doubleSub(double a, double b);
double lua_doubleDiv(double a, double b);
double lua_doubleMod(double a, double b);
double lua_doublePow(double a, double b);
double lua_doubleUnm(double x);
bool lua_doubleEq(double a, double b);
bool lua_doubleLt(double a, double b);
bool lua_doubleLe(double a, double b);

int lua_intAdd(int a, int b);
int lua_intMul(int a, int b);
int lua_intSub(int a, int b);
int lua_intDiv(int a, int b);
int lua_intMod(int a, int b);
int lua_intPow(int a, int b);

void lua_registerMathConstants(lua_State* luaState);

} // namespace lua
} // namespace filter
} // namespace precitec
#endif /*LuaMath_H_*/