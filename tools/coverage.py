#!/usr/bin/python

import subprocess
import sys
import re
import os
import os.path

class Function:
    def __init__(self, name, startline, filename):
        self.name = name.strip()
        self.startline = startline
        self.filename = filename
        self.total_lines = 0
        self.covered_lines = 0

class Module:
    def __init__(self, name):
        self.name = name
        self.functions = {}

    def total_functions(self):
        len(self.functions)

class Cover:
    def __init__(self):
        self.modules = {}
        self.functions = {}
        self.verbose = 0

    def parse_etags(self, ftags):
        filename = ''
        if self.verbose > 1:
            print("================= Parsing etags (%s) ===============" % ftags)
        for line in open(ftags):
            if line[0] == '.':
                res = re.match(r'^.*/(src/.*)$', line)
                if res:
                    filename = res.group(1)
                continue
            if re.match(r'^\s*,', line):
                continue
            if re.match(r'^[^ ]+\(\x7F', line):
                continue
            res = re.match(r'^(.*[(].*)\x7F.*\x01(\d+)[,].*$', line)
            if res is None:
                res = re.match(r'^(.*[(].*)\x7F(\d+)[,].*$', line)
            if res:
                name, startline = res.group(1, 2)
                if self.verbose > 1:
                    print("function found at %s %s" % (name.strip(), startline))
                self.functions[int(startline)] = Function(name, int(startline), filename)

    def compute_coverage(self, fgcov):
        current_function = None
        search_begin_body = False
        brackets_count = 0
        parentheses_count = 0
        open_parentheses = False
        block_executed = False

        if self.verbose > 1:
            print("================= Parsing gcov for %s ===============" % fgcov)

        for line in open(fgcov):
            res = re.match(r'^\s*(#####|[-]|\d+)[:]\s*(\d+)[:](.*)$', line)
            if res:
                if current_function and search_begin_body:
                    if res.group(3).count('{'):
                        search_begin_body = False
                    else:
                        continue

                if int(res.group(2)) in self.functions:
                    current_function = int(res.group(2))
                    f = self.functions[current_function]
                    if self.verbose > 1:
                        print("%s function found at %d" % (f.name, current_function))
                    self.functions[current_function].total_lines += 1
                    if not res.group(1) in ('#####', "-"):
                        self.functions[current_function].covered_lines += 1
                    brackets_count = res.group(3).count('{')
                    open_parentheses = False
                    if brackets_count > 0:
                        brackets_count -= res.group(3).count('}')
                        if brackets_count <= 0:
                            current_function = None
                    else:
                        search_begin_body = True
                    continue

                if current_function and not search_begin_body:
                    skip_line = False
                    parentheses_count += res.group(3).count('(')
                    parentheses_count -= res.group(3).count(')')
                    if not open_parentheses:
                        if parentheses_count > 0:
                            open_parentheses = True
                            block_executed = False
                            self.functions[current_function].total_lines += 1
                    if open_parentheses:
                        if not res.group(1) in ("#####", "-"):
                            block_executed = True
                        if (parentheses_count <= 0):
                            if block_executed:
                                self.functions[current_function].covered_lines += 1
                            open_parentheses = False
                            parentheses_count = 0
                        skip_line = True
                    brackets_count += res.group(3).count('{')
                    brackets_count -= res.group(3).count('}')
                    if brackets_count <= 0:
                        brackets_count = 0
                        current_function = None
                        skip_line = True
                    if skip_line:
                        continue

                # ignore comments
                if re.match('^\s*//', res.group(3)):
                    continue
                # ignore blank lines
                if re.match('^\s+$', res.group(3)):
                    continue
                #ignore case statement of switch
                if re.match('^\s*(case\s(.*):|default:|break;|goto)', res.group(3)):
                    continue
                # At least one identifier or number on the line (ie: ignore alone brackets)
                if re.match('^.*[a-zA-Z0-9]', res.group(3)) is None:
                    continue

                if current_function:
                    self.functions[current_function].total_lines += 1

                if not res.group(1) in ('#####', "-"):
                    if current_function:
                        self.functions[current_function].covered_lines += 1

        if self.verbose > 0:
            print("computing coverage for %s : done" % fgcov)


if __name__ == "__main__":
    cover = Cover()

    if len(sys.argv) < 3:
        print("================ Not enough data for computing coverage\n %s\n ===================" % str(sys.argv))
        exit(-1)

    cover.parse_etags(sys.argv[2])
    for f in sys.argv[3:]:
        cover.compute_coverage(f)

    print("================ Coverage Results: ===================")
    td = ""
    g_covered = 0
    g_total_number = 0
    covered = 0
    no_coverage = 0
    low_coverage = 0
    total_number = 0
    for fnl in sorted(cover.functions):
        fn = cover.functions[fnl]
        total_number += 1
        if fn.covered_lines == 0:
            t = "WARNING: NO COVERAGE"
            no_coverage += 1
        elif fn.covered_lines * 100 < fn.total_lines * 50:
            t = "WARNING: LOW COVERAGE"
            low_coverage += 1
        else:
            t = "COVERAGE"
            covered += 1
        print("%s %s%s:%s [%s] %s/%s"
                % (t, td, fn.filename, fnl, fn.name, fn.covered_lines, fn.total_lines))
    print("MODULE %s : %s COVERED, %s LOW_COVERAGE, %s NO_COVERAGE in %s TOTAL"
            % (sys.argv[1], covered, low_coverage, no_coverage, total_number))
    g_covered += covered
    g_total_number += total_number
    covered_rate = 0
    if g_total_number != 0:
        covered_rate = 100 * g_covered / g_total_number
    print("---------------- End of coverage Results -------------------")
