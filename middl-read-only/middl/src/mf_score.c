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


#include "mf_score.h"

#if 1 /*  **** PARSING */
/*
ooooooooo.         .o.       ooooooooo.    .oooooo..o oooooooooooo
`888   `Y88.      .888.      `888   `Y88. d8P'    `Y8 `888'     `8
 888   .d88'     .8"888.      888   .d88' Y88bo.       888
 888ooo88P'     .8' `888.     888ooo88P'   `"Y8888o.   888oooo8
 888           .88ooo8888.    888`88b.         `"Y88b  888    "
 888          .8'     `888.   888  `88b.  oo     .d8P  888       o
o888o        o88o     o8888o o888o  o888o 8""88888P'  o888ooooood8
*/



static unsigned char *mnotes = (unsigned char *)"\x09\x0B\x00\x02\x04\x05\x07";

static short in_127(short n)
{
  while (n < 0)    { n += 12; }
  while (n > 127)  { n -= 12; }
  return n;
}

static unsigned char ch_next(trk_data *trks)
{ trks->ptr++; return *(trks->ptr); }

static unsigned char *ch_curptr(trk_data *trks)
{ return trks->ptr; }

static void ch_setptr(trk_data *trks, unsigned char *ptr)
{ trks->ptr = ptr; }

static unsigned char *ch_skip(trk_data *trks)
{ return trks->ptr++; }

static unsigned char ch_get(trk_data *trks)
{ return *(trks->ptr++); }

static unsigned char ch_unget(trk_data *trks)
{ if (trks->ptr > trks->buf) trks->ptr--; return *(trks->ptr); }

static unsigned char ch_cur(trk_data *trks)
{ return *(trks->ptr); }

static unsigned char ch_ahead(trk_data *trks)
{ return *(trks->ptr + 1); }


static int seq_evt (trk_data *trks, short type, short data1, short data2)
{
  return mf_seq_evt(trks->ms, trks->tick[trks->track], type,
                    trks->chan[trks->track], data1, data2);

}

static int seq_sys(trk_data *trks, short type, short aux, long len, unsigned char *data)
{
  return mf_seq_sys(trks->ms,  trks->tick[trks->track], type, aux, len, data);
}


#define ispitchU(c) ('A' <= (c) && (c) <= 'G')
#define ispitchL(c) ('a' <= (c) && (c) <= 'g')
#define isdig(c)    ('0' <= (c) && (c) <= '9')

#define ispitch(c) (ispitchU(c) || ispitchL(c))

static void addnote(trk_data *trks, short n, short dur, short play)
{
  unsigned long d;
  trks->lastnote = n;
  play = play && !flg_chk(trks,FLG_NOPLAY);
  if (play) {
    if (dur != 0) {
      trks->err = seq_evt(trks, st_note_on, n, trks->vel[trks->track]);
    }
    _dbgmsg("TICK: %d DUR: %d\n",  trks->tick[trks->track], dur);
    if (dur != DUR_INFINITE) {
      d = (dur * trks->notepct) / 10000;
      trks->tick[trks->track] += d;
      trks->err = seq_evt(trks, st_note_off, n, 0);
      trks->tick[trks->track] += (dur-d);
    }
  }
}

static unsigned long notelen(trk_data *trks)
{
  unsigned char c;
  unsigned long  tmp = 0;
  unsigned long  dur = 0;
  unsigned short mul = 0;
  unsigned short div = 0;

  c = ch_get(trks);
  dur = trks->dur[trks->track];
  if ( c == '/') {

    c = ch_get(trks);

    if (c == '+') {
      dur = DUR_INFINITE;
      c = ch_get(trks);
    }
    else if (c == '-') {
      dur = 0;
      c = ch_get(trks);
    }
    else {
      dur = (trks->ppqn * 4);

      switch (c) {
        case 'w' : dur = (trks->ppqn * 4) ; c = ch_get(trks); break;
        case 'h' : dur = (trks->ppqn * 2) ; c = ch_get(trks); break;
        case 'q' : dur = (trks->ppqn    ) ; c = ch_get(trks); break;
        case 'e' : dur = (trks->ppqn / 2) ; c = ch_get(trks); break;
        case 's' : dur = (trks->ppqn / 4) ; c = ch_get(trks); break;
        case 't' : dur = (trks->ppqn / 8) ; c = ch_get(trks); break;
      }

      while ('0' <= c && c <= '9') {
        div = div * 10 + (c -'0');
        c = ch_get(trks);
      }
      if (div == 0) div = 1;

      if (c == '*') {
        c = ch_get(trks);
        while ('0' <= c && c <= '9') {
          mul = mul * 10 + (c -'0');
          c = ch_get(trks);
        }
      }
      if (mul == 0) mul = 1;

      dur = dur * mul;
      dur = dur / div;

      trks->dur[trks->track] = dur;
    }
  }
  tmp = dur;
  while ( c == ' ' || c == '\t' ||  c == '=' ) {
    if (c == '=') dur += tmp;
    c = ch_get(trks) ;
  }

  ch_unget(trks);

  return dur;
}

