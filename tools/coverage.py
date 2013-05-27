#!/usr/bin/python

import subprocess
import sys
import re
import os

gccinfo = subprocess.Popen(["gcc", "--version"], stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
#res = re.match("\s+(\d+[.]*\d+)[.]?\d+$", gccinfo)
res = re.search(r"(\d+[.]*\d+)[.]?\d+\n", gccinfo)
GCCVERSION = 'gcc-%s' % res.group(1)


class Cover:
    def __init__(self):
        self.results = {}

    def cover(self, module):
        modulepath = ''
        modulename = module
        if '/' in module:
            modulename = module.split('/')[-1]
            modulepath = '/'.join(module.split('/')[:-1])+'/'
            
        res = subprocess.Popen(["bjam", "coverage", "test_%s" % modulename], stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
    #    print res
    #gcov --all-blocks --branch-count --branch-probabilities --function-summaries -o bin/gcc-4.6/coverage/tests/test_stream.gcno bin/gcc-4.6/coverage/test_stream
        cmd = ["gcov", "--all-blocks", "--branch-count", "--branch-probabilities", "--function-summaries", "-o", "bin/%s/coverage/tests/%stest_%s.gcno" % (GCCVERSION, modulepath, modulename), "bin/%s/coverage/test_%s" % (GCCVERSION, modulename)]
#        print " ".join(cmd)
        res = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
    #    print res
        
        uncovered = 0
        total = 0
        for line in open("./%s.hpp.gcov" % modulename):
            res = re.match(r'^\s+#####[:]', line)
            if res:
                uncovered += 1
                total += 1
            res = re.match(r'^\s+\d+[:]', line)
            if res:
                total += 1
        
        self.results[module] = ((total - uncovered), total)

        os.system("mkdir -p coverage/%s" % module)
        os.system("mv *.gcov coverage/%s" % module)


    def coverall(self):
        target = open("coverage.summary", "w")
        for line in open("./tools/coverage.reference"):
            res = re.match(r'^([/A-Aa-z0-9]+)\s+(\d+)\s+(\d+)', line)
            if res:
                module, covered, total = res.group(1, 2, 3)
                print module
                covered = int(covered)
                total = int(total)
                self.cover(module)
                # if covered lines is lower in absolute count
                if (self.results[module][0] < covered
                # or if coverage percentage is lower
                or (self.results[module][0] * 100.0 / self.results[module][1]) < (covered * 100.0 / total)):
                    print("Lower coverage for module %s : old %d/%d new %d/%d" % (module, 
                        covered, total, 
                        self.results[module][0], self.results[module][1]))
                    target.write("%s: %d%s (%d / %d)\n" % ((
                        module, self.results[module][0] * 100.0 / self.results[module][1], "%") + self.results[module]))
                    target.write("Lower coverage for module %s : old %d/%d new %d/%d\n" % (module, 
                        covered, total, 
                        self.results[module][0], self.results[module][1]))

cover = Cover()
if sys.argv[1] == 'all':
    cover.coverall()
else:
    cover.cover(sys.argv[1])

print "Coverage Results:"
for module in cover.results:
    print "%s: %d%s (%d / %d)" % ((
        module, cover.results[module][0] * 100.0 / cover.results[module][1], "%") + cover.results[module])

