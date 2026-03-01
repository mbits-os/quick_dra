# Quick DRA

[![GitHub Release](https://img.shields.io/github/v/release/mbits-os/quick_dra?style=for-the-badge&logo=github)](https://github.com/mbits-os/quick_dra/releases)
[![Coverage Status](https://img.shields.io/coverallsCoverage/github/mbits-os/quick_dra?style=for-the-badge&logo=coveralls)](https://coveralls.io/github/mbits-os/quick_dra)
[![GitHub License](https://img.shields.io/github/license/mbits-os/quick_dra?style=for-the-badge)](LICENSE)

![Quick-DRA the Hydra](./data/assets/quick-dra-social-preview-dark.png)

Quick-DRA (rhymes with _hydra_) allows generating simple ZUS RCA/DRA document
sets for help hired directly by natural persons (one w/out REGON) to be able
to fullfil the insurance obligations.

## Usage

### Configuration

#### Add payer

```plain
> qdra payer \
    --first Piotr \
    --last Kowalski \
    --social-id 78070707132 \
    --tax-id 7680002466 \
    --id-card ABC523456 \
    -y
First name changed from <empty> to Jan
Last name changed from <empty> to Nowak
NIP changed from <empty> to 7680002466
PESEL changed from <empty> to 78070707132
Document kind changed from <empty> to 1
Document changed from <empty> to ABC523456
```

#### Add insured

```plain
> qdra insured add \
    --first Jan \
    --last Nowak \
    --title "0110 0 0" \
    --social-id 26211012346 \
    --scale 1/2 \
    -y
First name set to Jan
Last name set to Nowak
Document kind set to P
Document set to 26211012346
Insurance title set to 0110 0 0
Part-time scale set to 1/2
Salary set to minimal for a given month
```

#### List insured

```plain
> qdra insured list
#1: Jan Nowak [P 26211012346] 1/2 of <minimal>
```

### Prepare ZUD RCA/DRA

Generate RCA/DRA xml file for last month

```plain
> qdra xml
-- report: #1 2026-01
-- output: quick-dra_202601-01.xml
```

Generate RCA/DRA xml file for current month

```plain
> qdra xml -m0
-- report: #1 2026-02
-- output: quick-dra_202601-02.xml
```

Generate RCA/DRA xml file with payment information

```plain
> qdra xml --info
-- report: #1 2026-01
-- output: quick-dra_202601-01.xml
-- payments:
   - JAN NOWAK:      2073.55 zł
   - ZUS:             760.31 zł
   - Urząd Skarbowy:    0.00 zł
   sum total =       2833.86 zł
```

## SAST Tools

[PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
