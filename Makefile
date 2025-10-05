OBJECTS = vibetune.com
DEST = ../RomWBW/Binary/Apps
TOOLS = ../RomWBW/Tools

include $(TOOLS)/Makefile.inc

DEPS := vibetune.asm $(shell find . -name '*.inc')

vibetune.com: $(DEPS)
	$(TASM) -dWBW vibetune.asm vibetune.com vibetune.lst

all::
	mkdir -p $(DEST)/Tunes
	cp Tunes/* $(DEST)/Tunes
