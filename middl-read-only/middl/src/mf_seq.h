/*
**  (C) Remo Dentato (rdentato@gmail.com)
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

#ifndef M_SEQ_H
#define M_SEQ_H

#include "mf_rw.h"

#define MF_SEQ_TRACK_MAX 16
#define MF_SEQ_BUF_STEP  (1024*128)
#define MF_SEQ_EVT_STEP  (1024)

#define mf_type_seq 2

typedef union {
  unsigned long  l;
  unsigned char *p;
} mf_evt;

typedef struct {
  unsigned short type;
  unsigned char *buf;
  unsigned long  buf_cnt;
  unsigned long  buf_max;
  
  mf_evt *evt;
  unsigned long  evt_cnt;
  unsigned long  evt_max;
  
  char  *fname;
  short  division;
  short  curtrack;
} mf_seq;  

mf_seq *mf_seq_new (char *fname, short division);
int mf_seq_close(mf_seq *ms);
int mf_seq_set_track(mf_seq *ms, int track);
int mf_seq_get_track(mf_seq *ms, int track);
int mf_seq_evt (mf_seq *ms, unsigned long tick, short type, short chan, short data1, short data2);
int mf_seq_sys(mf_seq *ms, unsigned long tick, short type, short aux, long len, unsigned char *data);
int mf_seq_text(mf_seq *ms, unsigned long tick, short type, char *txt);

#endif
