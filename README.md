# Godwars: Anathema

Reliability and modernization branch of Genesis (God Wars MUD).
The Git branch is `Godwars-Anathema`: Git does not allow spaces or colons
in branch names.

## Source and historical archive

**Build and edit `Genesis/src/`.** The original `Genesis.zip` is retained
unchanged as a historical snapshot, not the patched distribution. Do not
extract its old source or object files over this tree.

Only source files were unpacked into version control. Historical player
backups, notes, logs, executables and object files were not republished as
individual files. The archive still contains historical runtime data:
do not use its accounts or credentials on a public server.

All original copyright notices are preserved. The source refers to Diku,
Merc and God Wars licensing/permission requirements; the referenced license
documents were not present in the supplied archive. This branch does not
grant new permissions. Resolve those requirements before operating or
redistributing the game.

## Build

Requires a C17-capable GCC-compatible compiler, GNU Make, and libcrypt
development files (for example `build-essential libcrypt-dev` on Ubuntu).

```sh
make -j2
make test
make sanitize
make -j2
```

The executable is `Genesis/src/merc`. `make sanitize` cleans the build and
runs the isolated regression suites under AddressSanitizer and
UndefinedBehaviorSanitizer; rebuild afterwards for normal use.
`LDLIBS` may be overridden for a platform-specific libcrypt path.
Never enable `NOCRYPT` to work around a missing library.

The historical Memwatch allocator is optional: `make clean` followed by
`make MEMWATCH=1`. Do not combine it with sanitizer tests.

For an isolated local runtime, supply the original area `.are` and `.hlp`
files plus `area.lst` under `Genesis/area/`, and create empty sibling
`player`, `backup`, `notes`, and `log` directories. Run `../src/merc 4000`
from `Genesis/area`. Do not import historical accounts. The server listens
on all interfaces by default; restrict access with the host firewall.

## Verified fixes

- GNU17 build, correct library link order, automatic header dependencies,
  and single definitions for leaderboard and extra-description globals.
- Removed alternate/master login credential acceptance and the embedded
  credential. Shared password verification rejects pass-the-hash login and
  handles crypt failures, including account deletion.
- Descriptor reads handle capacity, interruption and would-block correctly.
  Writes preserve pending bytes under backpressure and reject zero progress.
- Output overflow no longer frees a descriptor underneath its caller.
  Trailing color markers respect explicit input lengths; buffers terminate.
  Retried output does not regenerate prompts.
- Empty-string checks inspect string contents rather than pointer addresses.
- `channels +all/-all` no longer applies an uninitialized bit mask.
- Geas uses the charm skill identifier rather than an uninitialized value.
- Friction Curse parses quintessence correctly, initializes and retains
  adjusted spell power, checks mana before consuming quintessence, and
  restores the caster's room after a safe-room refusal.
- Unknown dragon skills return explicitly.
- Command parsing is sequenced and bounded. Removed broken allocation
  diagnostics that read uninitialized locals and could access a character
  after a command freed it.
- Data readers distinguish EOF from byte 255, terminate final unquoted words
  at EOF, reject incomplete quotes/strings, and bound string-pool writes.
  Error reporting no longer hangs on an EOF without a trailing newline.
- Corrected Memwatch neighbor integrity checks and bounded logging copies.

## Verification and remaining scope

Three isolated regression suites cover network/authentication, data readers,
and gameplay fixes. Normal and ASan/UBSan suites passed locally. The complete
server compiled and booted with original area/help data in an isolated
runtime, and returned a TCP greeting. No live player data was loaded.

This is not a claim that the entire legacy game is secure or bug-free.
Compiler warnings still identify legacy formatting/size risks and unused
variables. Password creation still uses legacy DES crypt; a modern,
versioned password migration is needed before public operation. Other
gameplay paths, persistence under disk failure, privileged commands and
long-running multiplayer behavior require further work. Some unflagged
legacy plaintext passwords resembling encoded hashes are deliberately
rejected; affected accounts require an operator-assisted reset.
