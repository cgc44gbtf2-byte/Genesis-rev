CC ?= cc
LDLIBS ?= -lcrypt
TEST_FLAGS = -std=gnu17 -O1 -g -ffunction-sections -fdata-sections
TEST_LDFLAGS = -Wl,--gc-sections
TESTS = tests/test_comm tests/test_db tests/test_gameplay

.PHONY: all test sanitize clean
all:
	$(MAKE) -C Genesis/src

tests/%: tests/%.c $(wildcard Genesis/src/*.c Genesis/src/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

tests/test_gameplay: tests/test_gameplay.c $(wildcard Genesis/src/*.c Genesis/src/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_FLAGS) $< Genesis/src/act_info.c Genesis/src/wwauspice.c Genesis/src/mage.c $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) -o $@

test: $(TESTS)
	@set -e; for t in $(TESTS); do ./$$t; done

sanitize:
	$(MAKE) clean
	$(MAKE) test CFLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" LDFLAGS="-fsanitize=address,undefined"

clean:
	$(MAKE) -C Genesis/src clean
	rm -f $(TESTS)
