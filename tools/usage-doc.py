import re
import subprocess
import sys
from pathlib import Path
from pprint import pprint

qdra_executable = sys.argv[1]

arg_pattern = re.compile(r"^(-[-a-z1-9]+(?: <[^>]+>(?:/<[^>]+>)?)?\s*)")

arg_patches = {
    "~/.quick_dra.yaml": "`~/.quick_dra.yaml`",
    "`#### # #'": "``#### # #'`",
    '"0110 0 0"': '`"0110 0 0"`',
    '"minimal"': '`"minimal"`',
}

split_args = [
    "[-y]",
    "[--pos <index>]",
    "[--tax-config <path>]",
    "[--today <YYYY-MM-DD>]",
]

split_rest_in = ["payer", "insured add", "insured edit"]


def run_help(cmd: list[str]):
    stdout = subprocess.run(
        [qdra_executable, *cmd, "-h"],
        shell=False,
        encoding="utf-8",
        capture_output=True,
        check=True,
        universal_newlines=True,
    ).stdout
    lines = stdout.rstrip().split("\n")
    indented = [line.startswith(" ") for line in lines]

    section_headers = [
        index
        for index in range(len(indented) - 1)
        if not indented[index] and indented[index + 1]
    ]

    header = ""
    prev = -1

    sections: list[tuple[str, list[str]]] = []

    for index in section_headers:
        section = lines[prev + 1 : index]

        while section and not section[-1]:
            section = section[:-1]

        sections.append((header, section))

        header = lines[index]
        prev = index

    if prev < len(lines):

        sections.append((header, lines[prev + 1 :]))

    usage: str | None = None

    if sections:
        lines = sections[0][1]
        sections = sections[1:]

        if lines and lines[0].startswith("usage: "):
            usage = lines[0]
            cmd_str = " ".join(cmd)
            sep = usage.split(cmd_str)[0] + " ".join(cmd)
            sep = " " * len(sep)
            sep = f" \\\n{sep} "
            for split_arg in split_args:
                usage = f"{split_arg}{sep}".join(usage.split(f"{split_arg} "))

            if cmd_str in split_rest_in:
                split_usage = usage.split(sep)
                last_one = split_usage[-1]
                split_usage[-1] = f"]{sep}[".join(last_one.split("] ["))
                usage = sep.join(split_usage)

    arguments = ["|Argument|Usage|", "|-|-|"]

    for header, lines in sections:
        if header == "known commands:":
            continue

        if header == "optional arguments:":
            lines = [line.lstrip() for line in lines]
            lines = [line for line in lines if not line.startswith("-h")]

            for line in lines:
                m = re.search(arg_pattern, line)
                pos = m.span()[1] if m else len(line)
                args = line[:pos].strip()
                line = line[pos:]
                first_word = line.split()[0]
                dscr = first_word.capitalize() + line[len(first_word) :]
                for key, patch in arg_patches.items():
                    dscr = dscr.replace(key, patch)
                arguments.append(f"|`{args}`|{dscr}|")
            continue

        print("BLOCK:", header.split(":", 1)[0])

    if len(arguments) == 2:
        arguments = []

    return usage, "\n".join(arguments)


usage_md = Path(__file__).parent.parent / "docs" / "usage.md"
usage_text = usage_md.read_text()

usage = usage_text.split("usage: qdra ")
result = usage[0]
usage = usage[1:]

for chunk in usage:
    pre, args = chunk.split("|Argument|Usage|\n", 1)
    command = re.split(" [-[]", pre, 1)[0]

    description_1 = "".join(pre.split("```\n", 1)[1:])
    args_lines = args.split("\n")
    while args_lines and args_lines[0].startswith("|"):
        args_lines = args_lines[1:]
    description_2 = "\n".join(args_lines)
    usage, arguments = run_help(command.split())
    result += f"{usage}\n```\n{description_1}{arguments}\n{description_2}"

usage_md.write_text(result, encoding="utf-8")
