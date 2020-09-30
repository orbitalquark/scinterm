-- Copyright 2012-2020 Mitchell. See LICENSE.

-- Markdown doclet for Luadoc.
-- @usage luadoc -doclet path/to/markdowndoc [file(s)] > api.md
local M = {}

local FIELD = '<a id="%s"></a>\n#### `%s` %s\n\n'
local FUNCTION = '<a id="%s"></a>\n#### `%s`(*%s*)\n\n'
local DESCRIPTION = '%s\n\n'
local LIST_TITLE = '%s:\n\n'
local PARAM = '* *`%s`*: %s\n'
local USAGE = '* `%s`\n'
local RETURN = '* %s\n'
local SEE = '* [`%s`](#%s)\n'
local TABLE = '<a id="%s"></a>\n#### `%s`\n\n'
local TFIELD = '* `%s`: %s\n'
local titles = {
  [PARAM] = 'Parameters', [USAGE] = 'Usage', [RETURN] = 'Return',
  [SEE] = 'See also', [TFIELD] = 'Fields'
}

-- Writes a LuaDoc description to the given file.
-- @param f The markdown file being written to.
-- @param description The description.
local function write_description(f, description)
  f:write(string.format(DESCRIPTION, description))
end

-- Writes a LuaDoc list to the given file.
-- @param f The markdown file being written to.
-- @param fmt The format of a list item.
-- @param list The LuaDoc list.
local function write_list(f, fmt, list)
  if not list or #list == 0 then return end
  if type(list) == 'string' then list = {list} end
  f:write(string.format(LIST_TITLE, titles[fmt]))
  for _, value in ipairs(list) do f:write(string.format(fmt, value, value)) end
  f:write('\n')
end

-- Writes a LuaDoc hashmap to the given file.
-- @param f The markdown file being written to.
-- @param fmt The format of a hashmap item.
-- @param list The LuaDoc hashmap.
local function write_hashmap(f, fmt, hashmap)
  if not hashmap or #hashmap == 0 then return end
  f:write(string.format(LIST_TITLE, titles[fmt]))
  for _, name in ipairs(hashmap) do
    f:write(string.format(fmt, name, hashmap[name] or ''))
  end
  f:write('\n')
end

-- Called by LuaDoc to process a doc object.
-- @param doc The LuaDoc doc object.
function M.start(doc)
  local modules, files = doc.modules, doc.files
  local f = io.stdout
  f:write('## Scinterm API Documentation\n\n')

  -- Create a map of doc objects to file names so their Markdown doc comments
  -- can be extracted.
  local filedocs = {}
  for _, name in ipairs(files) do filedocs[files[name].doc] = name end

  -- Loop over modules, writing the Markdown document to stdout.
  for _, name in ipairs(modules) do
    local module = modules[name]

    -- Write the description.
    write_description(f, module.description)

    -- Write fields.
    if module.doc[1].class == 'module' then
      local fields = module.doc[1].field
      if fields and #fields > 0 then
        table.sort(fields)
        f:write('### Fields defined by `', name, '`\n\n')
        for _, field in ipairs(fields) do
          local type, description = fields[field]:match('^(%b())%s*(.+)$')
          f:write(string.format(FIELD, field, field, type or ''))
          write_description(f, description or fields[field])
        end
        f:write('\n')
      end
    end

    -- Write functions.
    local funcs = module.functions
    if #funcs > 0 then
      f:write('### Functions defined by `', name, '`\n\n')
      for _, fname in ipairs(funcs) do
        local func = funcs[fname]
        f:write(string.format(
          FUNCTION, func.name, func.name,
          table.concat(func.param, ', '):gsub('_', '\\_')))
        write_description(f, func.description)
        write_hashmap(f, PARAM, func.param)
        write_list(f, USAGE, func.usage)
        write_list(f, RETURN, func.ret)
        write_list(f, SEE, func.see)
      end
      f:write('\n')
    end

    -- Write tables.
    local tables = module.tables
    if #tables > 0 then
      f:write('### Tables defined by `', name, '`\n\n')
      for _, tname in ipairs(tables) do
        local tbl = tables[tname]
        f:write(string.format(TABLE, tbl.name, tbl.name))
        write_description(f, tbl.description)
        write_hashmap(f, TFIELD, tbl.field)
        write_list(f, USAGE, tbl.usage)
        write_list(f, SEE, tbl.see)
      end
    end
  end
end

return M
