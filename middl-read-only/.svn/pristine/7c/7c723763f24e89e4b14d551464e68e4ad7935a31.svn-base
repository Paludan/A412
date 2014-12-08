/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the BSD license:
**   http://creativecommons.org/licenses/BSD/
**
*/

#ifndef UTL_H
#define UTL_H

/* .% Overview
**
** =========== 
** .v
**                           ___   __
**                       ___/  (_ /  )
**                ___  _(__   ___)  /
**               /  / /  )/  /  /  /
**              /  /_/  //  (__/  /
**             (____,__/(_____(__/
** ..
**
**   This file ('|utl.h|) provide the following basic elements:
**
**  .[Unit Testing]   A simple framework to create unit tests. Tests output
**                    is compliant with the TAP '(Test Anything Protocol)
**                    standard.
**
**   [Logging]        To print logging traces during program execution.
**                    It offers multilevel logging similar to '|log4j|
**                    but limited to files.
**
**   [Finite State Machine]
**                    Macros to use FSM as if they were a native C control
**                    structure (similar to switch).
**
**   [Exceptions]     A simple implementation of try/catch. Similar to C++.
**
**   [Guarded memory allocation]
**                    Replacement for malloc(), calloc(), realloc() and free()
**                    that account and report about misuse of memory.
**  ..
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <stdarg.h>
#include <stddef.h>

#include <setjmp.h>
#include <assert.h>

/*
** .% How to use '|utl|
** ====================
**
**  To access '|utl| functions you simply:
**
**  .# #include '|utl.h| in each source file
**   # in one (and only one) of the source files #define the symbol
**     '|UTL_LIB| before including '|utl.h| (a good place is the
**     file where  your '|main()| function is defined)
**  ..
**
**    As an alternative to the second step above, you can create a source
**  file (say '|utl.c|) with only the following lines:
**  .{{ C
**       #define  UTL_LIB
**       #include "utl.h"
**  .}}
**  and link it to your project.
**
*/

#ifdef UTL_C
#ifndef UTL_LIB
#define UTL_LIB
#endif
#endif

/*
**  The '{utl_extern} macro will take care of actually initializing the 
**  variables needed by '|utl.c| instead of simply declaring them as '|extern|
*/

#ifdef UTL_LIB
#define utl_extern(n,v) n v
#else
#define utl_extern(n,v) extern n
#endif

#define utl_initvoid ;

#define UTL_VERSION 0x0003  /* 0.3 */

/* .%% Enable/disable utl features
** -------------------------------
**
*/

#ifdef NDEBUG
#ifdef DEBUG
#undef DEBUG
#endif
#endif

#ifdef UTL_NOMEMCHECK
#ifdef UTL_MEMCHECK
#undef UTL_MEMCHECK
#endif
#endif

#ifdef UTL_MEMCHECK
#ifdef UTL_NOLOGGING
#undef UTL_NOLOGGING
#endif
#endif

#ifdef UTL_UNITTEST
#ifndef DEBUG
#define DEBUG
#endif
#endif


/* .% Constants
** ============
**
**  A set of constants for generic use. Provided by for convenience.
**
**  .[utlEmptyFun]  A pointer to a do-nothing function that can be used
**                  as a generic placeholder (or NULL indicator) for
**                  function pointers. This can be useful as the C standard
**                  doesn't guarantee that one could mix pointers to objects
**                  with pointers to function (even if in reality this is
**                  practically always the case).  
**  ..
*/

int utlEmptyFun(void); 
#ifdef UTL_LIB
int   utlEmptyFun(void) {return 0;}
#endif

/*  .[utlEmptyString]  A pointer to the empty string "" that provides unique
**                     representation for the empty string.  
**  ..
*/

utl_extern(char *utlEmptyString, = "");

/*   .[utlZero]  Is a constant whose value is 0 and that is to be used in
**               idioms like '|do { ... } while (utlZero)|.
**               This is useful to avoid compilers warning about testing
**               constant values.
**   ..
*/

  
#ifdef __GNUC__
#define utlZero 0
#else
utl_extern(const int utlZero, = 0);
#endif

/*  MS Visual C doesn't have '|snprintf()| ! How could it be?
*/
#ifdef _MSC_VER
#define snprintf _snprintf
#endif


/* .% Assumptions (static assertions)
** ~~~~~~~~~~~~~~~~~~~~~~~
** http://www.drdobbs.com/compile-time-assertions/184401873
*/

#define utl_assum1(e,l) typedef struct {int utl_assumption[(e)?1:-1];} utl_assumption_##l
#define utl_assum0(e,l) utl_assum1(e,l)
#define utlAssume(e)    utl_assum0(e,__LINE__)


