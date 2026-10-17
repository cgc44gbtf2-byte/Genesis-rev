CC ?= cc
CPPFLAGS += -DMERC
CFLAGS ?= -O2 -g
CFLAGS += -std=gnu17 -Wall -Wextra -Wno-unused-parameter -fno-common
LDLIBS ?= -lcrypt

# Opt in to the historical allocator debugger. Leave it off for sanitizers.
ifeq ($(MEMWATCH),1)
CPPFLAGS += -DMEMWATCH
endif

O_FILES = atomic_save.o act_comm.o act_info.o act_move.o act_obj.o act_wiz.o bit.o board.o \
  clan.o clearclass.o combat_status.o comm.o const.o core.o db.o disc.o dragon.o \
	  drow.o elemental.o fight.o flag.o handler.o highland.o interp.o \
	  kav_info.o kav_wiz.o mage.o magic.o memwatch.o monk.o more.o ninja.o \
	  olc.o random.o rename.o rot_wiz.o save.o signals.o sorcerer.o \
	  special.o stance.o string.o trap.o update.o werewolf.o wizutil.o \
	  wwauspice.o wwbreed.o wwtribe.o

merc: $(O_FILES)
	$(CC) $(LDFLAGS) -o $@ $(O_FILES) $(LDLIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(O_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(O_FILES) $(O_FILES:.o=.d) merc
