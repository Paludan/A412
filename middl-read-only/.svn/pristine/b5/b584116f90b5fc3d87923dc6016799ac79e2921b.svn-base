#include "mf_score.h"


/*
ooo        ooooo       .o.         .oooooo.   ooooooooo.     .oooooo.
`88.       .888'      .888.       d8P'  `Y8b  `888   `Y88.  d8P'  `Y8b
 888b     d'888      .8"888.     888           888   .d88' 888      888
 8 Y88. .P  888     .8' `888.    888           888ooo88P'  888      888
 8  `888'   888    .88ooo8888.   888           888`88b.    888      888
 8    Y     888   .8'     `888.  `88b    ooo   888  `88b.  `88b    d88'
o8o        o888o o88o     o8888o  `Y8bood8P'  o888o  o888o  `Y8bood8P'
*/

/*
   $macro-name{jsjakdjks}
   $macro-name
*/

static int is_idchar(unsigned char c)
{
  return (c && (('A' <= c && c <= 'Z') ||
                ('a' <= c && c <= 'z') ||
                (c == '_')) );
}

static int is_macro_def(unsigned char *q)
{
   while (is_idchar(*q))  q++;
   _dbgmsg("ismacro: [%s][%s]\n",p,q);
   return (*q == '{');
}

typedef struct {
  unsigned char **macro_vec;
  unsigned long   macro_cnt;
  unsigned long   macro_max;
} macro_defs;

#define MACRO_INCR 512

static int chkmacro(macro_defs *m, unsigned long n)
{
  unsigned long newmax;
  unsigned char **vec;

  if (!m) return 0;

  newmax = m->macro_max;
  while (n >= (newmax - m->macro_cnt))
    newmax += MACRO_INCR;

  if (newmax > m->macro_max) {
    vec = realloc(m->macro_vec, newmax * sizeof(unsigned char *));
    if (!vec) return 0;
    m->macro_vec = vec;
    m->macro_max = newmax;
  }
  return (m->macro_vec != NULL);
}

static unsigned char *skip_identifier(unsigned char *q)
{
   while (is_idchar(*q)) q++;
   return q-1;
}

static int id_cmp(unsigned char *a,unsigned char *b)
{
  while (is_idchar(*a) && is_idchar(*b) && (*a == *b)) {a++; b++;}

  if (is_idchar(*a) || is_idchar(*b)) return (*a - *b);
  return 0;
}

static unsigned char *getmacro(macro_defs *m,unsigned char *p)
{
   int k;
   _dbgmsg("getmacro: %s\n",p);
   for (k=0; k < m->macro_cnt; k++) {
     if (id_cmp(m->macro_vec[k] , p) == 0) {
       p = m->macro_vec[k];
       while (*p && *p != '{') p++;
       return p;
     }
   }
   return NULL;
}

typedef struct {
  unsigned char *buf;
  unsigned long  buf_cnt;
  unsigned long  buf_max;
} charbuf;

#define BUF_INCR (128*1024)

static int chkbuf(charbuf *b, unsigned long n)
{
  unsigned long newmax;
  unsigned char *buf;

  if (!b) return 0;

  newmax = b->buf_max;
  while (n >= (newmax - b->buf_cnt))
    newmax += BUF_INCR;

  if (newmax > b->buf_max) {
    buf = realloc(b->buf, newmax);
    if (!buf) return 0;
    b->buf = buf;
    b->buf_max = newmax;
  }
  return (b->buf != NULL);
}


static void addchar(charbuf *b, unsigned char c)
{
  if (chkbuf(b,2)) {
    b->buf[b->buf_cnt++] = c;
    b->buf[b->buf_cnt] = '\0';
  }
}

static void addstring(charbuf *b, char *s)
{
  while (*s) addchar(b,*s++);
}

static unsigned long addlong(charbuf *b, unsigned long n)
{
  char data[16];
  char *p;
  sprintf(data,"%lu",n);
  p=data;
  while (*p) addchar(b,*p++);
  addchar(b,' ');
  return 0;
}

#define MAX_STK_TOP 128

#define MACRO_FAIL(x) do {if (buf.buf) free(buf.buf); *err = line * 1000 + (x); return p;} while(0)

#define addline(b,x) do { addchar(b,'\t'); addlong(b, x); } while (0)