static int getnum(trk_data *trks)
{
  int n = 0;
  unsigned char c;
  short d = 1;

  c = ch_cur(trks);

  if ((c == '+' || c == '-')  && !isdigit(ch_ahead(trks))) return 0;

  if (c == '+')      { c = ch_get(trks);         }
  else if (c == '-') { c = ch_get(trks); d = -1; }

  while ((c = ch_cur(trks)) && isdigit(c)) {
    n = n * 10 + (c - '0');
    c = ch_get(trks);
  }

  return n * d;
}

static void getchord(trk_data *trks, short root, int play)
{
  unsigned long dur;
  unsigned long tck;
  unsigned char c;
  char *q;
  unsigned char inv=0;
  short k;
  short l;
  short n;

  c = ch_get(trks); /* skip '[' */
  _dbgmsg("GCH1: %s\n",ch_curptr(trks));

  q = mf_chordbyname((char *)ch_curptr(trks));

  if (!q) SCORE_FAIL(trks,901);

  _dbgmsg("GCH2: %p\n",q);

  /* look for inversion */
  while ( (c = ch_cur(trks)) && c != ']' && c != ':') c = ch_get(trks);
  if (c == ':') {
    if (isdigit(ch_ahead(trks))) {
      c = ch_get(trks);
      inv = c - '0';
    }
  }
  while ( (c = ch_cur(trks)) && c != ']')
    c = ch_get(trks);
  if (c) c = ch_get(trks);;

  if (q) {        /* chord found */
    if (root < 0) root = trks->notes[trks->track][0];

    trks->notes[trks->track][1] = root;
    for(k=2; *q && k < MAX_CHORDN; q++, k++) {
      n = trks->notes[trks->track][k-1] + *q;
      n = in_127(n);
      trks->notes[trks->track][k] = n;
    }
    trks->chord_n[trks->track] = k-1;
    dur = notelen(trks);
    l = trks->chord_n[trks->track];
    tck = trks->tick[trks->track];
    for (k = 1; k <= l; k++) {
      n = trks->notes[trks->track][k];
      trks->tick[trks->track] = tck;
      addnote(trks, n, dur, play);
    }
  }
}

static void rawchord_start(trk_data *trks, int play)
{
  ch_skip(trks);

  if (flg_chk(trks, FLG_RAWCHORD)) SCORE_FAIL(trks,905);
  flg_set(trks,FLG_RAWCHORD);
  if (!play) flg_set(trks,FLG_NOPLAY);
  trks->chord_n[trks->track] = 0;
}

static void rawchord_end(trk_data *trks)
{
  ch_skip(trks);

  if (!flg_chk(trks, FLG_RAWCHORD)) SCORE_FAIL(trks,905);
  if ( trks->chord_n[trks->track] >= MAX_CHORDN-1) SCORE_FAIL(trks,905);
  trks->chord_n[trks->track]++;
  trks->notes[trks->track][trks->chord_n[trks->track]] = trks->lastnote;
  flg_clr(trks,FLG_RAWCHORD);
  flg_clr(trks,FLG_NOPLAY);
}

static short getroman(trk_data *trks,  unsigned char c)
{
  short n = 0;

  while (c == 'I')  { n++;     c = ch_get(trks); }
  if (c == 'V')     { n = 5-n; c = ch_get(trks); }
  while ( c == 'I') { n++;     c = ch_get(trks); }
  ch_unget(trks);
  if (n <= 0) n = 1;
  if (n >= 8) n = 7;
  return n;
}

static short scalenote(trk_data *trks)
{
  short n;
  short oct;

  oct = trks->oct[trks->track];
  n = getnum(trks) - 1 ;

  if (n < 0) SCORE_FAIL(trks,909);
  else while (n >= trks->scale_n) { n -= trks->scale_n; oct++;}

  if (oct < 0)  oct = 0;
  if (oct > 10) oct = 10;

  n = trks->scale[n] + oct * 12;

  return n;
}

