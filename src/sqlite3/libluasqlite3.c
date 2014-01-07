

/*
 *  Author: Michael Roth <mroth@nessie.de>
 *
 *  Copyright (c) 2004, 2005, 2006 Michael Roth <mroth@nessie.de>
 *
 *  Permission is hereby granted, free of charge, to any person 
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge,
 *  publish, distribute, sublicense, and/or sell copies of the Software,
 *  and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be 
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */



#include <stdio.h>
#include <sqlite3.h>
#include <lua.h>
#include <lauxlib.h>


/*
 * Not exported to Lua:
 *
 * sqlite3_get_table		Really doesn't make sense, doesn't it?
 *
 * sqlite3_mprintf		Won't work because I don't know now how to pass a variable
 *				numbers of arguments from lua to a C vararg function.
 *
 * sqlite3_user_data		Makes no sense to export, only for internal usage.
 *
 */

#if LUA_VERSION_NUM >= 502 

int luaL_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s", tname,
      luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}

void luaL_register (lua_State *L, const char *libname, const luaL_Reg *l){
  if(libname) lua_newtable(L);
  luaL_setfuncs(L, l, 0);
}

#define lua_strlen lua_rawlen

#endif 


#define IS_INT(n)		( ( (lua_Number) ((int)(n)) )  == (n) )

#define CAST(type, arg)		( (type)(arg) )

#define FUNC(name)		static int name (lua_State * L)



#define CB_DATA(ptr)		CAST(CB_Data *, (ptr))

#define KEY(ptr, id)		CAST( void*, CAST(char*,(ptr))+(id) )

#define KEY_KEY2VALUE_TABLE(p)	KEY((p), 1)
#define KEY_FUNCTION_TABLE(p)	KEY((p), 2)
#define KEY_COLLATION_TABLE(p)	KEY((p), 3)
#define KEY_COLLNEEDED_DATA(p)	KEY((p), 4)
#define KEY_AUTHORIZER_DATA(p)	KEY((p), 5)
#define KEY_PROGRESS_DATA(p)	KEY((p), 6)
#define KEY_TRACE_DATA(p)	KEY((p), 7)
#define KEY_BUSY_DATA(p)	KEY((p), 8)
#define KEY_COMMIT_DATA(p)	KEY((p), 9)

#define KEY_XFUNC(p)		KEY((p), 1)
#define KEY_XSTEP(p)		KEY((p), 2)
#define KEY_XFINAL(p)		KEY((p), 3)
#define KEY_XCOMPARE(p)		KEY((p), 1)
#define KEY_XNEEDED(p)		KEY((p), 1)
#define KEY_XAUTH(p)		KEY((p), 1)
#define KEY_XPROGRESS(p)	KEY((p), 1)
#define KEY_XTRACE(p)		KEY((p), 1)
#define KEY_XBUSY(p)		KEY((p), 1)
#define KEY_XCOMMIT(p)		KEY((p), 1)



typedef struct
{
  sqlite3 * 	sqlite3;
  lua_State * 	L;
  int		key2value_pos;	/* Used by callback wrappers to find the key2value array on the lua stack */
} DB;


typedef struct
{
  DB * db;
  sqlite3_stmt * stmt;
} Stmt;


typedef struct
{
  DB * db;
} CB_Data;






static void push_private_table(lua_State * L, void * table_key);
static void delete_private_value(lua_State * L, void * value_key);

static CB_Data * new_cb_data(lua_State * L, DB * db);
static CB_Data * get_cb_data(lua_State * L, DB * db, void * data_key);
static CB_Data * get_named_cb_data(lua_State * L, DB * db, void * table_key, int name_pos);

#define get_function_cb_data(L, db, name_pos)	get_named_cb_data((L), (db), KEY_FUNCTION_TABLE(db), name_pos)
#define get_collation_cb_data(L, db, name_pos)	get_named_cb_data((L), (db), KEY_COLLATION_TABLE(db), name_pos)
#define get_collneeded_cb_data(L, db)		get_cb_data((L), (db), KEY_COLLNEEDED_DATA(db))
#define get_authorizer_cb_data(L, db)		get_cb_data((L), (db), KEY_AUTHORIZER_DATA(db))
#define get_progress_cb_data(L, db)		get_cb_data((L), (db), KEY_PROGRESS_DATA(db))
#define get_trace_cb_data(L, db)		get_cb_data((L), (db), KEY_TRACE_DATA(db))
#define get_busy_cb_data(L, db)			get_cb_data((L), (db), KEY_BUSY_DATA(db))
#define get_commit_cb_data(L, db)		get_cb_data((L), (db), KEY_COMMIT_DATA(db))

static void register_callback(lua_State * L, DB * db, void * cb_key, int callback_pos);
static void init_callback_usage(lua_State * L, DB * db);
static void push_callback(lua_State * L, DB * db, void * cb_key);

static int pop_break_condition(lua_State * L);
static void push_nil_or_string(lua_State * L, const char * str);




static void push_private_table(lua_State * L, void * table_key)
{
  lua_pushlightuserdata(L, table_key);
  lua_rawget(L, LUA_REGISTRYINDEX);
  if (lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushlightuserdata(L, table_key);
    lua_pushvalue(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);
  }
}


static void delete_private_value(lua_State * L, void * value_key)
{
  lua_pushlightuserdata(L, value_key);
  lua_rawget(L, LUA_REGISTRYINDEX);
  if (!lua_isnil(L, -1))
  {
    lua_pushlightuserdata(L, value_key);
    lua_pushnil(L);
    lua_rawset(L, LUA_REGISTRYINDEX);
  }
  lua_pop(L, 1);
}


static CB_Data * new_cb_data(lua_State * L, DB * db)
{
  CB_Data * cb_data = lua_newuserdata(L, sizeof(CB_Data));
  cb_data->db = db;
  return cb_data;
}


static CB_Data * get_cb_data(lua_State * L, DB * db, void * data_key)
{
  CB_Data * cb_data;
  
  lua_pushlightuserdata(L, data_key);
  lua_rawget(L, LUA_REGISTRYINDEX);
  
  if (lua_isnil(L, -1))
  {
    lua_pushlightuserdata(L, data_key);
    cb_data = new_cb_data(L, db);
    lua_rawset(L, LUA_REGISTRYINDEX);
  }
  else
    cb_data = lua_touserdata(L, -1);
  
  lua_pop(L, 1);
  return cb_data;
}


