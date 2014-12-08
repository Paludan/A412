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

#include "mf_seq.h"

mf_seq *mf_seq_new (char *fname, short division)
{

  mf_seq *ms = NULL;

  ms = malloc(sizeof(mf_seq));

  if (ms) {
    ms->buf     = NULL;
    ms->buf_cnt = 0;
    ms->buf_max = 0;

    ms->evt     = NULL;
    ms->evt_cnt = 0;
    ms->evt_max = 0;

    ms->fname    = fname;
    ms->division = division;
    ms->curtrack = 0;

    ms->type     = mf_type_seq;
  }
  return ms;
}

#define getlong(q)  ((q)[0] << 24 | (q)[1] << 16 | (q)[2] << 8 | (q)[3])

#if 0
static void dmp_evts(mf_seq *ms)
{
  unsigned long k;
  unsigned char *p;
  unsigned long l;

  if (!ms) return;


  for (k=0; k< ms->evt_cnt; k++) {
    p = ms->evt[k].p;

    printf("%02X %02X%02X%02X%02X ", p[0], p[1],p[2],p[3],p[4]);
    printf("%02X %02X ", p[5], p[6]);

    if (p[5] < 0xF0) {
      printf("%02X %02X", p[7], p[8]);
    } else {
      l = getlong(p+7);
      printf("%08X ", l);
      p = p+11;
      /*printf("** %d **",l);*/
       while (l--) printf("%02X", *p++);
    }
    printf("\n");
  }
}
#endif

static char *evt_ord = "F98ABCDE";
#define evt_cmp_st(x) (evt_ord[((x)>>4) & 0x07])

static int evt_cmp(const void *a, const void *b)
{
  int ret = 0;
  int k;
  unsigned char *pa = ((mf_evt *)a)->p;
  unsigned char *pb = ((mf_evt *)b)->p;

  for (k=0; ret == 0 && k < 5; k++) {
    _dbgmsg("%d: %02X %02X\n",k,pa[k],pb[k]);
    ret = pa[k] - pb[k] ;
  }
  if (ret == 0) {
    ret = evt_cmp_st(pb[5]) - evt_cmp_st(pa[5]);
  }

  return ret;
}

int mf_seq_close(mf_seq *ms)
{
  int k;
  int trk = -1;
  unsigned long tick=0;
  unsigned long delta;
  unsigned long nxtk;
  unsigned char *p;

  mf_writer *mw;

  if (!ms) return 799;

  /* Convert the offset into pointers */
  for (k=0; k< ms->evt_cnt; k++) {
    ms->evt[k].p = ms->buf + ms->evt[k].l;
  }

  /*
  dbgmsg("Events: %d\n", ms->evt_cnt);
  dmp_evts(ms);
  */

  qsort(ms->evt, ms->evt_cnt,sizeof(mf_evt), evt_cmp);

  /*
  dbgmsg("Events: %d\n", ms->evt_cnt);
  dmp_evts(ms);
  */

  mw = mf_new(ms->fname, ms->division);

  if (mw) {

    if (ms->evt_cnt == 0) {
         mf_track_start(mw);
         mf_sys_evt(mw, 0, st_meta_event, me_text, 5, (unsigned char *)"Empty");
    }
    else for (k=0; k< ms->evt_cnt; k++) {
       p = ms->evt[k].p;

       if ((int)(*p) != trk) {  /* Start a new track */
         mf_track_start(mw);
         trk = *p;
         tick = 0;
       }

       nxtk = getlong(p+1);
       delta = nxtk - tick;
       _dbgmsg("DELTA: (%d-%d) = %d\n", nxtk,tick,delta);
       tick = nxtk;

       if (p[5] < 0xF0) {
         mf_midi_evt(mw, delta, p[5], p[6], p[7],p[8]);
       } else {
         mf_sys_evt(mw, delta, p[5], p[6], getlong(p+7), p+11);
       }
    }

    mf_close(mw);
  }

  /* Clean up */
  if (ms->buf) free(ms->buf);
  if (ms->evt) free(ms->evt);
  free(ms);

  return 0;
}

static int chkbuf(mf_seq *ms, unsigned long spc)
{
   unsigned long newsize;
   unsigned char *buf;

   if (!ms) return 739;

   newsize = ms->buf_max;

   while (spc >= (newsize - ms->buf_cnt))
      newsize += MF_SEQ_BUF_STEP;

   if (newsize > ms->buf_max) {
      buf = realloc(ms->buf, newsize);
      if (!buf) return 730;
      ms->buf = buf;
      ms->buf_max = newsize;
   }
   _dbgmsg("CHKBUF: buf:%p cnt:%d max:%d need:%d\n", ms->buf, ms->buf_cnt, ms->buf_max,spc);

   return 0;
}

