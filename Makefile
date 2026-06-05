OBJECTS = vtune.com vtunecfg.com
DEST = ../RomWBW/Binary/Apps
TOOLS = ../RomWBW/Tools

include $(TOOLS)/Makefile.inc

DEPS := vibetune.asm vtversion.inc pt3engine.inc pt3notes.inc

vtune.com: $(DEPS)
	$(TASM) -dWBW vibetune.asm vtune.com vtune.lst

CFG_DEPS := vtunecfg.asm vtversion.inc

vtunecfg.com: $(CFG_DEPS)
	$(TASM) -dWBW vtunecfg.asm vtunecfg.com vtunecfg.lst