static CB_Data * get_named_cb_data(lua_State * L, DB * db, void * table_key, int name_pos)
{
  CB_Data * cb_data;
  
  push_private_table(L, table_key);
  lua_pushvalue(L, name_pos);
  lua_rawget(L, -2);
  
  if (lua_isnil(L, -1))
  {
    lua_pushvalue(L, name_pos);
    cb_data = new_cb_data(L, db);
    lua_rawset(L, LUA_REGISTRYINDEX);
  }
  else
    cb_data = lua_touserdata(L, -1);
  
  lua_pop(L, 2);
  return cb_data;
}


static void register_callback(lua_State * L, DB * db, void * cb_key, int callback_pos)
{
  push_private_table(L, KEY_KEY2VALUE_TABLE(db));
  lua_pushlightuserdata(L, cb_key);
  lua_pushvalue(L, callback_pos);
  lua_rawset(L, -3);
  lua_pop(L, 1);
}


static void init_callback_usage(lua_State * L, DB * db)
{
  db->L = L;
  db->key2value_pos = 0;	/* lazy initialized in push_callback() */
}


static void push_callback(lua_State * L, DB * db, void * cb_key)
{
  if (db->key2value_pos == 0)	/* lazy initializing of the key2value table */
  {
    push_private_table(L, KEY_KEY2VALUE_TABLE(db));
    db->key2value_pos = lua_gettop(L);
  }
  
  lua_pushlightuserdata(L, cb_key);
  lua_rawget(L, db->key2value_pos);
}


static int pop_break_condition(lua_State * L)
{
  int result;
  
  if (lua_isnil(L, -1))
    result = 0;
  else if (lua_isboolean(L, -1))
    result = lua_toboolean(L, -1);
  else if (lua_isnumber(L, -1))
    result = lua_tonumber(L, -1);
  else
    result = 1;
  
  lua_pop(L, 1);
  return result;
}


static void push_nil_or_string(lua_State * L, const char * str)
{
  if (str)
    lua_pushstring(L, str);
  else
    lua_pushnil(L);
}




/*
 *  Error Handling
 *  ==============
 *
 *  We try to work hard to be bullet proof. We even try to function 
 *  correctly in low memory situations.
 *  Usage errors of the API when detected raise an error also
 *  memory errors and such.
 *  Errors signaled from the sqlite library will result in the
 *  appropriate return codes.
 *
 */


static void report_error(lua_State * L, const char * msg)
{
  lua_settop(L, 0);	/* Clear the stack to make sure, our error message will get a chance */
  lua_pushstring(L, msg);
  lua_error(L);
}



#define checkany(L, narg)	( luaL_checkany((L), (narg)) )
#define checkstr(L, narg)	( luaL_checklstring((L), (narg), 0) )
#define checknumber(L, narg)	( luaL_checknumber((L), (narg)) )
#define checkint(L, narg)	( (int)		checknumber((L), (narg)) )
#define checkdouble(L, narg)	( (double)	checknumber((L), (narg)) )

static void * checkudata(lua_State * L, int narg)
{
  if (!lua_isuserdata(L, narg))
    luaL_typerror(L, narg, "userdata");
  return lua_touserdata(L, narg);
}

#define checkcontext(L, narg)	( (sqlite3_context *)	checkudata((L), (narg)) )
#define checkvalues(L, narg)	( (sqlite3_value **)	checkudata((L), (narg)) )
#define checkstmt(L, narg)	( (Stmt *)		checkudata((L), (narg)) )
#define checkdb(L, narg)	( (DB *)		checkudata((L), (narg)) )

static sqlite3_stmt * checkstmt_stmt(lua_State * L, int narg)
{
  return checkstmt(L, narg)->stmt;
}

static sqlite3 * checkdb_sqlite3(lua_State * L, int narg)
{
  return checkdb(L, narg)->sqlite3;
}

static int checknilornoneorfunc(lua_State * L, int narg)
{
  if (lua_isnil(L, narg) || lua_isnone(L, narg))
    return 0;
  if (lua_isfunction(L, narg))
    return 1;
  luaL_typerror(L, narg, "nil, none or function");
  return 0; /* never reached, make compiler happy... */
}






FUNC( l_sqlite3_bind_null )
{
  lua_pushnumber(L, sqlite3_bind_null(checkstmt_stmt(L, 1), checkint(L, 2)) );
  return 1;
}


FUNC( l_sqlite3_bind_text )
{
  lua_pushnumber(L, sqlite3_bind_text(checkstmt_stmt(L, 1), checkint(L, 2), checkstr(L, 3), lua_strlen(L, 3), SQLITE_TRANSIENT) );
  return 1;
}


FUNC( l_sqlite3_bind_blob )
{
  lua_pushnumber(L, sqlite3_bind_blob(checkstmt_stmt(L, 1), checkint(L, 2), checkstr(L, 3), lua_strlen(L, 3), SQLITE_TRANSIENT) );
  return 1;
}


FUNC( l_sqlite3_bind_int )
{
  lua_pushnumber(L, sqlite3_bind_int(checkstmt_stmt(L, 1), checkint(L, 2), checkint(L, 3)) );
  return 1;
}


FUNC( l_sqlite3_bind_double )
{
  lua_pushnumber(L, sqlite3_bind_double(checkstmt_stmt(L, 1), checkint(L, 2), checkdouble(L, 3)) );
  return 1;
}


FUNC( l_sqlite3_bind_number )
{
  sqlite3_stmt * stmt 	= checkstmt_stmt(L, 1);
  int index 		= checkint(L, 2);
  lua_Number number 	= checknumber(L, 3);
  
  if (IS_INT(number))
    lua_pushnumber(L, sqlite3_bind_int(stmt, index, (int)number) );
  else
    lua_pushnumber(L, sqlite3_bind_double(stmt, index, (double)number) );
  
  return 1;
}


FUNC( l_sqlite3_bind )
{
  sqlite3_stmt * stmt	= checkstmt_stmt(L, 1);
  int index		= checkint(L, 2);
  
  switch(lua_type(L, 3))
  {
    case LUA_TNONE:
    case LUA_TNIL:
      lua_pushnumber(L, sqlite3_bind_null(stmt, index) );
      break;
    
    case LUA_TNUMBER:
      {
        lua_Number number = lua_tonumber(L, 3);
        
        if (IS_INT(number))
          lua_pushnumber(L, sqlite3_bind_int(stmt, index, (int)number) );
        else
          lua_pushnumber(L, sqlite3_bind_double(stmt, index, (double)number) );
      }
      break;
    
    case LUA_TBOOLEAN:
      if (lua_toboolean(L, 3))
        lua_pushnumber(L, sqlite3_bind_int(stmt, index, 1) );
      else
        lua_pushnumber(L, sqlite3_bind_int(stmt, index, 0) );
      break;
    
    case LUA_TSTRING:
      lua_pushnumber(L, sqlite3_bind_text(stmt, index, lua_tostring(L, 3), lua_strlen(L, 3), SQLITE_TRANSIENT) );
      break;
    
    default:
      luaL_argerror(L, 3, "nil, boolean, number or string expected");
  }
  
  return 1;
}


