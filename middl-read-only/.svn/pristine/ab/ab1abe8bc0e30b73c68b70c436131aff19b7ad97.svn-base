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

#include "mf_rw.h"

#define MThd 0x4d546864
#define MTrk 0x4d54726b

/*
ooooooooo.   oooooooooooo       .o.       oooooooooo.
`888   `Y88. `888'     `8      .888.      `888'   `Y8b
 888   .d88'  888             .8"888.      888      888
 888ooo88P'   888oooo8       .8' `888.     888      888
 888`88b.     888    "      .88ooo8888.    888      888
 888  `88b.   888       o  .8'     `888.   888     d88'
o888o  o888o o888ooooood8 o88o     o8888o o888bood8P'
*/


/* Get the number of parameters needed by a channel message.
** s is the status byte.
*/

int mf_numparms(int s)  { return ("\2\2\2\2\1\1\2"[((s) & 0x70)>>4]); }

/********************************************************************/

/* == Reading Values
**
**  readnum(n)  reads n bytes and assembles them to create an integer
**              if n is 0, reads a variable length representation
*/

static long readvar(mf_reader *mfile)
{
  long v = 0;
  int c;

  if ((c = fgetc(mfile->file)) == EOF) return -1;

  while (c & 0x80 ) {
    v = (v << 7) | (c & 0x7f);
    if ((c = fgetc(mfile->file)) == EOF) return -1;
  }
  v = (v << 7) | c;
  return (v);
}

static long readnum(mf_reader *mfile,int k)
{
  long x = 0, v = 0;

  if (k == 0) return(readvar(mfile));

  while (k-- > 0) {
    if ((x = fgetc(mfile->file)) == EOF) return -1;
    v = (v << 8) | x;
  }
  return v;
}

/* === Read messages
**   readmsg(n)  reads n bytes, stores them in a buffer end returns
**               a pointer to the buffer;
*/

static unsigned char *chrbuf_set(mf_reader *mfile, long sz)
{
  unsigned char *t = NULL;
  if (sz > mfile->chrbuf_sz) {
    t = realloc(mfile->chrbuf, sz);
    if (t) {
      mfile->chrbuf    = t;
      mfile->chrbuf_sz = sz;
    }
  }
  return mfile->chrbuf;
}

static unsigned char *readmsg(mf_reader *mfile, long n)
{
  int   c;
  unsigned char *s;

  if (n == 0) return (unsigned char *)"";

  chrbuf_set(mfile, n);
  if (mfile->chrbuf_sz < n) return NULL;

  s = mfile->chrbuf;
  while (n-- > 0) {   /*** Read the message ***/
    if ((c = fgetc(mfile->file)) == EOF) return NULL;
    *s++ = c;
  }

  return mfile->chrbuf;
}


/* == Finite State Machines
**
**   These macros provide a simple mechanism for defining
** finite state machines (FSM).
**
**   Each state containse a block of instructions:
**
** | STATE(state_name) {
** |   ... C instructions ...
** | }
**
**   To move from a state to another you use the GOTO macro:
**
** | if (c == '*') GOTO(stars);
**
** or, in case of an error) the FAIL(x) macro:
**
** | if (c == '?') FAIL(404);  ... 404 is an error code
**
**   There must be two special states states: ON_FAIL and ON_END
**
*/

#define STATE(x)     x##_: if (0) goto x##_;
#define GOTO(x)      goto x##_
#define FAIL(e)      do {ERROR = e; goto fail_; } while(0)
#define ON_FAIL      fail_:
#define ON_END       FINAL_:
#define FINAL        FINAL_
#define GOTOEND      GOTO(FINAL)
#define FALLTHROUGH  do {ERROR=ERROR;} while (0)

