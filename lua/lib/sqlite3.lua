
--[[--------------------------------------------------------------------------

    Author: Michael Roth <mroth@nessie.de>

    Copyright (c) 2004, 2005 Michael Roth <mroth@nessie.de>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

--]]--------------------------------------------------------------------------




--[[

TODO:
	collation			set_collation(name, func)
	collation_needed		set_collation_handler(func)
	progress_handler		set_progress_handler(func)
	authorizer			set_auth_handler(func)
	commit_hook			set_commit_handler(func)

--]]





require "libluasqlite3-loader"

local api, ERR, TYPE, AUTH = load_libluasqlite3()

local db_class = { }
local stmt_class = { }





local function check_stmt(stmt)
  assert(type(stmt.handles) == "table", "Prepared statement expected")
  return stmt
end

local function check_single_stmt(stmt)
  assert(type(stmt.handles) == "table" and table.getn(stmt.handles) == 1, "Single prepared statement expected")
  return stmt.handles[1]
end

local function check_db(db)
  assert(db.handle, "Open database handle expected")
  return db
end

local function check_table(tab, msg)
  assert(type(tab) == "table", msg)
  return tab
end

local function check_string(str, msg)
  assert( type(str) == "string", msg )
  return str
end

local function check_number(num, msg)
  assert( type(num) == "number", msg )
  return num
end



-----------------------------------------------------
-- Error test und report helper for sqlite3 errors --
-----------------------------------------------------

local function is_error(status)
  return not ( status == ERR.OK or status == ERR.ROW or status == ERR.DONE )
end

local function is_row(status)
  return status == ERR.ROW
end

local function is_done(status)
  return status == ERR.DONE
end

local function errmsg(db_handle)
  return api.errmsg(db_handle) or "Unknown error"
end



-------------------------------------------------------------------------
-- Creates an oject. An object is a table with itself set as metatable --
-------------------------------------------------------------------------

local function object()
  local t = { }
  setmetatable(t, t)
  return t
end


-----------------------------------
-- Registry for tables (objects) --
-----------------------------------

local function create_registry()
  return { 0 }
end

local function register(registry, object)
  local id = registry[1]
  if id == 0 then
    table.insert(registry, object)
    return table.getn(registry)
  else
    registry[1] = registry[id]
    registry[id] = object
    return id
  end
end

local function unregister(registry, id)
  registry[id] = registry[1]
  registry[1] = id
end





-------------
-- sqlite3 --
-------------

sqlite3 = { }

function sqlite3.open(filename)
  check_string(filename, "Filename as string expected")
  
  local status, handle = api.open(filename)
  
  if is_error(status) then
    local errmsg = errmsg(handle)
    api.close(handle)
    return nil, errmsg
  end
  
  local db = object()
  
  db.__gc	= db_class.close
  db.__index	= db_class
  db.filename	= filename
  db.handle 	= handle
  db.stmts	= create_registry()
  
  return db
end


function sqlite3.open_memory()
  return sqlite3.open(":memory:")
end



--------------------
-- Database Class --
--------------------

function db_class.close(db)
  check_db(db)
  
  for _, obj in ipairs(db.stmts) do
    if type(obj) == "table" then
      obj:close()
    end
  end
  
  local status = api.close(db.handle)
  
  if is_error(status) then
    return nil, errmsg(db.handle)
  end
  
  db.handle = nil
  db.stmts  = nil
  db.__gc   = nil
  
  return db
end


function db_class.interrupt(db)
  check_db(db)
  
  local status = api.interrupt(db.handle)
  
  if is_error(status) then
    return nil, errmsg(db.handle)
  end
  
  return db
end


function db_class.last_insert_rowid(db)
  check_db(db)
  return api.last_insert_rowid(db.handle)
end


function db_class.changes(db)
  check_db(db)
  return api.changes(db.handle)
end


function db_class.total_changes(db)
  check_db(db)
  return api.total_changes(db.handle)
end


function db_class.exec(db, sql)
  check_db(db)
  check_string(sql)
  
  local status = api.exec(db.handle, sql)
  
  if is_error(status) then
    return nil, errmsg(db.handle)
  end
  
  return db
end


function db_class.irows(db, sql, tab)
  check_db(db)
  return db:prepare(sql):irows(tab, true)
end


function db_class.rows(db, sql, tab)
  check_db(db)
  return db:prepare(sql):rows(tab, true)
end


function db_class.cols(db, sql)
  check_db(db)
  return db:prepare(sql):cols(true)
end