FUNC( l_sqlite3_bind_parameter_count )
{
  lua_pushnumber(L, sqlite3_bind_parameter_count(checkstmt_stmt(L, 1)) );
  return 1;
}


FUNC( l_sqlite3_bind_parameter_name )
{
  const char * name = sqlite3_bind_parameter_name(checkstmt_stmt(L, 1), checkint(L, 2));
  if (name)
    lua_pushstring(L, name);
  else
    lua_pushnil(L);
  return 1;
}


FUNC( l_sqlite3_bind_parameter_name_x )
{
  const char * name = sqlite3_bind_parameter_name(checkstmt_stmt(L, 1), checkint(L, 2));
  if (name && *name)
    lua_pushstring(L, name + 1);	/* Ignore leading '$' or ':' */
  else
    lua_pushnil(L);
  return 1;
}


FUNC( l_sqlite3_busy_timeout )
{
  DB * db	= checkdb(L, 1);
  int timeout	= checkint(L, 2);
  
  delete_private_value(L, KEY_BUSY_DATA(db));
  
  lua_pushnumber(L, sqlite3_busy_timeout(db->sqlite3, timeout) );
  return 1;
}


FUNC( l_sqlite3_changes )
{
  lua_pushnumber(L, sqlite3_changes(checkdb_sqlite3(L, 1)) );
  return 1;
}


FUNC( l_sqlite3_close )
{
  DB * db = checkdb(L, 1);
  
  delete_private_value(L, KEY_KEY2VALUE_TABLE(db));
  delete_private_value(L, KEY_FUNCTION_TABLE(db));
  delete_private_value(L, KEY_COLLATION_TABLE(db));
  delete_private_value(L, KEY_COLLNEEDED_DATA(db));
  delete_private_value(L, KEY_AUTHORIZER_DATA(db));
  delete_private_value(L, KEY_PROGRESS_DATA(db));
  delete_private_value(L, KEY_TRACE_DATA(db));
  delete_private_value(L, KEY_BUSY_DATA(db));
  delete_private_value(L, KEY_COMMIT_DATA(db));
  
  lua_pushnumber(L, sqlite3_close(db->sqlite3) );
  return 1;
}


typedef const char * (*column_text_blob_t)(sqlite3_stmt *, int);

static int l_sqlite3_column_text_or_blob(lua_State * L, column_text_blob_t column_text_blob)
{
  sqlite3_stmt * stmt	=  checkstmt_stmt(L, 1);
  int column		= checkint(L, 2);
  
  lua_pushlstring(L,  column_text_blob(stmt, column), sqlite3_column_bytes(stmt, column));
  return 1;
}

FUNC( l_sqlite3_column_blob )
{
  return l_sqlite3_column_text_or_blob(L, (column_text_blob_t)sqlite3_column_blob);
}

FUNC( l_sqlite3_column_text )
{
  return l_sqlite3_column_text_or_blob(L, (column_text_blob_t)sqlite3_column_text); /* FIXME: remove cast when API changes!!! */
}


FUNC( l_sqlite3_column_int )
{
  lua_pushnumber(L, sqlite3_column_int(checkstmt_stmt(L, 1), checkint(L, 2)) );
  return 1;
}


FUNC( l_sqlite3_column_double )
{
  lua_pushnumber(L, sqlite3_column_double(checkstmt_stmt(L, 1), checkint(L, 2)) );
  return 1;
}


FUNC( l_sqlite3_column_number )
{
  sqlite3_stmt * stmt	= checkstmt_stmt(L, 1);
  int column		= checkint(L, 2);
  
  if ( sqlite3_column_type(stmt, column) == SQLITE_INTEGER )
    lua_pushnumber(L, sqlite3_column_int(stmt, column) );
  else
    lua_pushnumber(L,sqlite3_column_double(stmt, column) );
  
  return 1;
}



  
static void push_column(lua_State * L, sqlite3_stmt * stmt, int column)
{
  switch(sqlite3_column_type(stmt, column))
  {
    case SQLITE_NULL:
      lua_pushnil(L);
      break;
    
    case SQLITE_INTEGER:
      lua_pushnumber(L, sqlite3_column_int(stmt, column));
      break;
    
    case SQLITE_FLOAT:
      lua_pushnumber(L, sqlite3_column_double(stmt, column));
      break;
    
    case SQLITE_TEXT:
      lua_pushlstring(L, sqlite3_column_text(stmt, column), sqlite3_column_bytes(stmt, column));
      break;
    
    case SQLITE_BLOB:
      lua_pushlstring(L, sqlite3_column_blob(stmt, column), sqlite3_column_bytes(stmt, column));
      break;
    
    default:
      lua_pushboolean(L, 0);
  }
}


FUNC( l_sqlite3_column )
{
  push_column(L, checkstmt_stmt(L, 1), checkint(L, 2));
  return 1;
}


/*
 * mode: 0 = direct, 1 = integer, 2 = alphanumeric
 */
static int l_sqlite3_row_mode(lua_State * L, int mode)
{
  /* Old code / Just a reminder / To be removed: 
  ** checkargs(L, 1, 2, CHECK_PTR, CHECK_NILTABLE, 0);
  */
  
  sqlite3_stmt * stmt	= checkstmt_stmt(L, 1);
  int num_columns	= sqlite3_data_count(stmt);	/* Maybe wrong state, so don't use sqlite3_column_count */
  int index;
  
  /* XXX Should really be cleaned up... Fixme! */
  
  if (mode == 0)
    lua_checkstack(L, num_columns);
  else
    if (!lua_istable(L, -1))
      lua_newtable(L);
  
  for (index=0; index<num_columns; index++)
    switch(mode)
    {
      case 0:	/* direct mode */
        push_column(L, stmt, index);
        break;
      
      case 1:	/* integer mode */
        push_column(L, stmt, index);
        lua_rawseti(L, -2, index+1);
        break;
      
      case 2:	/* alphanumeric mode */
        lua_pushstring(L, sqlite3_column_name(stmt, index));
        push_column(L, stmt, index);
        lua_rawset(L, -3);
        break;
      
      default:
        report_error(L, "libluasqlite3: Internal error in sqlite3_row_mode");
    }
  
  if (mode)
    return 1;
  else
    return num_columns;
}

