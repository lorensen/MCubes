#
#	make cubes
#

.KEEP_STATE:
PROFILE = 
DEBUG =
ARCH = $(TARGET_ARCH:-%=%)
CFLAGS = $(DEBUG) $(PROFILE) $(FLOAT_OPTION) -O -fsingle
OBJECTS = cubes.o cases.o options.o \
	mask.o plane.o surface.o solid.o parser.o \
	clip.o sphere.o octant.o gradient.o \
	timer.o input.o

SOURCES = $(OBJECTS:%.o=%.c) \
	cubes.h init.h  \
	permutes cases.cas mkcase.c permute.c permute.h

.INIT:
	-mkdir $(ARCH)

all: $(SOURCES) cubes-$(ARCH)

cubes-$(ARCH): $(OBJECTS:%=$(ARCH)/%)
	cc $(PROFILE) $(DEBUG) -o cubes-$(ARCH) \
	$(OBJECTS:%=$(ARCH)/%) \
	$(FLOAT_OPTION) /usr/lib/f68881.il -lm

mkcase-$(ARCH): $(ARCH)/mkcase.o
	cc -o mkcase-$(ARCH) $(ARCH)/mkcase.o

permute-$(ARCH): $(ARCH)/permute.o
	cc -o permute-$(ARCH) $(ARCH)/permute.o

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

