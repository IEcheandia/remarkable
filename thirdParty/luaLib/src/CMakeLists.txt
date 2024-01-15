set(CMAKE_CXX_STANDARD 14)
project(luaLib VERSION 1.0.0 DESCRIPTION "Embeded LUA 5.3.6 library" LANGUAGES C)

set (SRCS lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
    lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c lbitlib.c
    lcorolib.c ldblib.c liolib.c lmathlib.c loslib.c lstrlib.c ltablib.c lutf8lib.c loadlib.c linit.c)

add_library(luaLib STATIC ${SRCS})
target_link_libraries(luaLib PUBLIC ${CMAKE_DL_LIBS})

add_definitions(-DLUA_USE_LINUX)
target_compile_options(luaLib PRIVATE -fPIC)