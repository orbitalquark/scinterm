-- Copyright 2012-2024 Mitchell. See LICENSE.

-- Markdown filter for LDoc.
-- @usage ldoc --filter markdowndoc.ldoc [ldoc opts] > api.md
local M = {}

local FIELD = '<a id="%s"></a>\n#### `%s` %s\n\n'
local FUNCTION = '<a id="%s"></a>\n#### `%s`(%s)\n\n'
local FUNCTION_NO_PARAMS = '<a id="%s"></a>\n#### `%s`()\n\n'
local DESCRIPTION = '%s\n\n'
local LIST_TITLE = '%s:\n\n'
local PARAM = '- *%s*: %s\n'
local USAGE = '- `%s`\n'
local RETURN = '- %s\n'
local SEE = '- [`%s`](#%s)\n'
local TABLE = '<a id="%s"></a>\n#### `%s`\n\n'
local TFIELD = '- `%s`: %s\n'
local titles = {
	[PARAM] = 'Parameters', [USAGE] = 'Usage', [RETURN] = 'Return', [SEE] = 'See also',
	[TFIELD] = 'Fields'
}

-- Writes an LDoc description to the given file.
-- @param f The markdown file being written to.
-- @param description The description.
-- @param name The name of the module the description belongs to. Used for headers in module
--   descriptions.
local function write_description(f, description, name)
	f:write(string.format(DESCRIPTION, description:gsub('\n ', '\n'))) -- strip leading spaces
end

-- Writes an LDoc list to the given file.
-- @param f The markdown file being written to.
-- @param fmt The format of a list item.
-- @param list The LDoc list.
local function write_list(f, fmt, list)
	if not list or #list == 0 then return end
	if type(list) == 'string' then list = {list} end
	f:write(string.format(LIST_TITLE, titles[fmt]))
	for _, value in ipairs(list) do f:write(string.format(fmt, value, value)) end
	f:write('\n')
end

-- Writes an LDoc hashmap to the given file.
-- @param f The markdown file being written to.
-- @param fmt The format of a hashmap item.
-- @param list The LDoc hashmap.
local function write_hashmap(f, fmt, hashmap)
	if not hashmap or #hashmap == 0 then return end
	f:write(string.format(LIST_TITLE, titles[fmt]))
	for _, name in ipairs(hashmap) do
		local description = hashmap.map and hashmap.map[name] or hashmap[name] or ''
		if fmt == PARAM then description = description:gsub('^%[opt%] ', '') end
		f:write(string.format(fmt, name, description))
	end
	f:write('\n')
end

-- Called by LDoc to process a doc object.
-- @param doc The LDoc doc object.
function M.ldoc(doc)
	local f = io.stdout
	f:write('## Scinterm API Documentation\n\n')

	table.sort(doc, function(a, b) return a.name < b.name end)

	-- Loop over modules, writing the Markdown document to stdout.
	for _, module in ipairs(doc) do
		local name = module.name

		-- Write the description.
		write_description(f, module.summary .. module.description, name)

		-- Write functions.
		local funcs = {}
		for _, item in ipairs(module.items) do
			if item.type == 'function' then funcs[#funcs + 1] = item end
		end
		table.sort(funcs, function(a, b) return a.name < b.name end)
		if #funcs > 0 then
			f:write('### Functions defined by `', name, '`\n\n')
			for _, func in ipairs(funcs) do
				f:write(string.format(FUNCTION, func.name:gsub(':', '.'), func.name,
					func.args:sub(2, -2):gsub('[%w_]+', '*%0*')))
				write_description(f, func.summary .. func.description)
				write_hashmap(f, PARAM, func.params)
				write_list(f, USAGE, func.usage)
				write_list(f, RETURN, func.ret)
				write_list(f, SEE, func.tags.see, name)
			end
			f:write('\n')
		end
	end
end

return M
