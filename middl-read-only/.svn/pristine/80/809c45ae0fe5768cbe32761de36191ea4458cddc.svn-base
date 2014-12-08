# 
#  (C) by Remo Dentato (rdentato@gmail.com)
# 
# This software is distributed under the terms of the BSD license:
#   http://creativecommons.org/licenses/BSD/
#   http://opensource.org/licenses/bsd-license.php
#

# This makefile is for GNU tool chain it has been tested under 
# Windows (Mingw gcc 3.4.2) and Ubuntu Linux 8.04 (gcc 4.1.2)

TOP=../../
PROG=mpad
PROGLIBS=-lutl

include $(TOP)config.mk

$(PROG)$(_EXE): $(CHKLIB) $(PROG)$(_OBJ)
	$(LN)$@ $(PROG)$(_OBJ) -lmf $(PROGLIBS) $(MLIB)

clean:
	$(RM)  $(PROG)$(_EXE)
	$(RM)  $(PROG)$(_OBJ)

include $(TOP)targets.mk
