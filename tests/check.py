#!/usr/bin/env python3
"""Run from any directory. Requires only Python 3 and the C build tools."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


def run_game(binary, directory, text):
    result = subprocess.run([str(binary)], cwd=directory, input=text,
                            text=True, capture_output=True, timeout=15)
    if result.returncode != 0:
        raise AssertionError(result.stdout + result.stderr)
    return result.stdout


class PortChecks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        subprocess.run(["make", "-s", "all"], cwd=ROOT, check=True)
        cls.temp = tempfile.TemporaryDirectory(prefix="trek-check-")
        cls.harness = Path(cls.temp.name) / "save-check"
        subprocess.run([os.environ.get("CC", "cc"), "-std=c11",
                        "-D_DARWIN_C_SOURCE", "-D_DEFAULT_SOURCE",
                        "-include", str(ROOT / "src/compat.h"),
                        "-I", str(ROOT / "src"),
                        str(ROOT / "tests/save_roundtrip.c"),
                        str(ROOT / "src/dumpgame.c"),
                        str(ROOT / "src/externs.c"),
                        "-o", str(cls.harness)], check=True)

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def harness_run(self, directory, mode):
        subprocess.run([str(self.harness), mode], cwd=directory,
                       check=True, capture_output=True, timeout=10)

    def test_save_in_separate_process_and_reject_bad_files(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.harness_run(tmp, "save")
            for _ in range(3):
                self.harness_run(tmp, "load")
            save = Path(tmp) / "trek.dump"
            good = save.read_bytes()
            for data in (good[:16], b"badmagic" + good[8:], good + b"x"):
                save.write_bytes(data)
                self.harness_run(tmp, "reject")
            save.unlink()
            self.harness_run(tmp, "reject")

    def test_game_commands_and_restart(self):
        with tempfile.TemporaryDirectory() as tmp:
            binary = ROOT / "bin/trek"
            out = run_game(binary, tmp,
                "\nshort\nnovice\ntournament\nmacos\n"
                "status\nlrscan\ncomputer chart\ncomputer trajectory\n"
                "damages\nwarp 3\nshields down\nmove 90 0.1\n"
                "impulse 90 0.1\nrest 0.05\ntorpedo 0 no\n"
                "status\ndump\nterminate\nno\n")
            for marker in ("S T A R", "Klingons", "Long range scan",
                           "Computer record of galaxy", "All devices functional",
                           "warp factor   3.0", "torpedoes     9",
                           "shields       down", "Game saved to trek.dump"):
                self.assertIn(marker, out)
            restored = run_game(binary, tmp,
                                "\nrestart\nstatus\nterminate\nno\n")
            for marker in ("Game restored from trek.dump", "warp factor   3.0",
                           "torpedoes     9", "shields       down"):
                self.assertIn(marker, restored)


if __name__ == "__main__":
    unittest.main(verbosity=2)