static void getnote(trk_data *trks,int play)
{
  unsigned char c;
  unsigned char istmp = 0;
  short n;
  short cur_oct = 0;
  short tmp_oct = 0;

  c = ch_get(trks);

  if (isdigit(c)) {
    ch_unget(trks);
    n = getnum(trks) + trks->notes[trks->track][0];
    istmp = 1;
  }
  else if (c == '[') {
    n = trks->notes[trks->track][0];
    ch_unget(trks);
  }
  else if (c == 'n' || c == 'N') {
    istmp = (c == 'n');
    _dbgmsg("n: %p [%c]\n",ch_curptr(trks),c);
    c = ch_cur(trks) ;
    n = ( c == '+' || c == '-') ? trks->notes[trks->track][0] : 0;
    n += getnum(trks);
  }
  else {
    cur_oct = trks->oct[trks->track];

    if ('A' <= c && c <= 'G')       n = mnotes[c-'A'] + cur_oct * 12;
    else if (c == 'X')              n = trks->notes[trks->track][0];
    else if (c == 'x')            { n = trks->notes[trks->track][0]; istmp = 1; }
    else if (c == 'I' || c == 'V')
            n = trks->scale[(getroman(trks,c)-1 ) % trks->scale_n] + cur_oct * 12;
    else if (c == '$' && isdigit(ch_cur(trks))) n = scalenote(trks);
    else if (c == '#' && isdigit(ch_cur(trks)))
            n = trks->notes[trks->track][1+((ch_get(trks) - '1') % trks->chord_n[trks->track])];
    else {SCORE_FAIL(trks,999) ;}

    c = ch_get(trks);
    if (c == 'b')      { n--; while ((c = ch_cur(trks)) == 'b')  {n--; c = ch_get(trks);} }
    else if (c == '#') { n++; while ((c = ch_cur(trks)) == '#')  {n++; c = ch_get(trks);} }
    else ch_unget(trks);

    c = ch_get(trks);
    if ('0' <= c && c <= '9')  { cur_oct = c-'0'+1; n = (n % 12) + cur_oct * 12; }
    else if ( c == 'N')        { cur_oct = 0;       n = (n % 12); }
    else if ( c == '\'')       { tmp_oct++; while ((c = ch_cur(trks)) == '\'' && tmp_oct <  10)  {tmp_oct++; c = ch_get(trks);} }
    else if ( c == ',')        { tmp_oct--; while ((c = ch_cur(trks)) == ','  && tmp_oct > -10)  {tmp_oct--; c = ch_get(trks);} }
    else ch_unget(trks);

    trks->oct[trks->track] = cur_oct;
  }

  n = in_127(n);

  if (!istmp) trks->notes[trks->track][0] = n;

  n += tmp_oct * 12;

  n = in_127(n);

  if (ch_cur(trks) == '[') {
    getchord(trks, n, play);
  }
  else {
    addnote(trks, n, notelen(trks), play);
  }
}

static void rest(trk_data *trks)
{
  unsigned char c;
  unsigned long dur;
  unsigned short mul = 0;

  if ((c = ch_cur(trks)) == '-') {
    for(;;) {
     c = ch_get(trks);
           if (c == '-' || c == '=')  { mul++; }
      else if (c != '\t' && c != ' ') { ch_unget(trks); break; }
    }
    dur = trks->dur[trks->track] * mul;
  }
  else {
    c = ch_get(trks);
    dur = notelen(trks);

    if (dur == 0 || c == 'r') {
      trks->err = seq_evt(trks, st_note_off, trks->lastnote, 0);
    }
  }

  if (dur != DUR_INFINITE) {  trks->tick[trks->track] += dur; }
}

static void chgtrack(trk_data *trks)
{
  unsigned short trk;

  ch_skip(trks); /* skip '|' */

  if (isdigit(ch_cur(trks)))  trk = getnum(trks);
  else trk = trks->track+1;

  if (trk < 0) trk = 0;
  else if (MAX_TRACKS <= trk) trk = MAX_TRACKS-1;

  trks->track = trk;
  mf_seq_set_track(trks->ms, trk);

}

#if 1/* *** REPEAT */

static void back(trk_data *trks)
{
  unsigned char  c;

  ch_skip(trks);
  c = ch_cur(trks);
  if ( isdigit(c)) {
    trks->tick[trks->track] = trks->trgt[c-'0'];
    c = ch_get(trks);
  }
  else if (trks->dur[trks->track] != DUR_INFINITE) {
    if (trks->tick[trks->track] < trks->dur[trks->track])
      trks->tick[trks->track] = 0;
    else
      trks->tick[trks->track] -= trks->dur[trks->track];
  }
  _dbgmsg("BACK TO: %d\n", trks->tick[trks->track]);
}

