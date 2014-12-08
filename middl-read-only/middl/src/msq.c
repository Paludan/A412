/*
**  (C) 2008 by Remo Dentato (rdentato@users.sourceforge.net)
**
** Permission to use, copy, modify and distribute this code and
** its documentation for any purpose is hereby granted without
** fee, provided that the above copyright notice, or equivalent
** attribution acknowledgement, appears in all copies and
** supporting documentation.
**
** Copyright holder makes no representations about the suitability
** of this software for any purpose. It is provided "as is" without
** express or implied warranty.
*/

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int luaopen_msq(lua_State *L);
int luaopen_lpeg(lua_State *L);

int main(int argc, char *argv[])
{
  int status;
  lua_State *L;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s filename\n" ,argv[0]);
    return 1;
  }

  L = luaL_newstate();

  luaL_openlibs(L);
  luaopen_msq(L);
  lua_setglobal(L,"msq");
  luaopen_lpeg(L);
  lua_setglobal(L,"lpeg");

  /* Create the arg[] table */
  lua_newtable(L);
  for (status = 1; status < argc; status++) {
    lua_pushstring(L, argv[status]);
    lua_rawseti(L, -2, status-1);
  }
  lua_setglobal(L,"arg");

  if ((status = luaL_loadfile(L, argv[1])) == 0)
     status = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (status || lua_type(L,-1) == LUA_TSTRING)
    fprintf(stderr, "%s\n",lua_tostring(L,-1));
  /* TODO: set pagkage.loaded so that require "msq" will work */
  lua_close(L);
  return 0;
}
