"""Structural catalog checks; complements, not replaces, actual-function tests."""
import collections
import pathlib
import re
import unittest

SRC = pathlib.Path(__file__).resolve().parents[1] / "Genesis" / "src"


class CatalogChecks(unittest.TestCase):
    def test_stun_is_wired_into_all_action_paths(self):
        comm = (SRC / "comm.c").read_text()
        self.assertEqual(comm.count("combat_status_update();"), 2)
        self.assertEqual(comm.count("IS_COMBAT_PAUSED(d->character)"), 2)
        fight = (SRC / "fight.c").read_text()
        for name in ("violence_update", "multi_hit", "one_hit"):
            body = fight.split("void " + name + "(", 1)[1].split("\n}", 1)[0]
            self.assertIn("IS_COMBAT_PAUSED(ch)", body)
        interp = (SRC / "interp.c").read_text().split("void interpret(", 1)[1]
        self.assertLess(interp.index("IS_COMBAT_PAUSED(ch)"),
                        interp.index("one_argument("))
        updates = (SRC / "update.c").read_text()
        mobile = updates.split("void mobile_update(", 1)[1].split("void weather_update", 1)[0]
        self.assertLess(mobile.index("IS_COMBAT_PAUSED(ch)"), mobile.index("(*ch->spec_fun)"))
        self.assertIn("||   IS_COMBAT_PAUSED(ch)", updates)

    def test_commands_are_unique_and_implemented(self):
        table = (SRC / "interp.c").read_text().split("social_table")[0]
        commands = re.findall(r'\{\s*"([^"]+)"\s*,\s*(do_\w+)', table)
        self.assertGreater(len(commands), 500)
        counts = collections.Counter(name for name, _ in commands)
        self.assertEqual([name for name, count in counts.items() if count > 1], [])
        source = "\n".join(path.read_text() for path in SRC.glob("*.c"))
        definitions = set(re.findall(r"void\s+(do_\w+)\s*\(", source))
        self.assertEqual({fn for _, fn in commands} - definitions, set())
        self.assertIn(("ward", "do_paradox_ward"), commands)
        self.assertIn(("spiritward", "do_spiritward"), commands)

    def test_literal_skill_references_resolve(self):
        source = (SRC / "const.c").read_text().split("skill_table", 1)[1]
        names = re.findall(r'\{\s*"([^"]+)"\s*,', source)
        self.assertGreater(len(names), 100)
        checked = 0
        for path in SRC.glob("*.c"):
            for literal in re.findall(r'skill_lookup\(\s*"([^"]+)"\s*\)', path.read_text()):
                # Production skill_lookup permits prefixes, e.g. blind -> blindness.
                self.assertTrue(any(name.lower().startswith(literal.lower()) for name in names),
                                f"{path.name}: unresolved skill {literal!r}")
                checked += 1
        self.assertGreater(checked, 20)


if __name__ == "__main__":
    unittest.main()