FUNC( l_sqlite3_drow )
{
  return l_sqlite3_row_mode(L, 0);
}

FUNC( l_sqlite3_irow )
{
  return l_sqlite3_row_mode(L, 1);
}

FUNC( l_sqlite3_arow )
{
  return l_sqlite3_row_mode(L, 2);
}


FUNC( l_sqlite3_column_type )
{
  lua_pushnumber(L, sqlite3_column_type(checkstmt_stmt(L, 1), checkint(L, 2)) );
  return 1;
}


FUNC( l_sqlite3_column_count )
{
  lua_pushnumber(L, sqlite3_column_count(checkstmt_stmt(L, 1)) );
  return 1;
}



static int l_sqlite3_column_info(lua_State * L, const char * (*info_func)(sqlite3_stmt*,int) )
{
  const char * info = info_func(checkstmt_stmt(L, 1), checkint(L, 2));
  
  if (info)
    lua_pushstring(L, info);
  else
    lua_pushstring(L, "");
  
  return 1;
}

FUNC( l_sqlite3_column_decltype )
{
  return l_sqlite3_column_info(L, sqlite3_column_decltype);
}

FUNC( l_sqlite3_column_name )
{
  return l_sqlite3_column_info(L, sqlite3_column_name);
}


FUNC( l_sqlite3_complete )
{
  lua_pushboolean(L, sqlite3_complete(checkstr(L, 1)) );
  return 1;
}


FUNC( l_sqlite3_data_count )
{
  lua_pushnumber(L, sqlite3_data_count(checkstmt_stmt(L, 1)) );
  return 1;
}


FUNC( l_sqlite3_errcode )
{
  lua_pushnumber(L, sqlite3_errcode(checkdb_sqlite3(L, 1)) );
  return 1;
}


FUNC( l_sqlite3_errmsg )
{
  lua_pushstring(L, sqlite3_errmsg(checkdb_sqlite3(L, 1)) );
  return 1;
}


FUNC( l_sqlite3_finalize )
{
  lua_pushnumber(L, sqlite3_finalize(checkstmt_stmt(L, 1)) );
  return 1;
}


FUNC( l_sqlite3_interrupt )
{
  sqlite3_interrupt(checkdb_sqlite3(L, 1));
  return 0;
}


FUNC( l_sqlite3_last_insert_rowid )
{
  lua_pushnumber(L, sqlite3_last_insert_rowid(checkdb_sqlite3(L, 1)) );
  return 1;
}


FUNC( l_sqlite3_open )
{
  sqlite3 * sqlite3 	= 0;
  int error 		= sqlite3_open(checkstr(L, 1), &sqlite3);
  
  lua_pushnumber(L, error);
  
  if (sqlite3)
  {
    DB * db = (DB *) lua_newuserdata(L, sizeof(DB));
    db->sqlite3 = sqlite3;
  }
  else
    lua_pushnil(L);
  
  return 2;	/* error code, database */
}


FUNC( l_sqlite3_prepare )
{
  /* XXX This piece of code is not so nice. This piece should be redone... */
  
  DB * db			= checkdb(L, 1);
  const char * sql		= checkstr(L, 2);
  int sql_size			= lua_strlen(L, 2);
  const char * leftover		= 0;
  sqlite3_stmt * sqlite3_stmt 	= 0;
  int error, leftover_size;
  Stmt * stmt;
  
  init_callback_usage(L, db); 	/* Needed by trace handler... FIXME: maybe to be removed... */
  
  error = sqlite3_prepare(db->sqlite3, sql, sql_size, &sqlite3_stmt, &leftover);
  
  leftover_size = leftover ? sql + sql_size - leftover : 0;
  
  lua_pushnumber(L, error);
  
  stmt = lua_newuserdata(L, sizeof(Stmt));
  stmt->db = checkdb(L, 1);
  stmt->stmt = sqlite3_stmt;
  
  if (leftover_size > 0)
    lua_pushlstring(L, leftover, leftover_size);
  else
    lua_pushnil(L);
  
  return 3;	/* error code, statement, left over */
}


FUNC( l_sqlite3_reset )
{
  lua_pushnumber(L, sqlite3_reset(checkstmt_stmt(L, 1)) );
  return 1;
}


FUNC( l_sqlite3_step )
{
  Stmt * stmt = checkstmt(L, 1);
  init_callback_usage(L, stmt->db);
  lua_pushnumber(L, sqlite3_step(stmt->stmt) );
  return 1;
}


FUNC( l_sqlite3_total_changes )
{
  lua_pushnumber(L, sqlite3_total_changes(checkdb_sqlite3(L, 1)) );
  return 1;
}



static int exec_callback_wrapper(void * cb_data, int num_columns, char ** values, char ** names)
{
  int index;
  lua_State * L = (lua_State *) cb_data;
  
  lua_pushvalue(L, 3);	/* Callback function, resulting stack position 4 */
  lua_newtable(L);	/* Value array, resulting stack position 5 */
  lua_newtable(L);	/* Names array, resulting stack position 6 */
  
  for(index=0; index<num_columns; index++)
  {
    lua_pushstring(L, values[index]);	/* Value */
    lua_rawseti(L, 5, index+1);		/* C-index are 0 based, Lua index are 1 based... */
    
    lua_pushstring(L, names[index]);	/* Name */
    lua_rawseti(L, 6, index+1);
  }
  
  if ( lua_pcall(L, 2, 1, 0) )		/* In: 2 arrays, Out: result code, On error: leave */
  {
    lua_pop(L, 1);			/* delete error message */
    return 1;
  }
  
  return pop_break_condition(L);
}

FUNC( l_sqlite3_exec )
{
  DB * db		= checkdb(L, 1);
  sqlite3_callback cb;
  void * cb_data;
  
  if ( checknilornoneorfunc(L, 3) )
  {
    cb = exec_callback_wrapper;
    cb_data = L;
  }
  else
  {
    cb      = 0;
    cb_data = 0;
  }
  
  init_callback_usage(L, db);
  
  lua_pushnumber(L, sqlite3_exec(db->sqlite3, checkstr(L, 2), cb, cb_data, 0) );
  return 1;
}



