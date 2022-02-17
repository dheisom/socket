#include "lua5.4/lua.h"
#include "lua5.4/lualib.h"
#include "lua5.4/lauxlib.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MTABLE "lib.socket"

typedef struct Connection {
  struct sockaddr_in address;
  int file_descriptor;
} Connection;

struct sockaddr_in newaddress() {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	return addr;
}

Connection* newconnection(lua_State *L) {
  Connection *conn = (Connection *)lua_newuserdata(L, sizeof(Connection));
  luaL_getmetatable(L, MTABLE);
  lua_setmetatable(L, -2);
  memset(&conn->address, '0', sizeof(conn->address));
  return conn;
}

Connection* getconnection(lua_State *L) {
  void *ud = luaL_checkudata(L, 1, MTABLE);
  luaL_argcheck(L, ud != NULL, 1, "`array' expected");
  return (Connection*)ud;
}

int ltcp(lua_State *L) {
  Connection *conn = newconnection(L);
  conn->file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  conn->address = newaddress();
  return 1;
}

int ludp(lua_State *L) {
  Connection *conn = newconnection(L);
  conn->file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
  conn->address = newaddress();
  return 1;
}

int lbind(lua_State *L) {
  Connection *server = getconnection(L);
  int port = luaL_checkinteger(L, 2);
  server->address.sin_port = htons(port);
  int err = bind(
    server->file_descriptor,
    (struct sockaddr*)&server->address,
    sizeof(server->address)
  );
  lua_pushboolean(L, err != 1);
  if (err)
    lua_pushstring(L, "error binding");
  else
    lua_pushnil(L);
  return 2;
}

int llisten(lua_State *L) {
  Connection *server = getconnection(L);
  int result = listen(server->file_descriptor, -1);
  lua_pushboolean(L, result == 0);
  if (result != 0)
    lua_pushstring(L, "error listening");
  else
    lua_pushnil(L);
  return 2;
}

int laccept(lua_State *L) {
  Connection *server = getconnection(L);
  Connection *client = newconnection(L);
  client->file_descriptor = accept(
    server->file_descriptor,
    // (struct sockaddr*)NULL,
    (struct sockaddr*)&client->address,
    // NULL
    (socklen_t *)sizeof(client->address)
  );
  if (client->file_descriptor == -1) {
    lua_pushnil(L);
    lua_pushstring(L, "failed to accept new connection");
    return 2;
  }
  return 1;
}

int lread(lua_State *L) {
  Connection *client = getconnection(L);
  int size = luaL_checkinteger(L, 2);
  char buffer[size];
  int rs = read(client->file_descriptor, buffer, size);
  lua_pushinteger(L, rs);
  lua_pushlstring(L, buffer, rs);
  return 2;
}

int lwrite(lua_State *L) {
  Connection *client = getconnection(L);
  size_t size = luaL_checkinteger(L, 3);
  char *data = (char *)luaL_checklstring(L, 2, &size);
  int ws = write(client->file_descriptor, data, size);
  lua_pushboolean(L, ws == size);
  return 1;
}

int lclose(lua_State *L) {
  Connection *conn = getconnection(L);
  close(conn->file_descriptor);
  return 0;
}

const struct luaL_Reg funcs[] = {
  { "tcp",    ltcp    },
  { "udp",    ludp    },
  { "listen", llisten },
  { "bind",   lbind   },
  { "close",  lclose  },
  { "accept", laccept },
  { "read",   lread   },
  { "write",  lwrite  },
  { NULL,     NULL    }
};

int luaopen_lib(lua_State *L) {
  luaL_newmetatable(L, MTABLE);
  lua_newtable(L);
  luaL_newlib(L, funcs);
  return 1;
}
