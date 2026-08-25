# Analysis

Inversion of survey output with [pyGIMLi](https://www.pygimli.org/).

## Install and run

pyGIMLi installs cleanly through conda:

```
conda create -n ert -c gimli -c conda-forge pygimli
conda activate ert
python invert_pygimli.py survey.dat
```

## Data format

Firmware sketch 06 emits pyGIMLi's unified data format directly. Capture
the serial output verbatim into `survey.dat`:

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
  electrodes at 0.5 m spacing for the current build).
- Each data row is `a b m n rhoa`. **a/b are the current electrodes, m/n
  are the potential pair**, and electrode numbers are **1-indexed**. An
  off-by-one here does not error out; it silently produces a wrong but
  plausible-looking model.
- `rhoa` is apparent resistivity in Ω·m, computed in the firmware as
  2πa·R from the measured V and I.
- The trailing `0` is the empty topography block.

## Regularisation

With only ~18 measurements (12 working electrodes gives 9 + 6 + 3 Wenner
readings across three spacings) the regularisation parameter `lam`
matters a great deal. Too low and the inversion fits noise, producing
structure that is not in the ground. The script uses `lam=20` to keep
the model smooth. A useful habit with data this sparse: treat any
feature that disappears when you double `lam` as an artefact.

## Status

The pipeline is written against sketch 06's output format but has not
been run on a real 2D field dataset. The instrument so far has one
validated single point field reading, not a survey. See the
[repository README](../README.md).
