#include "util.h"
#include "lua.hpp"

#include <QVariant>
#include <QStringList>
#include <QMessageBox>

extern "C" {
    int luaopen_sgs(lua_State *);
}

QVariant GetValueFromLuaState(lua_State *L, const char *table_name, const char *key){
    lua_getglobal(L, table_name);
    lua_getfield(L, -1, key);

    QVariant data;
    switch(lua_type(L, -1)){
    case LUA_TSTRING: {
        data = QString::fromUtf8(lua_tostring(L, -1));
        lua_pop(L, 1);
        break;
    }

    case LUA_TNUMBER:{
        data = lua_tonumber(L, -1);
        lua_pop(L, 1);
        break;
    }

    case LUA_TTABLE:{
        QStringList list;

        size_t size = lua_objlen(L, -1);
        for(size_t i=0; i<size; i++){
            lua_rawgeti(L, -1, i+1);
            QString element = QString::fromUtf8(lua_tostring(L, -1));
            lua_pop(L, 1);
            list << element;
        }

        data = list;
    }

    default:
        break;
    }

    lua_pop(L, 1);
    return data;
}

lua_State *CreateLuaState(){
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_sgs(L);

    return L;
}

void DoLuaScript(lua_State *L, const char *script){
    int error = luaL_dofile(L, script);
    if(error){
        QString error_msg = lua_tostring(L, -1);
        QMessageBox::critical(NULL, QObject::tr("Lua script error"), error_msg);
        exit(1);
    }
}

void DoLuaScripts(lua_State *L, const QStringList &scripts){
    foreach(QString script, scripts){
        DoLuaScript(L, script.toLocal8Bit());
    }
}