static void rtarget(trk_data *trks)
{
  unsigned char  c;

  ch_skip(trks);
  c = ch_get(trks);

  if (isdigit(c)) {
    trks->trgt[c-'0'] = trks->tick[trks->track];
  }
  else if (c == '(') {
    if (trks->trgt_n < MAX_TRGT) {
      trks->trgt_r[trks->trgt_n++] = trks->tick[trks->track];
      _dbgmsg("RTRGT: %d\n",trks->trgt_r[trks->trgt_n-1]);
    }
  }
  else if (c == ')') {
    if (trks->trgt_n > 0) trks->trgt_n--;
  }
  else if (c == '<') {
    ch_unget(trks) ; back(trks);
  }
  else if (c == '&') {
    if (trks->trgt_n > 0) {
      trks->tick[trks->track] = trks->trgt_r[trks->trgt_n-1];
      _dbgmsg("RGOTO: %d\n",trks->tick[trks->track]);
    }
    else {ch_unget(trks) ; back(trks);}
  }
  else {
    if (flg_chk(trks, FLG_RAWCHORD)) {
      if ( trks->chord_n[trks->track] >= MAX_CHORDN-1) SCORE_FAIL(trks,905);
      trks->chord_n[trks->track]++;
      trks->notes[trks->track][trks->chord_n[trks->track]] = trks->lastnote;
    }
    if (!flg_chk(trks, FLG_NOPLAY)) {
      trks->ptr -=2 ;
      back(trks);
    }
  }

}
#endif /**/

static void skipctrl(trk_data *trks)
{
  unsigned char  c;
  c = ch_get(trks);
  while (c && c != ':') c = ch_get(trks);
}

static void setvol(trk_data *trks)
{
  short n = 0;
  unsigned char  c;

  skipctrl(trks);
  c = ch_cur(trks);
  if (isdigit(c)) {
    n = getnum(trks);
  } else {
    SCORE_FAIL(trks,908);
  }
  trks->err = seq_evt(trks, st_control_change, cc_channel_volume, n);
}

static void setvel(trk_data *trks)
{
  short n = 0;
  skipctrl(trks);
  n = getnum(trks);
  trks->vel[trks->track] = n;
}

static void setmeter(trk_data *trks)
{
  short n = 0;
  short d = 2;

  unsigned char data[4];

  skipctrl(trks);
  n = getnum(trks);
  if ( n == 0)  n=4;

  if (ch_cur(trks) == '/') {
    ch_skip(trks);
    d = getnum(trks);
    switch (d) {
      case   2: d = 1; break;
      case   4: d = 2; break;
      case   8: d = 3; break;
      case  16: d = 4; break;
      case  32: d = 5; break;
      case  64: d = 6; break;
      default : d = 2;
    }
  }

  trks->num = n;
  trks->den = d;

  data[0] = n; data[1]= d; data[2] = 24; data[3] = 8;

  mf_seq_set_track(trks->ms, 0);
  trks->err = seq_sys(trks, st_meta_event, me_time_signature, 4, data);
  mf_seq_set_track(trks->ms, trks->track);

}

static void setbpm(trk_data *trks)
{
  unsigned char data[4];
  unsigned long tempo;

  skipctrl(trks);
  tempo = getnum(trks);

  if ( tempo <= 0)  tempo=90;

  _dbgmsg("TEMPO: %d ",tempo);
  tempo = 60000000  / tempo;
  _dbgmsg("  %d (us)\n",tempo);

  data[0] = (tempo >> 16) & 0xFF;
  data[1] = (tempo >>  8) & 0xFF;
  data[2] = (tempo      ) & 0xFF;

  mf_seq_set_track(trks->ms, 0);
  trks->err = seq_sys(trks, st_meta_event, me_set_tempo, 3, data);
  mf_seq_set_track(trks->ms, trks->track);
}

#if 1 /* *** SET KEY AND SCALE */

static char *scales[] = {
/*  name minor? scale
     /   /  ____/\_________
    / \ / \/               \   */
   "aeo\01\02\01\02\02\01\02",
   "dor\01\02\01\02\02\02\01",
   "har\01\02\01\02\02\01\03",
   "ion\00\02\02\01\02\02\02",
   "loc\01\01\02\02\01\02\02",
   "lyd\00\02\02\02\01\02\02",
   "maj\00\02\02\01\02\02\02",
   "min\01\02\01\02\02\01\02",
   "mix\00\02\02\01\02\02\01",
   "nmj\00\01\02\02\02\02\02",
   "nmn\01\01\02\02\02\01\03",
   "phr\01\01\02\02\02\01\02",
   "pmj\00\02\02\03\02"        ,
   "pmn\01\03\02\02\03"
};