/* .% Try/Catch
** ~~~~~~~~~~~~~~~~~~~~~~~
**   Exceptions can be very useful when dealing with error conditions is so
** complicate that the logic of errors handling obscures the logic of the 
** program itself.
**   If an error happens in the '|try| section, you can '|throw()| an exception
** (an error condition) and the control goes back to the '|catch| section
** where the proper actions can be taken.
**
**   Simple implementation of try/catch.
** .{{ c
**   tryenv env = NULL;
**   try(env) {
**      ... code ...
**      if (something_failed) throw(env, ERR_OUTOFMEM)  // must be > 0 
**      some_other_func(env); // you can trhow exceptions from other functions 
**      ... code ...          // as long as you pass it the try environment.
**   }  
**   catch({                  // NOTE the catch part is enclosed in special 
*       case ERR_OUTOFMEM :   // braces: '|({ ... })| and is the body of a
**      ... code ...          // '|switch()| statement (including fallthrough!)
**   });
** .}}
**
**  This comes useful when you throw an exception form a called function.
**  The example below, handles the "out of mem" condition in the same place
**  regardless of where the exception was raised.
**
** .{{ C
**
**   #define ERR_OUTOFMEM 0xF0CA
**   tryenv env = NULL; // Remember to set to NULL initally!
**   char *f1(tryenv ee, int x)   { ... throw(ee, ERR_OUTOFMEM} ... }
**   void *f2(tryenv ee, char *x) { ... throw(ee, ERR_OUTOFMEM} ... }
**   try(env) {
**      ... code ...
**      f1(env,3); 
**      ... code ...
**      if ... throw(env,ERR_OUTFOMEM) 
**      ... code ...
**      f2(env,"Hello"); 
**      ... code ...
**   }  
**   catch({                    // Note the use of '|({| and '|})| !!
**      case  ERR_OUTOFMEM : 
**                 ... code ... // Handle all your cleanup here!
**                 break;
**   });
**
** .}}
*/ 

#ifndef UTL_NOTRYCATCH

typedef struct utl_env_s { 
  jmp_buf jb;
  struct utl_env_s volatile *prev;
  struct utl_env_s volatile **orig;
} *tryenv; 

#define try(utl_env) \
            do { struct utl_env_s utl_cur_env; volatile int utl_err; \
                 utl_cur_env.prev = (void *)utl_env; \
                 utl_cur_env.orig = (void *)(&utl_env); \
                 utl_env = &utl_cur_env; \
                 if ((utl_err = setjmp(utl_cur_env.jb))==0)
                 
#define catch(y) if (utl_err) switch (utl_err) { \
                      y \
                   } \
                   utl_err = 0;\
                  *utl_cur_env.orig = utl_cur_env.prev; \
                } while(0)

#define throw(env,err) (env? longjmp(((tryenv)env)->jb, err): exit(err))
#define rethrow        (*utl_cur_env.orig = utl_cur_env.prev,throw(*utl_cur_env.orig,utl_err))

#endif

#ifndef UTL_NOFSM
/*  .% Finite state machine
**  =======================
**
**    A Finite State Machine (FSM) is very common paradigm for software that respond to 
**  external events.  There are many ways to code a FSM, this one has the advantage to 
**  closely mimic the graphical representation of a FSM.
**
** .v
**      fsm ({            // Note the use of '|({| and '|})| !!
**
**        case : { ...
**                   if (c == 0) fsmGoto(z);
**                   fsmGoto(y);
**        }
**
**        case z : { ...
**                   break;  // exit from the FSM
**        }
**
**        case y : { ...
**                   if (c == 1) fsmGoto(x);
**                   fsmGoto(z);
**        }
**      });
** ..
**
**   It's a good practice to include a drawing of the FSM in the technical
** documentation (e.g including the GraphViz description in comments).
*/

#define fsmSTART -1
#define fsmEND   -2

#define fsm(x)  do { int utl_fsm_next , utl_fsm_state; \
                      for (utl_fsm_next=fsmSTART; utl_fsm_next>fsmEND;) \
                        switch((utl_fsm_state=utl_fsm_next, utl_fsm_next=fsmEND, utl_fsm_state)) { \
                        x \
                }} while (utlZero)
                         
#define fsmGoto(x)  utl_fsm_next = (x); break
#define fsmRestart  utl_fsm_next = fsmSTART; break
#define fsmExit     utl_fsm_next = fsmEND; break

#endif

/* .% UnitTest
** ===========
**
**   These macros will help you writing unit tests.  The log produced
** is '<TAP 1.3=http://testanything.org> compatible and also contains
** all the information about passing/failing.
**
**   They are available only if the symbol '|UTL_UNITTEST| is defined before
** including the '|utl.h| header.
** 
**   '{UTL_UNITTEST} implies '{DEBUG}
*/

#ifdef UTL_UNITTEST

utl_extern(FILE *TST_FILE, = NULL);
#define TSTFILE (TST_FILE?TST_FILE:stderr)

/* Output is flushed every time to avoid we lose a message in case of
** abnormal exit. 
*/
#define TSTWRITE(...) (fprintf(TSTFILE,__VA_ARGS__),fflush(TSTFILE))

#define TSTTITLE(s) TSTWRITE("TAP version 13\n#\n# ** %s - (%s)\n",s,__FILE__)

#define TST_INIT0 (TSTRES=TSTNUM=TSTGRP=TSTSEC=TSTTOT= \
                               TSTGTT=TSTGPAS=TSTPASS=TSTNSK=TSTNTD=0)
                                 
#define TSTPLAN(s) for (TSTPASSED = TST_INIT0 + 1, TSTTITLE(s); \
                                             TSTPASSED; TSTDONE(),TSTPASSED=0)

/* Tests are divided in sections introduced by '{=TSTSECTION(title)} macro.
** The macro reset the appropriate counters and prints the section header 
*/
#define TSTSECTION(s)  if ((TSTSTAT(), TSTGRP = 0, TSTSEC++, \
                             TSTWRITE("#\n# * %d. %s (%s:%d)\n", \
                             TSTSEC, s, __FILE__, __LINE__), TSTPASS=0)) ((void)0); \
                       else                                               

