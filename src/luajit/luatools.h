/*
 *  Copyright (C) 2010  Pan, Shi Zhu
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __LUA_TOOLS_H
#define __LUA_TOOLS_H

extern void l_stackdump(lua_State *L, const char *func, int line);
extern int luaopen_tools(lua_State *);
extern int luaopen_struct(lua_State *);
extern int luaopen_LuaXML_lib(lua_State *);

#define STACKDUMP l_stackdump(L,__FUNCTION__,__LINE__)
#define rlog(x) printf("%s:%d: %s\n", __FUNCTION__, __LINE__, x)
#define l_register(L, name, f) (lua_pushcfunction(L,f), lua_setfield(L, -2, name))

#define l_register_global(L, alib) do { 				\
    for (int i = 0; i < (sizeof alib / sizeof alib[0]); ++i) {		\
        lua_pushcfunction(L, alib[i].func);				\
        lua_setfield(L, LUA_GLOBALSINDEX, alib[i].name);		\
    }									\
} while (0)

#endif

