#  (C) 2008 by Remo Dentato (rdentato@users.sourceforge.net)
#
# Permission to use, copy, modify and distribute this code and
# its documentation for any purpose is hereby granted without
# fee, provided that the above copyright notice, or equivalent
# attribution acknowledgement, appears in all copies and
# supporting documentation.
# 
# Copyright holder makes no representations about the suitability
# of this software for any purpose. It is provided "as is" without
# express or implied warranty.

include ../config.mk

ifeq ($(SYS),MINGW)
_EXE=.exe
endif

all: wtest rtest atest

AR=ar rcu

msq_write_tx$(_EXE): msq_write_ut.c ../src/msq_write.c
	gcc -DDEBUG -I../src -o msq_write_tx$(_EXE) msq_write_ut.c -L../src -lmsq -lm

msq_read_tx$(_EXE): msq_read_ut.c ../src/msq_read.c
	gcc -DDEBUG -I../src -o msq_read_tx$(_EXE) msq_read_ut.c -L../src -lmsq -lm

msq_allnotes_tx$(_EXE): msq_allnotes.c
	gcc -DDEBUG -I../src -o msq_allnotes_tx$(_EXE) msq_allnotes.c -L../src -lmsq -lm


	
wtest: msq_write_tx$(_EXE) 
	msq_write_tx$(_EXE)
	
rtest: msq_read_tx$(_EXE)
	msq_read_tx$(_EXE) test_file.mid
	
atest: msq_allnotes_tx$(_EXE)
	msq_allnotes_tx$(_EXE)
	
clean:
	rm -f *.o
	rm -f *_tx$(_EXE)
	rm -f test_*
	
        	