/* to disable an entire test section, just prepend '|_| or '|X|*/
 
#define XTSTSECTION(s) if (!utlZero) ((void)0); else 
#define _TSTSECTION(s) XTSTSECTION(s)

/* In each section, tests can be logically grouped so that different aspects
** of related functions can be tested.
*/
#define TSTGROUP(s) \
    if ((TSTWRITE("#\n# *   %d.%d %s\n",TSTSEC,++TSTGRP,s),TSTNUM=0)) ((void)0); else
                     
/* to disable a n entire test group , just prepend '|_| or '|X| */
#define XTSTGROUP(s) if (!utlZero) ((void)0); else  
#define _TSTGROUP(s) XTSTGROUP(s)

/* Test code will be skipped if needed */
#define TSTCODE   if (TSTSKP)   ((void)0); else  
#define XTSTCODE  if (!utlZero) ((void)0); else
#define _TSTCODE  XTSTCODE
                     
/* The single test is defined  with the '|TST(s,x)| macro.
**   .['|s|] is a short string that identifies the test
**    ['|x|] an assertion that has to be true for the test to succeed.
**   ..
*/
#define XTST(s,x)

#define TST(s,x) (TST_DO(s,(TSTSKP?1:(x))),TST_WRDIR, TSTWRITE("\n"), TSTRES)

#define TST_DO(s,x) (TSTRES = (x), TSTGTT++, TSTTOT++, TSTNUM++, \
                     TSTPASSED = (TSTPASSED && (TSTRES || TSTTD)), \
                     TSTWRITE("%s %4d - %s (:%d)", \
                              (TSTRES? (TSTGPAS++,TSTPASS++,TSTOK) : TSTKO), \
                               TSTGTT, s, __LINE__))

#define TST_WRDIR \
           (TSTSKP ? (TSTNSK++, TSTWRITE(" # SKIP %s",TSTSKP)) \
                   : (TSTTD ? (TSTNTD++, (TSTWRITE(" # TODO %s%s",TSTTD, \
                                        (TSTRES?TSTWRN:utlEmptyString)))) : 0))

#define TSTFAILED  (!TSTRES)

/* You can skip a set of tests giving a reason.
** Nested skips are not supported!
*/
#define TSTSKIP(x,r) if (!(x)) ((void)0); else for (TSTSKP=r; TSTSKP; TSTSKP=NULL)

#define TSTTODO(r)   for (TSTTD=r; TSTTD; TSTTD=NULL)

#define TSTNOTE(...) \
                 (TSTWRITE("#      "),TSTWRITE(__VA_ARGS__), TSTWRITE("\n"))
                                            
#define TSTFAILNOTE(...) (TSTRES? 0 : (TSTNOTE(__VA_ARGS__)))

#define TSTEXPECTED(f1,v1,f2,v2) \
                             (TSTRES? 0 : (TSTNOTE("Expected "f1" got "f2,v1,v2)))

#define TSTEQINT(s,e,r) do { int __exp = (e); int __ret = (r);\
                             TST(s,__exp==__ret);\
                             TSTEXPECTED("(int) %d",__exp,"%d",__ret); \
                           } while (utlZero)

#define TSTNEQINT(s,e,r) do { int __exp = (e); int __ret = (r);\
                             TST(s,__exp!=__ret);\
                             TSTEXPECTED("(int) other than %d",__exp,"%d",__ret); \
                           } while (utlZero)
                           
#define TSTEQPTR(s,e,r)  do { void *__exp = (e); void *__ret = (r); \
                              TST(s,__exp == __ret) ; \
                              TSTEXPECTED("(ptr) 0x%p",__exp,"0x%p",__ret); \
                            }  while (utlZero)

#define TSTNEQPTR(s,e,r) do { void *__exp = (e); void *__ret = (r); \
                              TST(s,__exp != __ret) ; \
                              TSTEXPECTED("(ptr) other than 0x%p",__exp,"0x%p",__ret); \
                            }  while (utlZero)

#define TSTNULL(s,r)     TSTEQPTR(s,NULL,r)

#define TSTNNULL(s,r)    TSTNEQPTR(s,NULL,r)

							
#define TSTBAILOUT(r) \
          if (!(r)) ((void)0); else {TSTWRITE("Bail out! %s\n",r); TSTDONE(); exit(1);}

/* At the end of a section, the accumulated stats can be printed out */
#define TSTSTAT() \
          (TSTTOT == 0 ? 0 : ( \
           TSTWRITE("#\n# SECTION %d OK: %d/%d\n",TSTSEC,TSTPASS,TSTTOT), \
           TSTTOT = 0))

/* At the end of all the tests, the accumulated stats can be printed out */
#define TSTDONE() \
  (TSTGTT <= 0 ? 0 : ( TSTSTAT(),  \
  TSTWRITE("#\n# TOTAL OK: %d/%d SKIP: %d TODO: %d\n",TSTGPAS,TSTGTT, \
                                                              TSTNSK,TSTNTD), \
  TSTWRITE("#\n# TEST PLAN: %s \n",TSTPASSED ? "PASSED" : "FAILED"), \
  TSTWRITE("#\n1..%d\n",TSTGTT),fflush(TSTFILE)) )

