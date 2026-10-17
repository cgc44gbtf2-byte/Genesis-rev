CC ?= cc
CPPFLAGS += -DMERC
CFLAGS ?= -O2 -g
CFLAGS += -std=gnu17 -Wall -Wextra -Wno-unused-parameter -fno-common
LDLIBS ?= -lcrypt

# Opt in to the historical allocator debugger. Leave it off for sanitizers.
ifeq ($(MEMWATCH),1)
CPPFLAGS += -DMEMWATCH
endif

include classes/modules.mk
CLASS_OBJECTS = $(CLASS_MODULE_SOURCES:.c=.o)

O_FILES = atomic_save.o act_comm.o act_info.o act_move.o act_obj.o act_wiz.o bit.o board.o \
  clan.o clearclass.o combat_status.o comm.o const.o core.o db.o \
  fight.o flag.o handler.o interp.o \
  kav_info.o kav_wiz.o magic.o memwatch.o more.o \
  olc.o random.o rename.o rot_wiz.o save.o signals.o \
  special.o stance.o string.o trap.o update.o wizutil.o $(CLASS_OBJECTS)

merc: $(O_FILES)
	$(CC) $(LDFLAGS) -o $@ $(O_FILES) $(LDLIBS)

%.o: %.c
	$(CC) -I. $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(O_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(O_FILES) $(O_FILES:.o=.d) merc
	# Also clear flat-layout artifacts left by an in-place branch update.
	rm -f $(notdir $(CLASS_OBJECTS)) $(notdir $(CLASS_OBJECTS:.o=.d))
