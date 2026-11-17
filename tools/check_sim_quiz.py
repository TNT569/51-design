"""Exercise the LCD quiz in uCsim with no XRAM chip attached.

Build with `make sim` and run this script with `s51` on PATH.
"""

import re
import subprocess
from pathlib import Path


def main():
    image = Path(__file__).resolve().parents[1] / "build" / "sim" / "main.hex"
    listing = image.with_suffix(".rst").read_text(encoding="utf-8")
    line = int(re.search(r"^\s*([0-9A-F]+) +\d+ _line:$", listing, re.M)[1], 16)
    word_row, result_row = re.findall(
        r"lcd_print\(1, line\);\n(?:[^\n]*\n)*?^\s*([0-9A-F]+) [^\n]*\blcall\s+_lcd_print$",
        listing,
        re.M,
    )

    # Browse ahead and back, then resume the same first question.
    commands = ["memory remove chip xram_chip", "step 60000"]
    expected = []

    def press(key, row=None, *, long=False):
        pins = 0xFF ^ (1 << (key - 1))
        commands.extend([f"set hardware port[1] {pins:#x}",
                         f"step {600000 if long else 28000}",
                         "set hardware port[1] 0xff"])
        if row is not None:
            address = result_row if row.startswith("Total:") else word_row
            commands.extend([f"break 0x{address}", "step 28000",
                             f"dump iram {line:#x} {line + 15:#x} 16",
                             f"clear 0x{address}"])
            expected.append(row.ljust(16))
        commands.append("step 28000")

    press(3, "01/12 abandon")
    press(3, "01/12 abandon")
    press(2, "02/12 accurate")
    press(2, "03/12 benefit")
    press(1, "02/12 accurate")
    press(3, "01/12 abandon")
    words = ["accurate", "benefit", "brief", "contrast", "eager", "fragile",
             "habit", "journey", "observe", "retain", "vivid"]
    for position, word in enumerate(words, start=2):
        press(2 if position == 2 else 1, f"{position:02}/12 {word}")
    press(1, "Total:12 Know:11")
    press(2, "01/12 abandon")
    press(3, "Total:12 Know:11")
    press(3, long=True)
    press(2, "01/01 abandon")
    press(1, "Total:01 Know:01")
    commands.append("quit")

    run = subprocess.run(
        ["s51", "-q", "-t", "C52", "-X", "11059200", "-c", "-", str(image)],
        input="\n".join(commands) + "\n", capture_output=True, text=True,
        check=True, timeout=20,
    )
    dumps = re.findall(rf"0x0*{line:x}[ \t]+((?:[0-9a-f]{{2}} ){{15}}[0-9a-f]{{2}})",
                       run.stdout)
    actual = [bytes.fromhex(dump).decode("ascii", errors="replace") for dump in dumps]
    if actual != expected:
        raise SystemExit(f"Expected LCD rows {expected!r}\nReceived LCD rows {actual!r}")
    print(f"Verified {len(actual)} LCD rows without XRAM, including all 12 words and wrong-word retry.")


if __name__ == "__main__":
    main()
