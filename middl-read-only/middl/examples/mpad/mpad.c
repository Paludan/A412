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

#include "libutl.h"
#include "mf.h"

#include <math.h>
#include <assert.h>

#define TK_IGNORE    xFE
#define TK_UNKNOWN   xFF

#define TK_TEMPO     x80
#define TK_PPQN      x81
#define TK_GDUTY     x82
#define TK_GVELOCITY x83
#define TK_GLOOSE    x84
#define TK_GVELVAR   x85
#define TK_GGUITAR   x86
#define TK_GTOMS     x87
#define TK_KEY       x88
#define TK_ACCNOTE   x89
#define TK_GINSTR    x8A

#define TK_NOTE       x81
#define TK_DEFNOTE    x82
#define TK_PAUSE      x83
#define TK_RATIO      x84
#define TK_OCTUP      x85
#define TK_OCTDOWN    x86
#define TK_CHANNEL    x87
#define TK_GUITON     x88
#define TK_GUITOFF    x89
#define TK_TOMSON     x8A
#define TK_TOMSOFF    x8B
#define TK_NUMBER     x8C
#define TK_INSTRUMENT x8D
#define TK_NUMNOTE    x8E
#define TK_STRESS     x8F
#define TK_SOFT       x90
#define TK_VELOCITY   x91
#define TK_GUITCHORD  x92
#define TK_CHORD      x93
#define TK_DUTY       x94
#define TK_PITCH      x95
#define TK_TRANSPOSE  x96
#define TK_CTRL       x97
#define TK_LOOSE      x98
#define TK_VELVAR     x99
#define TK_GCHORD     x9A

#define MAXTRACK  64

chs_t buf = NULL;
chs_t buf_tmp = NULL;

tbl_t macros = NULL;

typedef chs_t *tracklist;

static int cur_track = -1;
static chs_t tracks[MAXTRACK];
static int num_tracks = 0;


int ppqn = 192*4;
int tempo = 60;

#define timetotick(x) ((long)(0.5+((float)(x) * (float)tempo * ppqn)/240000.0))

unsigned long gmode = 0;
unsigned char gvel = 90;
unsigned char gduty = 100;

short glooseq  = 0 ;
short gloosew  = 0 ; 
short gvelvarw = 0 ;
short gvelvarq = 0 ;
char ginstr = 0;


#define M_GUIT 2
#define M_TOMS 3

static mf_handlers mp_handlers;

static int herror(int err, char *msg)
{
  fprintf(stderr,"ERR: %d - %s\n",err,msg); fflush(stderr);
  exit(err);
}

static char *getmacro(char *str, pmxMatches_t capt)
{
  char *name;
  char *endname;
  char c;
  char s[2];

  name = str + pmxStart(capt,2);
  endname = str + pmxEnd(capt,2);
  
  c=*endname;
  *endname = '\0';
      
  s[0] = ' '; s[1] = '\0';
  if (pmxLen(capt,1)>0)
    s[0] = str[pmxStart(capt,1)];
    
  buf_tmp = chsCpy(buf_tmp,s);

  if (pmxLen(capt,3) > 0)
    buf_tmp = chsAddStrL(buf_tmp,str+pmxStart(capt,3)+1,pmxLen(capt,3)-2);   

  macros = tblSetSS(macros,name,buf_tmp);
  /*printf("MACRO: %s = %s\n",name, buf_tmp);*/
  
  *endname = c;

  return utlEmptyString;
}

static char *rplmacro(char *str, pmxMatches_t capt)
{
  char *mc;
  char c;

  char *name;
  char *endname;

  name = str + pmxStart(capt,1);
  endname = str + pmxEnd(capt,1);
  
  c=*endname;
  *endname = '\0';

  mc = tblGetSS(macros,name,NULL);
  *endname = c;

  if (mc == NULL || *mc == '\0') return utlEmptyString;
  mc++;
  buf_tmp = chsCpy(buf_tmp,"(");
  buf_tmp = chsAddStr(buf_tmp,mc);
  buf_tmp = chsAddStr(buf_tmp,")");
  
  return buf_tmp;
}