/* Execute a statement if a test succeeded */
#define TSTIF_OK  if (TSTRES)

/* Execute a statement if a test failed */
#define TSTIF_NOTOK if (!TSTRES)

static int    TSTRES    = 0;  /* Result of the last performed '|TST()| */
static int    TSTNUM    = 0;  /* Last test number */
static int    TSTGRP    = 0;  /* Current test group */
static int    TSTSEC    = 0;  /* Current test SECTION*/
static int    TSTTOT    = 0;  /* Number of tests executed */
static int    TSTGTT    = 0;  /* Number of tests executed (Grand Total) */
static int    TSTGPAS   = 0;  /* Number of tests passed (Grand Total) */
static int    TSTPASS   = 0;  
static int    TSTPASSED = 1;  
static int    TSTNSK    = 0;  
static int    TSTNTD    = 0;  
static char  *TSTSKP    = NULL;
static char  *TSTTD     = NULL;

static const char *TSTOK  = "ok    ";
static const char *TSTKO  = "not ok";
static const char *TSTWRN = " (passed unexpectedly!)";


#endif /* UTL_UNITTEST */

/* .% Logging
** ==========
**
*/


#define log_D 7
#define log_I 6
#define log_M 5
#define log_W 4
#define log_E 3
#define log_C 2
#define log_A 1
#define log_F 0


#define log_X (log_D + 1)
#define log_L (log_D + 2)


/* Logging functions are available unless the symbol '{=UTL_NOLOGGING}
** has been defined before including '|utl.h|.
*/

#ifndef UTL_NOLOGGING

#define UTL_LOG_NEW 0x00    
#define UTL_LOG_ADD 0x01    /* append to existing file */
#define UTL_LOG_ERR 0x02    /* use stderr */
#define UTL_LOG_OUT 0x04    /* use stdout */

typedef struct {
  FILE          *file;
  unsigned char  level;
  unsigned char  flags;
  unsigned short rot;
} utl_log_s, *utlLogger;

#define utl_log_stdout_init {NULL, log_W, UTL_LOG_OUT,0}
utl_extern(utl_log_s utl_log_stdout , = utl_log_stdout_init);
#define logStdout (&utl_log_stdout)

#define utl_log_stderr_init {NULL, log_W, UTL_LOG_ERR,0}
utl_extern(utl_log_s utl_log_stderr , = utl_log_stderr_init);
#define logStderr (&utl_log_stderr)

#define logNull NULL

utl_extern(utlLogger utl_logger , = logNull);

#include <time.h>
#include <ctype.h>

/* .%% Logging levels
** ~~~~~~~~~~~~~~~~~~
**
**   Logging levels are hierarchical and structured. Default log level is WARN.
**
**   Use '{=logLevel()}    to set the desired level of logging.
**   Use '{=logLevelEnv()} to set the desired level of logging based on an
**                         enviroment variable.
*/

                                    /* 0   1   2   3   4   5   6   7   8   9    */
                                    /* 0   4   8   12  16  20  24  28  32  36   */
utl_extern(char const utl_log_abbrev[], = "FTL ALT CRT ERR WRN MSG INF DBG OFF LOG ");

int   utl_log_level(utlLogger lg);
int   utl_log_chrlevel(char *l);
int   logLevel(utlLogger lg, char *lv); 
int   logLevelEnv(utlLogger lg, char *var, char *level);

/*
** The table below shows whether a message of a certain level will be
** printed (Y) or not (N) given the current level of logging.
** .v
**                          message level 
**                    DBG INF MSG WRN ERR CRT ALT FTL
**               DBG   Y   Y   Y   Y   Y   Y   Y   Y
**               INF   N   Y   Y   Y   Y   Y   Y   Y
**               MSG   N   N   Y   Y   Y   Y   Y   Y
**      current  WRN   N   N   N   Y   Y   Y   Y   Y
**      logging  ERR   N   N   N   N   Y   Y   Y   Y
**       level   CRT   N   N   N   N   N   Y   Y   Y
**               ALT   N   N   N   N   N   N   Y   Y
**               FTL   N   N   N   N   N   N   N   Y
**               OFF   N   N   N   N   N   N   N   N
** ..
*/

/* .%% Logging file rotate
** ~~~~~~~~~~~~~~~~~~~~~~~
**
** For long running programs (servers, daemons, ...) it is important to rotate 
** the log files from time to time so that they won't become too big.
** The function logRotateOn() will check the current size of the log file and 
** if it's bigger than the threshold specified, will close it and open a new
** one with the same name and numbering it.
** Then new file will be renamed _1, _2, etc.
**
**   logRotate(lg,n)
**
*/



/* .%% Logging format
** ~~~~~~~~~~~~~~~~~~
** 
** Log files have the following format:
** .v
**     <date> <time> <level> <message>
** ..
**
**  For example:
** .v
**     2009-01-29 13:46:02 ERR An error!
**     2009-01-29 13:46:02 FTL An unrecoverable error
** ..
**
*/