static void func_callback_wrapper(int which, sqlite3_context * ctx, int num_args, sqlite3_value ** values)
{
  CB_Data *	cb_data	= sqlite3_user_data(ctx);
  DB *		db 	= cb_data->db;
  lua_State * 	L	= db->L;
  
  switch(which)
  {
    case 0:	push_callback(L, db, KEY_XFUNC(cb_data)); break;
    case 1:	push_callback(L, db, KEY_XSTEP(cb_data)); break;
    case 2:	push_callback(L, db, KEY_XFINAL(cb_data)); break;
  }
  
  if (lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    fprintf(stderr, "libluasqlite3: func_callback_wrapper: Warning: function is null\n");
    return;
  }
  
  lua_pushlightuserdata(L, ctx);
  
  if (values)
  {
    lua_pushnumber(L, num_args);
    lua_pushlightuserdata(L, values);
  }
  
  if (lua_pcall(L, values ? 3 : 1, 0, 0))
  {
    fprintf(stderr, "libluasqlite3: func_callback_wrapper: Warning: user function error: %s\n", lua_tostring(L, -1));
    sqlite3_result_error(ctx, lua_tostring(L, -1), lua_strlen(L, -1));
    lua_pop(L, 1);
  }
}

static void xfunc_callback_wrapper(sqlite3_context * ctx, int num_args, sqlite3_value ** values)
{
  func_callback_wrapper(0, ctx, num_args, values);
}

static void xstep_callback_wrapper(sqlite3_context * ctx, int num_args, sqlite3_value ** values)
{
  func_callback_wrapper(1, ctx, num_args, values);
}

static void xfinal_callback_wrapper(sqlite3_context * ctx)
{
  func_callback_wrapper(2, ctx, 0, 0);
}

FUNC( l_sqlite3_create_function )
{
  DB * db		= checkdb(L, 1);
  CB_Data * cb_data	= get_function_cb_data(L, db, 2);
  
  void (*xfunc)(sqlite3_context *, int, sqlite3_value **) = 0;
  void (*xstep)(sqlite3_context *, int, sqlite3_value **) = 0;
  void (*xfinal)(sqlite3_context *) = 0;
  
  if ( checknilornoneorfunc(L, 4) )
    xfunc = xfunc_callback_wrapper;
  else
    xfunc = 0;
  
  if ( checknilornoneorfunc(L, 5) )
    xstep = xstep_callback_wrapper;
  else
    xstep = 0;
  
  if ( checknilornoneorfunc(L, 6) )
    xfinal = xfinal_callback_wrapper;
  else
    xfinal = 0;
  
  register_callback(L, db, KEY_XFUNC(cb_data), 4);
  register_callback(L, db, KEY_XSTEP(cb_data), 5);
  register_callback(L, db, KEY_XFINAL(cb_data), 6);
  
  lua_pushnumber(L,
    sqlite3_create_function (
      db->sqlite3,
      checkstr(L, 2),
      checkint(L, 3),
      SQLITE_UTF8,
      cb_data,
      xfunc,
      xstep,
      xfinal ));
  
  return 1;
}


int xcompare_callback_wrapper(void * cb_data, int len_a, const void * str_a, int len_b, const void * str_b)
{
  DB *		db = CB_DATA(cb_data)->db;
  lua_State *	L  = db->L;
  int result;
  
  push_callback(L, db, KEY_XCOMPARE(cb_data));
  lua_pushlstring(L, str_a, len_a);
  lua_pushlstring(L, str_b, len_b);
  
  if ( lua_pcall(L, 2, 1, 0) )
    result = 0;			/* No way to signal errors to sqlite */
  else
    result = (int) lua_tonumber(L, -1);
  
  lua_pop(L, 1);
  return result;
}

FUNC( l_sqlite3_create_collation )
{
  DB * db		= checkdb(L, 1);
  CB_Data * cb_data	= get_collation_cb_data(L, db, 2);
  
  int (*xcompare)(void *, int, const void *, int, const void *);
  
  if ( checknilornoneorfunc(L, 3) )
    xcompare = xcompare_callback_wrapper;
  else
    xcompare = 0;
  
  register_callback(L, db, KEY_XCOMPARE(cb_data), 3);
  
  lua_pushnumber(L, sqlite3_create_collation(
                db->sqlite3, checkstr(L, 2), SQLITE_UTF8, cb_data, xcompare) );
  return 1;
}


void xneeded_callback_wrapper(void * cb_data, sqlite3 * sqlite3, int eTextRep, const char * collation_name)
{
  DB * 		db = CB_DATA(cb_data)->db;
  lua_State * 	L  = db->L;
  
  push_callback(L, db, KEY_XNEEDED(cb_data));
  lua_pushstring(L, collation_name);
  
  if (lua_pcall(L, 1, 0, 0))
    lua_pop(L, 1);
}

FUNC( l_sqlite3_collation_needed )
{
  DB * db		= checkdb(L, 1);
  CB_Data * cb_data	= get_collneeded_cb_data(L, db);
  
  void (*xneeded)(void *, sqlite3 *, int eTextRep, const char *);
  
  if ( checknilornoneorfunc(L, 2) )
    xneeded = xneeded_callback_wrapper;
  else
    xneeded = 0;
  
  register_callback(L, db, KEY_XNEEDED(cb_data), 2);
  
  lua_pushnumber(L, sqlite3_collation_needed(db->sqlite3, cb_data, xneeded) );
  return 1;
}




static void xtrace_callback_wrapper(void * cb_data, const char * str)
{
  DB *		db = CB_DATA(cb_data)->db;
  lua_State *	L  = db->L;
  
  push_callback(L, db, KEY_XTRACE(cb_data));
  lua_pushstring(L, str);
  
  if ( lua_pcall(L, 1, 0, 0) )
    lua_pop(L, 1);		/* pop error message and delete it (errors are ignored) */
}

FUNC( l_sqlite3_trace )
{
  DB * db		= checkdb(L, 1);
  CB_Data * cb_data	= get_trace_cb_data(L, db);
  
  void (*xtrace)(void *, const char *);
  
  if ( checknilornoneorfunc(L, 2) )
    xtrace = xtrace_callback_wrapper;
  else
    xtrace = 0;
  
  register_callback(L, db, KEY_XTRACE(cb_data), 2);
  
  sqlite3_trace(db->sqlite3, xtrace, cb_data);
  
  lua_pushnumber(L, SQLITE_OK);
  return 1;
}




FUNC( l_sqlite3_result_null )
{
  sqlite3_result_null(checkcontext(L, 1));
  return 0;
}