int mf_scan(mf_reader *mfile)
{
  long tmp;
  long v1, v2;
  int ERROR = 0;
  long track_time;
  long tracklen;
  long ntracks;
  long curtrack = 0;
  long status = 0;
  unsigned char *msg;
  long chan;

  STATE(mthd) {
    if (readnum(mfile, 4) != MThd) FAIL(110);
    tmp = readnum(mfile, 4); /* chunk length */
    if (tmp < 6) FAIL(111);
    v1 = readnum(mfile,2);
    ntracks = readnum(mfile,2);
    v2 = readnum(mfile,2);
    ERROR = mfile->on_header(v1, ntracks, v2);
    if (ERROR) FAIL(ERROR);
    if (tmp > 6) readnum(mfile,tmp-6);
    GOTO(mtrk);
  }

  STATE(mtrk) {
    if (curtrack++ == ntracks) GOTOEND;
    if (readnum(mfile,4) != MTrk) FAIL(120);
    tracklen = readnum(mfile,4);
    if (tracklen < 0) FAIL(121);
    track_time = 0;
    status = 0;
    ERROR = mfile->on_track(0, curtrack, tracklen);
    if (ERROR) FAIL(ERROR);
    GOTO(event);
  }

  STATE(event) {
    tmp = readnum(mfile,0); if (tmp < 0) FAIL(211);
    track_time += tmp;

    tmp = readnum(mfile,1); if (tmp < 0) FAIL(212);

    if ((tmp & 0x80) == 0) {
      if (status == 0) FAIL(223); /* running status not allowed! */
      GOTO(midi_evt);
    }

    status = tmp;
    v1 = -1;
    if (status == 0xFF) GOTO(meta_evt);
    if (status == 0xF0) GOTO(sys_evt);
    if (status == 0xF7) GOTO(sys_evt);
    if (status >  0xF0) FAIL(543);
    tmp = readnum(mfile,1);
    GOTO(midi_evt);
  }

  STATE(midi_evt) {
    chan = 1+(status & 0x0F);
    v1 = tmp;
    v2 = -1;
    if (mf_numparms(status) == 2) {
      v2 = readnum(mfile,1);
      if (v2 < 0) FAIL(212);
    }
    ERROR = mfile->on_midi_evt(track_time, status & 0xF0, chan, v1, v2);
    if (ERROR) FAIL(ERROR);

    GOTO(event);
  }

  STATE(meta_evt) {
    v1 = readnum(mfile,1);
    if (v1 < 0) FAIL(214);
    GOTO(sys_evt);
  }

  STATE(sys_evt) {
    v2 = readnum(mfile,0);
    if (v2 < 0) FAIL(215);

    msg = readmsg(mfile,v2);
    if (msg == NULL) FAIL(216);

    if (v1 == me_end_of_track) {
      ERROR = mfile->on_track(1, curtrack, track_time);
      if (ERROR) FAIL(ERROR);
      GOTO(mtrk);
    }
    ERROR = mfile->on_sys_evt(track_time, status, v1, v2, msg);
    if (ERROR) FAIL(ERROR);
    status = 0;
    GOTO(event);
  }

  ON_FAIL {
    if (ERROR < 0) ERROR = -ERROR;
    mfile->on_error(ERROR, NULL);
    GOTOEND;
  }

  ON_END {
    return ERROR;
  }
}


/*************************************************************/

static int mf_dmp_header (short type, short ntracks, short division)
{
  printf("HEADER: %u, %u, %u\n", type, ntracks, division);
  return 0;
}

static int mf_dmp_track (short eot, short tracknum, unsigned long tracklen)
{
  printf("TRACK %s: %d (%lu %s)\n", eot?"END":"START", tracknum, tracklen,eot?"ticks":"bytes");
  return 0;
}

static int mf_dmp_midi_evt(unsigned long tick, short type, short chan,
                                                  short data1, short data2)
{
  printf("%8ld %02X %02X %02X", tick, type, chan, data1);
  if (data2 >= 0) printf(" %02X", data2);  /* data2 < 0 means there's no data2! */
  printf("\n");
  return 0;
}

static int mf_dmp_sys_evt(unsigned long tick, short type, short aux,
                                               long len, unsigned char *data)
{
  printf("%8ld %02X ", tick, type);
  if (aux >= 0) printf("%02X ", aux);
  printf("%04lX ", (unsigned long)len);
  type = (type == 0xFF && (0x01 <= aux && aux <= 0x09));
  if (type) {  while (len-- > 0) printf("%c", *data++);   }  /* ASCII */
  else      {  while (len-- > 0) printf("%02X", *data++); }  /* DATA */
  printf("\n");

  return 0;
}

static int mf_dmp_error(short err, char *msg)
{
  if (msg == NULL) msg = "";
  fprintf(stderr, "Error %03d - %s\n", err, msg);
  return err;
};

/*************************************************************/

mf_reader *mf_reader_new(char  *fname)
{
  mf_reader *mr = NULL;
  FILE      *f  = NULL;

  f = fopen(fname,"rb");
  if (f) {
    mr = malloc(sizeof(mf_reader));
    if (mr) {
      mr->file = f;

      mr->on_error    = mf_dmp_error    ;
      mr->on_header   = mf_dmp_header   ;
      mr->on_track    = mf_dmp_track    ;
      mr->on_midi_evt = mf_dmp_midi_evt ;
      mr->on_sys_evt  = mf_dmp_sys_evt  ;

      mr->chrbuf      = NULL;
      mr->chrbuf_sz   = 0;

      mr->aux = NULL;
    }
  }
  return mr;
}

void mf_reader_close(mf_reader *mr)
{
  if (mr) {
    if (mr->file)   fclose(mr->file);
    if (mr->chrbuf) free(mr->chrbuf);
    free(mr);
  }
}