/*    Log files can be opened in "write" or "append" mode as any normal file 
** using the '{=logOpen()} function.
** For example:
** .v  
**   utlLogger lgr = NULL;
**   logOpen(lgr,"file1.log","w") // Delete old log file and create a new one
**   ...
**   logOpen(lgr,"file1.log","a") // Append to previous log file
** .. 
**
**   There are three predefined loggers:
**   .[{logNull}]    A null logger that won't output any message
**    [{logStdout}]  A logger that will output on stdout
**    [{logStderr}]  A logger that will output on stderr
**   ..
** They are '{logClose()} safe, i.e. you can pass them to logClose() and nothing
** bad will happen.
*/

#define logOpen(l,f,m)   (l=utl_log_open(f,m))
#define logClose(l)      (utl_log_close(l),l=NULL)

utlLogger utl_log_open(char *fname, char *mode);
utlLogger utl_log_close(utlLogger lg);
void utl_log_write(utlLogger lg,int lv, int tstamp, char *format, ...);

#define logFile(l) utl_logFile(l)
#define logLevel(lg,lv)      utl_logLevel(lg,lv)
#define logLevelEnv(lg,v,l)  utl_logLevelEnv(lg,v,l)

#define logIf(lg,lc) utl_log_if(lg,utl_log_chrlevel(lc))

#define utl_log_if(lg,lv) if ((lv) > utl_log_level(lg)) ((void)0) ; else
          
#define logDebug(lg, ...)      utl_log_write(lg, log_D, 1, __VA_ARGS__)
#define logInfo(lg, ...)       utl_log_write(lg, log_I, 1, __VA_ARGS__)
#define logMessage(lg, ...)    utl_log_write(lg, log_M, 1, __VA_ARGS__)
#define logWarn(lg, ...)       utl_log_write(lg, log_W, 1, __VA_ARGS__)
#define logError(lg, ...)      utl_log_write(lg, log_E, 1, __VA_ARGS__)
#define logCritical(lg, ...)   utl_log_write(lg, log_C, 1, __VA_ARGS__)
#define logAlarm(lg, ...)      utl_log_write(lg, log_A, 1, __VA_ARGS__)
#define logFatal(lg, ...)      utl_log_write(lg, log_F, 1, __VA_ARGS__)

#define logDContinue(lg, ...)  utl_log_write(lg, log_D, 0, __VA_ARGS__)
#define logIContinue(lg, ...)  utl_log_write(lg, log_I, 0, __VA_ARGS__)
#define logMContinue(lg, ...)  utl_log_write(lg, log_M, 0, __VA_ARGS__)
#define logWContinue(lg, ...)  utl_log_write(lg, log_W, 0, __VA_ARGS__)
#define logEContinue(lg, ...)  utl_log_write(lg, log_E, 0, __VA_ARGS__)
#define logCContinue(lg, ...)  utl_log_write(lg, log_C, 0, __VA_ARGS__)
#define logAContinue(lg, ...)  utl_log_write(lg, log_A, 0, __VA_ARGS__)
#define logFContinue(lg, ...)  utl_log_write(lg, log_F, 0, __VA_ARGS__)

#define logAssert(lg,e)        utl_log_assert(lg, e, #e, __FILE__, __LINE__)

/*
** .v
**   logError("Too many items at counter %d (%d)",numcounter,numitems);
**   logContinue("Occured %d times",times++);
** ..
** will produce:
** .v
**     2009-01-29 13:46:02 ERR Too many items at counter 9 (5)
**                             Occured 3 times
** ..
*/

#ifdef UTL_LIB
int   utl_log_level(utlLogger lg) { return (int)(lg ? lg->level : log_X) ; }

FILE *utl_logFile(utlLogger lg)
{
  if (!lg) return NULL;
  if (lg->flags & UTL_LOG_ERR) return stderr;
  if (lg->flags & UTL_LOG_OUT) return stdout;
  return lg->file;
}

int   utl_log_chrlevel(char *l) {
  int i=0;
  char c = l ? toupper(l[0]) : 'W';
  
  while (utl_log_abbrev[i] != ' ' && utl_log_abbrev[i] != c) i+=4;
  i = (i <= 4*7) ? (i>> 2) : log_W;
  return i;
}


int utl_logLevel(utlLogger lg, char *lv) 
{
  if (!lg) return log_X;
  
  if (lv && lv[0] && lv[0] != '?')
      lg->level = utl_log_chrlevel(lv);
  return utl_log_level(lg);  
}

int utl_logLevelEnv(utlLogger lg, char *var, char *level)
{
  char *lvl_str;
  
  lvl_str=getenv(var);
  if (!lvl_str) lvl_str = level;
  return utl_logLevel(lg,lvl_str);
}

utlLogger utl_log_open(char *fname, char *mode)
{
  char md[4];
  utlLogger lg = logNull;
  FILE *f = NULL;
  
  if (fname) {
    md[0] = mode[0]; md[1] = '+'; md[2] = '\0';
    if (md[0] != 'a' && md[0] != 'w') md[0] = 'a'; 
    f = fopen(fname,md);
  }
  
  if (f) {
    lg = malloc(sizeof(utl_log_s));
    if (lg) { 
      lg->flags = 0;
      lg->rot = 0;
      lg->file = f;
	  {/* Assume that log_L is the last level in utl_log_abbrev */
	    utlAssume( (log_L +1) == ((sizeof(utl_log_abbrev)-1)>>2));
        lg->level = log_L;
        utl_log_write(lg,log_L, 1, "%s \"%s\"", (md[0] == 'a') ? "ADDEDTO" : "CREATED",fname); 
	  }
      lg->level = log_W;
	}
  }
  if (f && !lg) fclose(f);
  return lg;
}

