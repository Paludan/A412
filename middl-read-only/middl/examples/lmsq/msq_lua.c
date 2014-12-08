/*
**  (C) 2008 by Remo Dentato (rdentato@users.sourceforge.net)
**
** Permission to use, copy, modify and distribute this code and
** its documentation for any purpose is hereby granted without
** fee, provided that the above copyright notice, or equivalent
** attribution acknowledgement, appears in all copies and
** supporting documentation.
**
** Copyright holder makes no representations about the suitability
** of this software for any purpose. It is provided "as is" without
** express or implied warranty.
*/

#include <stdio.h>
#include <stddef.h> 
#include <string.h>
#include <stdarg.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "msq.h"

static int ref = LUA_NOREF;
static lua_State *gL = NULL;


/** HANDLERS *********/

static int check_handler(char *event)
{
  /*fprintf(stderr,"-- %s\n",event);fflush(stderr);*/
  lua_rawgeti(gL, LUA_ENVIRONINDEX, ref);
  if (lua_istable(gL, -1)) {
    lua_getfield(gL, -1, event);
    if (lua_isfunction(gL, -1)) {
      return  1;
    }
  }   
  return 0;
}

static int l_header (short type, short ntracks, short division)
{
  int ret = check_handler("header");
  if (ret) {
    lua_pushinteger(gL, type);
    lua_pushinteger(gL, ntracks);
    lua_pushinteger(gL, division);
    lua_call(gL, 3, 1);
    ret = luaL_optnumber(gL, -1, 0);
  }
  lua_settop(gL,0);
  return ret;  
}

static int l_track (short eot, short tracknum, unsigned long tracklen )

{
  int ret = check_handler("track");
  if (ret) {
    lua_pushboolean(gL, eot);
    lua_pushinteger(gL, tracknum);
    lua_pushinteger(gL, tracklen);
    lua_call(gL, 3, 1);
    ret = luaL_optnumber(gL, -1, 0);
  }
  lua_settop(gL,0);
  return ret;    
}

static int l_midi_event(unsigned long tick, short type, short chan,
                                                     short data1, short data2)
{
  int ret = check_handler("midi_event");
  if (ret) {
    lua_pushinteger(gL, tick);
    lua_pushinteger(gL, type);
    lua_pushinteger(gL, chan);
    lua_pushinteger(gL, data1);
    lua_pushinteger(gL, data2);
    lua_call(gL, 5, 1);
    ret = luaL_optnumber(gL, -1, 0);
  }
  lua_settop(gL,0);
  return ret;    
}

static int l_sys_event(unsigned long tick, short type, short aux,
                                               long len, unsigned char *data)
{
  int ret = check_handler("sys_event");
  if (ret) {
    lua_pushinteger(gL, tick);
    lua_pushinteger(gL, type);
    lua_pushinteger(gL, aux);
    lua_pushlstring(gL, data, len);
    lua_call(gL, 4, 1);
    ret = luaL_optnumber(gL, -1, 0);
  }
  lua_settop(gL,0);
  return ret;    
}
                                               
static int l_error(short err, char *msg)
{
  int ret = check_handler("error");
  if (ret) {
    lua_pushinteger(gL, err);
    if (msg == NULL) msg = "";
    lua_pushstring(gL, msg);
    lua_call(gL, 2, 1);
    ret = luaL_optnumber(gL, -1, 0);
  }
  lua_settop(gL,0);
  return ret;    
}

static int l_writetrack(short trknum)
{
  int ret = check_handler("writetrack");
  if (ret) {
    lua_pushinteger(gL, trknum);
    lua_call(gL, 1, 1);
    ret = luaL_optnumber(gL, -1, 0);
  }
  lua_settop(gL,0);
  return ret;    
}

static mf_handlers l_handlers = {
  (mf_hndl)l_error,      (mf_hndl)l_writetrack,
  (mf_hndl)l_header,     (mf_hndl)l_track,
  (mf_hndl)l_midi_event, (mf_hndl)l_sys_event,
};

/** READ MIDI FILE *********/

static int l_read(lua_State *L)
{
  char *s = (char *)luaL_checkstring(L,1);
  
  luaL_checktype(L,2,LUA_TTABLE);
  lua_settop(L,2);
  
  ref = luaL_ref(L, LUA_ENVIRONINDEX);
  
  mf_sethandlers(l_handlers);
  
  lua_settop(gL,0);
  mf_read(s); 
  
  luaL_unref(L, LUA_ENVIRONINDEX, ref);
  ref = LUA_NOREF;
 
  return 0;
}

/** WRITE MIDI FILE *********/

/** LEVEL 1 **/

static int l_write(lua_State *L)
{
  char *s = (char *)luaL_checkstring(L,1);
  short format = luaL_checkinteger(L,2);
  short ntracks = luaL_checkinteger(L,3);
  short division = luaL_checkinteger(L,4);
  
  luaL_checktype(L,5,LUA_TTABLE);
  lua_settop(L,5);
  
  ref = luaL_ref(L, LUA_ENVIRONINDEX);
  
  mf_sethandlers(l_handlers);
  
  lua_settop(gL,0);
  
  mf_write(s,format,ntracks,division);
  
  luaL_unref(L, LUA_ENVIRONINDEX, ref);
  ref = LUA_NOREF;
 
  return 0;
}

