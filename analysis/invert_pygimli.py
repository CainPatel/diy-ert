"""Minimal pyGIMLi inversion for the diy-ert Wenner survey output.

Usage:
    python invert_pygimli.py [survey.dat] [lam]

The input is the data file printed by firmware sketch 06 or 07, in
pyGIMLi's unified data format. See README.md in this directory for the
exact layout and the 1-indexing caveat.
"""

import sys

import pygimli as pg
from pygimli.physics import ert

filename = sys.argv[1] if len(sys.argv) > 1 else "survey.dat"

# High lambda: with only 18-35 measurements the problem is badly
# underdetermined. Too low a lambda and the inversion fits noise, drawing
# structure that is not in the ground. Keep the model smooth.
lam = float(sys.argv[2]) if len(sys.argv) > 2 else 20.0

data = ert.load(filename)
mgr = ert.ERTManager(data)
mgr.invert(lam=lam)
mgr.showResult()
pg.wait()
