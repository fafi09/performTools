#export LD_LIBRARY_PATH=/lib:/usr/lib:.:
DEL_FILE      = rm -f
Jpt:lnk
	gcc Jpt.c  JptUtils.h -ljptutils -L. -oJpt
lnk:compile
	gcc -shared -olibjptutils.so JptUtils.o
compile:
	gcc -c -fpic JptUtils.c JptUtils.h
clean:
	$(DEL_FILE) JptUtils.o
	$(DEL_FILE) JptUtils.h.gch
	