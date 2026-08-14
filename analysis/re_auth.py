"""
anonymize_edf.py — Strip personal identifiers from .edf files, keeping only
sex and age. Uses only Python's built-in file I/O — no mne, no pyedflib,
no compiler needed.

EDF header layout (fixed, plain ASCII, per the EDF/EDF+ specification):
  bytes   0-7   : version
  bytes   8-87  : local patient identification   (80 bytes)  <- we edit this
  bytes  88-167 : local recording identification (80 bytes)  <- we edit this
  bytes 168-175 : recording start date
  bytes 176-183 : recording start time
  (rest of the header is untouched)
"""

import shutil
from pathlib import Path


def anonymize_edf(input_path, output_path, sex="M", age=61):
    """
    Create an anonymized COPY of an .edf file. The original file is
    never modified.

    sex: "M" or "F"
    age: integer age in years
    """
    input_path = Path(input_path)
    output_path = Path(output_path)

    # Always work on a copy — never overwrite your original recording
    shutil.copy(input_path, output_path)

    with open(output_path, "r+b") as f:
        # --- Local patient identification field (80 bytes, offset 8) ---
        f.seek(8)
        patient_field = f"X {sex} X X age{age}".ljust(80)[:80].encode("ascii")
        f.write(patient_field)

        # --- Local recording identification field (80 bytes, offset 88) ---
        f.seek(88)
        recording_field = "Startdate X X X X".ljust(80)[:80].encode("ascii")
        f.write(recording_field)

    print(f"Anonymized: {input_path.name} -> {output_path.name}")


def anonymize_folder(input_folder, output_folder, sex="M", age=61):
    """Anonymize every .edf file in a folder at once."""
    input_folder = Path(input_folder)
    output_folder = Path(output_folder)
    output_folder.mkdir(parents=True, exist_ok=True)

    edf_files = list(input_folder.glob("*.edf")) + list(input_folder.glob("*.EDF"))
    if not edf_files:
        print(f"No .edf files found in {input_folder}")
        return

    for edf_file in edf_files:
        out_path = output_folder / edf_file.name
        anonymize_edf(edf_file, out_path, sex=sex, age=age)

    print(f"\nDone. {len(edf_files)} file(s) anonymized into: {output_folder}")


def check_header(path):
    """Read back the patient/recording fields, to confirm anonymization."""
    with open(path, "rb") as f:
        f.seek(8)
        patient_id = f.read(80).decode("ascii", errors="replace")
        recording_id = f.read(80).decode("ascii", errors="replace")
    print(f"patient field:   {patient_id.strip()!r}")
    print(f"recording field: {recording_id.strip()!r}")


if __name__ == "__main__":
    # ---- Example: anonymize a whole folder of your 16 recordings ----
    anonymize_folder(
        input_folder=r"C:\university\6th Term\ssvep-robust-bci\data\raw\cVEP",     # your real files — keep this private
        output_folder=r"C:\university\6th Term\ssvep-robust-bci\data\raw\baseline_anonymized",              # anonymized copies — safe to publish
        sex="M",
        age=61,
    )

    # ---- Spot-check one result ----
    check_header(r"C:\university\6th Term\ssvep-robust-bci\data\raw\baseline_anonymized\SF.edf")  # replace with an actual filename