function db_class.first_irow(db, sql, tab)
  check_db(db)
  return db:prepare(sql):first_irow(tab, true)
end


function db_class.first_row(db, sql, tab)
  check_db(db)
  return db:prepare(sql):first_row(tab, true)
end


function db_class.first_cols(db, sql)
  check_db(db)
  return db:prepare(sql):first_cols(true)
end


function db_class.prepare(db, paranames, sql)
  check_db(db)
  
  if sql == nil then
    sql = paranames
    paranames = nil
  end
  
  check_string(sql, "db:prepare: SQL statement as string expected")
  
  local function cleanup(handles)
    for _, handle in ipairs(handles) do
      api.finalize(handle)
    end
  end
  
  local function count_parameters(handles)
    local parameter_count = 0
    for _, handle in ipairs(handles) do
      parameter_count = parameter_count + api.bind_parameter_count(handle)
    end
    return parameter_count
  end
  
  local function build_handles(sql)
    local status, handle
    local remaining = sql
    local handles = { }
    
    while remaining do
      status, handle, remaining = api.prepare(db.handle, remaining)
      
      if is_error(status) then
        local errmsg = errmsg(db.handle)
        cleanup(handles)
        return nil, errmsg
      end
      
      table.insert(handles, handle)
    end
    
    return handles
  end
  
  local function anonymous_parameters(handles)
    for _, handle in ipairs(handles) do
      for i = 1, api.bind_parameter_count(handle) do
        if api.bind_parameter_name_x(handle, i) then
          return false
        end
      end
    end
    return true
  end
  
  local function named_parameters(handles)
    for _, handle in ipairs(handles) do
      for i = 1, api.bind_parameter_count(handle) do
        if not api.bind_parameter_name_x(handle, i) then
          return false
        end
      end
    end
    return true
  end
  
  local function create_mapping(handles, paranames)
    local invers = { }
    
    for index, name in ipairs(paranames) do
      invers[name] = index
    end
    
    local mapping = { }
    
    for _, handle in ipairs(handles) do
      for index = 1, api.bind_parameter_count(handle) do
        local parameter_name = api.bind_parameter_name_x(handle, index)
        local pos = invers[parameter_name]
        if pos == nil then
          cleanup(handles)
          return nil, "db:prepare: Unknown parameter name '" .. parameter_name .. "'in statement."
        end
        table.insert(mapping, pos)
      end
    end
    
    return mapping
  end
  
  local function collect_parameter_names(handles)
    local seen = { }
    local names = { }
    
    for _, handle in ipairs(handles) do
      for index = 1, api.bind_parameter_count(handle) do
        local parameter_name = api.bind_parameter_name_x(handle, index)
        if not seen[parameter_name] then
          table.insert(names, parameter_name)
          seen[parameter_name] = true
        end
      end
    end
    
    return names
  end
  
  local function fix_parameter_names(unfixed_parameters)
    local fixed_parameters = { }
    for _, unfixed_name in ipairs(unfixed_parameters) do
      local _, _, fixed_name = string.find(unfixed_name, "^[:$]?(%a%w*)$")
      if not fixed_name then
        return nil, "db:prepare: Invalid parameter name: '" .. unfixed_name .."'."
      end
      table.insert(fixed_parameters, fixed_name)
    end
    return fixed_parameters
  end
  
  local function create_stmt(db, handles, parameter_count)
    local stmt = object()
    stmt.__gc		= stmt_class.close
    stmt.__index	= stmt_class
    stmt.handles	= handles
    stmt.db		= db
    stmt.reg_id		= register(db.stmts, stmt)
    stmt.parameter_count= parameter_count
    return stmt
  end
  
  local handles, errmsg = build_handles(sql)
  
  if errmsg then
    return nil, errmsg
  end
  
  local parameter_count = count_parameters(handles)
  
  if parameter_count == 0 then			-- No parameters at all
    return create_stmt(db, handles, 0)
  else
    if anonymous_parameters(handles) then	-- All parameters are anonymous ("?")
      
      return create_stmt(db, handles, parameter_count)
      
    elseif named_parameters(handles) then	-- All parameters are named (":foobar" & "$foobar")
      
      if paranames then				-- Fixed mapping of parameter names
        
        check_table(paranames, "db:prepare: Names of parameters expected as strings")
        
        local fixed_parameter_names, errmsg = fix_parameter_names(paranames)
        
        if errmsg then
          cleanup(handles)
          return nil, errmgs
        end
        
        local mapping, errmsg = create_mapping(handles, fixed_parameter_names)
        
        if errmsg then
          cleanup(handles)
          return nil, errmsg
        end
        
        local stmt = create_stmt(db, handles, table.getn(fixed_parameter_names))
        stmt.mapping = mapping
        stmt.paranames = fixed_parameter_names
        
        return stmt
        
      else					-- Automatic mapping of paramter names
        
        local parameter_names = collect_parameter_names(handles)
        local mapping = create_mapping(handles, parameter_names)
        local stmt = create_stmt(db, handles, table.getn(parameter_names))
        stmt.mapping = mapping
        stmt.paranames = parameter_names
        
        return stmt
        
      end
      
    else 					-- Mixed paramters are not allowed
      
      cleanup(handles)
      return nil, "db:prepare: Mixed anonymous and named parameters are not allowed."
      
    end
  end
