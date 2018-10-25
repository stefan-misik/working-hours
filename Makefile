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
SRC	= main.c main_wnd.c defs.c tray_icon.c about_dialog.c working_hours.c \
          wh_lua.c dbg_wnd.c lua_edit.c dialog_resize.c
RES	= resource.rc
RES_DEP = default.lua icon.svg working-hours.manifest
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
# Resource dependency
RES_DEP_FILES = $(addprefix res/,$(RES_DEP))
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

.PHONY: all clean mostlyclean

all: $(EXECUTABLE)

$(EXECUTABLE): $(LUA_LIB) $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ)  -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

defs.o: defs.c
	$(CC) $(CFLAGS) $(PROJ_DEFINES) $< -o $@

%.o: %.rc $(RES_DEP_FILES)
	$(MAKE) -C res
	$(WINDRES) $(RESFLAGS) $(subst \",\\\",$(PROJ_DEFINES)) -i $< -o $@

# LUA download and build

$(LUA_LIB): $(LUA_ARCH)
	$(TAR) -xzf $<
	$(MAKE) -C $(LUA_DIR) generic CC=$(CC)
	$(MAKE) -C $(LUA_DIR) install INSTALL_TOP=../install

$(LUA_ARCH):
	$(DOWNT) $(LUA_SRC)


mostlyclean:
	$(RM) $(EXECUTABLE) $(OBJ)
	$(MAKE) -C res clean
	-$(MAKE) -C $(LUA_DIR) uninstall INSTALL_TOP=../install
	-$(MAKE) -C $(LUA_DIR) clean

clean: mostlyclean
	$(RM) -r $(LUA_ARCH) $(LUA_DIR)