int mf_read( char          *fname      , mf_fn_error   fn_error   ,
             mf_fn_header   fn_header  , mf_fn_track   fn_track   ,
             mf_fn_midi_evt fn_midi_evt, mf_fn_sys_evt fn_sys_evt  )
{
  int ret = 0;
  mf_reader  *mr;

  mr = mf_reader_new(fname);

  if (!mr) return 79;

  if (fn_error)    mr->on_error    = fn_error;
  if (fn_header)   mr->on_header   = fn_header;
  if (fn_track)    mr->on_track    = fn_track;
  if (fn_midi_evt) mr->on_midi_evt = fn_midi_evt;
  if (fn_sys_evt)  mr->on_sys_evt  = fn_sys_evt;

  ret = mf_scan(mr);

  mf_reader_close(mr);

  return ret;
}

/***************************************************************/

#ifdef MF_READ_TEST
int main(int argc, char *argv[])
{

  if (argc > 1) {
    mf_read(argv[1],NULL,NULL,NULL,NULL,NULL);
  }

  return 0;
}
#endif

/*****************************************************************************/
/*
oooooo   oooooo     oooo ooooooooo.   ooooo ooooooooooooo oooooooooooo
 `888.    `888.     .8'  `888   `Y88. `888' 8'   888   `8 `888'     `8
  `888.   .8888.   .8'    888   .d88'  888       888       888
   `888  .8'`888. .8'     888ooo88P'   888       888       888oooo8
    `888.8'  `888.8'      888`88b.     888       888       888    "
     `888'    `888'       888  `88b.   888       888       888       o
      `8'      `8'       o888o  o888o o888o     o888o     o888ooooood8



*/
/*****************************************************************************/
#define eputc(c)  (mw->trk_len++, fputc(c,mw->file))

static void f_write8(mf_writer *mw, unsigned char n)
{  eputc(n);}

static void f_write7(mf_writer *mw, unsigned char n)
{  eputc(n & 0x7F); }

static void f_write16(mf_writer *mw, unsigned short n)
{
  eputc(n >> 8  );
  eputc(n & 0xFF);
}

#if 0  /* Not needed */
static void f_write14(mf_writer *mw, unsigned short n)
{
  eputc(n >> 7  );
  eputc(n & 0x7F);
}
#endif

static void f_write32(mf_writer *mw, unsigned long n)
{
  eputc((n >> 24)       );
  eputc((n >> 16) & 0xFF);
  eputc((n >>  8) & 0xFF);
  eputc((n      ) & 0xFF);
}

static void f_writemsg(mf_writer *mw, unsigned long len, unsigned char *data)
{  while(len-- > 0) eputc(*data++); }

static void f_writevar(mf_writer *mw, unsigned long n)
{
  unsigned long buf;

  n &= 0x0FFFFFFF;
  _dbgmsg("vardata: %08lX -> ", n);

  buf = n & 0x7F;
  while ((n >>= 7) != 0) {
    buf = (buf << 8) | (n & 0x7F) | 0x80;
  }
  _dbgmsg("%08lX\n", buf);
  while (1) {
    eputc(buf & 0xFF);
    if ((buf & 0x80) == 0) break;
    buf >>= 8;
  }
}

mf_writer *mf_new(char *fname, short division)
{
  mf_writer *mw = NULL;

  mw = malloc(sizeof(mf_writer));
  if (!mw) return NULL;

  mw->file = fopen(fname, "wb");

  if (!mw->file) {  free (mw); return NULL; }

  mw->type    = mf_type_file;
  mw->len_pos = 0;
  mw->trk_len = 0;
  mw->trk_cnt = 0;
  mw->trk_in  = 0;
  mw->division  = division;

  /* Write Header chunk (to be rewrite at the end) */
  f_write32(mw, MThd);
  f_write32(mw, 6);
  f_write16(mw, 0);
  f_write16(mw, 1);
  f_write16(mw, division);

  return mw;
}

int mf_track_start (mf_writer *mw)
{
  if (!mw || !mw->file ) { return 309; }

  if (mw->trk_in) mf_track_end(mw);

  mw->trk_cnt++;
  mw->trk_in  = 1;
  mw->chan    = 0;

  f_write32(mw, MTrk);

  /* Save current position for later */
  if ((mw->len_pos = ftell(mw->file)) <  0) { return 301; }

  f_write32(mw, 0);  /* just a place-holder for now */

  mw->trk_len = 0;
  return 0;
}


int mf_track_end(mf_writer *mw)
{
  unsigned long pos_cur;

  if (!mw || !mw->file || !mw->trk_in) { return 329; }

  f_writevar(mw, 0);  f_write8(mw, 0xFF);  f_write8(mw, 0x2F);  f_write8(mw, 0x00);

  if ((pos_cur = ftell(mw->file)) <  0) {  return 322; }

  if (fseek(mw->file, mw->len_pos, SEEK_SET) < 0) {  return 323; }

  f_write32(mw, mw->trk_len);

  if (fseek(mw->file, pos_cur, SEEK_SET) < 0) { return 324; }
  mw->trk_in = 0;
  return 0;
}

