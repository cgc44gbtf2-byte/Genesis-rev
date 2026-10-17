"""Structural ownership/build checks, not a gameplay-equivalence proof."""
import collections
import pathlib
import re
import subprocess
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
SRC = ROOT / "Genesis" / "src"
EXPECTED = {
    "vampire": ("disc", "vampire", "presence", "beast"),
    "werewolf": ("werewolf", "wwbreed", "wwauspice", "wwtribe", "instincts"),
    "sorcerer": ("sorcerer",),
    "drow": ("drow",),
    "highlander": ("highland",),
    "mage": ("mage",),
    "dragon": ("dragon",),
    "ninja": ("ninja",),
    "monk": ("monk",),
    "elemental": ("elemental",),
}
IDS = {"NONE": 0, "VAMPIRE": 1, "WEREWOLF": 2, "SORCERER": 4,
       "DROW": 8, "HIGHLANDER": 16, "MAGE": 32, "DRAGON": 64,
       "NINJA": 128, "MONK": 256, "ELEM": 512}


def manifest():
    return re.findall(r"classes/[\w/]+\.c",
                      (SRC / "classes/modules.mk").read_text())


def make_values(directory, expression):
    # Inspect the real make expansion, without building or running the engine.
    result = subprocess.run(
        ["make", "-s", "--no-print-directory", "-f", "Makefile", "-f", "-",
         "print-class-layout"],
        input='print-class-layout:\n\t@printf "%s\\n" $(' + expression + ')\n',
        text=True, cwd=directory, capture_output=True, check=True,
    )
    return result.stdout.splitlines()


class ClassLayoutChecks(unittest.TestCase):
    def test_all_ten_classes_have_explicit_owned_sources(self):
        expected = {f"classes/{family}/{stem}.c"
                    for family, stems in EXPECTED.items() for stem in stems}
        selected = manifest()
        self.assertEqual(len(selected), 17)
        self.assertEqual(len(selected), len(set(selected)))
        self.assertEqual(set(selected), expected)
        self.assertEqual(
            {str(p.relative_to(SRC)) for p in (SRC / "classes").rglob("*.c")},
            expected,
        )

    def test_old_root_duplicates_are_absent(self):
        for stems in EXPECTED.values():
            for stem in stems:
                self.assertFalse((SRC / (stem + ".c")).exists(), stem)
        self.assertFalse((SRC / "mage.h").exists())

    def test_serialized_class_ids_are_unchanged(self):
        source = (SRC / "classes/class_ids.h").read_text()
        ids = {name: int(value) for name, value in re.findall(
            r"^#define\s+CLASS_(\w+)\s+(\d+)\s*$", source, re.M)}
        self.assertEqual(ids, IDS)
        self.assertFalse(re.search(r"^#define\s+CLASS_\w+",
                                   (SRC / "merc.h").read_text(), re.M))

    def test_identity_is_still_exact_not_ancestry(self):
        header = (SRC / "merc.h").read_text()
        for name in IDS.keys() - {"NONE"}:
            self.assertRegex(
                header, r"#define\s+IS_" + name + r"\(ch\)\s+"
                r"\(\(ch\)->class == CLASS_" + name + r" && !IS_NPC\(ch\)\)")

    def test_production_and_full_engine_tests_link_the_same_sources(self):
        production = make_values(SRC, "O_FILES:.o=.c")
        fixtures = make_values(ROOT, "ENGINE_SOURCES")
        fixture_paths = [str(pathlib.Path(p).relative_to("Genesis/src"))
                         for p in fixtures]
        self.assertEqual(len(production), len(set(production)))
        self.assertEqual(len(fixture_paths), len(set(fixture_paths)))
        self.assertEqual(set(production), set(fixture_paths))
        self.assertEqual(set(production), {
            str(p.relative_to(SRC)) for p in SRC.glob("*.c")
        } | set(manifest()))
        self.assertTrue(all((SRC / p).is_file() for p in production))

    def test_headers_and_both_builds_use_the_owned_layout(self):
        header = (SRC / "merc.h").read_text()
        self.assertIn('#include "classes/class_ids.h"', header)
        self.assertIn('#include "classes/mage/mage.h"', header)
        self.assertTrue((SRC / "classes/mage/mage.h").is_file())
        self.assertIn("include classes/modules.mk", (SRC / "Makefile").read_text())
        self.assertIn("include Genesis/src/classes/modules.mk",
                      (ROOT / "Makefile").read_text())

    def test_public_class_commands_have_one_definition(self):
        owners = collections.defaultdict(list)
        for source in manifest():
            text = (SRC / source).read_text()
            text = re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)
            for handler in re.findall(
                    r"(?m)^void\s+(do_\w+)\s*\([^;{}]*\)\s*\{", text):
                owners[handler].append(source)
        self.assertGreater(len(owners), 100)
        self.assertEqual({fn: paths for fn, paths in owners.items()
                          if len(paths) != 1}, {})

    def test_clean_includes_pre_layout_build_artifacts(self):
        result = subprocess.run(
            ["make", "-n", "--no-print-directory", "clean"], cwd=ROOT,
            text=True, capture_output=True, check=True)
        removed = {item for line in result.stdout.splitlines()
                   if line.startswith("rm -f ") for item in line.split()[2:]}
        for source in manifest():
            stem = pathlib.Path(source).stem
            self.assertTrue({stem + ".o", stem + ".d",
                             f"tests/stun_engine/{stem}.o"} <= removed, stem)
        self.assertFalse(any(p.endswith((".c", ".h")) for p in removed))

    def test_fixtures_do_not_include_removed_source_paths(self):
        old_paths = [f'../Genesis/src/{stem}.c'
                     for stems in EXPECTED.values() for stem in stems]
        for source in (ROOT / "tests").glob("*.c"):
            includes = re.findall(r'#include\s+"([^"]+)"', source.read_text())
            self.assertFalse(set(includes) & set(old_paths), source.name)

    def test_extracted_and_shared_handlers_keep_their_owners(self):
        owners = collections.defaultdict(list)
        for path in [*SRC.glob("*.c"), *(SRC / p for p in manifest())]:
            source = re.sub(r"/\*.*?\*/|//[^\n]*", "", path.read_text(), flags=re.S)
            for fn in re.findall(r"(?m)^void\s+(do_\w+)\s*\([^;{}]*\)\s*\{", source):
                owners[fn].append(str(path.relative_to(SRC)))
        expected = {
            "do_majesty": "classes/vampire/presence.c",
            "do_awe": "classes/vampire/presence.c",
            "do_humanity": "classes/vampire/beast.c",
            "do_beastlike": "classes/vampire/beast.c",
            "do_shadowsight": "classes/werewolf/instincts.c",
            "do_rage": "classes/werewolf/instincts.c",
        }
        for fn, path in expected.items():
            self.assertEqual(owners[fn], [path], fn)
        for fn in ("do_fangs", "do_claws", "do_nightsight", "do_calm",
                   "do_flex", "do_earthmeld", "do_werewolf", "do_unwerewolf"):
            self.assertEqual(owners[fn], ["clan.c"], fn)


if __name__ == "__main__":
    unittest.main()
