"""Minimal pyGIMLi inversion for the DIY-ERT Wenner survey output.

Usage:
    python invert_pygimli.py [survey.dat]

The input is the verbatim serial capture from firmware sketch 06, in
pyGIMLi's unified data format. See README.md in this directory for the
exact layout and the 1-indexing caveat.
"""

import sys

import pygimli as pg
from pygimli.physics import ert

filename = sys.argv[1] if len(sys.argv) > 1 else "survey.dat"

data = ert.load(filename)
mgr = ert.ERTManager(data)

# High lambda: with only ~18 measurements the problem is badly
# underdetermined: too low a lambda and the inversion fits noise,
# drawing structure that is not in the ground. Keep the model smooth.
mgr.invert(lam=20)
mgr.showResult()
pg.wait()