/* see  http://www.musictheory.net/lessons/25 */
                               /* A   B   C   D   E   F   G */
static char *keyacc = "\004\006\001\003\005\000\002";
/*          maj        #         10   12  7   9   11   6   8
                                 -2   0      -3   -1      -4

            min        #          7   9   4   6    8   3   5
                                     -3           -4

            maj        b         -4  -2  -7  -5   -3  -8  -6
                                                       4

            min        b         -7  -5 -10  -8   -6 -11  -9
                                          2   4        1   3

*/

static int sc_cmp(const void *a, const void *b)
{
  _dbgmsg("SC_CMP: %.3s %.3s\n",*(char**)a, *(char **)b);
  return strncmp(*(char**)a, *(char **)b,3) ;
}

static char *getscale(trk_data *trks)
{
  char **q = NULL;
  unsigned char *s = NULL;
  unsigned char c;

  c = ch_cur(trks);

  if (c == ':') {
    ch_skip(trks);
    s = ch_curptr(trks);
    q = bsearch(&s, scales, sizeof(scales)/sizeof(scales[0]),
                            sizeof(scales[0]), sc_cmp);
  }
  if (q) {
    ch_skip(trks); ch_skip(trks); ch_skip(trks);
  }
  else {
    s = (unsigned char *)"maj";
    q = bsearch(&s, scales, sizeof(scales)/sizeof(scales[0]),
                            sizeof(scales[0]), sc_cmp);
  }
  if (q)  return *q;
  SCORE_FAIL(trks,999); ; /* WHAT? Where's my major scale?? */
}

unsigned char *keyroot = (unsigned char *)
    /* Cb  Gb  Db  Ab  Eb  Bb  F   C   G   D   A   E   B  F#   C# */
     "\x0B\x06\x01\x08\x03\x0A\x06\x00\x07\x02\x09\x04\x0B\x06\x01"
     "\x08\x03\x0A\x05\x00\x07\x02\x09\x04\x0B\x06\x01\x08\x03\x0A";
    /* Ab  Eb  Bb  F   C   G   D   A   E   B   F#  C#  G#  D#  A# */
    /*-7  -6  -5  -4  -3  -2  -1   0   1   2   3   4   5   6   7  */

#define NO_KEY 100

static void setkey(trk_data *trks)
{
  /*  @key:+3:min  @key:Db:min */
  short n = NO_KEY;
  short m = 0;
  short r = -1;
  short a = 0;
  short k = 0;
  unsigned char data[4];
           char *q = NULL;
  unsigned char c;

  skipctrl(trks);

  c = ch_cur(trks);

  if (c == '+' || c == '-' || isdigit(c)) {
    n = getnum(trks);
  }
  else if (ispitchU(c)) {
    r = ch_get(trks) - 'A';
    switch(ch_get(trks)) {
      case 'b' : a = -1; break;
      case '#' : a =  1; break;
      default  : ch_unget(trks);
    }
  }

  c = ch_cur(trks);
  q = getscale(trks);

  if (!q) return; /* ??? */
  m = q[3];

  if (n == NO_KEY) {
    n = keyacc[r]-1;
    n += a * 7;
    n -= m * 3;
  }

  while (n < -7) n += 12;
  while (n >  7) n -= 12;

  _dbgmsg("KEY %.3s %d %d\n",q,m,n);

  trks->scale[0] = keyroot[(7+n)+m*15];

  for (k=1, q += 4; *q && k < MAX_SCALE; k++,q++) {
    trks->scale[k] = trks->scale[k-1] + *q;
  }
  trks->scale_n = k;

  _dbgmsg("Key: r=%d a=%d m=%d n=%d p=[%s]\n",r,a,m,n,p);

  data[0] = n; data[1] = m;

  mf_seq_set_track(trks->ms, 0);
  trks->err = seq_sys(trks, st_meta_event, me_key_signature, 2, data);
  mf_seq_set_track(trks->ms, trks->track);
}

#endif /**/

static void setchannel(trk_data *trks)
{

  short n;
  skipctrl(trks);
  n = getnum(trks);
  if ( n < 1 || 16 < n) SCORE_FAIL(trks,906);
  trks->chan[trks->track] = n-1;
}