end


local function call_user_func(context, func, num_values, values)
  -- Don't use table.insert() because of nils in lua-5.1
  local arg = { }
  for index = 1, num_values do
    arg[index] = api.value(values, index-1)
  end
  -- Make lua-5.0.2 unpack() happy
  arg.n = num_values
  -- lua-5.1 unpack() style / lua-5.0.2 ignores additional arguments
  local ok, result = pcall(func, unpack(arg, 1, num_values))	
  
  if not ok then
    api.result_error(context, tostring(result))
  else
    api.result(context, result)
  end
end


function db_class.set_function(db, name, num_args, func)
  check_db(db)
  
  local function xfunc(context, num_values, values)
    call_user_func(context, func, num_values, values)
  end
  
  local status = api.create_function(db.handle, name, num_args, xfunc, nil, nil)
  
  if is_error(status) then
    return nil, errmsg(db.handle)
  end
  
  return db
end


function db_class.set_aggregate(db, name, num_args, create_funcs)
  check_db(db)
  
  local step, final
  
  local function xstep(context, num_values, values)
    if not step and not final then
      step, final = create_funcs()
    end
    call_user_func(context, step, num_values, values)
  end
  
  local function xfinal(context)
    local ok, result = pcall(final, api.aggregate_count(context))
    if not ok then
      api.result_error(context, tostring(result))
    else
      api.result(context, result)
    end
  end
  
  local status = api.create_function(db.handle, name, num_args, nil, xstep, xfinal)
  
  if is_error(status) then
    return nil, errmsg(db.handle)
  end
  
  return db
end


function db_class.set_trace_handler(db, func)
  check_db(db)
  local status = api.trace(db.handle, func)
  if is_error(status) then
    return nil, errmsg(db.handle)
  end
  return db
end


function db_class.set_busy_timeout(db, ms)
  check_db(db)
  local status = api.busy_timeout(db.handle, ms)
  if is_error(status) then
    return nil, errmsg(db.handle)
  end
  return db
end


function db_class.set_busy_handler(db, func)
  check_db(db)
  local status = api.busy_handler(db.handle, func)
  if is_error(status) then
    return nil, errmsg(db.handle)
  end
  return db
end





---------------------
-- Statement Class --
---------------------

function stmt_class.bind(stmt, ...)
  
  local function bind_with_mapping(parameters)
    local mapping = stmt.mapping
    local map_index = 1
    for _, handle in ipairs(stmt.handles) do
      for index = 1, api.bind_parameter_count(handle) do
        local status = api.bind(handle, index,  parameters[mapping[map_index]])
        if is_error(status) then
          return nil, errmsg(stmt.db.handle)
        end
        map_index = map_index + 1
      end
    end
  end
  
  local function bind_without_mapping(parameters)
    local parameter_index = 1
    for _, handle in ipairs(stmt.handles) do
      for index = 1, api.bind_parameter_count(handle) do
        local status = api.bind(handle, index, parameters[parameter_index])
        if is_error(status) then
          return nil, errmsg(stmt.db.handle)
        end
        parameter_index = parameter_index + 1
      end
    end
  end
  
  local function bind_by_names(parameters)
    local parameter_names = stmt.paranames
    local mapping = stmt.mapping
    for _, handle in ipairs(stmt.handles) do
      for index = 1, api.bind_parameter_count(handle) do
        local status = api.bind(handle, index, parameters[parameter_names[mapping[index]]])
        if is_error(status) then
          return nil, errmsg(stmt.db.handle)
        end
      end
    end
  end
  
  check_stmt(stmt)
  if stmt.parameter_count == 0 then error("stmt:bind: statement contains no parameters.") end
  
  if type(arg[1]) == "table" and arg.n == 1 and stmt.mapping and stmt.paranames then
    bind_by_names(arg[1])
  else
    if arg.n < stmt.parameter_count then error("stmt:bind: to few parameters.") end
    if arg.n > stmt.parameter_count then error("stmt:bind: to many parameters.") end
    
    if stmt.mapping then
      bind_with_mapping(arg)
    else
      bind_without_mapping(arg)
    end
  end
  
  return stmt