FUNC( l_sqlite3_result_error )
{
  sqlite3_result_error(checkcontext(L, 1), checkstr(L, 2), lua_strlen(L, 2));
  return 0;
}


FUNC( l_sqlite3_result_double )
{
  sqlite3_result_double(checkcontext(L, 1), checkdouble(L, 2));
  return 0;
}


FUNC( l_sqlite3_result_int )
{
  sqlite3_result_int(checkcontext(L, 1), checkint(L, 2));
  return 0;
}


FUNC( l_sqlite3_result_number )
{
  lua_Number number = checknumber(L, 2);
  
  if (IS_INT(number))
    sqlite3_result_int(checkcontext(L, 1), (int)number);
  else
    sqlite3_result_double(checkcontext(L, 1), (double)number);
  
  return 0;
}


FUNC( l_sqlite3_result_blob )
{
  sqlite3_result_blob(checkcontext(L, 1), checkstr(L, 2), lua_strlen(L, 2), SQLITE_TRANSIENT);
  return 0;
}


FUNC( l_sqlite3_result_text )
{
  sqlite3_result_text(checkcontext(L, 1), checkstr(L, 2), lua_strlen(L, 2), SQLITE_TRANSIENT);
  return 0;
}


FUNC( l_sqlite3_result_value )
{
  sqlite3_value ** values = checkvalues(L, 2);
  int index = checkint(L, 3);
  sqlite3_result_value(checkcontext(L, 1), values[index] );
  return 0;
}

FUNC( l_sqlite3_result )
{
  sqlite3_context * context = checkcontext(L, 1);
  
  switch(lua_type(L, 2))
  {
    case LUA_TNONE:
    case LUA_TNIL:
      sqlite3_result_null(context);
      break;
    
    case LUA_TNUMBER:
      {
        lua_Number number = lua_tonumber(L, 2);
        
        if (IS_INT(number))
          sqlite3_result_int(context, (int)number);
        else
          sqlite3_result_double(context, (double)number);
      }
      break;
    
    case LUA_TBOOLEAN:
      if (lua_toboolean(L, 2))
        sqlite3_result_int(context, 1);
      else
        sqlite3_result_int(context, 0);
      break;
    
    case LUA_TSTRING:
      sqlite3_result_text(context, lua_tostring(L, 2), lua_strlen(L, 2), SQLITE_TRANSIENT);
      break;
    
    default:
      report_error(L, "libluasqlite3: Api usage error: Invalid argument to l_sqlite3_result:");
  }
  
  return 0;
}


FUNC( l_sqlite3_aggregate_count )
{
  lua_pushnumber(L, sqlite3_aggregate_count(checkcontext(L, 1)) );
  return 1;
}


FUNC( l_sqlite3_aggregate_context )
{
  lua_pushlightuserdata(L, sqlite3_aggregate_context(checkcontext(L, 1), 1));
  return 1;
}


FUNC( l_sqlite3_value_int )
{
  sqlite3_value ** values = checkvalues(L, 1);
  int index = checkint(L, 2);
  lua_pushnumber(L, sqlite3_value_int(values[index]) );
  return 1;
}


FUNC( l_sqlite3_value_double )
{
  sqlite3_value ** values = checkvalues(L, 1);
  int index = checkint(L, 2);
  lua_pushnumber(L, sqlite3_value_double(values[index]) );
  return 1;
}


FUNC( l_sqlite3_value_number )
{
  sqlite3_value ** values = checkvalues(L, 1);
  int index = checkint(L, 2);
  sqlite3_value * value = values[index];
  if (sqlite3_value_type(value) == SQLITE_INTEGER)
    lua_pushnumber(L, sqlite3_value_int(value) );
  else
    lua_pushnumber(L, sqlite3_value_double(value) );
  return 1;
}


FUNC( l_sqlite3_value_blob )
{
  sqlite3_value ** values = checkvalues(L, 1);
  int index = checkint(L, 2);
  lua_pushlstring(L, sqlite3_value_blob(values[index]), sqlite3_value_bytes(values[index]) );
  return 1;
}


FUNC( l_sqlite3_value_text )
{
  sqlite3_value ** values = checkvalues(L, 1);
  int index = checkint(L, 2);
  lua_pushlstring(L, sqlite3_value_text(values[index]), sqlite3_value_bytes(values[index]) );
  return 1;
}


FUNC( l_sqlite3_value )
{
  sqlite3_value ** values	= checkvalues(L, 1);
  int index			= checkint(L, 2);
  sqlite3_value * value		= values[index];
  
  switch(sqlite3_value_type(value))
  {
    case SQLITE_INTEGER:
      lua_pushnumber(L, sqlite3_value_int(value) );
      break;
    
    case SQLITE_FLOAT:
      lua_pushnumber(L, sqlite3_value_double(value) );
      break;
    
    case SQLITE_TEXT:
      lua_pushlstring(L, sqlite3_value_text(value), sqlite3_value_bytes(value) );
      break;
    
    case SQLITE_BLOB:
      lua_pushlstring(L, sqlite3_value_blob(value), sqlite3_value_bytes(value) );
      break;
    
    case SQLITE_NULL:
      lua_pushnil(L);
      break;
    
    default:
      report_error(L, "libluasqlite3: Internal error: Unknonw SQLITE data type.");
  }
  return 1;
}


FUNC( l_sqlite3_value_type )
{
  sqlite3_value ** values = checkvalues(L, 1);
  int index = checkint(L, 2);
  lua_pushnumber(L, sqlite3_value_type(values[index]) );
  return 1;
}


FUNC( l_sqlite3_libversion )
{
  lua_pushstring(L, sqlite3_libversion() );
  return 1;
}


int xcommit_callback_wrapper(void * cb_data)
{
  DB *		db = CB_DATA(cb_data)->db;
  lua_State *	L  = db->L;
  
  push_callback(L, db, KEY_XCOMMIT(cb_data));
  
  if ( lua_pcall(L, 0, 1, 0) )
  {
    lua_pop(L, 1);
    return 1;		/* on errors, rollback */
  }
  
  return pop_break_condition(L);
}

FUNC( l_sqlite3_commit_hook )
{
  DB * db = checkdb(L, 1);
  CB_Data * cb_data = get_commit_cb_data(L, db);
  
  int (*xcommit)(void *);
  
  if ( checknilornoneorfunc(L, 1) )
    xcommit = xcommit_callback_wrapper;
  else
    xcommit = 0;
  
  register_callback(L, db, KEY_XCOMMIT(cb_data), 2);
  sqlite3_commit_hook(db->sqlite3, xcommit, cb_data);
  
  lua_pushnumber(L, sqlite3_errcode(db->sqlite3) );
  return 1;
}


