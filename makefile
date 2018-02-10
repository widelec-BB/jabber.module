# makefile for GNU make (automatically generated by makemake 14-Kwi-13, 11:49:08)
# NOTE: indent with TAB in GNU makefiles!

# MESSAGES #

COMPILE_FILE=printf "\033[K\033[0;33mCompiling \033[1;33m$<\033[0;33m...\033[0m\n"
TARGET_DONE=printf "\033[K\033[0;32mTarget \"$@\" successfully done.\033[0m\n"
LINKING=printf "\033[K\033[1;34mLinking project \"$@\"... \033[0m\n"

# PROJECT #
# paths are relative to the project directory (current directory during make)

OUTFILE = jabber.module

OUTDIR  = bin/
OBJDIR  = o/

PROJECT = $(OUTDIR)$(OUTFILE)

# path to directory containing KwaKwa source code
KWAKWAAPI = ../kwakwa

# COMPILER #
CC = ppc-morphos-gcc
CWARNS = -Wall -Wno-pointer-sign
CDEFS  = -DUSE_INLINE_STDARG -D__NOLIBBASE__ -D__AMIGADATE__=\"\ \($(shell date "+%d.%m.%Y")\)\ \" -DAROS_ALMOST_COMPATIBLE
# -D__DEBUG__
CFLAGS = -O3 -noixemul
CLIBS  = -I$(KWAKWAAPI)

COMPILE = $(CC) $(TARGET) $(CWARNS) $(CDEFS) $(CFLAGS) $(CLIBS)

# LINKER #
LD = ppc-morphos-gcc

LWARNS =
LDEFS  =
LFLAGS = -noixemul -nostdlib -nostartfiles
LIBS   = -Liksemel -liksemel -labox -lvstring
# -ldebug

LINK   = $(LD) $(TARGET) $(LWARNS) $(LDEFS) $(LFLAGS)

# subdirectories #
DIRS = iksemel
BUILDDIRS = $(DIRS:%=build-%)
CLEANDIRS = $(DIRS:%=clean-%)

# target 'all' (default target)
all: $(BUILDDIRS) $(PROJECT)
	@$(TARGET_DONE)

# target 'compiler' (compile target)
$(OBJDIR)class.c.o: class.c class.h globaldefines.h
	@$(COMPILE_FILE)
	@$(COMPILE) -c -o $(OBJDIR)class.c.o class.c

$(OBJDIR)lib.c.o: lib.c class.h globaldefines.h
	@$(COMPILE_FILE)
	@$(COMPILE) -c -o $(OBJDIR)lib.c.o lib.c

$(OBJDIR)gui.c.o: gui.c globaldefines.h gui.h locale.h translations.h
	@$(COMPILE_FILE)
	@$(COMPILE) -c -o $(OBJDIR)gui.c.o gui.c

$(OBJDIR)locale.c.o: locale.c locale.h translations.h
	@$(COMPILE_FILE)
	@$(COMPILE) -c -o $(OBJDIR)locale.c.o locale.c

$(OBJDIR)ikshooks.c.o: ikshooks.c iksemel/iksemel.h globaldefines.h class.h
	@$(COMPILE_FILE)
	@$(COMPILE) -c -o $(OBJDIR)ikshooks.c.o ikshooks.c

$(OBJDIR)events.c.o: events.c globaldefines.h events.h
	@$(COMPILE_FILE)
	@$(COMPILE) -c -o $(OBJDIR)events.c.o events.c

$(OBJDIR)support.c.o: support.c support.h globaldefines.h
	@$(COMPILE_FILE)
	@$(COMPILE) -c -o $(OBJDIR)support.c.o support.c

OBJS = $(OBJDIR)lib.c.o $(OBJDIR)class.c.o $(OBJDIR)gui.c.o $(OBJDIR)locale.c.o $(OBJDIR)ikshooks.c.o $(OBJDIR)events.c.o\
 $(OBJDIR)support.c.o

# link all file(s)
$(PROJECT): $(OBJS)
	@$(LINKING)
	@$(LINK) $(OBJS) -o $(PROJECT) $(LIBS)

# any other targets

iksemel:
	$(MAKE) -C iksemel

strip:
	@strip --strip-unneeded --remove-section=.comment $(PROJECT)
	@$(TARGET_DONE)

clean: $(CLEANDIRS)
	@-rm $(PROJECT)
	@-rm $(OBJDIR)*.o
	@$(TARGET_DONE)

dump:
	@objdump -dC $(OUTDIR)/$(OUTFILE) >RAM:$(OUTFILE).txt
	@$(TARGET_DONE)

dist: all
	@rm -rf RAM:$(OUTFILE) RAM:$(OUTFILE).lha
	@mkdir RAM:$(OUTFILE) >NIL:
	@mkdir RAM:$(OUTFILE)/modules >NIL:
	@copy $(OUTDIR)$(OUTFILE) RAM:$(OUTFILE)/modules/$(OUTFILE) >NIL:
	@copy $(OUTDIR)catalogs RAM:$(OUTFILE)/catalogs ALL >NIL:
	@copy doc RAM:$(OUTFILE) ALL >NIL:
	@strip --strip-unneeded --remove-section .comment RAM:$(OUTFILE)/modules/$(OUTFILE) >NIL:
	@find RAM:$(OUTFILE) -name .svn -printf "\"%p\"\n" | xargs rm -rf
	@MOSSYS:C/LHa a -r -a RAM:$(OUTFILE).lha RAM:$(OUTFILE)/ >NIL:
	@$(TARGET_DONE)

$(BUILDDIRS):
	@$(MAKE) -C $(@:build-%=%)

$(CLEANDIRS): 
	@$(MAKE) -C $(@:clean-%=%) clean

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)
