# ------------------------------------------------------------------------------
#  Project : DWIN-T5L-SDCC-Template
#  Author  : Recep Şenbaş (https://github.com/recepsenbas)
#  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
#  Contact : recepsenbas@gmail.com
#  Description : Cross-platform Makefile for building DWIN T5L/T5L51 firmware 
#                with SDCC (mcs51 large model). Automatically detects paths,
#                compiles all modules, and prints memory usage summary.
# ------------------------------------------------------------------------------
SDCC     = sdcc
AS       = sdas8051
PACKIHX  = packihx
MAKEBIN  = makebin


# Auto-detect SDCC mcs51 include and large-model lib directories
SDCC_MCS51_INCLUDEDIR := $(shell $(SDCC) --print-search-dirs | awk '/^includedir:/{inc=1; next} inc && /include\/mcs51/{print; exit}')
SDCC_MCS51_LIBDIR := $(patsubst %/include/mcs51,%/lib/large,$(SDCC_MCS51_INCLUDEDIR))
ifeq ($(strip $(SDCC_MCS51_LIBDIR)),)
SDCC_MCS51_LIBDIR := /opt/homebrew/Cellar/sdcc/4.5.0/share/sdcc/lib/large
endif

# Include directory path
INCLUDES = 	-Isrc -Isrc/app -Isrc/app/app_defs -Iinclude -Istartup -Ilib/uart -Ilib/sys \
			-Ilib/crc16 -Ilib/timer -Ilib/rtc -Isrc/app/functions

# Common flags for SDCC
CFLAGS  = -mmcs51 --model-large --xram-loc 0x8000 --xram-size 0x8000 $(INCLUDES)
LDFLAGS = -mmcs51 --model-large --xram-loc 0x8000 --xram-size 0x8000 -L"$(SDCC_MCS51_LIBDIR)"

OBJDIR   = build/obj
BINDIR   = build/dist

SRCS = \
    src/main.c \
	src/app/app_defs/app_defs.c \
	lib/uart/uart.c \
	lib/sys/sys.c \
	lib/crc16/crc16.c \
	lib/timer/timer.c \
	lib/rtc/rtc.c \


ASMS = startup/startup_T5L.s
RELS     = 	$(OBJDIR)/main.rel $(OBJDIR)/startup_T5L.rel $(OBJDIR)/uart.rel $(OBJDIR)/sys.rel \
			$(OBJDIR)/crc16.rel $(OBJDIR)/timer.rel $(OBJDIR)/rtc.rel $(OBJDIR)/app_defs.rel \


# Link-time relative object list from inside build/dist
LINK_RELS := $(patsubst $(OBJDIR)/%,../obj/%,$(RELS))

TARGET   = output
IHX      = $(BINDIR)/$(TARGET).ihx
HEX      = $(BINDIR)/$(TARGET).hex
BIN      = $(BINDIR)/T5L51.bin
MAP      = $(BINDIR)/$(TARGET).map

all: $(BIN)

$(OBJDIR)/main.rel: src/main.c
	@mkdir -p $(OBJDIR)
	$(SDCC) $(CFLAGS) -c $< -o $@ 

$(OBJDIR)/startup_T5L.rel: startup/startup_T5L.s
	@mkdir -p $(OBJDIR)
	$(AS) -plos $@ $<

$(OBJDIR)/uart.rel: lib/uart/uart.c
	@mkdir -p $(OBJDIR)
	$(SDCC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/sys.rel: lib/sys/sys.c
	@mkdir -p $(OBJDIR)
	$(SDCC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/crc16.rel: lib/crc16/crc16.c
	@mkdir -p $(OBJDIR)
	$(SDCC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/timer.rel: lib/timer/timer.c
	@mkdir -p $(OBJDIR)
	$(SDCC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/rtc.rel: lib/rtc/rtc.c
	@mkdir -p $(OBJDIR)
	$(SDCC) $(CFLAGS) -c $< -o $@

# app/app_defs/app_defs.c
$(OBJDIR)/app_defs.rel: src/app/app_defs/app_defs.c
	@mkdir -p $(OBJDIR)
	$(SDCC) $(CFLAGS) -c $< -o $@


$(BINDIR):
	@mkdir -p $(BINDIR)

# Link
$(IHX): $(RELS)
	@mkdir -p $(BINDIR)
	cd $(BINDIR) && $(SDCC) $(LDFLAGS) -Wl-m -o $(TARGET).ihx $(LINK_RELS)

# Hex
$(HEX): $(IHX)
	$(PACKIHX) $(IHX) > $(HEX)

# Bin + Map özeti
# Bin + Map özeti
$(BIN): $(HEX)
	$(MAKEBIN) -p $(HEX) $(BIN)
	@MAPFILE="$(MAP)"; \
	if [ -f "$$MAPFILE" ]; then \
	  LC_ALL=C awk '\
	  function hex2dec(h, i, d, v){ h=toupper(h); if (substr(h,1,2)=="0X") h=substr(h,3); v=0; \
	    for(i=1;i<=length(h);i++){ d=substr(h,i,1); v=v*16 + index("0123456789ABCDEF",d)-1 } return v } \
	  /^[A-Za-z0-9_.]+[ \t]+[0-9A-Fa-f]+[ \t]+[0-9A-Fa-f]+[ \t]*=/{ \
	    area=$$1; size=hex2dec("0x"$$3); line=$$0; \
	    if (area=="XSEG") xdata += size; \
	    if (area=="DSEG") data  += size; \
	    if (area=="ISEG") idata += size; \
	    if (area ~ /^BSEG/) bit += size; \
	    if (line ~ /\(.*CODE.*\)/) code += size; \
	  } \
	  END{ \
	    printf("📊 Memory usage (%s):\n", FILENAME); \
	    printf("  CODE   %8d / 65536 (%.1f%%)\n", code+0, (code/65536)*100.0); \
	    printf("  XDATA  %8d / 32768 (%.1f%%)\n", xdata+0, (xdata/32768)*100.0); \
	    printf("  DATA   %8d / 256   (%.1f%%)\n", data+0, (data/256)*100.0); \
	    printf("  IDATA  %8d / 256   (%.1f%%)\n", idata+0, (idata/256)*100.0); \
	    printf("  BIT    %8d / 256   (%.1f%%)\n", bit+0, (bit/256)*100.0); \
	    if ((data + idata) > 256) { \
	      printf("⚠️  Warning: DATA + IDATA exceeds 256 bytes!\n"); \
	    } \
	  }' "$$MAPFILE"; \
	else \
	  echo "⚠️  Map file not found."; \
	fi
	@echo "✅ Build complete: $(BIN)"
	@echo "📦 Project: DWIN-T5L-SDCC-Template"

clean:
	@rm -rf build
	@echo "🧹 Cleaned."

# ------------------------------------------------------------------------------
#  Generated as part of the DWIN-T5L-SDCC-Template project.
#  © 2025 Recep Şenbaş — All rights reserved under CC BY-NC-SA 4.0.
# ------------------------------------------------------------------------------