static void setinstr(trk_data *trks)
{
  unsigned char  c;
  short n = -1;
  char iname[32];
  short k = 0;

  skipctrl(trks);

  c = ch_cur(trks);
  if (isdigit(c)) {
    n = getnum(trks);
  }
  else {
    c = ch_get(trks);
    while (c && isalnum(c) && k<30) {
      iname[k++] = c;
      c = ch_get(trks);
    }
    iname[k] = '\0';
    while (c && isalnum(c)) {
      c = ch_get(trks);
    }
    ch_unget(trks);

    n = mf_instrbyname(iname);
  }

  if (n < 0) SCORE_FAIL(trks, 903);
  if (n < 128) {
    trks->err = seq_evt(trks, st_program_change, n, 0);
    if (!trks->err && k>0)
      trks->err = seq_sys(trks, st_meta_event, me_instrument_name, k, (unsigned char *)iname);
  }
  else {  /* drums */
    n &= 0x7F;
    trks->chan[trks->track] = 9;
    trks->notes[trks->track][0] = n;
  }
}

static void setcc(trk_data *trks)
{
  short n;
  short v = 0;

  skipctrl(trks);
  n = getnum(trks);
  if (n < 0 || 127 < n) SCORE_FAIL(trks,907);
  if (ch_cur(trks) != ':') SCORE_FAIL(trks,907);
  ch_skip(trks);
  v = getnum(trks);
  if (v < 0 || 127 < v) SCORE_FAIL(trks,908);

  trks->err = seq_evt(trks, st_control_change, n, v);
}

static void setnotepct(trk_data *trks)
{
  short n;
  skipctrl(trks);
  n = getnum(trks);
  if (n < 1 || 100 < n) SCORE_FAIL(trks,907);
  trks->notepct = n * 100;
}

static void ctrl(trk_data *trks)
{
  char *p;

  ch_skip(trks);
  p = (char *)ch_curptr(trks);

  _dbgmsg("CTRL: [%c]\n",c);

       if (strncmp(p,"meter:",6) == 0)    { setmeter(trks);   }
  else if (strncmp(p,"key:"  ,4) == 0)    { setkey(trks);     }
  else if (strncmp(p,"bpm:"  ,4) == 0)    { setbpm(trks);     }
  else if (strncmp(p,"pan:"  ,4) == 0)    {                   }
  else if (strncmp(p,"rev:"  ,4) == 0)    {                   }
  else if (strncmp(p,"cc:"   ,3) == 0)    { setcc(trks);      }
  else if (strncmp(p,"ch:"   ,3) == 0)    { setchannel(trks); }
  else if (strncmp(p,"vol:"  ,4) == 0)    { setvol(trks);     }
  else if (strncmp(p,"vel:"  ,4) == 0)    { setvel(trks);     }
  else if (strncmp(p,"instr:",6) == 0)    { setinstr(trks);   }
  else if (strncmp(p,"npct:", 5) == 0)    { setnotepct(trks); }
  else    { SCORE_FAIL(trks,902); }
}

/* ppp: 16 pp: 32 p: 48 mp: 64 mf: 80 f: 96 ff: 112 fff: 127 */
/*                0   1   2   3   4   5   6   7
                  ppp pp  p   mp  mf  f   ff  fff */
static char *dyn ="\x10\x20\x30\x40\x50\x60\x70\x7F";

static int try_dynamics(trk_data *trks)
{
  unsigned char  c;
  char *p;
  short n = -1;
  short k = 0 ;

  p = (char *)ch_curptr(trks);

       if (strncmp(p,"ppp", 3) == 0)  { n = 0; k = 3; }
  else if (strncmp(p,"pp",  2) == 0)  { n = 1; k = 2; }
  else if (strncmp(p,"p",   1) == 0)  { n = 2; k = 1; }
  else if (strncmp(p,"mp",  2) == 0)  { n = 3; k = 2; }
  else if (strncmp(p,"mf",  2) == 0)  { n = 4; k = 2; }
  else if (strncmp(p,"fff", 3) == 0)  { n = 7; k = 3; }
  else if (strncmp(p,"ff",  2) == 0)  { n = 6; k = 2; }
  else if (strncmp(p,"f",   1) == 0)  { n = 5; k = 1; }

  if (n<0) return 0;

  trks->err = seq_evt(trks, st_control_change, cc_channel_volume, dyn[n]);

  while (k--) ch_skip(trks);

  return 1;
}

