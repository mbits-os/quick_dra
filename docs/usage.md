# Usage

The ultimate goal of Quick-DRA is to prepare an XML document recognizable by [ePłatnik](https://www.zus.pl/portal/eplMain.npi) in order to simplify monthly reporting. This role is fulfilled by `qdra xml` command, but for that command to work, the configuration must be provided.

## Command line interface

![CLI flow, from payer and insured config, through GitHub config and current date, to KEDU XML for ePłatnik](usage.svg)

### Payer/insured configuration

Each of the commands take all the needed data from the command line and will work in two modes: interactive and command-line-only. In interactive mode, it will combine command line arguments with pre-existing information (if any) and will prompt user for each of them. Pressing Enter will accept the command line/pre-existing info, entering new value will overwrite any position. In command-line-only mode only combined command line arguments are used to update the information. If any data is still missing in this mode, the command will fail.

#### Add/edit payer

```plain
usage: qdra payer [-h] [--config <path>] [-y] \
                  [--first <name>] \
                  [--last <name>] \
                  [--social-id <number>] \
                  [--tax-id <number>] \
                  [--id-card <number>] \
                  [--passport <number>]
```

The `qdra payer` command allows to provide/modify the identity of the insurance payer.

|Argument|Usage|
|-|-|
|`--config <path>`|Select config file; defaults to `~/.quick_dra.yaml`|
|`-y`|Use answers from command line and previous entries in config file; do not ask additional questions|
|`--first <name>`|Provide first name of the payer|
|`--last <name>`|Provide last name of the payer|
|`--social-id <number>`|Provide PESEL number|
|`--tax-id <number>`|Provide NIP number|
|`--id-card <number>`|Provide state-issued id number; if passport is used in the config file, it will be replaced by this field|
|`--passport <number>`|Provide passport number; if stated-issued id is used in the config file, it will be replaced by this field|

For example, calling the command for the first time might produce output such as this:

```plain
> qdra payer \
    --first Piotr \
    --last Kowalski \
    --social-id 78070707132 \
    --tax-id 7680002466 \
    --id-card ABC523456 \
    -y
First name changed from <empty> to Piotr
Last name changed from <empty> to Kowalski
NIP changed from <empty> to 7680002466
PESEL changed from <empty> to 78070707132
Document kind changed from <empty> to 1
Document changed from <empty> to ABC523456
```

#### Add insured

```plain
usage: qdra insured add [-h] [--config <path>] [-y] \
                        [--first <name>] \
                        [--last <name>] \
                        [--social-id <number>] \
                        [--id-card <number>] \
                        [--passport <number>] \
                        [--title <code>] \
                        [--scale <num>/<den>] \
                        [--salary <zł>]
```

The `qdra insured add` command allows to add a new insured person to the configuration.

|Argument|Usage|
|-|-|
|`--config <path>`|Select config file; defaults to `~/.quick_dra.yaml`|
|`-y`|Use answers from command line; do not ask additional questions|
|`--first <name>`|Provide first name of the payer|
|`--last <name>`|Provide last name of the payer|
|`--social-id <number>`|Provide PESEL number|
|`--id-card <number>`|Provide state-issued id number|
|`--passport <number>`|Provide passport number|
|`--title <code>`|Provide insurance title code as six digits in ``#### # #'` format; for instance, for title of 0110, no social benefits, no disability, it should be `"0110 0 0"`|
|`--scale <num>/<den>`|For part time workers, what scale should be applied to their salary; defaults to 1/1|
|`--salary <zł>`|Provide gross salary amount, before applying the scale, represented by a number with 0.01 increment, with optional PLN or zł suffix; alternatively, single word `"minimal"` to represent a minimal pay in a given month|

For example, calling the command in non-interactive way might produce output such as this:

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

#### Edit insured

```plain
usage: qdra insured edit [-h] [--config <path>] [--pos <index>] \
                         [--find <keyword>] [-y] \
                         [--first <name>] \
                         [--last <name>] \
                         [--social-id <number>] \
                         [--id-card <number>] \
                         [--passport <number>] \
                         [--title <code>] \
                         [--scale <num>/<den>] \
                         [--salary <zł>]
```

The `qdra insured edit` command allows to update data of a selected insured person.

|Argument|Usage|
|-|-|
|`--config <path>`|Select config file; defaults to `~/.quick_dra.yaml`|
|`--pos <index>`|1-based position of the insured person to remove from config|
|`--find <keyword>`|First or last name, or a document number to use as a search key|
|`-y`|Use answers from command line; do not ask additional questions|
|`--first <name>`|Provide first name of the payer|
|`--last <name>`|Provide last name of the payer|
|`--social-id <number>`|Provide PESEL number|
|`--id-card <number>`|Provide state-issued id number|
|`--passport <number>`|Provide passport number|
|`--title <code>`|Provide insurance title code as six digits in ``#### # #'` format; for instance, for title of 0110, no social benefits, no disability, it should be `"0110 0 0"`|
|`--scale <num>/<den>`|For part time workers, what scale should be applied to their salary; defaults to 1/1|
|`--salary <zł>`|Provide gross salary amount, before applying the scale, represented by a number with 0.01 increment, with optional PLN or zł suffix; alternatively, single word `"minimal"` to represent a minimal pay in a given month|

#### Remove insured

```plain
usage: qdra insured remove [-h] [--config <path>] [--pos <index>] \
                           [--find <keyword>] [-y]
```

The `qdra insured remove` command allows to remove an insured person from configuration.

|Argument|Usage|
|-|-|
|`--config <path>`|Select config file; defaults to `~/.quick_dra.yaml`|
|`--pos <index>`|1-based position of the insured person to remove from config|
|`--find <keyword>`|First or last name, or a document number to use as a search key|
|`-y`|Remove record if possible; do not ask additional questions|

#### List people

```plain
usage: qdra list [-h] [--config <path>] [--find <keyword>] [--pipe] [-z]
```

The `qdra list` command allows to list people in configuration

|Argument|Usage|
|-|-|
|`--config <path>`|Select config file; defaults to `~/.quick_dra.yaml`|
|`--find <keyword>`|First or last name, or a document number to use as a search key|
|`--pipe`|Generate tab-separated output|
|`-z`|Use zero as field separator, when --pipe is also used|

For example

```plain
> qdra list
Payer: Piotr Kowalski [1 ABC523456] ...
#1: Jan Nowak [P 26211012346] 1/2 of <minimal>
```

### Prepare ZUD RCA/DRA report

```plain
usage: qdra xml [-h] [-v ...] [--config <path>] [--tax-config <path>] \
                [-n <NN>] [-m <month>] [--today <YYYY-MM-DD>] \
                [--pretty] [--info]
```

The `qdra xml` command produces a KEDU 5.6 XML file.

|Argument|Usage|
|-|-|
|`-v`|Set the output to be more verbose, output will change with each added -v, e.g. -vv will differ from -vvv|
|`--config <path>`|Select config file; defaults to `~/.quick_dra.yaml`|
|`--tax-config <path>`|Provide tax parameters file; will take precedent before data from repository and installation|
|`-n <NN>`|Choose serial number of this particular report set; defaults to 1|
|`-m <month>`|Choose how many months away from today the report should use; defaults to -1|
|`--today <YYYY-MM-DD>`|Choose the date for the XML production; defaults to date setup on the host machine|
|`--pretty`|Pretty-print resulting XML document|
|`--info`|End terminal printout with a summary of amounts to pay|

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