static int l_wrt_midi_event(lua_State *L)
{
  short type = -1;
  
  if (lua_type(L,2) == LUA_TSTRING)
    type = mf_eventbyname((char *)lua_tostring(L,2));
    
  if (type < 0)
    type = luaL_checkinteger(L,2);
    
  mf_midi_event(luaL_checkinteger(L,1),  /* tick */
                type,                    /* */ 
                luaL_checkinteger(L,3),  /* chan */
                luaL_checkinteger(L,4),  /* data1 */
                luaL_optinteger(L,5,0)   /* data2 */
               );
               
  lua_settop(L,0);
  return 0;
}

static int l_wrt_sys_event(lua_State *L)
{
  short type = -1;
  short aux = -1;
  int k = 2;
  int len = 0;
  unsigned char *s;
  int top = lua_gettop(L);
  luaL_Buffer b;
  
  if (lua_type(L,k) == LUA_TSTRING) 
    type = mf_eventbyname((char *)lua_tostring(L,k));
  if (type < 0)
    type = luaL_checkinteger(L,k);
  k++;
  
  if (type == st_meta_event) {
    if (lua_type(L,k) == LUA_TSTRING)
      aux = mf_eventbyname((char *)lua_tostring(L,k));
    if (type < 0)
      aux = luaL_checkinteger(L,k);
    k++;
  }
  
  if (!(type & 0x80) || ((type & 0xF0) == 0xF0)) {
    
    if ((type & 0x80) == 0) {
      aux = type;
      type = st_meta_event;
    }
    
    luaL_buffinit(L,&b);
    
    for ( ;k <= top; k++) {
       if (lua_type(L, k) == LUA_TNUMBER) {
         luaL_addchar(&b, lua_tointeger(L,k) & 0xFF);
       }
       else if (lua_type(L, k) == LUA_TSTRING) {
         s = (unsigned char *)lua_tolstring(L,k,&len);
         luaL_addlstring(&b, s, len);
       }
    }
    luaL_pushresult(&b);
    s = (unsigned char *)lua_tolstring(L,k,&len);
      
    mf_sys_event(luaL_checkinteger(L,1), type, aux, len, s);
  }
  lua_settop(L,0);
  return 0;
}
                      
#define w_instr    1
#define w_ctrl     2
#define w_chord    3
#define w_gchord   4
#define w_event    5
#define w_pitch    6
#define w_duration 7

static int int_or_string(lua_State *L, int ndx, short what)
{
  int val = -1;
  char * s;
  if (lua_type(L,ndx) == LUA_TSTRING) {
    s = (char *)lua_tostring(L, ndx);
    switch (what) {
      case w_pitch    : val = (int)mf_pitchbyname(s); break;
      case w_instr    : val = (int)mf_instrbyname(s); break;
      case w_ctrl     : val = (int)mf_ctrlbyname(s);  break;
      case w_chord    : val = (int)mf_chordbyname(s); break;
      case w_event    : val = (int)mf_eventbyname(s); break;        
      case w_duration : val = (int)mf_durationbyname(s); break;        

      default: break;
    }
  }
  _dbgmsg("IOS: %d\n",val);
  if (val == -1)  val = luaL_checkinteger(L, ndx);
  return val;
}

static int l_note_on(lua_State *L) /* also note_off! */
{
  mf_note_on(luaL_checkinteger(L,1), /* tick */
             luaL_checkinteger(L,2), /* channel */
             int_or_string(L,3,w_pitch), 
             luaL_optinteger(L, 4,0));  
             
  lua_settop(L,0);
  return 0;
}

static int l_key_pressure(lua_State *L)
{
  mf_key_pressure(luaL_checkinteger(L,1),
                  luaL_checkinteger(L,2),
                  int_or_string(L,3,w_pitch),
                  luaL_checkinteger(L, 4));  
             
  lua_settop(L,0);
  return 0;
}

static int l_control_change(lua_State *L)
{
  lua_settop(L,0);
  return 0;
}

static int l_program_change(lua_State *L)
{
  mf_program_change(luaL_checkinteger(L,1),
                    luaL_checkinteger(L,2),
                    int_or_string(L,3,w_instr));

  lua_settop(L,0);
  return 0;
}

static int l_channel_pressure(lua_State *L)
{
  lua_settop(L,0);
  return 0;
}

static int l_pitch_bend(lua_State *L)
{
  mf_pitch_bend(luaL_checkinteger(L,1),
                  luaL_checkinteger(L,2),luaL_checkinteger(L,3));
  lua_settop(L,0);
  return 0;
}

int l_note(lua_State *L)
{
  mf_note(luaL_checkinteger(L,1),
          luaL_checkinteger(L,2),
          int_or_string(L,3,w_duration),
          int_or_string(L,4,w_pitch),
          luaL_checkinteger(L,5));
  return 0;
}

int l_tick(lua_State *L)
{
  lua_pushinteger(L,mf_tick(luaL_optinteger(L, 1,MF_GET_TICK)));
  return 1;
}

/** REGISTER FUNCTIONS TO LUA  *********/

static struct luaL_reg msq_funcs[] = {
  {"read"             , l_read},
  {"write"            , l_write},
  
  {"note_on"          , l_note_on},
  {"note_off"         , l_note_on},
  {"midi_event"       , l_wrt_midi_event},
  {"sys_event"        , l_wrt_sys_event},
  {"key_pressure"     , l_key_pressure},
  {"program_change"   , l_program_change},
  {"control_change"   , l_control_change},
  {"channel_pressure" , l_channel_pressure},
  {"pitch_bend"       , l_pitch_bend},
  {"note"             , l_note},
  {"tick"             , l_tick},
  
  {NULL, NULL}
};

int luaopen_msq(lua_State *L)
{
  gL = L;
  luaL_register(gL, "msq", msq_funcs);
  return 0; 
}

