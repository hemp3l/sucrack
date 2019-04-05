sucrackprgdir=$(prefix)/bin
sucrackprg_PROGRAMS=sucrack
sucrack_LDFLAGS=$(SUCRACK_LINK_FLAGS)
sucrack_CFLAGS=-Wall -O2 -D_GNU_SOURCE $(SUCRACK_COMPILE_FLAGS)
sucrack_SOURCES=sucrack.c worker.c dictionary.c pty.c su.c rewriter.c util.c stat.c rules.c \
		sucrack.h worker.h dictionary.h pty.h su.h rewriter.h util.h stat.h rules.h