utlLogger utl_log_close(utlLogger lg)
{
  if (lg && lg != logStdout && lg != logStderr) {
    if (lg->file) fclose(lg->file);
    lg->file = NULL;
    free(lg);
  }
  return NULL;
}

/* 
** if limit reached, close the log and open a new one incrementing
** the name.
**      mylog.log
**      mylog_001.log
**      mylog_002.log
**       etc...
*/
static void utl_log_rotate(utlLogger lg)
{
  // TODO:
}

void utl_log_write(utlLogger lg, int lv, int tstamp, char *format, ...)
{
  va_list args;
  char tstr[32];
  time_t t;
  FILE *f = stderr;
  int lg_lv = log_W;
  
  if (!lg) return; 
  
  if (lg->flags & UTL_LOG_OUT) f = stdout;
  else if (lg->flags & UTL_LOG_ERR) f = stderr;
  else f = lg->file;
  
  lg_lv = lg->level;
  lv = lv & 0x0F;
  if( lv <= lg_lv) {
    if (tstamp) {
      time(&t);
      strftime(tstr,32,"%Y-%m-%d %X",localtime(&t));
	} else {
	  strcmp(tstr,"                   ");          
	}
    fprintf(f, "%s %.4s", tstr, utl_log_abbrev+(lv<<2));
    va_start(args, format);  vfprintf(f,format, args);  va_end(args);
    fputc('\n',f);
    fflush(f);
    if (lg->rot >0) utl_log_rotate(lg);
  }    
}

void utl_log_assert(utlLogger lg,int e,char *estr, char *file,int line)
{ 
  if (!e) {
    logFatal(lg,"Assertion failed:  %s, file %s, line %d", estr, file, line);
#ifndef NDEBUG
	abort();
#endif
  }
}
							   
#endif  /*- UTL_LIB */

#else   /*- UTL_NOLOGGING */

#define logLevel(lg,lv)       log_W
#define logLevelEnv(lg,v,l)   log_W     
#define logDebug(lg, ...)     ((void)0)
#define logInfo(lg, ...)      ((void)0)
#define logMessage(lg, ...)   ((void)0)
#define logWarn(lg, ...)      ((void)0)
#define logError(lg, ...)     ((void)0)
#define logCritical(lg, ...)  ((void)0)
#define logAlarm(lg, ...)     ((void)0)
#define logFatal(lg, ...)     ((void)0)

#define logDContinue(lg, ...) ((void)0)
#define logIContinue(lg, ...) ((void)0)
#define logMContinue(lg, ...) ((void)0)
#define logWContinue(lg, ...) ((void)0)
#define logEContinue(lg, ...) ((void)0)
#define logCContinue(lg, ...) ((void)0)
#define logAContinue(lg, ...) ((void)0)
#define logFContinue(lg, ...) ((void)0)

#define logAssert(lg,e)       ((void)0)

#define logIf(lg,lv) if (!utlZero) (void)0 ; else

#define logOpen(lg,f,m) (lg=NULL)
#define logClose(lg)    (lg=NULL)

typedef void *utlLogger;

#define logFile(x) NULL
#define logStdout  NULL
#define logStderr  NULL

#endif /*- UTL_NOLOGGING */

#ifdef NDEBUG
#undef logDebug
#define logDebug(lg,...) ((void)0)
#endif  /*- NDEBUG */

#define logNDebug(lg,...) ((void)0)

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*  .% Traced memory check
**  ======================
*/
#define utlMemInvalid    -2
#define utlMemOverflow   -1
#define utlMemValid       0
#define utlMemNull        1

#ifdef UTL_MEMCHECK
void *utl_malloc  (size_t size, char *file, int line );
void *utl_calloc  (size_t num, size_t size, char *file, int line);
void *utl_realloc (void *ptr, size_t size, char *file, int line);
void  utl_free    (void *ptr, char *file, int line );
void *utl_strdup  (void *ptr, char *file, int line);

int utl_check(void *ptr,char *file, int line);

utl_extern(utlLogger utlMemLog , = &utl_log_stderr);

#ifdef UTL_LIB
/*************************************/

static char *BEG_CHK = "\xBE\xEF\xF0\x0D";
static char *END_CHK = "\xDE\xAD\xC0\xDA";
static char *CLR_CHK = "\xDE\xFA\xCE\xD0";

static size_t utl_mem_allocated = 0;

typedef struct {
   size_t size;
   char   chk[4];
   char   data[4];
} utl_mem_t;

#define utl_mem(x) ((utl_mem_t *)((char *)(x) -  offsetof(utl_mem_t, data)))

int utl_check(void *ptr,char *file, int line)
{
  utl_mem_t *p;
  
  if (ptr == NULL) return utlMemNull;
  p = utl_mem(ptr);
  if (memcmp(p->chk,BEG_CHK,4)) { 
    logError(utlMemLog,"Invalid or double freed %p (%u %s %d)",p->data, \
                                               utl_mem_allocated, file, line);     
    return utlMemInvalid; 
  }
  if (memcmp(p->data+p->size,END_CHK,4)) {
    logError(utlMemLog,"Boundary overflow detected %p [%d] (%u %s %d)", \
                              p->data, p->size, utl_mem_allocated, file, line); 
    return utlMemOverflow;
  }
  logInfo(utlMemLog,"Valid pointer %p (%u %s %d)",ptr, utl_mem_allocated, file, line); 
  return utlMemValid; 
}

