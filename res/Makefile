#!/usr/bin/env make -f
# ImageMagic convert command
CONVERT = convert
CP = cp

SVG = icon.svg

################################################################################
ICO  = $(SVG:.svg=.ico)
PNG  = $(addsuffix .*.png, $(basename $(SVG)))

# Make ICO dependencies
makeicodeps = $(1).ico: $(addsuffix .png,$(addprefix $(1).,$(2)))
# Get the size
getsize = $(subst .,,$(suffix $(basename $@)))
# Get SVG name
getsvg = $(addsuffix .svg,$(basename $(basename $@)))
################################################################################

all: $(SVG) $(ICO) 

# Icon sizes
$(call makeicodeps, icon,16 24 32 48 64 96 128 256 512)

%.ico:
	$(CONVERT) $^ $@ 
	
%.png: $(SVG)
	$(CONVERT) -background transparent \
	-size $(call getsize, $@)x$(call getsize, $@) $(call getsvg, $@) $@

clean:
	$(RM) $(ICO) $(PNG)

.PHONY: all clean install
