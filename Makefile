
SRC = m30845.c thermo.c dio.c lcd.c eeprom.c misc.c workset.c pressure.c \
    fo1400_common.c fo1400_mode_adj.c fo1400_mode_manual.c fo1400_mode_auto.c fo1400.c \
    ui/ui_main.c ui/ui_settings.c ui/ui_library.c ui/ui_users.c ui/ui_lan.c \
    eth/uip/uip.c eth/uip/uip_arp.c eth/uip/uiplib.c eth/uip/psock.c \
    eth/enc28j60.c eth/tcpip.c

TARGET = fo1400
SRCDIR = src
BUILDDIR = build

CC = wine "c:\m32c-elf\bin\m32c-elf-gcc.exe"
OC = wine "c:\m32c-elf\bin\m32c-elf-objcopy.exe"

CFLAGS = -mcpu=m32c -std=gnu99 -Wall -Wno-unused
LDFLAGS = -T m32c84.ld -Wl,-Map=$(BUILDDIR)/$(TARGET).map -fstack-check
LIBS = -lm

OBJS=$(patsubst %.c, $(BUILDDIR)/%.o, $(SRC))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILDDIR)/$(TARGET).elf $(OBJS) $(LIBS)
	${OC} -O srec $(BUILDDIR)/$(TARGET).elf $(TARGET).mot

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f $(OBJS)
	@rm -rf $(BUILDDIR)
	@rm -f $(TARGET).mot