static char *multiply(char *str, pmxMatches_t capt)
{
  int n;
  char *s;
  int l;
  
  n = atoi(str+pmxStart(capt,3));
  /*printf("REP: %d\n",n);*/
  l =  pmxLen(capt,1);
  if (pmxLen(capt,1)>0) {
    s = str+pmxStart(capt,1)+1;
    l -=2;
  }
  else {
    l = pmxLen(capt,2);
    s = str+pmxStart(capt,2);
  }
  buf_tmp = chsCpyL(buf_tmp,s,l);
  while (--n > 0) {
    buf_tmp = chsAddChr(buf_tmp,' ');
    buf_tmp = chsAddStrL(buf_tmp,s,l);  
  }
  
  return buf_tmp;
}

static int gettracks(char *txt, pmxMatches_t mtc)
{
  int len;
  
  len = pmxLen(mtc,2);
  while (len>0 && isspace(txt[len-1]))
    len--;
  if (len >0) {
    if (pmxLen(mtc,1)>0) 
      cur_track = atoi(txt+pmxStart(mtc,1));
    else cur_track++;
    
    if (cur_track >= MAXTRACK) cur_track = MAXTRACK-1;
    
    tracks[cur_track] = chsAddChr(tracks[cur_track],' ');
    tracks[cur_track] = chsAddStrL(tracks[cur_track],txt+pmxStart(mtc,2),len);
  }
    
  return 0;
}

static void spcstr(char *s,int len)
{
  while (len--) *s++=' ';
}

