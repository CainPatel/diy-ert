"""Extract the pyGIMLi data block from a raw serial capture.

Usage:
    python clean_capture.py capture.txt survey.dat

Sketch 07 prints a banner and command replies around the survey output.
This keeps only the data file: from the electrode-count line that is
immediately followed by "# x z", through the closing "0" line.
"""

import sys

if len(sys.argv) != 3:
    sys.exit("usage: clean_capture.py capture.txt survey.dat")

lines = [ln.rstrip("\r\n") for ln in open(sys.argv[1], errors="replace")]

start = None
for i in range(len(lines) - 1):
    if lines[i].strip().isdigit() and lines[i + 1].strip().startswith("# x"):
        start = i
        break
if start is None:
    sys.exit("no data block found (expected a count line followed by '# x z')")

end = None
for i in range(start + 2, len(lines)):
    if lines[i].strip() == "0":
        end = i
        break
if end is None:
    sys.exit("data block never closed (expected a final '0' line); capture cut short?")

with open(sys.argv[2], "w") as f:
    f.write("\n".join(lines[start:end + 1]) + "\n")

n_data = sum(1 for ln in lines[start:end] if len(ln.split()) == 5 and not ln.startswith("#"))
print(f"wrote {sys.argv[2]}: lines {start + 1}-{end + 1} of the capture, {n_data} data rows")
