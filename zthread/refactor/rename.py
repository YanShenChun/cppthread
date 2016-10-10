#!/usr/bin/env python

import re,glob,os,sys

def refactor(dir_path):
    for root, _, file_names in os.walk(dir_path):
        for file_path in file_names:
            base_name, ext = os.path.splitext(os.path.basename(file_path))
            full_path = os.path.join(root, file_path)

            if not(ext == ".cxx" or ext == ".h"):
                continue
            print "refactor " + full_path + " .."
            refactor_file(full_path)
            new_file_name = "_".join([s.group(0).lower()  for s in
                re.finditer(r"[A-Z][a-z]+", base_name)])
            if ext == ".cxx":
                ext = ".cc"
            if new_file_name:
                os.rename(full_path, os.path.join(root, new_file_name + ext))
            else:
                os.rename(full_path, os.path.join(root, base_name.lower() + ext))


def refactor_file(file_path):
    def header_repl_func(match):
        header_name = "_".join([s.group(0).lower()  for s in
             re.finditer(r"[A-Z][a-z]+", match.group("header_name"))])
        return "".join([match.group("prefix"), header_name, ".h"])

    def ns_repl_func(match):
        return "".join([match.group("prefix"), match.group("ns_name").lower()])

    def rename_func(line):
        # Heade file: CamelCase -> Underline 
        result = re.sub(r'(?P<prefix>#\s*include\s+"(\w+/)*)(?P<header_name>\w+)\.h',
            header_repl_func, line)
        # namespace: tolower
        result = re.sub(r'(?P<prefix>^\s*namespace\s+)(?P<ns_name>\w+)', ns_repl_func,
                result)
        result = re.sub(r'(?P<prefix>^\s*using namespace\s+)(?P<ns_name>\w+)', ns_repl_func,
                result)
        return result

    result = ""
    with open(file_path, "r") as file_stream:
        result = "".join(rename_func(line) for line in file_stream.readlines())

    # commit
    with open(file_path, "w") as file_stream:
        file_stream.write(result)

dir_path = sys.argv[1]
refactor(dir_path)