int mf_midi_evt (mf_writer *mw, unsigned long delta, short type, short chan,
                                                  short data1, short data2)
{
  unsigned char st;

  if (!mw || !mw->file || !mw->trk_in) { return 319; }

  st = (type & 0xF0);

  if (st == st_system_exclusive)  {return 318; }  /* No sysex accepted here! */

  if (st == st_note_on && data2 == 0) st = st_note_off;

  f_writevar(mw, delta);
  f_write8(mw, st | (chan & 0x0F));
  f_write7(mw,data1);
  if (mf_numparms(st) > 1)  f_write7(mw, data2);
  mw->chan = chan;
  return 0;
}


int mf_sys_evt(mf_writer *mw, unsigned long delta,
                              short type, short aux,
                              long len, unsigned char *data)
{
  if (!mw || !mw->file || !mw->trk_in) { return 349; }

  f_writevar(mw, delta);
  f_write8(mw, type);
  if (type == st_meta_event) f_write8(mw, aux);
  f_writevar(mw, len);
  f_writemsg(mw, len, data );

  return 0;
}

int mf_text_evt(mf_writer *mw, unsigned long delta, short type, char *str)
{
  return mf_sys_evt(mw, delta, st_meta_event, type & 0x0F, strlen(str), (unsigned char *)str);
}

int mf_close (mf_writer *mw)
{
  int ret = 0;
  int format = 0;
  unsigned long pos_cur;

  if (!mw || !mw->file) { return 399; }

  if (mw->trk_in) ret = mf_track_end(mw);

  if (mw->trk_cnt > 1) format = 1;

  if ((pos_cur = ftell(mw->file)) <  0) {  return 392; }

  if (fseek(mw->file, 0, SEEK_SET) < 0) {  return 393; }

  f_write32(mw, MThd);
  f_write32(mw, 6);
  f_write16(mw, format);
  f_write16(mw, mw->trk_cnt);
  f_write16(mw, mw->division);

  if (fseek(mw->file, pos_cur, SEEK_SET) < 0) { return 394; }

  fclose(mw->file);
  free(mw);

  return ret;
}

int mf_pitch_bend(mf_writer *mw, unsigned long delta, unsigned char chan, short bend)
{/* bend is in the range  -8192 .. 8191 */

  if (bend < -8192) bend = -8192;
  if (bend >  8191) bend =  8191;

  bend += 8192;

  return mf_midi_evt(mw, delta, st_pitch_bend, chan, bend, bend  >> 7);
}

int mf_set_tempo(mf_writer *mw, unsigned long delta, long tempo)
{
  unsigned char buf[4];

  buf[0] = (tempo >> 16) & 0xFF;
  buf[1] = (tempo >>  8) & 0xFF;
  buf[2] = (tempo      ) & 0xFF;

  return mf_sys_evt(mw, delta, st_meta_event, me_set_tempo, 3, buf);
}

int mf_set_keysig(mf_writer *mw, unsigned long delta, short acc, short min)
{
  unsigned char buf[4];

  buf[0] = (unsigned char)(acc & 0xFF);
  buf[1] = (unsigned char)(min & 0x01);
  return mf_sys_evt(mw, delta, st_meta_event, me_key_signature, 2, buf);
}


/** **/


#ifdef MF_WRITE_TEST

int main(int argc, char *argv[])
{
   mf_writer *m;
   int ret = 999;

   if ((m = mf_new("xx.mid",192))) {  ret = 0; }

   if (!ret)  {ret = mf_track_start(m);}
   if (!ret)  {ret = mf_text(m, 0, "First");}
   if (!ret)  {ret = mf_midi_evt(m,  0,st_note_on ,0,64,100);}
   if (!ret)  {ret = mf_midi_evt(m,192,st_note_off,0,64,0);}

   if (!ret)  {ret = mf_midi_evt(m,  0,st_note_on ,0,67,100);}
   if (!ret)  {ret = mf_midi_evt(m,192,st_note_off,0,67,0);}
   if (!ret)  {ret = mf_track_end(m);}

   if (!ret)  {ret = mf_track_start(m);}
   if (!ret)  {ret = mf_copyright_notice(m, 0, "(C) by me");}
   if (!ret)  {ret = mf_midi_evt(m,  0,st_note_on ,0,60,100);}
   if (!ret)  {ret = mf_midi_evt(m,384,st_note_off,0,60,0);}
   if (!ret)  {ret = mf_track_end(m);}

   if (!ret)  {ret = mf_close(m);}

   if (ret)   { fprintf(stderr, "ERROR: %d\n",ret); }
   return ret;
}

#endif
