/*
**  (C) 2008 by Remo Dentato (rdentato@users.sourceforge.net)
**
** Permission to use, copy, modify and distribute this code and
** its documentation for any purpose is hereby granted without
** fmakee, provided that the above copyright notice, or equivalent
** attribution acknowledgement, appears in all copies and
** supporting documentation.
**
** Copyright holder makes no representations about the suitability
** of this software for any purpose. It is provided "as is" without
** express or implied warranty.
*/

/*
* Create a file with all the 128 notes played by a piano
*
*/

#define UTL_UNITTEST

#include "msq.h"

int wrt_allnotes(int trknum)
{
  int k =0;
  mf_program_change(0,1,in_acousticgrand);
  for (k = 0; k<128; k++) {
    mf_note(mf_curtick, 1, d_quarter, k, 100);
  }
  return 480;
}

int myerror(int err, char *msg)
{
  fprintf(stderr,msg); fflush(stderr);
  return err;
}

mf_handlers hh;

int main(int argc, char *argv[])
{
  int status;

  hh[hndl_writetrack] = (mf_hndl) wrt_allnotes;
  hh[hndl_error] =(mf_hndl) myerror;
  mf_sethandlers(hh);
  status = mf_write("test_allnotes.mid",0,1,120);

  return (0);
}
