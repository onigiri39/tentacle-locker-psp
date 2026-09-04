TARGET = EBOOT
OBJS = src/main.o

CFLAGS = -O2 -G0 -Wall -Wextra
LIBS = -lpspgu -lpspdisplay -lpspctrl
BUILD_PRX = 1
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Tentacle Locker PSP Port
PSP_EBOOT_ICON = assets/ICON0.png
PSP_EBOOT_PIC1 = assets/PREVIEW.png

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

DIST = dist/TENTACLE_LOCKER_PSP

.PHONY: all clean dist

all: EBOOT.PBP

dist: EBOOT.PBP
	@rm -rf $(DIST)
	@mkdir -p $(DIST)
	@cp EBOOT.PBP $(DIST)/
	@cp -a assets $(DIST)/
	@rm -rf $(DIST)/assets/audio_fsb
	@rm -rf $(DIST)/assets/psp_textures/*.preview.png
	@rm -f dist/TENTACLE_LOCKER_PSP.tar.gz
	@tar -czf dist/TENTACLE_LOCKER_PSP.tar.gz -C dist TENTACLE_LOCKER_PSP
	@echo "Built: $(DIST)/EBOOT.PBP"
	@echo "Package: dist/TENTACLE_LOCKER_PSP.tar.gz"

clean:
	@rm -rf $(TARGET).elf $(TARGET).prx $(TARGET).o src/*.o PARAM.SFO EBOOT.PBP dist