void *utl_malloc(size_t size, char *file, int line )
{
  utl_mem_t *p;
  
  if (size == 0) logWarn(utlMemLog,"Shouldn't allocate 0 bytes (%u %s %d)", \
                                                utl_mem_allocated, file, line);
  p = malloc(sizeof(utl_mem_t) +size);
  if (p == NULL) {
    logCritical(utlMemLog,"Out of Memory (%u %s %d)",utl_mem_allocated, file, line);
    return NULL;
  }
  p->size = size;
  memcpy(p->chk,BEG_CHK,4);
  memcpy(p->data+p->size,END_CHK,4);
  utl_mem_allocated += size;
  logInfo(utlMemLog,"alloc %p [%d] (%u %s %d)",p->data,size,utl_mem_allocated,file,line);
  return p->data;
};

void *utl_calloc(size_t num, size_t size, char *file, int line)
{
  void *ptr;
  
  size = num * size;
  ptr = utl_malloc(size,file,line);
  if (ptr)  memset(ptr,0x00,size);
  return ptr;
};

void utl_free(void *ptr, char *file, int line)
{
  utl_mem_t *p=NULL;
  
  switch (utl_check(ptr,file,line)) {
    case utlMemNull  :    logWarn(utlMemLog,"free NULL (%u %s %d)", 
                                                utl_mem_allocated, file, line);
                          break;
                          
    case utlMemOverflow : logWarn(utlMemLog, "Freeing an overflown block  (%u %s %d)", 
                                                           utl_mem_allocated, file, line);
    case utlMemValid :    p = utl_mem(ptr); 
                          memcpy(p->chk,CLR_CHK,4);
                          utl_mem_allocated -= p->size;
                          if (p->size == 0)
                            logWarn(utlMemLog,"Freeing a block of 0 bytes (%u %s %d)", 
                                                utl_mem_allocated, file, line);

                          logInfo(utlMemLog,"free %p [%d] (%u %s %d)", ptr, 
                                    p?p->size:0,utl_mem_allocated, file, line);
                          free(p);
                          break;
                          
    case utlMemInvalid :  logError(utlMemLog,"free an invalid pointer! (%u %s %d)", \
                                                utl_mem_allocated, file, line);
                          break;
  }
}

void *utl_realloc(void *ptr, size_t size, char *file, int line)
{
  utl_mem_t *p;
  
  if (size == 0) {
    logWarn(utlMemLog,"realloc() used as free() %p -> [0] (%u %s %d)",ptr,utl_mem_allocated, file, line);
    utl_free(ptr,file,line); 
  } 
  else {
    switch (utl_check(ptr,file,line)) {
      case utlMemNull   : logWarn(utlMemLog,"realloc() used as malloc() (%u %s %d)", \
                                             utl_mem_allocated, file, line);
                          return utl_malloc(size,file,line);
                        
      case utlMemValid  : p = utl_mem(ptr); 
                          p = realloc(p,sizeof(utl_mem_t) + size); 
                          if (p == NULL) {
                            logCritical(utlMemLog,"Out of Memory (%u %s %d)", \
                                             utl_mem_allocated, file, line);
                            return NULL;
                          }
                          utl_mem_allocated -= p->size;
                          utl_mem_allocated += size; 
                          logInfo(utlMemLog,"realloc %p [%d] -> %p [%d] (%u %s %d)", \
                                          ptr, p->size, p->data, size, \
                                          utl_mem_allocated, file, line);
                          p->size = size;
                          memcpy(p->chk,BEG_CHK,4);
                          memcpy(p->data+p->size,END_CHK,4);
                          ptr = p->data;
                          break;
    }
  }
  return ptr;
}

void *utl_strdup(void *ptr, char *file, int line)
{
  char *dest;
  size_t size;
	
  if (ptr == NULL) {
    logWarn(utlMemLog,"strdup NULL (%u %s %d)", utl_mem_allocated, file, line);
    return NULL;
  }
  size = strlen(ptr)+1;

  dest = utl_malloc(size,file,line);
  if (dest) memcpy(dest,ptr,size);
  logInfo(utlMemLog,"strdup %p [%d] -> %p (%u %s %d)", ptr, size, dest, \
                                                utl_mem_allocated, file, line);
  return dest;
}
#undef utl_mem

/*************************************/
#endif

#define malloc(n)     utl_malloc(n,__FILE__,__LINE__)
#define calloc(n,s)   utl_calloc(n,s,__FILE__,__LINE__)
#define realloc(p,n)  utl_realloc(p,n,__FILE__,__LINE__)
#define free(p)       utl_free(p,__FILE__,__LINE__)
#define strdup(p)     utl_strdup(p,__FILE__,__LINE__)

#define utlMemCheck(p)    utl_check(p,__FILE__, __LINE__)
#define utlMemAllocated   utl_mem_allocated
#define utlMemValidate(p) utl_mem_validate(p)

#define utlMalloc(n)     utl_malloc(n,__FILE__,__LINE__)
#define utlCalloc(n,s)   utl_calloc(n,s,__FILE__,__LINE__)
#define utlRealloc(p,n)  utl_realloc(p,n,__FILE__,__LINE__)
#define utlFree(p)       utl_free(p,__FILE__,__LINE__)
#define utlStrdup(p)     utl_strdup(p,__FILE__,__LINE__)

