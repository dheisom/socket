#include "lua5.4/lua.h"
#include "lua5.4/lualib.h"
#include "lua5.4/lauxlib.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MTABLE "net.l"

typedef struct Connection {
  struct sockaddr_in address;
  int file_description;
} Connection;

static Connection* getconnection(lua_State *L) {
  void *ud = luaL_checkudata(L, 1, MTABLE);
  luaL_argcheck(L, ud != NULL, 1, "`array' expected");
  return (Connection*)ud;
}

static int ltcp(lua_State *L) {
  int port = luaL_checkinteger(L, 1);
  size_t n = sizeof(Connection);
  Connection *conn = (Connection *)lua_newuserdata(L, n);
  luaL_getmetatable(L, MTABLE);
  lua_setmetatable(L, -2);
  conn->file_description = socket(AF_INET, SOCK_STREAM, 0);
  memset(&conn->address, '0', sizeof(conn->address));
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
  conn->address = addr;
  return 1;
}

static int ludp(lua_State *L) {
  int port = luaL_checkinteger(L, 1);
  size_t n = sizeof(Connection);
  Connection *conn = (Connection *)lua_newuserdata(L, n);
  luaL_getmetatable(L, MTABLE);
  lua_setmetatable(L, -2);
  conn->file_description = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&conn->address, '0', sizeof(conn->address));
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
  conn->address = addr;
  return 1;
}

static int laccept(lua_State *L) {
  Connection *server = getconnection(L);
  size_t n = sizeof(Connection);
  Connection *client = (Connection *)lua_newuserdata(L, n);
  luaL_getmetatable(L, MTABLE);
  lua_setmetatable(L, -2);
  client->file_description = accept(server->file_description, (struct sockaddr*)NULL, NULL);
  if (client->file_description == -1) {
    lua_pushnil(L);
    lua_pushstring(L, "failed to accept new connection");
    return 2;
  }
  return 1;
}

static int llisten(lua_State *L) {
  Connection *conn = getconnection(L);
  int err = bind(conn->file_description, (struct sockaddr*)&conn->address, sizeof(conn->address));
  if (err) {
    lua_pushstring(L, "error binding");
    return 1;
  }
  int result = listen(conn->file_description, 10);
  if (result != 0) {
    lua_pushstring(L, "error listening");
    return 1;
  }
  return 0;
}

static int lread(lua_State *L) {
  Connection *client = getconnection(L);
  int s = luaL_checkinteger(L, 2);
  char buffer[s];
  int bs = sizeof(char) * s;
  int rs = read(client->file_description, buffer, bs);
  lua_pushlstring(L, buffer, rs);
  return 1;
}

static int lwrite(lua_State *L) {
  Connection *client = getconnection(L);
  char *data = (char *)luaL_checkstring(L, 2);
  int size = strlen(data) - 1;
  int ws = write(client->file_description, data, size);
  if (ws != size){
    lua_pushboolean(L, 0);
  } else {
    lua_pushboolean(L, 1);
  }
  return 1;
}

static int lclose(lua_State *L) {
  Connection *conn = getconnection(L);
  close(conn->file_description);
  return 0;
}

static const struct luaL_Reg funcs[] = {
  { "tcp",    ltcp    },
  { "udp",    ludp    },
  { "listen", llisten },
  { "close",  lclose  },
  { "accept", laccept },
  { "read",   lread   },
  { "write",  lwrite  },
  { NULL,     NULL    }
};

int luaopen_net(lua_State *L) {
  luaL_newmetatable(L, MTABLE);
  lua_newtable(L);
  luaL_newlib(L, funcs);
  return 1;
}
