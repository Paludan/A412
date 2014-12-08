# 
#  (C) by Remo Dentato (rdentato@gmail.com)
# 
# This software is distributed under the terms of the BSD license:
#   http://creativecommons.org/licenses/BSD/
#   http://opensource.org/licenses/bsd-license.php
#

# This makefile is for GNU tool chain 


TOP=../

include $(TOP)config.mk

HDRS= mf_rw.h mf_seq.h mf_instr.h mf_chords.h mf_errors.h
LIBOBJ = mf_rw$(_OBJ) mf_seq$(_OBJ) mf_instr$(_OBJ) mf_chords$(_OBJ) mf_errors$(_OBJ)
LIBDIR =.

$(LIBNAME) : mf.h $(LIBOBJ) 
	$(AR) $@ $(LIBOBJ)

CCOPTS = -DDEBUG -g	

mf.h: $(HDRS)
	$(CAT) $(HDRS) > mf.h

ut_seq$(_EXE): mf_rw$(_OBJ) ut_seq$(_OBJ)
	$(CC) -o $@ mf_rw$(_OBJ) ut_seq$(_OBJ)
	
ut_seq$(_OBJ): mf_seq.c
	$(CC) $(CCOPTS)  -DMF_SEQ_TEST -c -o ut_seq$(_OBJ) mf_seq.c
	
ut_score$(_EXE): mf_rw$(_OBJ) mf_seq$(_OBJ) mf_instr$(_OBJ) mf_chords$(_OBJ) mf_errors$(_OBJ) ut_score$(_OBJ)
	$(CC) -o $@  mf_rw$(_OBJ) mf_seq$(_OBJ) mf_instr$(_OBJ) mf_chords$(_OBJ) mf_errors$(_OBJ) ut_score$(_OBJ)
	
ut_score$(_OBJ): mf_score.c
	$(CC) $(CCOPTS)  -DMF_SCORE_TEST -c -o ut_score$(_OBJ) mf_score.c
	
ut_read$(_EXE): mf_rw.c mf_rw.h
	$(CC) $(CCOPTS)  -DMF_READ_TEST -o ut_read$(_EXE) mf_rw.c
	
ut_write$(_EXE): mf_rw.c mf_rw.h
	$(CC) -$(CCOPTS)  -DMF_WRITE_TEST -o ut_write$(_EXE) mf_rw.c

lualibs:
	$(CP) $(LUADIR)/src/lualib$(_LIB) $(LUADIR)/src/lua.h $(LUADIR)/src/lualib.h $(LUADIR)/src/lauxlib.h .
	
mf_lua$(_EXE): lualibs mf_lua$(_OBJ)
	xx

clean:
	$(RM) mf.h
	$(RM) $(LIBNAME)
	$(RM) $(LIBOBJ)
	$(RM) ut_read$(_EXE) ut_write$(_EXE) ut_seq$(_EXE) ut_score$(_EXE)
	$(RM) ut_seq$(_OBJ) ut_score$(_OBJ)
	$(RM) gmon.out

include $(TOP)targets.mk