static int chkevt(mf_seq *ms, unsigned long n)
{
   unsigned long newsize;
   mf_evt *evt = NULL;

   if (!ms) return 749;

   newsize = ms->evt_max;
   while (n >= (newsize - ms->evt_cnt))
      newsize += MF_SEQ_EVT_STEP;

   if (newsize > ms->evt_max) {
      evt = realloc(ms->evt, newsize * sizeof(mf_evt));
      if (!evt) return 740;
      ms->evt = evt;
      ms->evt_max = newsize;
   }

   _dbgmsg("CHKEVT: evt:%p cnt:%d max:%d need:%d\n", ms->evt, ms->evt_cnt, ms->evt_max,n);
   return 0;
}

int mf_seq_set_track(mf_seq *ms, int track)
{
  if (!ms) return 719;
  ms->curtrack = track & 0xFF;
  return 0;
}

int mf_seq_get_track(mf_seq *ms, int track)
{
  if (!ms) return 789;
  return ms->curtrack;
}

#define add_evt(ms)    (ms->evt[ms->evt_cnt++].l = ms->buf_cnt)
#define add_byte(ms,b) (ms->buf[ms->buf_cnt++] = (unsigned char)(b))

static void add_data(mf_seq *ms, unsigned long l, unsigned char *d)
{  if (ms) while (l--) add_byte(ms,*d++); }

static void add_ulong(mf_seq *ms, unsigned long l)
{
  add_byte(ms,(l >>24) & 0xFF);
  add_byte(ms,(l >>16) & 0xFF);
  add_byte(ms,(l >> 8) & 0xFF);
  add_byte(ms,(l     ) & 0xFF);
}

/*
static void add_str(mf_seq *ms, char *s)
{
  add_data(ms,strlen(s),(unsigned char *)s);
}
*/

int mf_seq_evt (mf_seq *ms, unsigned long tick, short type, short chan, short data1, short data2)
{
  int ret = 0;

  type &= 0xF0;

  if (!ms)  ret = 759;
  if (!ret) ret = chkbuf(ms,32);
  if (!ret) ret = chkevt(ms,1);
  if (!ret) ret = type == 0xF0 ? 758 : 0;

  if (!ret) {
    add_evt(ms);

    add_byte(ms, ms->curtrack);
    add_ulong(ms,tick);

    add_byte(ms, type );
    add_byte(ms, chan & 0x0F);
    add_byte(ms, data1 & 0xFF);
    add_byte(ms, data2 & 0xFF);
  }
  return ret;
}

int mf_seq_sys(mf_seq *ms, unsigned long tick, short type, short aux,
                                               long len, unsigned char *data)
{
  int ret = 0;

  if (!ms)  ret = 779;
  if (!ret) ret = chkbuf(ms,32+len);
  if (!ret) ret = chkevt(ms,1);
  if (!ret) ret = (type >= 0xF0) ? 0 : 778;
  if (!ret) {
    _dbgmsg("SEQSYS: %d %d\n",ms->curtrack, type);
    add_evt(ms);

    add_byte(ms, ms->curtrack);
    add_ulong(ms,tick);

    add_byte(ms,type);
    add_byte(ms,aux);

    add_ulong(ms,len);
    add_data(ms,len,data);
  }

  return ret;
}

int mf_seq_text(mf_seq *ms, unsigned long tick, short type, char *txt)
{
  return mf_seq_sys(ms, tick, st_meta_event, type & 0x0F, strlen(txt), (unsigned char *)txt);
}

#ifdef MF_SEQ_TEST

int main(int argc, char *argv[])
{
  mf_seq *m;

  m = mf_seq_new("ss.mid", 384);

  if (m) {
    mf_seq_set_track(m, 2);
    mf_seq_evt (m, 0, st_note_on, 0, 60, 90);
    mf_seq_evt (m, 192, st_note_off, 0, 60, 0);
    mf_seq_evt (m, 192, st_note_on, 0, 64, 90);
    mf_seq_evt (m, 2*192, st_note_off, 0, 64, 0);

    mf_seq_set_track(m, 1);

    mf_seq_sys(m,0,st_meta_event,1,5,"ABCDE");

    mf_seq_evt (m, 0, st_note_on, 1, 64, 90);
    mf_seq_evt (m, 255, st_note_off, 1, 64, 0);

    mf_seq_close(m);
  }

}

#endif
