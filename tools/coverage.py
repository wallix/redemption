#!/usr/bin/python

import subprocess
import sys
import re
import os

gccinfo = subprocess.Popen(["gcc", "--version"], stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
#res = re.match("\s+(\d+[.]*\d+)[.]?\d+$", gccinfo)
res = re.search(r"(\d+[.]*\d+[.]?\d+)\n", gccinfo)
GCCVERSION = 'gcc-%s' % res.group(1)

TESTSSUBDIR = ''
if GCCVERSION[:7] in ['gcc-4.6', 'gcc-4.7']:
    GCCVERSION = GCCVERSION[:7]
    TESTSSUBDIR = 'tests/'

print GCCVERSION, TESTSSUBDIR

class Cover:
    def __init__(self):
        self.results = {}

    def cover(self, module):
        modulepath = ''
        modulename = module
        if '/' in module:
            modulename = module.split('/')[-1]
            modulepath = '/'.join(module.split('/')[:-1])+'/'


        cmd = ["bjam", "coverage", "test_%s" % modulename]
        print " ".join(cmd)
        res = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]

        if '/widget2/' in module:
            modulename = modulename.replace('widget2_', '')

    #    print res
    #gcov --all-blocks --branch-count --branch-probabilities --function-summaries -o bin/gcc-4.6/coverage/tests/test_stream.gcno bin/gcc-4.6/coverage/test_stream
        cmd = ["gcov", "--all-blocks", "--branch-count", "--branch-probabilities", "--function-summaries", "-o", "bin/%s/coverage/%s%stest_%s.gcno" % (GCCVERSION, TESTSSUBDIR, modulepath if TESTSSUBDIR else '', modulename), "bin/%s/coverage/test_%s" % (GCCVERSION, modulename)]
        print " ".join(cmd)
        res = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
    #    print res

        extension = '.hpp'
        if '/rio/' in module:
            extension = '.h'

        uncovered = 0
        total = 0
        try:
            for line in open("./%s%s.gcov" % (modulename, extension)):
                res = re.match(r'^\s+#####[:]', line)
                if res:
                    uncovered += 1
                    total += 1
                res = re.match(r'^\s+\d+[:]', line)
                if res:
                    total += 1
        except IOError:
            total = 100
            uncovered = 100

        self.results[module] = ((total - uncovered), total)

        os.system("mkdir -p coverage/%s" % module)
        os.system("mv *.gcov coverage/%s" % module)


    def coverall(self):
        target = open("coverage.summary", "w")
        for line in open("./tools/coverage.reference"):
            res = re.match(r'^([/A-Za-z0-9_]+)\s+(\d+)\s+(\d+)', line)
            if res:
                module, covered, total = res.group(1, 2, 3)
                print module
                extension = '.hpp'
                if '/rio/' in module:
                    extension = '.h'
                covered = int(covered)
                total = int(total)
                self.cover(module)
                # if coverage percentage is lower
                if (self.results[module][0] * 100 / self.results[module][1]) < (covered * 100 / total):
                    print("Lower coverage for module %s : old %d/%d new %d/%d" % (module,
                        covered, total,
                        self.results[module][0], self.results[module][1]))
                    target.write("%s: %d%s (%d / %d)\n" % ((
                        module, self.results[module][0] * 100.0 / self.results[module][1], "%") + self.results[module]))
                    try:
                        for line in open("./coverage/%s/%s%s.gcov" % (module, module.split('/')[-1], extension)):
                            res = re.match(r'^\s+#####[:]', line)
                            if res:
                                print module, ' ', line
                    except IOError:
                        for i in range(0, 100):
                            print module, ' #####: %u: NO COVERAGE' % i
            else:
                print "Line '%s' does not match" % line
cover = Cover()
if sys.argv[1] == 'all':
    cover.coverall()
else:
    cover.cover(sys.argv[1])

print "Coverage Results:"
for module in cover.results:
    print "%s: %d%s (%d / %d)" % ((
        module, cover.results[module][0] * 100.0 / cover.results[module][1], "%") + cover.results[module])

