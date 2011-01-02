%naturalvar LuaFunction;
%typemap(in, checkfn="lua_isfunction") LuaFunction
%{
lua_pushvalue(L, $input);
$1 = luaL_ref(L, LUA_REGISTRYINDEX);
%}


// binding of QString to lua basic type string
// -------------------------------------------

%naturalvar QString;

%typemap(in, checkfn="lua_isstring") QString
%{$1 = lua_tostring(L,$input); %}

%typemap(out) QString
%{lua_pushstring(L, $1.toUtf8()); SWIG_arg++; %}

// -------------------------------------------

%naturalvar QStringList;

%typemap(in, checkfn="lua_istable") QStringList
%{
size_t len = lua_objlen(L, $input);

int i;
for(i=0; i<len; i++){
	lua_rawgeti(L, $input, i+1);
	const char *elem = luaL_checkstring(L, -1);
	$1 << elem;
	lua_pop(L, 1);
}

%}

%typemap(out) QStringList
%{
lua_createtable(L, $1.length(), 0);

int i;
for(i=0; i<$1.length(); i++){
	QString str = $1.at(i);
	lua_pushstring(L, str.toUtf8());
	lua_rawseti(L, -2, i+1);
}

SWIG_arg++;
%}

//-----------------------------------------

// binding of QString to lua basic type string
%naturalvar QList<int>;

%typemap(in, checkfn="lua_istable") QList<int>
%{
size_t len = lua_objlen(L, $input);

int i;
for(i=0; i<len; i++){
	lua_rawgeti(L, $input, i+1);
	int elem = luaL_checkint(L, -1);
	$1 << elem;
	lua_pop(L, 1);
}

%}

%typemap(out) QList<int>
%{
lua_createtable(L, $1.length(), 0);

int i;
for(i=0; i<$1.length(); i++){
	int elem = $1.at(i);
	lua_pushnumber(L, elem);
	lua_rawseti(L, -2, i+1);
}

SWIG_arg++;
%}
