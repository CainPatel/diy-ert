# Analysis

Inversion of survey output with [pyGIMLi](https://www.pygimli.org/), and
a helper for turning a raw serial capture into a data file.

## Install and run

pyGIMLi installs cleanly through conda:

```
conda create -n ert -c gimli -c conda-forge pygimli
conda activate ert
python invert_pygimli.py survey.dat        # lam = 20
python invert_pygimli.py survey.dat 50     # smoother
```

`clean_capture.py` needs only Python:

```
python clean_capture.py capture.txt survey.dat
```

It keeps the data block (from the electrode-count line to the closing
`0`) and drops the banner and command replies that sketch 07 prints
around it. Sketch 06 prints nothing but the data, so its capture can be
used directly.

## Data format

Firmware sketches 06 and 07 emit pyGIMLi's unified data format directly:

```
12
# x z
0.00 0.0
0.50 0.0
...
18
# a b m n rhoa
1 4 2 3 72.196
...
0
```

- The header block declares the electrode positions in metres (12
  electrodes at 0.5 m spacing for the breadboard build's working
  channels; 16 by default on the PCB).
- Each data row is `a b m n rhoa`. **a/b are the current electrodes, m/n
  are the potential pair**, and electrode numbers are **1-indexed**. An
  off-by-one here does not error out; it silently produces a wrong but
  plausible-looking model.
- `rhoa` is apparent resistivity in Ω·m, computed in the firmware as
  2πa·R from the measured V and I.
- The trailing `0` is the empty topography block.

## Regularisation

The surveys are small: 12 working electrodes give 9 + 6 + 3 = 18 Wenner
readings across three spacings, and 16 electrodes give 35 across five.
With that little data the regularisation parameter `lam` matters a great
deal. Too low and the inversion fits noise, producing structure that is
not in the ground. The default is `lam=20`. A useful habit with data
this sparse: treat any feature that disappears when you double `lam` as
an artefact.

## Status

The pipeline is written against the firmware's output format but has not
been run on a real 2D field dataset. The instrument so far has one
validated single point field reading, not a survey. See the
[repository README](../README.md).
