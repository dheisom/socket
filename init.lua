local lib = require 'lib'
local l = {}

local socket = {}

---@param size integer
function socket:read(size)
  return lib.read(self.conn, size)
end

---@param data string
function socket:write(data)
  return lib.write(self.conn, data, #data)
end

---Create TCP socket
---@type function
l.tcp = function()
  local r = {
    conn = lib.tcp(),
    __index = socket
  }
  setmetatable(r, r)
  return r
end

---Create UDP socket
---@type function
---Create TCP socket
---@type function
l.udp = function()
  local r = {
    conn = lib.udp(),
    __index = socket
  }
  setmetatable(r, r)
  return r
end

return l
