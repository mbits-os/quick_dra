# Copyright (c) 2025 Marcin Zdun
# This file is licensed under MIT license (see LICENSE for details)

import os

headers = []
code = []

for root, dirs, files in os.walk("."):
    for filename in files:
        if os.path.splitext(filename)[1] == ".hpp":
            path = os.path.join(root, filename)
            if path[:2] == f".{os.sep}":
                path = path[2:]
            headers.append(path)
        if os.path.splitext(filename)[1] == ".cpp":
            path = os.path.join(root, filename)
            if path[:2] == f".{os.sep}":
                path = path[2:]
            code.append(path)

for filename in headers:
    found = False
    with open(filename, encoding="UTF-8") as text:
        for line in text:
            line = line.rstrip()
            if line == "#pragma once":
                found = True
                break
    if not found:
        print(filename)

for filename in code:
    found = False
    with open(filename, encoding="UTF-8") as text:
        for line in text:
            line = line.rstrip()
            if line == "#pragma once":
                found = True
                break
    if found:
        print(filename)
