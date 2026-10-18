"""Structural catalog checks; complements, not replaces, actual-function tests."""
import collections
import pathlib
import re
import unittest
import zipfile

SRC = pathlib.Path(__file__).resolve().parents[1] / "Genesis" / "src"

def class_sources():
    return [SRC / path for path in re.findall(
        r"classes/[\w/]+\.c", (SRC / "classes/modules.mk").read_text())]


def engine_sources():
    return [*SRC.glob("*.c"), *class_sources()]

def bundled_objects(path):
    """Read object records only; colored descriptions also begin with '#'.
    This is a reference archive, not evidence of deployed area contents."""
    with zipfile.ZipFile(SRC.parents[1] / "Genesis.zip") as archive:
        text = archive.read("Genesis/area/" + path).decode("latin1")
    section = text.split("#OBJECTS", 1)[1]
    result = {}
    for match in re.finditer(r"^#(\d+)\s*\n(.*?)(?=^#\d+\s*$|\Z)",
                             section, re.M | re.S):
        vnum = int(match[1])
        if not vnum:
            break
        data = match[2].split("~", 4)[4].strip().splitlines()
        result[vnum] = (int(data[0].split()[0]),
                        [int(value) for value in data[1].split()])
    return result


class CatalogChecks(unittest.TestCase):
    def test_drow_ammunition_constants_match_existing_world_templates(self):
        header = (SRC / "merc.h").read_text()
        expected = {"OBJ_VNUM_DROW_CROSSBOW": (29658, 31),
                    "OBJ_VNUM_DROW_BOLT": (29659, 32)}
        relics = bundled_objects("relic.are")
        for name, (vnum, kind) in expected.items():
            match = re.search(r"^#define\s+" + name + r"\s+(\d+)\s*$", header, re.M)
            self.assertIsNotNone(match, name)
            self.assertEqual(int(match[1]), vnum)
            self.assertEqual(relics[vnum][0], kind)
        self.assertEqual(relics[29658][1][3], relics[29659][1][3])

    def test_class_crafting_templates_exist_with_expected_types(self):
        relics = bundled_objects("relic.are")
        weapons = {29621, 29639, 29650}
        vnums = {
            29600, 29601, 29602, 29603, 29610, 29611, 29612, 29613, 29614,
            29620, 29621, 29630, 29631, 29632, 29639, 29640, 29641, 29642,
            29650, 29652, 29653, 29654, 29655, 29658, 29660, 29661, 29662,
        }
        self.assertEqual(len(vnums), 27)
        for vnum in vnums:
            with self.subTest(vnum=vnum):
                kind, values = relics[vnum]
                self.assertEqual(kind, 5 if vnum in weapons else 31 if vnum == 29658 else 9)
                self.assertEqual(len(values), 4)
                if kind == 5:
                    self.assertGreaterEqual(values[1], 0)
                    self.assertGreaterEqual(values[2], values[1])

    def test_sorcerer_material_and_drow_ammo_reference_templates(self):
        self.assertIn(30037, bundled_objects("kavir.are"))
        relics = bundled_objects("relic.are")
        kind, missile = relics[29658]
        ammo_kind, ammo = bundled_objects("galaxy.are")[missile[0]]
        self.assertEqual((kind, ammo_kind, relics[29659][0]), (31, 32, 32))
        self.assertEqual(missile[3], ammo[3])
        self.assertEqual(missile[3], relics[29659][1][3])

    def test_wolf_gear_no_longer_advertises_unimplemented_recipes(self):
        body = (SRC / "random.c").read_text().split("void do_wolfarmor(", 1)[1]
        body = body.split("\nvoid ", 1)[0]
        self.assertIn("Vest, Mask, or Talisman", body)
        for obsolete in ("Klaive", "Necklace", "Snarl", "Scars", "Veil", "Helios"):
            self.assertNotIn(obsolete, body)

    def test_displayed_vampire_commands_dispatch_to_the_right_handler(self):
        table = (SRC / "interp.c").read_text().split("social_table")[0]
        commands = re.findall(r'\{\s*"([^"]+)"\s*,\s*(do_\w+)', table)
        expected = {
            "songserenity": "do_serenity", "beastwithin": "do_beastwithin",
            "frenzytrigger": "do_frenzy", "horrid": "do_horrid",
            "empathy": "do_empathy", "malleable": "do_malleable",
            "fist": "do_fistofcaine", "possession": "do_possession",
            "spiriteyes": "do_spiriteyes", "preserve": "do_preservation",
            "spiritgate": "do_spiritgate", "spiritguardian": "do_spiritguardian",
            "zombie": "do_zombie", "shield": "do_shield", "leech": "do_leechblood",
        }
        for name, handler in expected.items():
            selected = next(fn for command, fn in commands if command.startswith(name))
            self.assertEqual(selected, handler, name)

    def test_disabled_gifts_are_not_advertised_as_working(self):
        source = (SRC / "classes/werewolf/werewolf.c").read_text()
        for label in ("Quicksand[disabled]", "Lycaon[disabled]"):
            self.assertTrue(label in source, f"missing explicit disabled label: {label}")

    def test_skill_names_are_unique(self):
        source = (SRC / "const.c").read_text().split("skill_table", 1)[1]
        names = re.findall(r'\{\s*"([^"]+)"\s*,', source)
        self.assertEqual(len(names), len(set(name.lower() for name in names)))

    def test_empty_legacy_spells_cannot_be_cast_by_players(self):
        magic = (SRC / "magic.c").read_text()
        magic = re.sub(r"/\*.*?\*/|//[^\n]*", "", magic, flags=re.S)
        empty = set(re.findall(
            r"void\s+(spell_\w+)\s*\([^;{}]*\)\s*\{\s*(?:return;\s*)?\}", magic))
        table = (SRC / "const.c").read_text().split("skill_table", 1)[1]
        entries = re.findall(
            r'\{\s*"([^"]+)"\s*,\s*(\w+)\s*,\s*(spell_\w+)\s*,', table)
        checked = 0
        for name, level, handler in entries:
            if handler in empty:
                self.assertEqual(level, "99", f"empty but castable spell: {name}")
                checked += 1
        self.assertEqual(checked, 17)

    def test_basic_buffs_keep_historical_slots_and_gated_strength(self):
        table = (SRC / "const.c").read_text().split("skill_table", 1)[1]
        pattern = (
            r'\{\s*"([^"]+)"\s*,\s*(\w+)\s*,\s*(spell_\w+)\s*,'
            r'\s*(TAR_\w+)\s*,\s*(POS_\w+)\s*,\s*(?:NULL|&\w+)\s*,'
            r'\s*SLOT\(\s*(\d+)\s*\)\s*,\s*(\d+)\s*,\s*(\d+)')
        rows = {r[0]: r[1:] for r in re.findall(pattern, table)}
        expected = {
            "armor": ("spell_armor", "TAR_CHAR_DEFENSIVE", "1", "5", "12"),
            "bless": ("spell_bless", "TAR_CHAR_DEFENSIVE", "3", "5", "12"),
            "shield": ("spell_shield", "TAR_CHAR_DEFENSIVE", "67", "12", "18"),
            "stone skin": ("spell_stone_skin", "TAR_CHAR_SELF", "66", "12", "18"),
        }
        for name, (handler, target, slot, mana, beats) in expected.items():
            self.assertEqual(rows[name], ("LEV_AWA", handler, target,
                                         "POS_STANDING", slot, mana, beats))
        for name in ("giant strength", "weaken", "refresh", "frenzy", "darkblessing"):
            self.assertEqual(rows[name][0], "99", name)

    def test_literal_class_world_references(self):
        # Read only the area files named by area.lst, never account archives.
        kinds = {"obj": "OBJECTS", "mob": "MOBILES", "room": "ROOMS"}
        world = {kind: set() for kind in kinds.values()}
        with zipfile.ZipFile(SRC.parents[1] / "Genesis.zip") as archive:
            loaded = set(re.findall(r"[^\s$]+\.are",
                archive.read("Genesis/area/area.lst").decode("latin1")))
            for name in archive.namelist():
                if not name.startswith("Genesis/area/") or pathlib.PurePosixPath(name).name not in loaded:
                    continue
                text = archive.read(name).decode("latin1")
                for kind, body in re.findall(
                    r"^#(OBJECTS|MOBILES|ROOMS)\s*\n(.*?)(?=^#[A-Z$]|\Z)",
                    text, re.M | re.S):
                    world[kind].update(int(n) for n in re.findall(r"^#(\d+)\s*$", body, re.M))
        header = (SRC / "merc.h").read_text()
        constants = {name: int(value) for name, value in re.findall(
            r"^#define\s+(\w+)\s+\(?([0-9]+)\)?\s*(?:/\*.*)?$", header, re.M)}
        self.assertIn(constants["ROOM_VNUM_TEMPLE"], world["ROOMS"])
        self.assertIn(constants["ROOM_VNUM_IN_OBJECT"], world["ROOMS"])
        files = [SRC / "clan.c", SRC / "more.c", *class_sources()]
        for path in files:
            for kind, name in re.findall(r"get_(obj|mob|room)_index\(\s*(\w+)\s*\)",
                                        path.read_text()):
                value = int(name) if name.isdigit() else constants.get(name)
                # Optional Hell is handled by the engine-tested temple fallback.
                if value is not None and name != "ROOM_VNUM_HELL":
                    self.assertTrue(value in world[kinds[kind]], f"{path}: missing {kind} {name}")

    def test_vampire_port_keeps_ids_and_registers_real_powers(self):
        header = (SRC / "merc.h").read_text()
        for name, value in (("THAUMATURGY", 17), ("VICISSITUDE", 18),
                            ("NECROMANCY", 7), ("OBFUSCATE", 9)):
            self.assertRegex(header, rf"#define DISC_{name}\s+{value}\b")
        source = (SRC / "classes/vampire/vampire.c").read_text()
        self.assertNotIn("blood[2]", source)
        interp = (SRC / "interp.c").read_text()
        for name in ("tasteblood", "bloodrage", "potency", "theftvitae", "cauldronblood"):
            self.assertRegex(interp, rf'\{{\s*"{name}"\s*,\s*do_{name}\b')
        const = (SRC / "const.c").read_text()
        self.assertLess(const.index('"mage invis",'), const.index('"blood potency",'))
        clan = (SRC / "clan.c").read_text()
        self.assertNotIn("cland[3]", clan)
        self.assertNotIn('Thaumaturgy: Paths', clan)

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
        source = "\n".join(path.read_text() for path in engine_sources())
        definitions = set(re.findall(r"void\s+(do_\w+)\s*\(", source))
        self.assertEqual({fn for _, fn in commands} - definitions, set())
        self.assertIn(("ward", "do_paradox_ward"), commands)
        self.assertIn(("spiritward", "do_spiritward"), commands)

    def test_literal_skill_references_resolve(self):
        source = (SRC / "const.c").read_text().split("skill_table", 1)[1]
        names = re.findall(r'\{\s*"([^"]+)"\s*,', source)
        self.assertGreater(len(names), 100)
        checked = 0
        for path in engine_sources():
            for literal in re.findall(r'skill_lookup\(\s*"([^"]+)"\s*\)', path.read_text()):
                # Production skill_lookup permits prefixes, e.g. blind -> blindness.
                self.assertTrue(any(name.lower().startswith(literal.lower()) for name in names),
                                f"{path.name}: unresolved skill {literal!r}")
                checked += 1
        self.assertGreater(checked, 20)


if __name__ == "__main__":
    unittest.main()
