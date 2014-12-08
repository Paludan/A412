# 
#  (C) by Remo Dentato (rdentato@gmail.com)
# 
# This software is distributed under the terms of the BSD license:
#   http://creativecommons.org/licenses/BSD/
#   http://opensource.org/licenses/bsd-license.php
#

debug:
	make CCOPTS="-DDEBUG -g"

release:
	make CCOPTS="-O2"

profile:
	make CCOPTS="-g -pg" LNOPTS="-pg"

$(CHKLIB):
	cd $(TOP)src ; make CCOPTS="$(CCOPTS)" LNOPTS="$(LNOPTS)" $(LIBNAME)

