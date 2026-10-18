CC ?= cc
LDLIBS ?= -lcrypt
TEST_FLAGS = -std=gnu17 -O1 -g -ffunction-sections -fdata-sections
TEST_LDFLAGS = -Wl,--gc-sections
TEST_CPPFLAGS = -IGenesis/src $(CPPFLAGS)
include Genesis/src/classes/modules.mk
TESTS = tests/test_comm tests/test_db tests/test_gameplay tests/test_buffers tests/test_save tests/test_signals tests/test_classes tests/test_powers tests/test_wall tests/test_mage_powers tests/test_combat_status tests/test_vampire tests/test_class_runtime tests/test_restored_spells tests/test_basic_buffs tests/test_class_identity tests/test_class_handlers tests/test_vampire_modules tests/test_damage_messages tests/test_transformations tests/test_clans
ENGINE_SOURCES = $(wildcard Genesis/src/*.c) $(addprefix Genesis/src/,$(CLASS_MODULE_SOURCES))
ENGINE_HEADERS = $(wildcard Genesis/src/*.h Genesis/src/classes/*.h Genesis/src/classes/*/*.h)
ENGINE_OBJECTS = $(patsubst Genesis/src/%.c,tests/stun_engine/%.o,$(ENGINE_SOURCES))
LEGACY_CLASS_OBJECTS = $(addprefix tests/stun_engine/,$(notdir $(CLASS_MODULE_SOURCES:.c=.o)))

.PHONY: all test check-layout sanitize clean
all:
	$(MAKE) -C Genesis/src

tests/%: tests/%.c $(ENGINE_SOURCES) $(ENGINE_HEADERS)
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

tests/test_gameplay: tests/test_gameplay.c $(ENGINE_SOURCES) $(ENGINE_HEADERS)
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< Genesis/src/act_info.c Genesis/src/classes/werewolf/wwauspice.c Genesis/src/classes/mage/mage.c $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

tests/test_buffers: tests/test_buffers.c $(ENGINE_SOURCES) $(ENGINE_HEADERS)
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) --param=asan-globals=0 -c Genesis/src/interp.c -o tests/buffers_interp.o
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< Genesis/src/string.c tests/buffers_interp.o $(LDFLAGS) $(TEST_LDFLAGS) -o $@

tests/test_signals: tests/test_signals.c $(ENGINE_SOURCES) $(ENGINE_HEADERS)
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< Genesis/src/signals.c $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

tests/test_classes tests/test_wall tests/test_mage_powers: tests/%: tests/%.c $(ENGINE_SOURCES) $(ENGINE_HEADERS)
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) -O0 $< $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

tests/test_powers: tests/test_powers.c $(ENGINE_SOURCES) $(ENGINE_HEADERS)
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< Genesis/src/classes/ninja/ninja.c Genesis/src/classes/monk/monk.c Genesis/src/classes/elemental/elemental.c Genesis/src/classes/sorcerer/sorcerer.c Genesis/src/classes/drow/drow.c Genesis/src/combat_status.c $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

tests/test_class_runtime: tests/test_class_runtime.c $(ENGINE_SOURCES) $(ENGINE_HEADERS)
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< Genesis/src/classes/elemental/elemental.c Genesis/src/classes/monk/monk.c Genesis/src/classes/ninja/ninja.c Genesis/src/classes/dragon/dragon.c Genesis/src/classes/drow/drow.c Genesis/src/classes/highlander/highland.c Genesis/src/classes/werewolf/wwtribe.c Genesis/src/combat_status.c $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

check-layout:
	python3 tests/test_class_layout.py

test: check-layout $(TESTS)
	@set -e; for t in $(TESTS); do ./$$t; done

# Link the real engine, including actual safety, combat and interpreter paths.
# Only the server entry point is renamed; no real accounts/world files load.
tests/test_combat_status tests/test_vampire tests/test_basic_buffs tests/test_class_handlers tests/test_vampire_modules tests/test_damage_messages tests/test_transformations tests/test_clans: tests/%: tests/%.c $(ENGINE_OBJECTS)
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< $(ENGINE_OBJECTS) $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

# Separate test objects keep sanitizer flags out of the production executable.
tests/test_restored_spells: tests/test_restored_spells.c $(filter-out tests/stun_engine/fight.o,$(ENGINE_OBJECTS)) tests/restored_fight.o
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< $(filter-out tests/stun_engine/fight.o,$(ENGINE_OBJECTS)) tests/restored_fight.o $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

tests/restored_fight.o: Genesis/src/fight.c $(ENGINE_HEADERS)
	$(CC) $(TEST_CPPFLAGS) -DMERC -DCOMBAT_CALLBACK_TESTING $(CFLAGS) $(TEST_FLAGS) -c $< -o $@

tests/stun_engine/%.o: Genesis/src/%.c $(ENGINE_HEADERS)
	mkdir -p $(dir $@)
	$(CC) $(TEST_CPPFLAGS) -DMERC $(CFLAGS) $(TEST_FLAGS) -c $< -o $@

tests/stun_engine/comm.o: Genesis/src/comm.c $(ENGINE_HEADERS)
	mkdir -p tests/stun_engine
	$(CC) $(TEST_CPPFLAGS) -DMERC $(CFLAGS) $(TEST_FLAGS) -Dmain=genesis_server_main -c $< -o $@

tests/stun_engine/atomic_save.o: Genesis/src/atomic_save.c $(ENGINE_HEADERS)
	mkdir -p tests/stun_engine
	$(CC) $(TEST_CPPFLAGS) -DMERC $(CFLAGS) $(TEST_FLAGS) -DATOMIC_SAVE_TESTING -c $< -o $@

sanitize:
	$(MAKE) clean
	$(MAKE) test CFLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" LDFLAGS="-fsanitize=address,undefined"

clean:
	$(MAKE) -C Genesis/src clean
	rm -f $(TESTS) tests/buffers_interp.o $(ENGINE_OBJECTS) $(LEGACY_CLASS_OBJECTS) tests/stun_comm.o tests/restored_fight.o
