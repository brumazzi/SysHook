GCC=gcc
GPP=g++
CFLAGS=
INCLUDE=
LIBS=-lpthread
OUT_HOOK=hook
OUT_LCOPY=lcopy
SOURCE_HOOK=hook.c key-hook.c
SOURCE_LCOPY=key-hook.c list_copy.cc

build:
	${GCC} -o ${OUT_HOOK} ${SOURCE_HOOK} ${LIBS} ${INCLUDE} ${CFLAGS}
	${GPP} -o ${OUT_LCOPY} ${SOURCE_LCOPY} ${LIBS} ${INCLUDE} ${CFLAGS}

install:
	mkdir /opt/bin -p
	cp ${OUT_HOOK} /opt/bin
	cp hook.service /etc/init.d/hook
	cp $(OUT_LCOPY) /opt/bin
	service hook start

uninstall:
	service hook stop
	rm /opt/bin/${OUT_HOOK}
	rm /opt/bin/${OUT_LCOPY}
	rm /etc/init.d/hook
