# Extraction of Robust Commands from SSVEP Signals for Use in Brain–Computer Interfaces

**Author:** Farooq Danesh Amooz
**Department:** Electrical and Computer Engineering, University of Tabriz
**Supervisor:** Prof. Hadi Seyedarabi

## What this project is

This project builds a physical five-key brain–computer interface (BCI) control panel for
wheelchair control (Forward, Backward, Left, Right, STOP), based on steady-state visually
evoked potentials (SSVEP) — a brain signal that appears when a person looks at something
flickering at a fixed rate. The full literature review, method, and proposal are in
[`report/ssvep_report_full.pdf`](report/ssvep_report_full.pdf).

The report argues that "robustness" in SSVEP-BCI has three separate parts, and proposes one
concrete, testable mechanism for each:

| Dimension | What can go wrong | Proposed mechanism |
|---|---|---|
| **Within-trial noise** | Not enough signal, too little time to decide | **H1** — a second, color-based cue added to the safety-critical STOP key, on top of its flicker frequency |
| **Cross-subject / low-calibration** | A new user, or signal drift during a session | **H2** — the classifier updates itself, trial by trial, as the session goes on |
| **Adversarial robustness** | A crafted signal could trick the system | **H3** (stretch goal) — replacing STOP's fixed-frequency flicker with a broadband, code-modulated (c-VEP) signal |

A five-key hardware panel (STM32F103C8T6 microcontroller) was built to support both SSVEP mode
and c-VEP mode. A first single-subject pilot recording validated the SSVEP decoding pipeline;
H1–H3 have not yet been tested under controlled conditions — see Section 8 of the report for
exactly what is done and what remains.

## How to navigate this repository

| Folder | Contents |
|---|---|
| [`literature/`](literature/) | Six-field summaries of ~20 papers, and the project bibliography (`refs.bib`) |
| [`hardware/`](hardware/) | Firmware source, circuit schematic, photos, and bill of materials |
| [`data/`](data/) | EEG recordings — see `data/README.md` for access and consent status **before** looking here |
| [`analysis/`](analysis/) | The Python/Jupyter signal-processing pipeline (SSVEP + c-VEP decoding) |
| [`report/`](report/) | The full report, in LaTeX source and compiled PDF |

## Reproducing the analysis

```bash
git clone https://github.com/<your-username>/ssvep-robust-bci.git
cd ssvep-robust-bci/analysis
python -m venv venv
source venv/bin/activate        # Windows: venv\Scripts\activate
pip install -r requirements.txt
jupyter notebook SSVEP_cVEP_Analysis.ipynb
```

Note: the notebook expects `.edf` files in `data/raw/`. If that folder is empty in this
repository, see [`data/README.md`](data/README.md) for why, and how to request access.

## License

- **Code** (firmware, analysis notebook): MIT — see [`LICENSE`](LICENSE).
- **Hardware designs** (schematic, PCB, BOM): CERN-OHL-P v2 — see [`hardware/LICENSE`](hardware/LICENSE).
- **Data**: not automatically covered by the above. See [`data/README.md`](data/README.md) and
  [`data/CONSENT.md`](data/CONSENT.md).
- **Report text**: © Farooq Danesh Amooz. Shared here for transparency and review; please
  contact the author before reuse beyond reading/citing.

## Citation

If you use this work, please cite it — see [`CITATION.cff`](CITATION.cff), or use the
"Cite this repository" button on the GitHub page.

## Status

🚧 Active undergraduate thesis project. H1–H3 validation experiments are the current next step.
