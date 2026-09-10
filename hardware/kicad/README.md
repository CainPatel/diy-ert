# KiCad project

KiCad 10.0.5 sources for the diy-ert board.

| File | What |
|---|---|
| `diy-ert.kicad_pro` | project settings, design rules, net classes |
| `diy-ert.kicad_sch` | schematic, one A4 sheet |
| `diy-ert.kicad_pcb` | board, 2 layers, 127.05 x 117.55 mm |

Open `diy-ert.kicad_pro` in KiCad 10 or newer. Every symbol and footprint
comes from the standard KiCad libraries, so nothing else needs installing.
The `.kicad_prl` file (per-machine view settings) and the `.history/`
backup directory that KiCad creates are ignored by git.

The project was drawn under the default name `Untitled` and renamed here;
the rename touched only the project-name strings in the files, and the
netlist, ERC, DRC, and gerbers exported from the renamed project match the
originals.

## Regenerating the exports

Everything in the parent directory (schematic PDF, board renders, BOM,
reports, gerbers) is produced from these sources with `kicad-cli`:

```
kicad-cli sch export pdf -o ../schematic.pdf diy-ert.kicad_sch
kicad-cli sch export bom -o ../bom.csv --fields "Reference,Value,Footprint,\${QUANTITY}" --group-by "Value,Footprint" diy-ert.kicad_sch
kicad-cli sch erc -o ../reports/erc.rpt --severity-all diy-ert.kicad_sch
kicad-cli pcb drc -o ../reports/drc.rpt --severity-all diy-ert.kicad_pcb
kicad-cli pcb render -o ../images/pcb-top.png --side top --width 1500 --height 1400 diy-ert.kicad_pcb
kicad-cli pcb render -o ../images/pcb-bottom.png --side bottom --width 1500 --height 1400 diy-ert.kicad_pcb
kicad-cli pcb export gerbers -o ../gerbers/ --no-protel-ext --layers "F.Cu,B.Cu,F.SilkS,B.SilkS,F.Mask,B.Mask,Edge.Cuts" diy-ert.kicad_pcb
kicad-cli pcb export drill -o ../gerbers/ diy-ert.kicad_pcb
```

On macOS `kicad-cli` lives at
`/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli`.
