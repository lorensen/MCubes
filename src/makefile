#
#	make cubes
#

.KEEP_STATE:
PROFILE =
DEBUG = -g
CFLAGS = $(DEBUG) $(PROFILE) -fsingle -f68881 -O
HOST = iag
DIRECTORY = [.cubes.code7]
SOURCES = cubes.c menu.c cases.c kbmenu.c \
		mask.c plane.c surface.c solid.c parser.c \
		clip.c sphere.c octant.c \
		timer.c openfile.c input.c dummy.c \
		cubes.h init.h kbmenu.h \
		permutes cases.cas mkcase.c permute.c permute.h

all: $(SOURCES) cubes

cubes: cubes.o menu.o cases.o kbmenu.o timer.o openfile.o clip.o \
	input.o \
	mask.o plane.o surface.o solid.o parser.o dummy.o sphere.o octant.o
	cc $(PROFILE) $(DEBUG) -o cubes \
	cubes.o parser.o menu.o cases.o kbmenu.o timer.o openfile.o clip.o \
	input.o \
	mask.o plane.o surface.o solid.o sphere.o octant.o \
	dummy.o \
	-f68881 -lm

mkcase: mkcase.o
	cc $(CFLAGS) -o mkcase mkcase.o

permute: permute.o
	cc $(CFLAGS) -g -o permute permute.o

cases.h: permutes cases.cas mkcase permute
	permute cases.cas >junk <permutes
	sort <junk | mkcase >cases.h

lint: $(SOURCES)
	lint cubes.c menu.c cases.c kbmenu.c clip.c \
	mask.c plane.c surface.c solid.c parser.c \
	dummy.c timer.c openfile.c >lint


cubes.o: cubes.h init.h
cases.o: cases.h
kbmenu.o: kbmenu.h
mask.o: cubes.h
surface.o: cubes.h
plane.o: cubes.h
clip.o: cubes.h
solid.o: cubes.h
parser.o: cubes.h

vax: cubes.vax menu.vax cases.vax kbmenu.vax mask.vax plane.vax surface.vax \
	solid.vax parser.vax init.vax dummy.vax clip.vax

.SUFFIXES: .vax .c .h

.c.vax:
	@echo "cd $(DIRECTORY)" >doit
	@if (test -f $*.c); then echo "put $*.c" >>doit; fi
	@if (test -f $*.h ); then echo "put $*.h" >>doit; fi
	@echo "quit" >>doit
	ftp $(HOST) <doit
	@rm doit
	@touch $*.vax

.h.vax:
	@echo "cd $(DIRECTORY)" >doit
	@if (test -f $*.c); then echo "put $*.c" >>doit; fi
	@if (test -f $*.h ); then echo "put $*.h" >>doit; fi
	@echo "quit" >>doit
	ftp $(HOST) <doit
	@rm doit
	@touch $*.vax

$(SOURCES):
	sccs get $@