end


function stmt_class.reset(stmt)
  check_stmt(stmt)
  for _, handle in ipairs(stmt.handles) do
    api.reset(handle)
  end
  return stmt
end


function stmt_class.close(stmt)
  check_stmt(stmt)
  
  for _, handle in ipairs(stmt.handles) do
    api.finalize(handle)
  end
  
  unregister(stmt.db.stmts, stmt.reg_id)
  
  stmt.db	= nil
  stmt.handles	= nil
  stmt.mapping	= nil
  stmt.__gc	= nil
  
  return stmt
end


local no_parameter_names = { }
function stmt_class.parameter_names(stmt)
  check_stmt(stmt)
  if not stmt.paranames then
    return no_parameter_names
  else
    return stmt.paranames
  end
end


local function stmt_column_info(stmt, info_func)
  local handle = check_single_stmt(stmt)
  local info = { }
  for index = 1, api.column_count(handle) do
    table.insert(info, info_func(handle, index-1) )
  end
  return info
end

function stmt_class.column_names(stmt)
  return stmt_column_info(stmt, api.column_name)
end

function stmt_class.column_decltypes(stmt)
  return stmt_column_info(stmt, api.column_decltype)
end


function stmt_class.column_count(stmt)
  local handle = check_single_stmt(stmt)
  return api.column_count(handle)
end


function stmt_class.exec(stmt)
  check_stmt(stmt)
  stmt:reset()
  for _, handle in ipairs(stmt.handles) do
    while true do
      local status = api.step(handle)
      
      if is_error(status) then
        local errmsg = errmsg(stmt.db.handle)
        api.reset(handle)
        return nil, errmsg
      end
      
      if is_done(status) then
        break
      end
    end
    api.reset(handle)
  end
  return stmt
end


local function stmt_rows(stmt, get_row_func, tab, autoclose)
  local handle = check_single_stmt(stmt)
  api.reset(handle)
  
  local function check_autoclose()
    if autoclose == true then
      stmt:close()
    else
      api.reset(handle)
    end
  end
  
  local iterator = function()
    local status = api.step(handle)
    
    if is_error(status) then
      local errmsg = errmsg(stmt.db.handle)
      check_autoclose()
      return nil, errmsg
    end
    
    if is_row(status) then
      return get_row_func(handle, tab)
    end
    
    if is_done(status) then
      check_autoclose()
      return nil
    end
    
    return nil, "stmt:rows: Internal error!"
  end
  
  return iterator
end

function stmt_class.irows(stmt, tab, autoclose)
  return stmt_rows(stmt, api.irow, tab, autoclose)
end

function stmt_class.rows(stmt, tab, autoclose)
  return stmt_rows(stmt, api.arow, tab, autoclose)
end

function stmt_class.cols(stmt, autoclose)
  return stmt_rows(stmt, api.drow, nil, autoclose)
end


local function first_row(stmt, get_row_func, tab, autoclose)
  local handle = check_single_stmt(stmt)
  api.reset(handle)
  
  local function check_autoclose()
    if autoclose == true then
      stmt:close()
    else
      api.reset(handle)
    end
  end
  
  local status = api.step(handle)
  
  if is_error(status) then
    local errmsg = errmsg(stmt.db.handle)
    check_autoclose()
    return nil, errmsg
  end
  
  if is_row(status) then
    local row = get_row_func(handle, tab)
    check_autoclose()
    return row
  end
  
  if is_done(status) then
    check_autoclose()
    return nil, "No row returned."
  end
  
  return nil, "stmt:first_row: Internal error!"
  
end

function stmt_class.first_irow(stmt, tab, autoclose)
  return first_row(stmt, api.irow, tab, autoclose)
end

function stmt_class.first_row(stmt, tab, autoclose)
  return first_row(stmt, api.arow, tab, autoclose)
end

function stmt_class.first_cols(stmt, autoclose)
  local count = api.column_count(stmt.handles[1])
  local row, errmsg = first_row(stmt, api.irow, nil, autoclose)
  if errmsg then
    return nil, errmsg
  else
    row.n = count			-- Make lua-5.0.2 unpack() happy
    return unpack(row, 1, count)	-- lua-5.1 style / lua-5.0.2 ignores additional arguments
  end
end



