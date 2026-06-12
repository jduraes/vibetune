OBJECTS = vtune.com vtunecfg.com
DEST = ../RomWBW/Binary/Apps
TOOLS = ../RomWBW/Tools

include $(TOOLS)/Makefile.inc

DEPS := vibetune.asm vtversion.inc timing.inc pt3bulba.inc pt3bulba_shim.inc

vtune.com: $(DEPS)
	$(TASM) -dWBW vibetune.asm vtune.com vtune.lst

CFG_DEPS := vtunecfg.asm vtversion.inc

vtunecfg.com: $(CFG_DEPS)
	$(TASM) -dWBW vtunecfg.asm vtunecfg.com vtunecfg.lst
