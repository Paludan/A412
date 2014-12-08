#ifndef MF_SCORE_H
#define MF_SCORE_H

#include "mf_seq.h"
#include "mf_chords.h"
#include "mf_instr.h"
#include "mf_errors.h"
#include <setjmp.h>


#define MAX_TRACKS  20
#define MAX_REPT    32
#define MAX_TRGT    10
#define MAX_SCALE   16
#define MAX_CHORDN  16
#define DUR_INFINITE 0xFFFFFFFF

#define SCORE_FAIL(t,e) ((t)->err = (e), longjmp((t)->errjmp, (e)))

#define FLG_RAWCHORD 0x01
#define FLG_NOPLAY   0x02

#define flg_set(x,f)  ((x)->flags |=  (f))
#define flg_clr(x,f)  ((x)->flags &= ~(f))
#define flg_chk(x,f)  ((x)->flags & (f))



typedef struct { /* trk_data */
  mf_seq *ms;
  unsigned char *buf;
  unsigned char *ptr;
  unsigned long line;
  jmp_buf errjmp;
  unsigned long flags;
  short ppqn;

  short track;
  short err;

  short num;
  short den;

  short lastnote;
  short notepct;

  short rpt_top;
  unsigned char *rpt_pos[MAX_REPT];
           short rpt_cnt[MAX_REPT];

  unsigned long     trgt[MAX_TRGT];

  unsigned long   trgt_r[MAX_TRGT];
           short  trgt_n;

  unsigned char    scale[MAX_SCALE];
  unsigned short scale_n;

  unsigned long     tick[MAX_TRACKS];
  unsigned long      dur[MAX_TRACKS];
  unsigned char    notes[MAX_TRACKS][MAX_CHORDN];  /* 0: cur note  1..n: cur chord */
  unsigned char  chord_n[MAX_TRACKS];              /* length of current chord */
  unsigned char     chan[MAX_TRACKS];
  unsigned char     inst[MAX_TRACKS];
  unsigned char      vel[MAX_TRACKS];              /* cur velocity for notes */
  unsigned char      oct[MAX_TRACKS];              /* cur octave */
  unsigned char trkflags[MAX_TRACKS];
  unsigned char  chnctrl[16][128];
} trk_data;

/* whether the volume has been set at the beginning of the track */
#define TRKF_VOLSET  0x01


unsigned char *mf_demacro(unsigned char *inbuf, int *err);



#endif /* MF_SCORE_H */