int xprogress_callback_wrapper(void * cb_data)
{
  DB *		db = CB_DATA(cb_data)->db;
  lua_State *	L  = db->L;
  
  push_callback(L, db, KEY_XPROGRESS(cb_data));
  
  if ( lua_pcall(L, 0, 1, 0) )
  {
    lua_pop(L, 1);
    return 1;		/* on errors, rollback */
  }
  
  return pop_break_condition(L);
  return 1;
}

FUNC( l_sqlite3_progress_handler )
{
  DB * db		= checkdb(L, 1);
  CB_Data * cb_data	= get_progress_cb_data(L, db);
  
  int (*xprogress)(void *);
  
  if ( checknilornoneorfunc(L, 1) )
    xprogress = xprogress_callback_wrapper;
  else
    xprogress = 0;
  
  register_callback(L, db, KEY_XPROGRESS(cb_data), 3);
  sqlite3_progress_handler(db->sqlite3, checkint(L, 2), xprogress, cb_data);
  
  lua_pushnumber(L, sqlite3_errcode(db->sqlite3) );
  return 1;
}


int xbusy_callback_wrapper(void * cb_data, int num_called)
{
  DB *		db = CB_DATA(cb_data)->db;
  lua_State *	L  = db->L;
  
  push_callback(L, db, KEY_XBUSY(cb_data));
  lua_pushnumber(L, num_called);
  
  if ( lua_pcall(L, 1, 1, 0) )
  {
    lua_pop(L, 1);
    return 0;		/* On errors, sqlite should return SQLITE_BUSY */
  }
  
  return pop_break_condition(L);	/* WARNING: In reality, the semantic is inverted !!!*/
}

FUNC( l_sqlite3_busy_handler )
{
  DB * db		= checkdb(L, 1);
  CB_Data * cb_data	= get_busy_cb_data(L, db);
  
  int (*xbusy)(void *, int);
  
  if ( checknilornoneorfunc(L, 2) )
    xbusy = xbusy_callback_wrapper;
  else
    xbusy = 0;
  
  register_callback(L, db, KEY_XBUSY(cb_data), 2);
  
  lua_pushnumber(L, sqlite3_busy_handler(db->sqlite3, xbusy, cb_data) );
  return 1;
}



int xauth_callback_wrapper(void * cb_data, int auth_request, const char * name1, const char * name2, const char * db_name, const char * trigger_name)
{
  DB *		db = CB_DATA(cb_data)->db;
  lua_State *	L  = db->L;
  int result;
  
  push_callback(L, db, KEY_XAUTH(cb_data));
  lua_pushnumber(L, auth_request);
  push_nil_or_string(L, name1);
  push_nil_or_string(L, name2);
  push_nil_or_string(L, db_name);
  push_nil_or_string(L, trigger_name);
  
  if ( lua_pcall(L, 5, 1, 0) )
  {
    lua_pop(L, 1);
    return SQLITE_DENY;		/* On errors, sqlite should deny access */
  }
  
  if (lua_isnumber(L, -1))
    result = lua_tonumber(L, -1);
  else
    result = SQLITE_DENY;	/* Wrong result values should deny access */
  
  lua_pop(L, 1);
  return result;
}

FUNC( l_sqlite3_set_authorizer )
{
  DB * db		= checkdb(L, 1);
  CB_Data * cb_data	= get_authorizer_cb_data(L, db);
  
  int (*xauth)(void *, int, const char *, const char *, const char *, const char *);
  
  if ( checknilornoneorfunc(L, 2) )
    xauth = xauth_callback_wrapper;
  else
    xauth = 0;
  
  register_callback(L, db, KEY_XAUTH(cb_data), 2);
  
  lua_pushnumber(L, sqlite3_set_authorizer(db->sqlite3, xauth, cb_data) );
  return 1;
}






typedef struct { char * name; int (*func)(lua_State *); } f_entry;
typedef struct { char * name; int value; } d_entry;



static void f(lua_State * L, f_entry entries[])
{
  int index;
  lua_newtable(L);
  for( index=0; entries[index].name; index++)
  {
    lua_pushstring(L, entries[index].name);
    lua_pushcfunction(L, entries[index].func);
    lua_rawset(L, -3);
  }
}



static void d(lua_State * L, d_entry entries[])
{
  int index;
  lua_newtable(L);
  for( index=0; entries[index].name; index++)
  {
    lua_pushstring(L, entries[index].name);
    lua_pushnumber(L, entries[index].value);
    lua_rawset(L, -3);
  }
}



