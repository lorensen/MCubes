#
#	make cubes
#

.KEEP_STATE:
PROFILE = 
DEBUG =
CFLAGS = -O2 $(DEBUG) $(PROFILE) $(FLOAT_OPTION)

LIBS-sun4-solaris = -lc -lnsl -lsocket -lucb -lm
LIBS = $(LIBS-$(ARCH))
MIO = ../mio/$(ARCH)/mio.o ../crdidbmutil/$(ARCH)/libidbmutil.a
OBJECTS = cubes.o cases.o options.o \
	mask.o plane.o surface.o solid.o parser.o \
	clip.o sphere.o octant.o gradient.o \
	timer.o input.o

SOURCES = $(OBJECTS:%.o=%.c) \
	cubes.h init.h  \
	permutes cases.cas mkcase.c permute.c permute.h

LOAD = $(CC)

.INIT:
	@-mkdir $(ARCH) 2>/dev/null; exit 0

all: .INIT $(SOURCES) cubes-$(ARCH) toold-$(ARCH)

cubes-$(ARCH): $(OBJECTS:%=$(ARCH)/%)
	$(LOAD) $(PROFILE) $(DEBUG) -o cubes-$(ARCH) \
	$(OBJECTS:%=$(ARCH)/%) \
	$(MIO) \
	$(FLOAT_OPTION) $(LIBS) -lm

toold-$(ARCH): $(ARCH)/toold.o
	cc -o toold-$(ARCH) $(ARCH)/toold.o

mkcase-$(ARCH): $(ARCH)/mkcase.o
	cc -o mkcase-$(ARCH) $(ARCH)/mkcase.o

permute-$(ARCH): $(ARCH)/permute.o
	cc -o permute-$(ARCH) $(ARCH)/permute.o

$(ARCH)/cases.o: cases.h

cases.h: permutes cases.cas mkcase-$(ARCH) permute-$(ARCH)
	permute-$(ARCH) cases.cas >junk <permutes
	sort <junk | mkcase-$(ARCH) cases.cas >cases.h
	rm junk

lint: $(SOURCES)
	lint cubes.c cases.c clip.c \
	mask.c plane.c surface.c solid.c parser.c options.c \
	timer.c >lint

$(SOURCES):
	sccs get $@

$(ARCH)/%.o: %.c
	$(CC) $(CFLAGS) -o $(ARCH)/$*.o -c $*.c

