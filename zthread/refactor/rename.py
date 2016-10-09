#!/usr/bin/env python

import re,glob,os,sys

def rename(dir_path, search_pattern):
    for file_path in glob.iglob(os.path.join(dir_path, search_pattern)):
        title, ext = os.path.splitext(os.path.basename(file_path))
        res = "_".join([s.group(0).lower()  for s in
            re.finditer(r"[A-Z][a-z]+", title)])
        print title
        print ext
        print res

        if res:
            print "ddd"
            if ext == ".cxx":
                os.rename(file_path, os.path.join(dir_path, res + ".cc"))
            elif ext == ".h":
                print "psss me"
                os.rename(file_path, os.path.join(dir_path, res + ".h"))


dir_path = sys.argv[1]
rename(dir_path, "*.cxx")
rename(dir_path, "*.h")

# re.finditer(