static chs_t parsemp(chs_t mptext)
{
  char *t;
  int c,k;

  mptext = chsSubStr(mptext, 0,"><s>#&L"," ");   /* remove comments */

  mptext = chsSubStr(mptext, 0,">[&K(<l><!:><*!]>)]","[x&1]");
  mptext = chsSubStr(mptext, 0,">[&K(<+=0-9(b):>)]","[x&1]");
  
  for (t = mptext; *t; t++)  /* mpad is case insensitive */
    *t = tolower(*t);
    
  /* get globals */
  pmxScannerBegin(mptext)
   
    pmxTokSet("<+!abcdefgtrpvk[\">"                     , TK_IGNORE)  
    pmxTokSet("&b\"\""                                  , TK_IGNORE)    
    pmxTokSet("tempo&K(&d)"                             , TK_TEMPO)    
    pmxTokSet("resolution&K(&d)"                        , TK_PPQN)     
    pmxTokSet("ppqn&K(&d)"                              , TK_PPQN)     
    pmxTokSet("duty&K(&d)"                              , TK_GDUTY)     
    pmxTokSet("velocity&K(&d)"                          , TK_GVELOCITY) 
    pmxTokSet("globalvel&K(&d)"                         , TK_GVELOCITY) 
    pmxTokSet("globalloose&K(&d)&K,&K(<?=g>&D)"         , TK_GLOOSE)   
    pmxTokSet("globalvelvar&K(&d)&K,&K(<?=g>&D)"        , TK_GVELVAR)  
    pmxTokSet("globalguitmode"                          , TK_GGUITAR)  
    pmxTokSet("globalguiton"                            , TK_GGUITAR)  
    pmxTokSet("globalinstrument&K(<*d>)(<*a>)"          , TK_GINSTR)  
    pmxTokSet("globaltomson"                            , TK_GTOMS)    
    pmxTokSet("key&K(&d)()&K(<?$min$maj>)"              , TK_KEY)
    pmxTokSet("key&K(<=a-g>)(<?=+&-#b>)&K(<?$min$maj>)" , TK_KEY)
    pmxTokSet("<=a-g><=+&->"                            , TK_ACCNOTE)
    pmxTokSet("<.>"                                     , TK_IGNORE)
      
  pmxTokSwitch
   
    pmxTokCase(TK_GINSTR) :
      if (pmxTokLen(1)>0) {
        ginstr = atoi(pmxTokStart(1));
      }
      else if (pmxTokLen(2) > 0) {
        c = *pmxTokEnd(2);
        *pmxTokEnd(2) = '\0';
        k = mf_instrbyname(pmxTokStart(2));
        if (k >= 0)
          ginstr = k;
        *pmxTokEnd(2) = c;
      }      
      spcstr(pmxTokStart(0),pmxTokLen(0)); 
      continue;
  
    pmxTokCase(TK_ACCNOTE) :
      switch (pmxTokStart(0)[1]) {
        case '-' : pmxTokStart(0)[1] = 'b'; break;
        case '+' : pmxTokStart(0)[1] = '#'; break;
      } 
      continue;

    pmxTokCase(TK_GLOOSE) :
      if (pmxTokLen(2) > 0) {
        glooseq = (*pmxTokStart(2) == 'g') ? -1 : atoi(pmxTokStart(2));
      }
      gloosew = timetotick(atoi(pmxTokStart(1)));
      spcstr(pmxTokStart(0),pmxTokLen(0)); 
      continue;

    pmxTokCase(TK_GVELVAR) :
      if (pmxTokLen(2) > 0) {
        gvelvarq = (*pmxTokStart(2) == 'g') ? -1 : atoi(pmxTokStart(2));
      }
      gvelvarw = atoi(pmxTokStart(1));
      spcstr(pmxTokStart(0),pmxTokLen(0)); 
      continue;

    pmxTokCase(TK_GGUITAR) :
      gmode = (gmode << 2) | M_GUIT;
      spcstr(pmxTokStart(0),pmxTokLen(0)); 
      continue;

    pmxTokCase(TK_GTOMS) :
      gmode = (gmode << 2) | M_TOMS;
      spcstr(pmxTokStart(0),pmxTokLen(0)); 
      continue;

    pmxTokCase(TK_TEMPO) :
      tempo = atoi(pmxTokStart(1));
      spcstr(pmxTokStart(0),pmxTokLen(0)); 
      continue;
      
    pmxTokCase(TK_PPQN) : 
      ppqn = atoi(pmxTokStart(1))*4; 
      spcstr(pmxTokStart(0),pmxTokLen(0)); 
      continue;

    pmxTokCase(TK_GDUTY) : 
      gduty = atoi(pmxTokStart(1)); 
      spcstr(pmxTokStart(0),pmxTokLen(0)); 
      continue;

    pmxTokCase(TK_GVELOCITY) : 
      gvel = atoi(pmxTokStart(1)) ;
      spcstr(pmxTokStart(0),pmxTokLen(0)); 
      continue;
      
    default: continue;
    
  pmxScannerEnd ;

  mptext = chsSubStr(mptext, 0,">&s"," ");               /* compact spaces */
  mptext = chsSubFun(mptext, 0,">m(<*a>)$(<+w>)&K(&B())",getmacro);
  mptext = chsSubFun(mptext, 0,"&*>$(<+w>)",rplmacro);
  mptext = chsSubFun(mptext, 0,"&*>(&b())()&K*&K(&d)"   ,multiply);
  mptext = chsSubFun(mptext, 0,"&*>()(<+! *>)&K*&K(&d)" ,multiply);
  mptext = chsSubStr(mptext, 0,"><+= \t\n\r>"," ");    /* compact spaces */
  mptext = chsSubStr(mptext, 0,"><s>(<=-=.>)","&1");       /* compact note durations and pauses */

  pmxScanStr(mptext,"|(&D)&K(<*!|>)",gettracks);         /* group tracks */

  return mptext;
}

