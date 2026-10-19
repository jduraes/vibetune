OBJECTS = vtune.com vtunecfg.com

# Dual-context: RomWBW Source/Apps/VibeTune (junction) vs sibling ../VibeTune
ifeq ($(wildcard ../../../Tools/tasm32),)
TOOLS = ../RomWBW/Tools
DEST = ../RomWBW/Binary/Apps
VTFLAGS =
else
TOOLS = ../../../Tools
DEST = ../../../Binary/Apps
VTFLAGS = -dVT_IN_ROMWBW
endif

include $(TOOLS)/Makefile.inc

DEPS := vibetune.asm vtver.inc timing.inc pt3bulba.inc pt3shim.inc ansiui.inc

vtune.com: $(DEPS)
	$(TASM) -dWBW $(VTFLAGS) vibetune.asm vtune.com vtune.lst

CFG_DEPS := vtunecfg.asm vtver.inc

vtunecfg.com: $(CFG_DEPS)
	$(TASM) -dWBW $(VTFLAGS) vtunecfg.asm vtunecfg.com vtunecfg.lst
