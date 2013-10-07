#!/usr/bin/python

import subprocess
import sys
import re
import os
import os.path

gccinfo = subprocess.Popen(["gcc", "--version"], stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
#res = re.match("\s+(\d+[.]*\d+)[.]?\d+$", gccinfo)
res = re.search(r"(\d+[.]*\d+[.]?\d+)\n", gccinfo)
GCCVERSION = 'gcc-%s' % res.group(1)

TESTSSUBDIR = ''
if GCCVERSION[:9] in ['gcc-4.6.1']:
    GCCVERSION = GCCVERSION[:9]
    TESTSSUBDIR = 'tests/'
elif GCCVERSION[:7] in ['gcc-4.6', 'gcc-4.7']:
    GCCVERSION = GCCVERSION[:7]
    TESTSSUBDIR = 'tests/'

print GCCVERSION, TESTSSUBDIR


class Function:
    def __init__(self, name, startline):
        self.name = name
        self.startline = startline
        self.total_lines = 0
        self.covered_lines = 0

class Module:
    def __init__(self, name):
        self.name = name
        self.extension = '.hpp'
        if '/rio/' in self.name:
            self.extension = '.h'
        self.lines = 0
        self.covered = 0
        self.functions = {}


def list_modules():
    for line in open("./tools/coverage.reference"):
        res = re.match(r'^((?:[/A-Za-z0-9_]+[/])*([/A-Za-z0-9_]+))\s+(\d+)\s+(\d+)', line)
        if res:
            module, name = res.group(1, 2)
            extension = '.hpp'
            if '/rio/' in module:
                extension = '.h'
            yield module, name, extension
        else:
            if line[0] != '\n' and line[0] != '#':
                print "Line '%s' does not match in coverage.reference" % line
                sys.exit(-1)

class Cover:
    def __init__(self):
        self.modules = {}
        self.bestcoverage = {} # module: (lincov, lintotal)
        self.functions = {}
        self.verbose = 1

    def cover(self, module, name, extension):
        print "Computing coverage for %s" % module
        cmd1 = ["bjam", "coverage", "test_%s" % name]
        cmd2 = ["gcov", "--all-blocks", "--branch-count", "--branch-probabilities", "--function-summaries", "-o", "bin/%s/coverage/%s%stest_%s.gcno" % (GCCVERSION, TESTSSUBDIR, "%s" % module[:-len(name)] if TESTSSUBDIR else '', name), "bin/%s/coverage/test_%s" % (GCCVERSION, name)]
        cmd3 = ["etags", "-o", "coverage/%s/%s%s.TAGS" % (module, name, extension), "%s%s" % (module, extension)]

        res = subprocess.Popen(cmd1, stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
        res = subprocess.Popen(cmd2, stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
        subprocess.call("mkdir -p coverage/%s" % module, shell=True)
        res = subprocess.Popen(cmd3, stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]

        try:
            subprocess.check_call("mv *.gcov coverage/%s" % module, shell=True)
        except:
            print "Failed to compute coverage for module"
            print " ".join(cmd1)
            print " ".join(cmd2)
            print " ".join(cmd3)


    def compute_coverage(self, module, name, extension):
        ftags = "./coverage/%s/%s%s.TAGS" % (module, name, extension)
        fgcov = "./coverage/%s/%s%s.gcov" % (module, name, extension)

        import os.path
        if os.path.isfile(fgcov) and os.path.isfile(ftags):

            self.modules[module] = Module(module)
            for line in open(ftags):
                if re.match(r'^.*(TODO|REDOC|BODY)', line):
                    continue
                res = re.match(r'^(.*[(].*)\x7F.*\x01(\d+)[,].*$', line)
                if res is None:
                    res = re.match(r'^(.*[(].*)\x7F(\d+)[,].*$', line)
                if res:
                    name, startline = res.group(1, 2)
                    print "function found at %s %s" % (name, startline)
                    self.modules[module].functions[int(startline)] = Function(name, int(startline))

            current_function = None
            for line in open(fgcov):
                res = re.match(r'^.*(\d+)[:]\s*BEGINBODY(.*)$', line)
                if res and current_function:
                    if self.modules[module].functions[current_function].covered_lines > 0:
                        self.modules[module].functions[current_function].covered_lines = 1
                        self.modules[module].functions[current_function].total_lines = 1
                    else:
                        self.modules[module].functions[current_function].covered_lines = 0
                        self.modules[module].functions[current_function].total_lines = 0

                    continue

                res = re.match(r'^.*(\d+)[:]\s*ENDBODY(.*)$', line)
                if res:
                    current_function = None
                    continue

                res = re.match(r'^\s*(#####|[-]|\d+)[:]\s*(\d+)[:](.*)$', line)
                if res:
                    if int(res.group(2)) in self.modules[module].functions:
                        print "current function found at %d" % int(res.group(2))
                        current_function = int(res.group(2))

                    # ignore comments
                    if re.match('^\s*//', res.group(3)):
                        continue
                    # ignore blank lines
                    if re.match('^\s+$', res.group(3)):
                        continue
                    #ignore case statement of switch
                    if re.match('^\s*(case|default)', res.group(3)):
                        continue
                    # At least one identifier or number on the line (ie: ignore alone brackets)
                    if re.match('^.*[a-zA-Z0-9]', res.group(3)) is None:
                        continue

                    if current_function:
                        self.modules[module].functions[current_function].total_lines += 1

                    if not res.group(1) in ('#####', "-"):
                        if current_function:
                            self.modules[module].functions[current_function].covered_lines += 1

            if self.verbose > 0:
                print "computing coverage for %s : done" % module
        else:
            if self.verbose > 1:
                print "computing coverage for %s : FAILED" % module

    def coverall(self):
        for module, name, extension in list_modules():
            self.cover(module, name, extension)
        for module, name, extension in list_modules():
            self.compute_coverage(module, name, extension)

    def covercurrent(self):
        for module, name, extension in list_modules():
            ftags = "./coverage/%s/%s%s.TAGS" % (module, name, extension)
            fgcov = "./coverage/%s/%s%s.gcov" % (module, name, extension)
            if os.path.isfile(fgcov) and os.path.isfile(ftags):
                self.compute_coverage(module, name, extension)


cover = Cover()
if len(sys.argv) < 2:
    cover.covercurrent()
elif sys.argv[1] == 'all':
    cover.coverall()
else:
    print "Computing coverage for one file"
    res = re.match(r'^((?:[/A-Za-z0-9_]+[/])*([/A-Za-z0-9_]+))', sys.argv[1])
    if res:
        module, name = res.group(1, 2)
        extension = '.hpp'
        if '/rio/' in module:
            extension = '.h'
        cover.cover(module, name, extension)
        for module, name, extension in [(module, name, extension)]:
            cover.compute_coverage(module, name, extension)
    else:
        print "Input does not match expected format"
        sys.exit(0)

print "Coverage Results:"
for m in cover.modules:
    for fnl in sorted(cover.modules[m].functions):
        fn = cover.modules[m].functions[fnl]
        if fn.covered_lines == 0:
            print "WARNING: NO COVERAGE %s%s:%s [%s] %s/%s" % (m, cover.modules[m].extension,
                                                fnl, fn.name, fn.covered_lines, fn.total_lines)
        elif fn.covered_lines * 100 < fn.total_lines * 50:
            print "WARNING: LOW COVERAGE %s%s:%s [%s] %s/%s" % (m, cover.modules[m].extension,
                                                fnl, fn.name, fn.covered_lines, fn.total_lines)
        else:
            print "COVERAGE %s%s:%s [%s] %s/%s" % (m, cover.modules[m].extension,
                                                fnl, fn.name, fn.covered_lines, fn.total_lines)