static tracklist parsemp_file(FILE *f)
{
  int k,j;
  chs_t mptext = NULL;
  
  mptext = chsCpy(mptext,"|0\n");
  mptext = chsRead(mptext,f,'a');
  mptext = chsAddChr(mptext,'\n');

  mptext = parsemp(mptext);  
  /*printf("<<%s>>\n",mptext);*/
  mptext = chsFree(mptext);
  
  j = MAXTRACK; k = 0;
  num_tracks = 0;
  do {
    while (k<j && tracks[k]) {
      k++;
      num_tracks++;
    } 
    while (j>k && !tracks[j]) j--;  
    if (j>k) {
      tracks[k] = tracks[j];
      tracks[j] = NULL; 
    }
  } while (j>k);

#if 10
  for (k=0;k<num_tracks;k++) {
    printf("%s\n",tracks[k]);
  }
  fflush(stdout);
#endif

  
  return tracks;
}

int len(int ppqn, int length, float ratio, int lendot, int lenhold)
{
  float ln;
  
  ln = (float)(length * (1<<lendot)) / (float)((1<<(lendot+1)) - 1);
  assert(ln!=0);
  ln = (int)(((ppqn * (1 + lenhold)) * ratio) / ln);
  
  return (int)ln;
}

int vel(int v, int stress, int soft, char modifier)
{
  /*printf("V: %d %d %d %d\n",v,stress,soft,modifier);*/
  switch (modifier) {
    case '\'' : v = (int)((float)v * (1.0+(float)stress/100.0)); break;
    case ','  : v = (int)((float)v * (1.0-(float)soft/100.0)); break;
  }
  if (v<=0) v=1;
  else if (v>127) v = 127;
  return v;  
}


