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


def list_modules():
    for line in open("./tools/coverage.reference"):
        res = re.match(r'^([/A-Za-z0-9_]+)\s+(\d+)\s+(\d+)', line)
        if res:
            module, covered, total = res.group(1, 2, 3)
            extension = '.hpp'
            if '/rio/' in module:
                extension = '.h'
            covered = int(covered)
            total = int(total)
            yield module, extension, covered, total
        else:
            if line[0] != '\n' and line[0] != '#': 
                print "Line '%s' does not match" % line



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

        subprocess.call("mkdir -p coverage/%s" % module, shell=True)
        subprocess.call("mv *.gcov coverage/%s" % module, shell=True)


    def coverall(self):
        target = open("coverage.summary", "w")
        for module, extension, covered, total in list_modules():
            print module
            self.cover(module)
            # if coverage percentage is lower
            if (self.results[module][0] * 100 * total) <= (self.results[module][1] * covered * 100):
                target.write("%s: %d%s (%d / %d)\n" % ((
                    module, self.results[module][0] * 100.0 / self.results[module][1], "%") + self.results[module]))
                target.flush()
                try:
                    for line in open("./coverage/%s/%s%s.gcov" % (module, module.split('/')[-1], extension)):
                        res = re.match(r'^\s+#####[:]', line)
                        if res:
                            print module, ' ', line
                except IOError:
                    for i in range(0, 100):
                        print module, ' #####: %u: NO COVERAGE' % i

cover = Cover()
if sys.argv[1] == 'all':
    cover.coverall()
elif sys.argv[1] == 'touchall':
    for module, extension, covered, total in list_modules():
        
        path = '/'.join(module.split('/')[:-1])
        fname = module.split('/')[-1]
        fullname = "tests/%s/test_%s%s" % (path, fname, ".cpp")
        if os.path.getsize(fullname) == 1016:
            print(
            "unit-test test_%(f)s : tests/%(p)s/test_%(f)s.cpp libboost_unit_test ;\nunit-test test_%(f)s : tests/%(p)s/test_%(f)s.cpp libboost_unit_test gcov : <variant>coverage ;\n"
        % ({'f':fname, 'p':path}))
#        os.system("touch tests/%s/test_%s%s" % (path, fname, ".cpp"))
#        if module.split('/')[:-1]:
#           os.system('touch %s' % ('/'.join(module.split('/')[:-1])))
    exit(0)
else:
    cover.cover(sys.argv[1])

print "Coverage Results:"
for module in cover.results:
    print "%s: %d%s (%d / %d)" % ((
        module, cover.results[module][0] * 100.0 / cover.results[module][1], "%") + cover.results[module])

