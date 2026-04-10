# Quick DRA

[![GitHub Release](https://img.shields.io/github/v/release/mbits-os/quick_dra?style=for-the-badge&logo=github)](https://github.com/mbits-os/quick_dra/releases)
[![Coverage Status](https://img.shields.io/coverallsCoverage/github/mbits-os/quick_dra?style=for-the-badge&logo=coveralls)](https://coveralls.io/github/mbits-os/quick_dra)
[![GitHub License](https://img.shields.io/github/license/mbits-os/quick_dra?style=for-the-badge)](LICENSE)

![Quick-DRA the Hydra](./data/assets/quick-dra-social-preview-dark.png)

Quick-DRA (rhymes with _hydra_) allows generating simple ZUS RCA/DRA document sets for help hired directly by natural persons (one w/out REGON) to be able to fullfil the insurance obligations.

The CLI tool aids with configuration of both insurance payer and insured people. This data, along with insurance details and configuration is used to generate KEDU report, uploadable to ZUS platform ePłatnik. Insurance configuration is present in every installation, with backup downloadable from this repo, so that every installation is future-proofed against acts and regulations.

![CLI flow, from payer and insured config, through GitHub config and current date, to KEDU XML for ePłatnik](docs/usage.svg)

## See Also

- [Usage](docs/usage.md)
- [Building](docs/building.md)
- [Roadmap](docs/roadmap.md)

## SAST Tools

[PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