unsigned char *mf_demacro(unsigned char *inbuf, int *err)
{
  macro_defs macros;
  charbuf buf;

  unsigned long line = 0;
  unsigned long track = 0;

  unsigned char *p = inbuf;
  unsigned char *q;

  unsigned char *stk[MAX_STK_TOP];
  unsigned long  stk_ln[MAX_STK_TOP];
  int stk_top = 0;

  macros.macro_vec = NULL;
  macros.macro_cnt = 0;
  macros.macro_max = 0;
  *err = 0;

  buf.buf = NULL;
  buf.buf_cnt = 0;
  buf.buf_max = 0;

  addchar(&buf,'\n');
  addline(&buf,++line);

  while (*p) {
    if (*p == '}') {
      if (stk_top == 0) MACRO_FAIL(901); /* stray close brace */
      stk_top--;
      p = stk[stk_top];
      line = stk_ln[stk_top];
      addchar(&buf,'\n'); addline(&buf, line);
    } else if (*p == '\t' || *p == '\v') {
      addchar(&buf,' ');
    } else if (*p == '\r') {
      if (p[1] != '\n') {
        addchar(&buf,'\n');
        if (stk_top == 0) {addline(&buf, ++line); }
      }
    } else if (*p == '\n') {
      addchar(&buf,'\n');
      if (stk_top == 0) { addline(&buf, ++line);  }
    } else if (*p == '{') {
      MACRO_FAIL(903);                   /* stray open brace */
    } else if (*p == ')' && p[1] == '&') { /*  transform ")&" in "&)"  */
      addchar(&buf,'&');addchar(&buf,')'); p++;
    } else if (*p == '$') {
      if (p[1] == '$') {
         addchar(&buf,'$'); p++;
      }
      else if (isdigit(p[1])) {
         if (p[1] == '0') MACRO_FAIL(909);
         addchar(&buf,'$'); addchar(&buf,p[1]);
         p++;
      }
      else if (p[1] == '[') {
         p+=2;
         if (p[1] != ']') MACRO_FAIL(905);
         switch (*p) {
           case '1' : addstring(&buf,"[$1  & $3  & $5 ]"); break;
           case '2' : addstring(&buf,"[$2  & $4  & $6 ]"); break;
           case '3' : addstring(&buf,"[$3  & $5  & $7 ]"); break;
           case '4' : addstring(&buf,"[$4  & $6  & $1']"); break;
           case '5' : addstring(&buf,"[$5, & $7, & $2 ]"); break;
           case '6' : addstring(&buf,"[$6, & $1  & $3 ]"); break;
           case '7' : addstring(&buf,"[$7, & $2  & $4 ]"); break;
           default  : MACRO_FAIL(905);
         }
         p++;
      }
      else if (is_macro_def(p+1)) {
        p++;
        if (chkmacro(&macros,1)) {
          macros.macro_vec[macros.macro_cnt++] = p;
          while (*p && *p != '}') {
            if (*p == '\n') line++;
            if (*p == '\r' &&  p[1] != '\n') line++;
            p++;
          }
        }
      }
      else {
        q = getmacro(&macros, p+1);
        if (q) {
          _dbgmsg("foundmacro: %s\n",q);
          if (stk_top == MAX_STK_TOP) MACRO_FAIL(902); /* too many levels (infinite loop?) */
          stk[stk_top] = skip_identifier(p+1);
          stk_ln[stk_top] = line;
          p = q;
          stk_top++;
        }
        else MACRO_FAIL(904); /* undefined macro */
      }
    }
    else if (*p == '%') { /* strip Comment */
      while (*p && *p != '\r' && *p != '\n') p++;
      if (*p == '\r' && p[1] == '\n') p++;
      addchar(&buf,'\n'); addline(&buf, ++line);
    }
    else  addchar(&buf,*p);

    if (*p) p++;
  }
  addchar(&buf,'\n');
  addline(&buf, ++line);

  #if 0
  fprintf(stderr,"%s",buf.buf);
  dbgmsg("macros: %d\n", macros.macro_cnt);
  {
    int k;
    for (k=0;k<macros.macro_cnt;k++) {
      dbgmsg("%s\n",macros.macro_vec[k]);
    }
  }
  exit(1);
  #endif
  return buf.buf;
}


