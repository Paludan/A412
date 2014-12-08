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


#define UTL_UNITTEST

#include "../src/utl.h"
#include "../src/mf_read.h"

int main(int argc, char *argv[])
{

  if (argc > 1) {
    mf_sethandlers(mf_dmp_handlers);
    mf_read(argv[1]);
  }

  return 0;
}
