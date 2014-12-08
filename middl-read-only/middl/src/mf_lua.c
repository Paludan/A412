/*
**  (C) by Remo Dentato (rdentato@gmail.com)
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

#include "mf_seq.h"

static char *MSQ_METADATA = "msq_writer";

/* WRITE ONLY */

typedef struct {
  mf_seq *ms;
} seq_ptr;


static int l_open(lua_State *L)
{
  char *fname    = (char *)luaL_checkstring(L,1);
  short division = luaL_optinteger(L,2,384);
  mf_seq *ms = NULL;
  seq_ptr *sp = NULL;

  ms = mf_seq_new(fname, division);

  if (ms) {
    sp = lua_newuserdata(L,sizeof(seq_ptr));
    sp->ms = ms;
    luaL_getmetatable(L, MSQ_METADATA);
    lua_setmetatable(L, -2);
    return 1;
  }

  return 0;
}


static int l_close(lua_State *L)
{
  seq_ptr *sp        = luaL_checkudata(L,1,MSQ_METADATA);

  mf_seq_close(sp->ms);
  sp->ms = NULL;
  return 0;
}

static int l_wrt_midi_event(lua_State *L)
{
  seq_ptr *sp        = luaL_checkudata(L,1,MSQ_METADATA);
  unsigned long tick = luaL_checkinteger(L,2);
  short type         = luaL_checkinteger(L,3);
  short chan         = luaL_checkinteger(L,4);
  short data1        = luaL_checkinteger(L,5);
  short data2        = luaL_optinteger(L,6,-1);

  mf_seq_evt (sp->ms, tick, type, chan, data1, data2);

  return 0;
}

static int l_wrt_sys_event(lua_State *L)
{
  seq_ptr *sp         = luaL_checkudata(L,1,MSQ_METADATA);
  unsigned long tick  = luaL_checkinteger(L,2);
  short type          = luaL_checkinteger(L,3);
  short aux           = luaL_checkinteger(L,4);
  long len            = luaL_checkinteger(L,5);
  unsigned char *data = (unsigned char *)luaL_checkstring(L,6);

  mf_seq_sys(sp->ms, tick, type, aux, len, data);
  return 0;
}

static int l_wrt_set_track(lua_State *L)
{
  seq_ptr *sp  = luaL_checkudata(L,1,MSQ_METADATA);
  int track    = luaL_checkinteger(L,2);

  mf_seq_set_track(sp->ms, track);
  return 0;
}

static struct luaL_Reg msq_funcs_f[] = {
  {"open"            , l_open},
  {NULL, NULL}
};

static struct luaL_Reg msq_funcs_m[] = {
  {"open"            , l_open},
  {"close"           , l_close},
  {"midievent"       , l_wrt_midi_event},
  {"sysevent"        , l_wrt_sys_event},
  {"settrack"        , l_wrt_set_track},
  {"__gc"            , l_close},
  {NULL, NULL}
};

struct mf_const {
  char *name;
  int   val;
};

static struct mf_const consts[] = {
  {"note_off"          ,   st_note_off           },
  {"note_on"           ,   st_note_on            },
  {"key_pressure"      ,   st_key_pressure       },
  {"control_change"    ,   st_control_change     },
  {"program_change"    ,   st_program_change     },
  {"channel_pressure"  ,   st_channel_pressure   },
  {"pitch_bend"        ,   st_pitch_bend         },
  {"system_exclusive"  ,   st_system_exclusive   },
  {"system_continue"   ,   st_system_continue    },
  {"meta_event"        ,   st_meta_event         },
  {"sequence_number"   ,   me_sequence_number    },
  {"text"              ,   me_text               },
  {"copyright_notice"  ,   me_copyright_notice   },
  {"sequence_name"     ,   me_sequence_name      },
  {"track_name"        ,   me_track_name         },
  {"instrument_name"   ,   me_instrument_name    },
  {"lyric"             ,   me_lyric              },
  {"marker"            ,   me_marker             },
  {"cue_point"         ,   me_cue_point          },
  {"device_name"       ,   me_device_name        },
  {"program_name"      ,   me_program_name       },
  {"end_of_track"      ,   me_end_of_track       },
  {"set_tempo"         ,   me_set_tempo          },
  {"smpte_offset"      ,   me_smpte_offset       },
  {"time_signature"    ,   me_time_signature     },
  {"key_signature"     ,   me_key_signature      },
  {"sequencer_specific",   me_sequencer_specific },
  {"channel_prefix"    ,   me_channel_prefix     },
  {"port_prefix"       ,   me_port_prefix        },
  {NULL, 0}
};

int luaopen_msq(lua_State *L)
{
  int k;

  luaL_newmetatable(L,MSQ_METADATA);
  lua_pushvalue(L,-1);
  lua_setfield(L,-2, "__index");

  luaL_setfuncs (L,msq_funcs_m,0);

  luaL_newlib(L, msq_funcs_f);

  for (k=0; consts[k].name; k++) {
    lua_pushinteger(L,consts[k].val);
    lua_setfield(L,-2,consts[k].name);
  }
  return 1;  /* Leave table on the top */
}

