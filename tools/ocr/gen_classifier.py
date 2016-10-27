#! /usr/bin/python
import csv
import os
import cart
import numpy

VERBOSE = int(os.getenv("DEBUG", 0))

import sys

if len(sys.argv) != 3:
    print "Usage: python gen_classifier.py features.txt classifier.hxx"
    sys.exit(-1)

char2lbl = {}

f = open(sys.argv[1])
reader = csv.reader(f, delimiter="\t")
labels = []
data = []
nlabel = 0
for record in reader:
     c = record[0]
     if not c in char2lbl:
          char2lbl[c] = nlabel
          nlabel += 1
     labels.append(char2lbl[c])
     data.append(map(int, numpy.array(record[1:])))

cart.Node.lbl2char = dict((v,k) for k, v in char2lbl.iteritems())

#if VERBOSE > 0:
#     rev = {}
#     for k, v in zip(labels, data):
#          k2 = (v[0], v[1], v[2])
#          k = cart.Node.lbl2char[k]
#          if k2 in rev:
#               rev[k2].append((k,v))
#          else:
#               rev[k2] = [(k,v)]

tree = cart.cart(0, numpy.array(data, dtype=object), numpy.array(labels))
tree.value = "root"
tree.dim = 0

output = open(sys.argv[2], "w")
output.write( tree.c_str() )


def dotNode(f, node):
     nodeid = node.id
     if len(node.children) > 0: # Non terminal
          if node.dim == 0:
               f.write('%i [label="dim: npixels"];\n' % node.id)
          elif node.dim == 1:
               f.write('%i [label="dim: width"];\n' % node.id)
          elif node.dim == 2:
               f.write('%i [label="dim: height"];\n' % node.id)
          else:
               f.write('%i [label="dim: pixel %i"];\n' % (node.id, node.dim-3))

          for c in node.children:
               nodeid += 1
               c.id = nodeid
               f.write('%i -> %i [label="%s"];\n' % (node.id, c.id, c.value))
               nodeid = dotNode(f, c)
     else: # Terminal
          f.write('%i [label="char: %s"];\n' % (node.id, cart.Node.lbl2char[node.label]))
     return nodeid


if VERBOSE > 0:
     gfile = open("decisiontree.dot", "w")
     gfile.write("digraph G {\n")
     tree.id = 1
     dotNode(gfile, tree)
     gfile.write("}\n")
     gfile.close()


#
# for k in sorted(rev.iterkeys()):
#     if (len(rev[k]) == 1):
#         print "if (count == %s && width == %s && height == %s) {" % k
#         print "\treturn '%s';" % rev[k][0]
#         print "} else",

# print "return '?';"
