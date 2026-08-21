# Data

## What's here

This folder is meant to hold the raw EEG recordings (`.edf` files) collected for this project:
baseline (eyes-open/eyes-closed), SSVEP single-key and multi-key trials, and c-VEP single-key
and multi-key trials, from one pilot subject (see Section 7.1 of the report for the exact
protocol).

## Consent status

- [ ] The subject gave informed consent for their EEG data to be **recorded** for this project.
- [ ] The subject gave *separate, explicit* consent for their EEG data to be **shared publicly**
      on the internet (this is a stronger, different form of consent than consent to be recorded).
- [ ] Consent was reviewed/approved by an ethics body or supervisor (Prof. Seyedarabi), if your
      department requires this for human-subject EEG recordings.

See [`CONSENT.md`](CONSENT.md) for the consent statement itself. **The signed consent document
is NOT stored in this repository** — only a statement confirming that consent was obtained.

## How to access the data

the `.edf` files are included directly in
`data/raw/`, and no request is needed.

- Farooq Danesh Amooz — far0u0q83@gmail.com
- Prof. Hadi Seyedarabi (supervisor) — department of electrical and computer engineering-univeisity of Tabriz

Please state your name, affiliation, and intended use in your request.

## Format

- `.edf` (European Data Format) — 250 Hz sampling rate, 20 channels.
- File naming follows the parts described in Section 7.1 of the report (baseline, SSVEP
  single-key/multi-key, c-VEP single-key/multi-key).

## A note on why this matters

EEG data can, in principle, be used to infer information about a person beyond the immediate
experiment (health-related signal characteristics, for example). Treat it the same way you
would treat any other personally identifiable biometric data — this is why consent for
*recording* and consent for *public release* are tracked separately above.