static void expression(trk_data *trks)
{
  char *p;
  short done = 0;
  ch_skip(trks);        /* skip ':' */

  if (!done) done = try_dynamics(trks);

}

static void defvalue(trk_data *trks)
{
  unsigned char c;
  short oct = 0;

  ch_skip(trks);        /* skip ':' */
  c = ch_cur(trks);
  _dbgmsg("DEFV: [%c]\n",c);

  if (!c || isspace(c)) return;

  if ( c == '\'')       { while ((c = ch_cur(trks)) == '\'' && oct <  10)  {oct++; c = ch_get(trks);} }
  else if ( c == ',')   { while ((c = ch_cur(trks)) == ','  && oct > -10)  {oct--; c = ch_get(trks);} }
  else if ( c == '[')   { rawchord_start(trks,0); }
  else if ( c == '/')   { notelen(trks); }
  else getnote(trks,0);

  if (oct != 0) {
    trks->oct[trks->track] += oct;
    if (trks->oct[trks->track] > 9) trks->oct[trks->track] = 9;
    if (trks->oct[trks->track] < 0) trks->oct[trks->track] = 0;
  }
}

static void gettxt(trk_data *trks)
{
   short n = 0;
   unsigned char c;
   unsigned char q;
   unsigned char *data;
   short e;
   unsigned long dur;

   q = ch_get(trks);
   data = ch_curptr(trks);
   c = ch_get(trks);

   while (c && c != q) {
     _dbgmsg("C: %d [%c]\n",n,c);
     if (c == '\\' && ch_cur(trks)) { n++; c = ch_get(trks); }
     c = ch_get(trks);
     n++;
   }
   if (!c) ch_unget(trks);

   dur = notelen(trks);

   e = me_lyric;
   if (q == '"') e = me_text;
   seq_sys(trks, st_meta_event, e, n, data);
   if (dur != DUR_INFINITE) {
     trks->tick[trks->track] += dur;
   }
}

#if 1 /* REPEAT */

static void rptstart(trk_data *trks)
{
   unsigned char c;

    ch_skip(trks);
    c = ch_cur(trks);

    if (c && trks->rpt_top < MAX_REPT) {
      trks->rpt_pos[trks->rpt_top] = ch_curptr(trks);
      trks->rpt_cnt[trks->rpt_top] = -1;
      trks->rpt_top++;
    }
}

static void rptskipcnt(trk_data *trks)
{
   unsigned char c;
   c = ch_cur(trks);

   if (c == '*') {
     c = ch_get(trks);
     while (isdigit((c = ch_cur(trks)))) {c = ch_get(trks);}
   }
}

static void rptend(trk_data *trks)
{
  unsigned char c;
  int k;

  ch_skip(trks);     /* skip ) */
  c = ch_cur(trks);

  if (trks->rpt_top == 0) {
    rptskipcnt(trks);
    return;
  }

  k =  trks->rpt_top - 1;

  if (trks->rpt_cnt[k] > 0) { /* we are repeating */
    trks->ptr = trks->rpt_pos[k];
    trks->rpt_cnt[k]--;
  }
  else if (trks->rpt_cnt[k] == 0) { /* that was last repetition */
    rptskipcnt(trks);
    trks->rpt_top--;
  }
  else {  /* that was the first repetition (set cnt) */
    if (c == '*') {
      if (trks->rpt_top == MAX_REPT) {
        rptskipcnt(trks);
      }
      else {
        trks->rpt_cnt[k] = 0;
        c = ch_get(trks);
        trks->rpt_cnt[k] = getnum(trks)-2;
        if (trks->rpt_cnt[k] >= 0)  { trks->ptr = trks->rpt_pos[k]; }
        else trks->rpt_top--;
      }
    }
  }
}
#endif /**/

static void getlinenum(trk_data *trks)
{
  unsigned char c;

  ch_skip(trks);       /* skip \n */
  c = ch_get(trks);
  if (c == '\t') {
    trks->line = getnum(trks);
    ch_skip(trks);    /* skip ' ' */
  }
  else ch_unget(trks);
  _dbgmsg("LINE: %d\n",trks->line);
}

