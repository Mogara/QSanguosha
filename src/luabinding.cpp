extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "engine.h"
#include "settings.h"

#include <QDir>

static int GetFileNames(lua_State *lua){
    const char *dirname = luaL_checkstring(lua, 1);
    QDir dir(dirname);
    QStringList filenames = dir.entryList(QDir::Files);

    lua_createtable(lua, filenames.length(), 0);

    int i;
    for(i=0; i<filenames.length(); i++){
        lua_pushstring(lua, filenames.at(i).toAscii());
        lua_rawseti(lua, -2, i);
    }

    return 1;
}

static int Print(lua_State *lua){
    const char *msg = luaL_checkstring(lua, 1);
    qDebug("%s", msg);

    return 0;
}

static int AddTranslationEntry(lua_State *lua){
    const char *key = luaL_checkstring(lua, 1);
    const char *value = luaL_checkstring(lua, 2);

    Sanguosha->addTranslationEntry(key, value);

    return 0;    
}

static int GetConfig(lua_State *lua){
    const char *key = luaL_checkstring(lua, 1);
    int type = lua_type(lua, 2);
    switch(type){
    case LUA_TNUMBER:{
            int n = luaL_checkint(lua, 2);
            lua_pushinteger(lua, Config.value(key, n).toInt());

            break;
        }

    case LUA_TBOOLEAN:{
            bool b = lua_toboolean(lua, 2);
            lua_pushboolean(lua, Config.value(key, b).toBool());

            break;
        }

    case LUA_TSTRING:{
            const char *str = luaL_checkstring(lua, 2);
            QString qstr = Config.value(key, str).toString();
            lua_pushstring(lua, qstr.toUtf8().constData());

            break;
        }

    default:
        luaL_error(lua, "The second argument of %s should be a number, boolean or a string", __func__);
    }

    return 1;
}

void Engine::doStartScript(){
    lua = luaL_newstate();
    luaL_openlibs(lua);

    lua_register(lua, "GetFileNames", GetFileNames);
    lua_register(lua, "Print", Print);
    lua_register(lua, "AddTranslationEntry", AddTranslationEntry);
    lua_register(lua, "GetConfig", GetConfig);

    luaL_dofile(lua, "sanguosha.lua");
}
