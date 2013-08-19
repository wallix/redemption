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
if GCCVERSION[:9] in ['gcc-4.6.1']:
    GCCVERSION = GCCVERSION[:9]
    TESTSSUBDIR = 'tests/'
elif GCCVERSION[:7] in ['gcc-4.6', 'gcc-4.7']:
    GCCVERSION = GCCVERSION[:7]
    TESTSSUBDIR = 'tests/'

print GCCVERSION, TESTSSUBDIR


class Module:
    def __init__(self, name):
        self.name = name
        self.lines = 0
        self.covered = 0
        self.functions = {}

    def __repr__(self):
        return "\n".join(("Module: %s" % self.name
               ,"Total Lines: %s" % self.lines
               ,"Covered Lines: %s" % self.covered
               ,"Total functions: %s" % len(self.functions))) + "\n"


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
        self.modules = {}
        self.results = {}
        self.coverset = set()
        self.bestcoverage = {} # module: (lincov, lintotal)
        self.functions = {}

    def cover(self, module):
        modulepath = ''
        modulename = module
        if '/' in module:
            modulename = module.split('/')[-1]
            modulepath = '/'.join(module.split('/')[:-1])+'/'
        extension = '.hpp'
        if '/rio/' in module:
            extension = '.h'


        cmd = ["bjam", "coverage", "test_%s" % modulename]
        print " ".join(cmd)
        res = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]

        cmd = ["gcov", "--all-blocks", "--branch-count", "--branch-probabilities", "--function-summaries", "-o", "bin/%s/coverage/%s%stest_%s.gcno" % (GCCVERSION, TESTSSUBDIR, modulepath if TESTSSUBDIR else '', modulename), "bin/%s/coverage/test_%s" % (GCCVERSION, modulename)]
        print " ".join(cmd)
        res = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
        subprocess.call("mkdir -p coverage/%s" % module, shell=True)
        subprocess.call("mv *.gcov coverage/%s" % module, shell=True)

        cmd = ["etags", "%s%s" % (module, extension), "-o", "coverage/%s/%s%s.TAGS" % (module, modulename, extension)]
        print " ".join(cmd)
        res = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]

        self.modules[module] = Module(module)
        self.results[module] = self.compute_coverage("./coverage/%s/%s%s.gcov" % (module, modulename, extension))

    def compute_functions_list(self, f):
        try:
            print "Looking for functions in %s" % (f)
            allf = []
            for line in open(f):
                res = re.match(r'^.*[(]\x7F(.*)\x01(\d*)[,]', line)
                if res:
                    print "match:", res.group(1), ":", f[:-5][11:], ':', res.group(2)
                    allf.append((res.group(2), res.group(1)))
            allf = sorted(allf)
            if len(allf):
                lastl, lastfname = allf[0]
                rangefn = []
                for l, fname in allf[1:]:
                    rangefn.append((int(lastl), int(l) - 1, lastfname))
                    lastl, lastfname = l, fname
                rangefn.append((int(lastl), int(lastl) + 1000, lastfname))
                for l1, l2, name in rangefn:
                    self.functions[(f[:-5][11:], l1)] = [l1, l2, name]
        except IOError:
            print "Error in tags"

    def compute_coverage(self, f):
    
        print "computing coverage for ", f
        uncovered = 0
        total = 0
        try:
            current_function = None
            functions_lines = {}
            for line in open(f):
                res = re.match(r'^\s+#####[:]\s*(\d+)[:]', line)
                if res:
                    uncovered += 1
                    total += 1
                    module = f[11:][:-5]
                    if (module, int(res.group(1))) in self.functions:
                        current_function = (module, int(res.group(1)))
                        functions_lines[current_function] = [0, 0 
                                                             ,self.functions[current_function][0]
                                                             ,self.functions[current_function][1]
                                                             ,self.functions[current_function][2]]
                        print "function ", self.functions[current_function], " found"
                    if current_function:
                        functions_lines[current_function][1] += 1
                    
                res = re.match(r'^\s+\d+[:]\s*(\d+)[:]', line)
                if res:
                    total += 1
                    module = f[11:][:-5]
                    if (module, int(res.group(1))) in self.functions:
                        current_function = (module, int(res.group(1)))
                        functions_lines[current_function] = [0, 0 
                                                             ,self.functions[current_function][0]
                                                             ,self.functions[current_function][1]
                                                             ,self.functions[current_function][2]]
                        print "function ", self.functions[current_function], " found"
                    if current_function:
                        functions_lines[current_function][0] += 1

                res = re.match(r'^\s+[-][:]\s*(\d+)[:]', line)
                if res:
                    module = f[11:][:-5]
                    if (module, int(res.group(1))) in self.functions:
                        current_function = (module, int(res.group(1)))
                        functions_lines[current_function] = [0, 0 
                                                             ,self.functions[current_function][0]
                                                             ,self.functions[current_function][1]
                                                             ,self.functions[current_function][2]]
                        print "function ", self.functions[current_function], " found"
                    if current_function:
                        functions_lines[current_function][1] += 1

            if functions_lines:
                print functions_lines
                        
                        
        except IOError:
            total = 100
            uncovered = 100

        return ((total - uncovered), total, len([t for t in functions_lines if functions_lines[t][0] > 0]), functions_lines)

    def findbest(self):
        for d, ds, fs in os.walk("./coverage/"):
            for x in fs:
                if x[-5:] == '.TAGS':
                    self.compute_functions_list("%s/%s" % (d, x))

        for d, ds, fs in os.walk("./coverage/"):
            i = self.coverset.intersection(fs)
            for x in i:
                if not d[:11] in self.modules:
                    self.modules[d[11:]] = Module(d[11:])
                covered, total, covered_fn, fns = self.compute_coverage("%s/%s" % (d, x))
                if x in self.bestcoverage:
                    old_cover, old_total, old_path = self.bestcoverage[x]
                    if total > old_total:
                        self.bestcoverage[x] = (covered, total, [d[11:]])
                    elif old_total == total:
                        self.bestcoverage[x][2].append(d[11:])
                else:
                    self.bestcoverage[x] = (covered, total, [d[11:]])

        res = []
        for x, (c, t, p) in self.bestcoverage.iteritems():
            res.append((sorted(p), (c*100/t), x, x.split('.')[0], c, t))
            print x

        target = open("coverage.summary", "w")

        for p, pc, covname, module, c, t in sorted(res):
            print self.fullmodules[module], p, pc, c, t

            target.write("%s: %d%s (%d / %d) %s\n" % (self.fullmodules[module], pc, "%", c, t, 
                '[]' if  self.fullmodules[module] in p else p))
            target.flush()

            if c == 0:
                for i in range(0, 100):
                    print module, ' #####: %u: NO COVERAGE FILE' % i
            else:
                try:
                    print "./coverage/%s/%s" % (p[0], covname)
                    for line in open("./coverage/%s/%s" % (p[0], covname)):
                        res = re.match(r'^\s+#####[:]', line)
                        if res:
                            print module, ' ', line
                except IOError:
                    for i in range(0, 100):
                        print module, ' #####: %u: NO COVERAGE' % i

        print self.modules


    def coverall(self):
        self.coverset = set([ "%s%s.gcov" % (module.split('/')[-1], extension) for module, extension, covered, total in list_modules()])
        self.fullmodules = dict([(module.split('/')[-1], module) for module, extension, covered, total in list_modules()])
        for module, extension, covered, total in list_modules():
            self.cover(module)
        self.findbest()

    def covercurrent(self):
        self.coverset = set([ "%s%s.gcov" % (module.split('/')[-1], extension) for module, extension, covered, total in list_modules()])
        self.fullmodules = dict([(module.split('/')[-1], module) for module, extension, covered, total in list_modules()])
        self.findbest()


cover = Cover()
if len(sys.argv) < 2:
    cover.covercurrent()
elif sys.argv[1] == 'all':
    cover.coverall()
elif sys.argv[1] == 'function' and len(sys.argv) > 2:
    pass
else:
    module = sys.argv[1]
    extension = ".hpp"
    if '/rio/' in module:
        extension = '.h'

    cover.cover(sys.argv[1])
    cover.coverset = set([ "%s%s.gcov" % (module.split('/')[-1], extension)])
    cover.fullmodules = dict([(module.split('/')[-1], module)])
    cover.findbest()



print "Coverage Results:"
for line in open("coverage.summary"):
    print line