f_entry api_entries[] = {
  { "bind_null",		l_sqlite3_bind_null },
  { "bind_text",		l_sqlite3_bind_text },
  { "bind_blob",		l_sqlite3_bind_blob },
  { "bind_int",			l_sqlite3_bind_int },
  { "bind_double",		l_sqlite3_bind_double },
  { "bind_number",		l_sqlite3_bind_number },
  { "bind",			l_sqlite3_bind },
  { "bind_parameter_name",	l_sqlite3_bind_parameter_name },
  { "bind_parameter_name_x",	l_sqlite3_bind_parameter_name_x },
  { "bind_parameter_count",	l_sqlite3_bind_parameter_count },
  { "busy_timeout",		l_sqlite3_busy_timeout },
  { "changes",			l_sqlite3_changes },
  { "close",			l_sqlite3_close },
  { "column_blob",		l_sqlite3_column_blob },
  { "column_text",		l_sqlite3_column_text },
  { "column_int",		l_sqlite3_column_int },
  { "column_double",		l_sqlite3_column_double },
  { "column_number",		l_sqlite3_column_number },
  { "column",			l_sqlite3_column },
  { "column_type",		l_sqlite3_column_type },
  { "column_count",		l_sqlite3_column_count },
  { "column_decltype",		l_sqlite3_column_decltype },
  { "column_name",		l_sqlite3_column_name },
  { "complete",			l_sqlite3_complete },
  { "data_count",		l_sqlite3_data_count },
  { "errcode",			l_sqlite3_errcode },
  { "errmsg",			l_sqlite3_errmsg },
  { "finalize",			l_sqlite3_finalize },
  { "interrupt",		l_sqlite3_interrupt },
  { "last_insert_rowid",	l_sqlite3_last_insert_rowid },
  { "open",			l_sqlite3_open },
  { "prepare",			l_sqlite3_prepare },
  { "reset",			l_sqlite3_reset },
  { "step",			l_sqlite3_step },
  { "total_changes",		l_sqlite3_total_changes },
  { "exec",			l_sqlite3_exec },
  { "create_function",		l_sqlite3_create_function },
  { "create_collation",		l_sqlite3_create_collation },
  { "trace",			l_sqlite3_trace },
  { "collation_needed",		l_sqlite3_collation_needed },
  { "result_null",		l_sqlite3_result_null },
  { "result_error",		l_sqlite3_result_error },
  { "result_double",		l_sqlite3_result_double },
  { "result_int",		l_sqlite3_result_int },
  { "result_number",		l_sqlite3_result_number },
  { "result_blob",		l_sqlite3_result_blob },
  { "result_text",		l_sqlite3_result_text },
  { "result_value",		l_sqlite3_result_value },
  { "result",			l_sqlite3_result },
  { "aggregate_count",		l_sqlite3_aggregate_count },
  { "aggregate_context",	l_sqlite3_aggregate_context },
  { "value_int",		l_sqlite3_value_int },
  { "value_double",		l_sqlite3_value_double },
  { "value_number",		l_sqlite3_value_number },
  { "value_blob",		l_sqlite3_value_blob },
  { "value_text",		l_sqlite3_value_text },
  { "value",			l_sqlite3_value },
  { "value_type",		l_sqlite3_value_type },
  { "libversion",		l_sqlite3_libversion },
  { "commit_hook",		l_sqlite3_commit_hook },
  { "progress_handler",		l_sqlite3_progress_handler },
  { "busy_handler",		l_sqlite3_busy_handler },
  { "set_authorizer",		l_sqlite3_set_authorizer },
  { "drow",			l_sqlite3_drow },
  { "irow",			l_sqlite3_irow },
  { "arow",			l_sqlite3_arow },
  { 0, 0 }
};


d_entry error_entries[] = {
  { "OK",			SQLITE_OK },
  { "ERROR",			SQLITE_ERROR },
  { "INTERNAL",			SQLITE_INTERNAL },
  { "PERM",			SQLITE_PERM },
  { "ABORT",			SQLITE_ABORT },
  { "BUSY",			SQLITE_BUSY },
  { "LOCKED",			SQLITE_LOCKED },
  { "NOMEM",			SQLITE_NOMEM },
  { "READONLY",			SQLITE_READONLY },
  { "INTERRUPT",		SQLITE_INTERRUPT },
  { "IOERR",			SQLITE_IOERR },
  { "CORRUPT",			SQLITE_CORRUPT },
  { "NOTFOUND",			SQLITE_NOTFOUND },
  { "FULL",			SQLITE_FULL },
  { "CANTOPEN",			SQLITE_CANTOPEN },
  { "PROTOCOL",			SQLITE_PROTOCOL },
  { "EMPTY",			SQLITE_EMPTY },
  { "SCHEMA",			SQLITE_SCHEMA },
  { "TOOBIG",			SQLITE_TOOBIG },
  { "CONSTRAINT",		SQLITE_CONSTRAINT },
  { "MISMATCH",			SQLITE_MISMATCH },
  { "MISUSE",			SQLITE_MISUSE },
  { "NOLFS",			SQLITE_NOLFS },
  { "AUTH",			SQLITE_AUTH },
  { "ROW",			SQLITE_ROW },
  { "DONE",			SQLITE_DONE },
  { "DENY",			SQLITE_DENY },
  { "IGNORE",			SQLITE_IGNORE },
  { 0, 0 }
};


d_entry type_entries[] = {
  { "INTEGER",			SQLITE_INTEGER },
  { "INT",			SQLITE_INTEGER },
  { "FLOAT",			SQLITE_FLOAT },
  { "DOUBLE",			SQLITE_FLOAT },
  { "TEXT",			SQLITE_TEXT },
  { "BLOB",			SQLITE_BLOB },
  { "NULL",			SQLITE_NULL },
  { 0, 0 }
};


d_entry auth_entries[] = {
  { "CREATE_INDEX",		SQLITE_CREATE_INDEX },
  { "CREATE_TABLE",		SQLITE_CREATE_TABLE },
  { "CREATE_TRIGGER",		SQLITE_CREATE_TRIGGER },
  { "CREATE_VIEW",		SQLITE_CREATE_VIEW },
  { "CREATE_TEMP_INDEX",	SQLITE_CREATE_TEMP_INDEX },
  { "CREATE_TEMP_TABLE",	SQLITE_CREATE_TEMP_TABLE },
  { "CREATE_TEMP_TRIGGER",	SQLITE_CREATE_TEMP_TRIGGER },
  { "CREATE_TEMP_VIEW",	SQLITE_CREATE_TEMP_VIEW },
  { "DROP_INDEX",		SQLITE_DROP_INDEX },
  { "DROP_TABLE",		SQLITE_DROP_TABLE },
  { "DROP_TRIGGER",		SQLITE_DROP_TRIGGER },
  { "DROP_VIEW",		SQLITE_DROP_VIEW },
  { "DROP_TEMP_INDEX",		SQLITE_DROP_TEMP_INDEX },
  { "DROP_TEMP_TABLE",		SQLITE_DROP_TEMP_TABLE },
  { "DROP_TEMP_TRIGGER",	SQLITE_DROP_TEMP_TRIGGER },
  { "DROP_TEMP_VIEW",		SQLITE_DROP_TEMP_VIEW },
  { "INSERT",			SQLITE_INSERT },
  { "PRAGMA",			SQLITE_PRAGMA },
  { "READ",			SQLITE_READ },
  { "SELECT",			SQLITE_SELECT },
  { "TRANSACTION",		SQLITE_TRANSACTION },
  { "UPDATE",			SQLITE_UPDATE },
  { "ATTACH",			SQLITE_ATTACH },
  { "DETACH",			SQLITE_DETACH },
  { 0, 0 }
};




int luaopen_sqlite3(lua_State * L)
{
  lua_newtable(L);
  f(L, api_entries);
  lua_setfield(L, -2, "api");
  d(L, error_entries);
  lua_setfield(L, -2, "errors");
  d(L, type_entries);
  lua_setfield(L, -2, "types");
  d(L, auth_entries);
  lua_setfield(L, -2, "auth");
  
  return 1;	/* api, error codes, type codes, auth requests */
}

int luaopen_sqlite3_core(lua_State * L){ return luaopen_sqlite3(L); }
