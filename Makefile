#! /usr/bin/env make -f
# Makefile to build working-hours application using cygwin with mingw compiler
# or mingw itself
#
# Author: Stefan Misik (mail@stefanmisik.eu)

# Set commands names
ifeq ($(shell uname -o),Cygwin)
	CC	= i686-w64-mingw32-gcc
	WINDRES = i686-w64-mingw32-windres
else
	CC	= gcc
	WINDRES = windres
endif

# Project settings
PROJ	= working-hours
SRC	= main.c main_wnd.c defs.c tray_icon.c about_dialog.c working_hours.c
RES	= resource.rc

# Compile flags
RESFLAGS    =
CFLAGS	    = -c -municode
LDFLAGS	    = -static  -mwindows -municode
LDLIBS	    = -lcomctl32 -luser32 -lkernel32 -lgdi32 -luxtheme
# Number to subtract from the last git commits count
LAST_COMMIT = 14

################################################################################
# Git versions
GIT_VERSION = $(shell git describe --dirty --always)
GIT_TAG     = $(shell git describe --abbrev=0 --tags)
GIT_COMMITS = $$(( $(shell git rev-list --all --count) - $(LAST_COMMIT) ))

# Git defines
PROJ_DEFINES := -DPROJECT_EXE=\"$(PROJ)\"		    \
		-DPROJECT_COMMITS=$(GIT_COMMITS)	    \
		-DPROJECT_LAST_RELEASE=\"$(GIT_TAG)\"	    \
	        -DPROJECT_GIT=\"$(GIT_VERSION)\"	    \
		-DPROJECT_LAST_COMMIT=$(LAST_COMMIT)

# Objects and outputs
OBJ = $(RES:.rc=.o) $(SRC:.c=.o)
EXECUTABLE = $(addsuffix .exe,$(PROJ))

# Debug flags
ifeq ($(DBG),y)
    CFLAGS   += -D_DEBUG -ggdb -Wall
    RESFLAGS += -D_DEBUG
    LDFLAGS  += -ggdb
endif
################################################################################

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ)  -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

defs.o: defs.c
	$(CC) $(CFLAGS) $(PROJ_DEFINES) $< -o $@

%.o: %.rc
	$(MAKE) -C res
	$(WINDRES) $(RESFLAGS) $(subst \",\\\",$(PROJ_DEFINES)) -i $< -o $@

clean:
	$(RM) $(EXECUTABLE) $(SRC:.c=.o) $(RES:.rc=.o)
	$(MAKE) -C res $@

