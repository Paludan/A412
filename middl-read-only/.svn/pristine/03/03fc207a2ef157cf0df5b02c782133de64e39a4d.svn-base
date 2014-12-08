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
#include "mf_errors.h"

char *mf_errmsg(unsigned long errnum)
{

  switch (mf_errnum(errnum)) {
    case 901: return "Unknown chord";
    case 902: return "Unknown command";
    case 903: return "Unknown instrument";
    case 904: return "Syntax error";
    case 905: return "Invalid chord";
    case 906: return "Invalid channel";
    case 907: return "Invalid controller";
    case 908: return "Invalid value";
    case 909: return "Invalid scale note";
  }

  return "Unexpected error";
}