int wrttrack(int trknum)
{
  char note   = 0;
  char octave = 5;
  char length = 4;
  char transpose = 0;
  char channel = 1;
  float ratio = 1.0;
  unsigned char soft = 25;
  unsigned char stress = 50;
  unsigned char velocity = gvel;
  unsigned long mode = gmode;
  char instrument = ginstr ;
  char duty = gduty;
  short looseq  = glooseq  ;
  short loosew  = gloosew  ; 
  short velvarw = gvelvarw ;
  short velvarq = gvelvarq ;

  int k,c,v,t,n;
  char *crd;
  char *trk;
  unsigned long tm;
  
  
  if (trknum == 0) {
    mf_set_tempo(0, tempo, tm_bpm);
    return 0;
  }

  mf_humanize(loosew, looseq, velvarw, velvarq);

  trk = tracks[trknum-1];
  if (trk == NULL || *trk == '\0') return 0;

  if (instrument != 0)
     mf_program_change(mf_curtick, channel, instrument);

  pmxScannerBegin(trk)

    pmxTokSet("&K<?=(>&K<?=)>&K" , TK_IGNORE)
    pmxTokSet("ch&K(<+d>)", TK_CHANNEL)
    pmxTokSet("guiton", TK_GUITON)
    pmxTokSet("guitoff", TK_GUITOFF)
    pmxTokSet("tomson", TK_TOMSON)
    pmxTokSet("tomsoff", TK_TOMSOFF)
    pmxTokSet("stress&K(<+d>)", TK_STRESS)
    pmxTokSet("soft&K(<+d>)", TK_SOFT)
    pmxTokSet("pitch&K(&f)", TK_PITCH)
    pmxTokSet("loose&K(&d)&K<?=,>&K(<?=g>&D)", TK_LOOSE)   
    pmxTokSet("velvar&K(&d)&K<?=,>&K(<?=g>&D)", TK_VELVAR)  
    pmxTokSet("u&K(<?=+&->)(<+d>)", TK_DUTY)
    pmxTokSet("v&K(<?=+&->)(<+d>)", TK_VELOCITY)
    pmxTokSet("t&K(&d)", TK_TRANSPOSE)
    pmxTokSet("<?=',>(<+d>)(<*=.>)(<*==>)", TK_NUMBER)
    pmxTokSet("ctrl&K(<+w>),(&d)", TK_CTRL)
    pmxTokSet("i&K(<*d>)(<*a>)<?=,>(&Q)", TK_INSTRUMENT)
    pmxTokSet("<?=',>n(<?=t>)(<?=+&->)(<+d>)<?=/>(<*d>)(<*=.>)(<*==>)",TK_NUMNOTE)
    pmxTokSet("on()(<?=+&->)(<+d>)<?=/>(<*d>)(<*=.>)(<*==>)",TK_NUMNOTE)
    pmxTokSet("<?=',>(<=a-g>)(<?=#+b>)(<*d>)<?=/>(<*d>)(<*=.>)(<*==>)", TK_NOTE)
    pmxTokSet("<?=',>(x)()()()()(<*==>)", TK_NOTE)
    pmxTokSet("o(<?=a-g>)(<?=#+b>)(<*d>)<?=/>(<*d>)" , TK_NOTE)
    pmxTokSet("r&K(<+d>)&K<?=/>&K(<*d>)", TK_RATIO)
    pmxTokSet("/", TK_OCTUP)
    pmxTokSet("\\", TK_OCTDOWN)
    pmxTokSet("p<?=/>(<*d>)(<*=.>)(<*==>)", TK_PAUSE)
    pmxTokSet("-()()(<*=&->)", TK_PAUSE) 
    pmxTokSet("<?=',>[&Kg:(<+!]>)]<?=/>(<*d>)(<*=.>)(<*==>)", TK_GCHORD)
    pmxTokSet("<?=',>[&K(<+!]>)]<?=/>(<*d>)(<*=.>)(<*==>)", TK_CHORD)
    pmxTokSet("<.>", TK_UNKNOWN)
      
  pmxTokSwitch
  
    pmxTokCase(TK_IGNORE) :  continue;
       
    pmxTokCase(TK_UNKNOWN) :
      continue; 
      
    pmxTokCase(TK_LOOSE) :
      if (pmxTokLen(2) > 0)
        looseq = (*pmxTokStart(2) == 'g') ? -1 : atoi(pmxTokStart(2));
      loosew = timetotick(atoi(pmxTokStart(1)));
      mf_humanize(loosew, looseq, velvarw, velvarq);  
      continue; 
      
    pmxTokCase(TK_VELVAR) :
      if (pmxTokLen(2) > 0)
        velvarq = (*pmxTokStart(2) == 'g') ? -1 : atoi(pmxTokStart(2));
      velvarw = atoi(pmxTokStart(1));
      mf_humanize(loosew, looseq, velvarw, velvarq);  
      continue; 
      
    pmxTokCase(TK_STRESS) :
      stress = atoi(pmxTokStart(1));
      continue; 
      
    pmxTokCase(TK_SOFT) :
      soft = atoi(pmxTokStart(1));
      continue; 
      
    pmxTokCase(TK_VELOCITY) :
      c = atoi(pmxTokStart(2));
      if (pmxTokLen(1)>0) {
        switch(*pmxTokStart(1)) {
          case '+' : c = velocity + c; break;
          case '-' : c = velocity - c; break;
        }
      }
      if (c<=0) c = 1;
      else if (c>127) c = 127;
      velocity = c;
      continue; 
      
    pmxTokCase(TK_TRANSPOSE) :
      transpose = atoi(pmxTokStart(1));
      continue; 
      
    pmxTokCase(TK_PITCH) :
      mf_pitch_bend(mf_curtick, channel, atof(pmxTokStart(1)));
      continue; 
      
    pmxTokCase(TK_DUTY) :
      c = atoi(pmxTokStart(2));
      if (pmxTokLen(1)>0) {
        switch(*pmxTokStart(1)) {
          case '+' : c = duty + c; break;
          case '-' : c = duty - c; break;
        }
      }
      if (c<=0) c = 1;
      else if (c>127) c = 127;
      duty = c;
      continue; 
       
    pmxTokCase(TK_NUMNOTE) :
      c = atoi(pmxTokStart(3));
      if (pmxTokLen(4) > 0) length = atoi(pmxTokStart(4));
      
      if (pmxTokLen(2) > 0) {
        switch(*pmxTokStart(2)) {
          case '+' : c = note + (12 * octave) + c; break;
          case '-' : c = note + (12 * octave) - c; break;
        }
      }
      if (pmxTokLen(1)==0) {
        note = c % 12; octave = c / 12;
      }
      if (*pmxTokStart(0) != 'o') {
        k = len(ppqn,length,ratio,pmxTokLen(5),pmxTokLen(6));
        v = vel(velocity,stress,soft,*pmxTokStart(0));
        mf_note(mf_curtick, channel, transpose + c, k, duty, v);
      }      
      continue; 
      
    pmxTokCase(TK_NOTE) :
      if (*pmxTokStart(1) != 'x') {
        if (pmxTokLen(1) > 0) {
          note = mf_midinote(*pmxTokStart(1),
                             (pmxTokLen(2) > 0) ? *pmxTokStart(2) : '?',
                             0) ;                             
        }
        if (pmxTokLen(3) > 0) octave = atoi(pmxTokStart(3));
        if (pmxTokLen(4) > 0) length = atoi(pmxTokStart(4));
      }
      if (*pmxTokStart(0) != 'o') {
        k = len(ppqn,length,ratio,pmxTokLen(5),pmxTokLen(6));
        /*printf("NOTE: %d %d ch:%d\n",note+12*octave, k,channel);*/
        v = vel(velocity,stress,soft,*pmxTokStart(0));
        mf_note(mf_curtick, channel, transpose + note+12*octave, k, duty, v);
      }               
      continue;
      
    pmxTokCase(TK_RATIO) :
      ratio = (float)atoi(pmxTokStart(1));
      if (pmxTokLen(2) > 0) ratio /= (float)atoi(pmxTokStart(2));
      continue;
       
    pmxTokCase(TK_OCTUP) :
      if (octave <= 10) octave++;
      continue;
      
    pmxTokCase(TK_OCTDOWN) :
      if (octave > 0) octave--;
      continue;
      
    pmxTokCase(TK_INSTRUMENT) :
      if (pmxTokLen(3)>2) {
        pmxTokEnd(3)[-1] = '\0';
        mf_instrument_name(mf_curtick,pmxTokStart(3)+1);
        pmxTokEnd(3)[-1] = pmxTokStart(3)[0];
      }
      if (pmxTokLen(1)>0) {
        k = atoi(pmxTokStart(1));
        if (k != instrument) {
          instrument = atoi(pmxTokStart(1));
          mf_program_change(mf_curtick, channel, instrument);
        }
      }
      else if (pmxTokLen(2) > 0) {
        c = *pmxTokEnd(2);
        *pmxTokEnd(2) = '\0';
        k = mf_percbyname(pmxTokStart(2));
        if (k >= 0) {
          k &= 0x7F;
          channel = 10;  note = k % 12; octave = k / 12;
        } 
        else {
          k = mf_instrbyname(pmxTokStart(2));
          if (k >= 0 && k != instrument) {
            instrument = k;
            mf_program_change(mf_curtick, channel, instrument);
          }
        }
        *pmxTokEnd(2) = c;
      }      
      continue;
      
    pmxTokCase(TK_CTRL) :
      k = -1;
      if (isdigit(*pmxTokStart(1))) {
        k = atoi(pmxTokStart(1));
      }
      else {
        c = *pmxTokEnd(1);
        *pmxTokEnd(1) = '\0';
        k = mf_ctrlbyname(pmxTokStart(1));        
        /*printf("[%s]%d\n",pmxTokStart(1),k);*/        
        *pmxTokEnd(1) = c;
      }
      if (k>=0) {
        mf_control_change(mf_curtick, channel, k, atoi(pmxTokStart(2)));
      }      
      continue;

    pmxTokCase(TK_CHANNEL) :
      channel = atoi(pmxTokStart(1));
      if (channel < 1)  channel = 1;
      else if (channel > 16) channel = 16;
      continue;
      
    pmxTokCase(TK_PAUSE) :
      if (pmxTokLen(1) > 0) 
        length = atoi(pmxTokStart(1));
      k = len(ppqn,length,ratio,pmxTokLen(2),pmxTokLen(3));
      mf_rest(mf_curtick, k);      
      continue;
      
    pmxTokCase(TK_TOMSON) :
      mode = (mode << 2) | M_TOMS;
      continue;
      
    pmxTokCase(TK_TOMSOFF) :
      if ((mode & 3) == M_TOMS) mode >>= 2;
      continue;
      
    pmxTokCase(TK_GUITON) :
      mode = (mode << 2) | M_GUIT;
      continue;
      
    pmxTokCase(TK_GUITOFF) :
      if ((mode & 3) == M_GUIT) mode >>= 2;
      continue; 
      
    pmxTokCase(TK_NUMBER) :
      c = atoi(pmxTokStart(1));
      k = len(ppqn,length,ratio,pmxTokLen(2),pmxTokLen(3)); 
      v = vel(velocity,stress,soft,*pmxTokStart(0));
      switch (mode & 3) {
        case M_GUIT : 
          mf_note(mf_curtick, channel, transpose + note+12*octave + c, k, duty, v);
          break;

        case M_TOMS :
          if (1 <= c && c <= 6) { 
            c = "\251\253\255\257\260\262"[c-1] & 0x7F;
            note = c % 12; octave = c / 12;
            mf_note(mf_curtick, 10, transpose + c, k, duty, v);
          }
          break;

        default :
          length = c;
          k = len(ppqn,length,ratio,pmxTokLen(2),pmxTokLen(3)); 
          v = vel(velocity,stress,soft,*pmxTokStart(0));
          mf_note(mf_curtick, channel, transpose + note+12*octave, k, duty, v);
          break;
      }
      continue;  
      
    pmxTokCase(TK_CHORD) :
      if (pmxTokLen(2) > 0) 
        length = atoi(pmxTokStart(2));

      k = len(ppqn,length,ratio,pmxTokLen(3),pmxTokLen(4));
      v = vel(velocity,stress,soft,*pmxTokStart(0));
      
      c = pmxTokLen(1);
      while (c>0 && isspace(pmxTokStart(1)[c-1])) c--;
      t = pmxTokStart(1)[c];
      pmxTokStart(1)[c] = '\0';
      crd = mf_getchord(pmxTokStart(1), note, octave, transpose);
      pmxTokStart(1)[c] = t;
      
      mf_chord(mf_curtick, channel, crd, k, duty,  v);
      
    default:
      continue;
      
  pmxScannerEnd ;
   
  return 1;
}

int mf_mp2midi(char *mpfile, char *mfile)
{
  FILE *f = stdin;
  tracklist tl;
  int k;

  if ( mpfile && *mpfile && *mpfile != '-' && !(f = fopen(mpfile,"r")))
    return herror(101,"Unable to open input file");

  tl = parsemp_file(f);
  
  if (f != stdin) fclose(f);
  
  mp_handlers[hndl_error     ] = (mf_hndl)herror   ;
  mp_handlers[hndl_writetrack] = (mf_hndl)wrttrack ;

  mf_sethandlers (mp_handlers);
 
  k = mf_write(mfile,1,num_tracks+1,ppqn/4);
  
  return k;
}



int main(int argc, char *argv[])
{
  char *fname = NULL;
  
  if (argc>1) fname = argv[1];
  
  mf_mp2midi(fname,"noname.mid");
  
  exit(0);
}