#else /* UTL_MEMCHECK */

#define utlMemCheck(p) utlMemValid
#define utlMemAllocated 0
#define utlMemValidate(p) (p)

#endif /* UTL_MEMCHECK */

/* .% Variable length strings
** ~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
#ifndef UTL_NOCHS
 
#define chs_blk_inc 16

typedef struct {
  long size;
  long len;
  char chs[chs_blk_inc];
} chs_blk_t;

typedef char *chs_t;

chs_t utl_chs_setsize(chs_t s, long ndx);
#define chsNew(s) (s = utl_chs_setsize(NULL,0)) 

chs_t utl_chs_free(chs_t s);
#define chsFree(s) (s=utl_chs_free(s))

#define chsLen(s)  utl_chsLen(s)
#define chsSize(s) utl_chsSize(s)

long utl_chsLen(chs_t s);
long utl_chsSize(chs_t s);

#define chsChrAt(s,n) utl_chsChrAt(s,n)
char   utl_chsChrAt  (chs_t s, long ndx);

chs_t utl_chs_set(chs_t s, long ndx, uint32_t c);
#define chsSetChr(s, n, c) (s = utl_chs_set(s,n,c))

#ifdef UTL_LIB

#define chs_blk(s) ((chs_blk_t *)(((char*)(s)) - offsetof(chs_blk_t,chs)))
#define chs_chs(b) ((char *)(((char *)b)+ offsetof(chs_blk_t,chs)))

chs_t utl_chs_setsize(chs_t s, long ndx)
{
  long sz = 0;
  chs_blk_t *cb = NULL;
  
  if (s) cb = chs_blk(s);
  
  if (cb) sz = cb->size;
  
  if (ndx < sz) return s; /* enough room already */
 
  sz = (ndx / chs_blk_inc) * chs_blk_inc; /* round to the next block size */
  cb = realloc(cb, sizeof(chs_blk_t) + sz);
  
  if (!cb) {logDebug(utl_logger,"realloc() failed"); return NULL;}

  sz += chs_blk_inc;   /* chs_blk_inc are in the chs_blk_t struct already */
  
  cb->size = sz;

  if (!s) {  /* created a fresh string */
    cb->len    = 0;
    cb->chs[0] = '\0';
  }
  return cb->chs;  
}

chs_t utl_chs_free(chs_t s) { 
  logDebug(utl_logger,"FREE: %p",s);
  if (s) free(chs_blk(s));
  return NULL;
}

long utl_chsLen(chs_t s)
{
  chs_blk_t *cb;
  long l;
  cb = chs_blk(s);
  l = (s? cb->len  : 0);
  logDebug(utl_logger,"CHSLEN: %p %ld",s,l);
  return l;
}

static long fixndx(chs_t s, long n)
{
  logDebug(utl_logger,"fixndx: %p  %d -> ",s,n);
  if (s) {
    if (n < 0) n += utl_chsLen(s);
    if (n > utl_chsLen(s)) n = utl_chsLen(s)-1;
  }
  if (n < 0) n = 0;
  logDContinue(utl_logger,"           %d",n);
  
  return n;
}

chs_t utl_chs_set(chs_t s, long ndx, uint32_t c)
{
  chs_blk_t *cb;
  
  if (ndx < 0) ndx = fixndx(s,ndx);
  s = utl_chs_setsize(s,ndx+8); /* ensure enough space for an UTF char */

  s[ndx] = c;
  cb = chs_blk(s);
  
  if (c == '\0') 
    cb->len = ndx;
  else if (ndx >= cb->len)
    cb->len = ndx+1;
    
  s[ndx+1] = '\0';
  logDebug(utl_logger,"chs_Set: [%d] = %d",ndx,c);
  return s;
}

char utl_chsChrAt(chs_t s, long ndx)
{
  ndx = fixndx(s,ndx);
  return (s && ndx < utl_chsLen(s)) ? s[ndx] : '\0';
}

#endif  /*- UTL_LIB */

#define chslen     chsLen            
                                     
#define chscpy     chsCpy            
#define chsncpy    chsCpyL           
#define chscat     chsAddStr         
#define chsins     chsInsStr         
#define chsncat    chsAddStrL        
#define chsnins    chsInsStrL        
                                     
#define chssetchr  chsChrSet         
#define chscatchr  chsAddChar        
#define chsinschr  chsInsChar        

#define chsprintf chsPrintf
                                     
#define chsdel     chsDel            
#define chstrim    chsTrim           
                                     
#define chsupper   chsUpper          
#define chslower   chsLower          
#define chsnupper  chsUpper          
#define chsnlower  chsLower          
#define chsreverse chsReverse        
                                     
#define chsgetline chsCpyLine        
#define chsgetfile chsCpyFile        
#define chscatline chsAddLine        
#define chscatfile chsAddFile        
#define chslines   chsForLines       
                                     
#define chsmatch   chsMatch          
#define chssub     chsSubStr         
#define chssubarr  chsSubArr         
#define chssubfun  chsSubfun         
#define chssubfun_t chsSubF_t
                                             
#endif /*- UTL_NOCHS */

#endif /* UTL_H */

/**************************************************************************/

