CC ?= cc
LDLIBS ?= -lcrypt
TEST_FLAGS = -std=gnu17 -O1 -g -ffunction-sections -fdata-sections
TEST_LDFLAGS = -Wl,--gc-sections
TESTS = tests/test_comm tests/test_db tests/test_gameplay tests/test_buffers tests/test_save tests/test_signals tests/test_classes tests/test_powers tests/test_wall tests/test_mage_powers tests/test_combat_status tests/test_vampire
ENGINE_OBJECTS = $(patsubst Genesis/src/%.c,tests/stun_engine/%.o,$(wildcard Genesis/src/*.c))

.PHONY: all test sanitize clean
all:
	$(MAKE) -C Genesis/src

tests/%: tests/%.c $(wildcard Genesis/src/*.c Genesis/src/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

tests/test_gameplay: tests/test_gameplay.c $(wildcard Genesis/src/*.c Genesis/src/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< Genesis/src/act_info.c Genesis/src/wwauspice.c Genesis/src/mage.c $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

tests/test_buffers: tests/test_buffers.c $(wildcard Genesis/src/*.c Genesis/src/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) --param=asan-globals=0 -c Genesis/src/interp.c -o tests/buffers_interp.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< Genesis/src/string.c tests/buffers_interp.o $(LDFLAGS) $(TEST_LDFLAGS) -o $@

tests/test_signals: tests/test_signals.c $(wildcard Genesis/src/*.c Genesis/src/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< Genesis/src/signals.c $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

tests/test_classes tests/test_wall tests/test_mage_powers: tests/%: tests/%.c $(wildcard Genesis/src/*.c Genesis/src/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) -O0 $< $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

tests/test_powers: tests/test_powers.c $(wildcard Genesis/src/*.c Genesis/src/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< Genesis/src/ninja.c Genesis/src/monk.c Genesis/src/elemental.c Genesis/src/sorcerer.c Genesis/src/drow.c $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

test: $(TESTS)
	@set -e; for t in $(TESTS); do ./$$t; done

# Link the real engine, including actual safety, combat and interpreter paths.
# Only the server entry point is renamed; no real accounts/world files load.
tests/test_combat_status tests/test_vampire: tests/%: tests/%.c $(ENGINE_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< $(ENGINE_OBJECTS) $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

# Separate test objects keep sanitizer flags out of the production executable.
tests/stun_engine/%.o: Genesis/src/%.c $(wildcard Genesis/src/*.h)
	mkdir -p tests/stun_engine
	$(CC) $(CPPFLAGS) -DMERC $(CFLAGS) $(TEST_FLAGS) -c $< -o $@

tests/stun_engine/comm.o: Genesis/src/comm.c $(wildcard Genesis/src/*.h)
	mkdir -p tests/stun_engine
	$(CC) $(CPPFLAGS) -DMERC $(CFLAGS) $(TEST_FLAGS) -Dmain=genesis_server_main -c $< -o $@

tests/stun_engine/atomic_save.o: Genesis/src/atomic_save.c $(wildcard Genesis/src/*.h)
	mkdir -p tests/stun_engine
	$(CC) $(CPPFLAGS) -DMERC $(CFLAGS) $(TEST_FLAGS) -DATOMIC_SAVE_TESTING -c $< -o $@

sanitize:
	$(MAKE) clean
	$(MAKE) test CFLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" LDFLAGS="-fsanitize=address,undefined"

clean:
	$(MAKE) -C Genesis/src clean
	rm -f $(TESTS) tests/buffers_interp.o $(ENGINE_OBJECTS) tests/stun_comm.o