static void parse(trk_data *trks)
{
  unsigned char c;

  while ((c = ch_cur(trks))) {
    _dbgmsg("C: %c\n",c);
    if (ispitch(c) || isdigit(c) || (c == 'I') || (c == 'V') ||
        (c == '$') || (c == 'N') || (c == 'n') ||
        (c == '#') || (c == 'x') || (c == 'X' ) ) { getnote(trks,1);   }
    else if ( c == 'r' || c == 'R' || c == '-')   { rest(trks);        }
    else if ( c == '|' )                          { chgtrack(trks);    }
    else if ( c == '&' )                          { rtarget(trks);     }
    else if ( c == '@' )                          { ctrl(trks);        }
    else if ( c == ':' )                          { defvalue(trks);    }
    else if ( c == '!' )                          { expression(trks);  }
    else if ( c == '"' )                          { gettxt(trks);      }
    else if ( c == '\'' )                         { gettxt(trks);      }
    else if ( c == '(' )                          { rptstart(trks);    }
    else if ( c == ')' )                          { rptend(trks);      }
    else if ( c == '[' )                          { rawchord_start(trks,1); }
    else if ( c == ']' )                          { rawchord_end(trks); }
    else if ( c == '\n' )                         { getlinenum(trks);  }
    else if ( c == ';' || isspace(c))             { ch_skip(trks);     }
    else SCORE_FAIL(trks, 904);
  }
}

static int tomidi(char *fname, short division, unsigned char *s)
{
  int err = 0;
  trk_data tracks;
  int k;

  if (!fname)    return 911;
  if (!s || !*s) return 912;

  for (k = 0; k < MAX_TRGT ; k++) {
    tracks.trgt[k] = 0;
  }

  tracks.scale[0] =  0;  /* C */
  tracks.scale[1] =  2;  /* D */
  tracks.scale[2] =  4;  /* E */
  tracks.scale[3] =  5;  /* F */
  tracks.scale[4] =  7;  /* G */
  tracks.scale[5] =  9;  /* A */
  tracks.scale[6] = 11;  /* B */

  tracks.scale_n = 7;

  for (k = 0; k < MAX_TRACKS; k++) {
       tracks.dur [k]     = division;  /* a quarter note */
       tracks.tick[k]     = 0;
       tracks.chan[k]     = 0;
       tracks.inst[k]     = 0;
      tracks.notes[k][0]  = 60;
    tracks.chord_n[k]     = 0;
        tracks.vel[k]     = 90;
        tracks.oct[k]     = 5;
  }

  tracks.line = 0;

  tracks.track   = 0;
  tracks.ppqn    = division;
  tracks.buf     = s;
  tracks.ptr     = tracks.buf;
  tracks.err     = 0;
  tracks.rpt_top = 0;
  tracks.trgt_n  = 0;

  tracks.notepct = 9000; /* 90% */

  tracks.num = 4;
  tracks.den = 2;

  tracks.lastnote = 60;
  tracks.flags = 0;

  tracks.ms = mf_seq_new(fname, division);

  if (tracks.ms) {

    /* Set channels volume */
    for (k=0; k<16; k++) {
      mf_seq_evt(tracks.ms, 0, st_control_change, k, cc_channel_volume, 80);
    }


    if ((err = setjmp(tracks.errjmp)) == 0) {
      parse(&tracks);
    }
    if (!err) err = mf_seq_close(tracks.ms);
  }
  else err = 910; /* Unable to open midifile */

  if (tracks.err) err = tracks.err;

  if (err) err = tracks.line * 1000 + err;
  return err;
}

int mf_score(char *fname, short division, unsigned char *score)
{
  unsigned char *p = NULL;
  int err=0;

  p = mf_demacro(score, &err);
  if (!err) {
    err = tomidi(fname, division, p);
    if (p) free(p);
  }

  return err;
}

#endif /**/

#ifdef MF_SCORE_TEST

/* 100Kb max */
#define SCORE_BUF_SIZE (102400-1)
unsigned char score_buf[SCORE_BUF_SIZE+1];

int main(int argc, char *argv[])
{
  unsigned char *s;
  unsigned long k;
  mf_error err = 0;
  int c;
  FILE *f;

  if (argc > 1) {
    f = fopen(argv[1],"r");
    if (f) {
      s = score_buf;
      k = 0;
      while (((c = fgetc(f)) != EOF) && (k < SCORE_BUF_SIZE)) {
        _dbgmsg("R: %c\n",c);
        *s++ = c;
        k++;
      }
      *s = '\0';

      fclose(f);
      err = mf_score("sc.mid",384, score_buf);
    }
    else { fprintf(stderr,"Unable to open input file\n"); }
  }
  else { fprintf(stderr,"Usage:  middl infile [outfile]\n"); }

  if (err) fprintf(stderr, "LINE:%d ERR:%d %s\n", mf_errline(err), mf_errnum(err), mf_errmsg(err));

  return(err);
}

#endif

