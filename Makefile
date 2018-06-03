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
# Download tool
DOWNT = curl -R -O
# TAR tool
TAR = tar

# Project settings
PROJ	= working-hours
SRC	= main.c main_wnd.c defs.c tray_icon.c about_dialog.c working_hours.c
RES	= resource.rc
LUA_SRC = http://www.lua.org/ftp/lua-5.3.4.tar.gz

# Lua stuff
LUA_ARCH    = $(notdir $(LUA_SRC))
LUA_DIR     = $(basename $(basename $(LUA_ARCH)))
LUA_LIB     = $(LUA_DIR)/install/lib/liblua.a
# Compile flags
RESFLAGS    =
CFLAGS	    = -c -municode -I$(LUA_DIR)/install/include
LDFLAGS	    = -static  -mwindows -municode -L$(LUA_DIR)/install/lib
LDLIBS	    = -lcomctl32 -luser32 -lkernel32 -lgdi32 -luxtheme \
              -llua
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

$(EXECUTABLE): $(LUA_LIB) $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ)  -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

defs.o: defs.c
	$(CC) $(CFLAGS) $(PROJ_DEFINES) $< -o $@

%.o: %.rc
	$(MAKE) -C res
	$(WINDRES) $(RESFLAGS) $(subst \",\\\",$(PROJ_DEFINES)) -i $< -o $@

# LUA download and build

$(LUA_LIB): $(LUA_ARCH)
	$(TAR) -xzf $<
	$(MAKE) -C $(LUA_DIR) mingw local CC=$(CC)

$(LUA_ARCH):
	$(DOWNT) $(LUA_SRC)


clean:
	$(RM) $(EXECUTABLE) $(OBJ) $(LUA_ARCH)
	$(RM) -r $(LUA_DIR)
	$(MAKE) -C res $@

