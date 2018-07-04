#! /usr/bin/python
# -*- encoding: utf-8 -*-
from numpy import *
import copy

unknown_char = "?"

class Node:
    lbl2char = []

    def __init__(self):
        self.children = []
        self.dim = None
        self.value = None

    def printme(self, tab):
        res = tab * "  " + str((self.value, self.dim))
        if len(self.children) == 0:
            res += " --> " + str(self.label)
        res += "\n"
        for c in self.children:
            res += c.printme(tab+1)
        return res

    def _get_name_var(self):
        if self.dim == 0:
            return "count"
        elif self.dim == 1:
            return "width"
        elif self.dim == 2:
            return "height"
        else:
            return "pixel(%s)" % (self.dim-3)

    def _c_str(self, tab):
        if len(self.children) == 0:
            return "  " * tab + "return \"%s\";\n" % self.lbl2char[self.label]
        s = "  " * tab
        if (len(self.children) > 4):
            i = len(self.children)/2
            s += "if ("
            s += self._get_name_var()
            s += " < %s) {\n" % self.children[i].value
            c = Node()
            c.children = self.children[:i]
            c.dim = self.dim
            s += c._c_str(tab + 1)
            s += "  " * tab
            s += "} else /* NOLINT */ {\n"
            c.children = self.children[i:]
            s += c._c_str(tab + 1)
            s += "  " * tab
            s += "}\n"
        else :
            for c in self.children:
                s += "if ("
                s += self._get_name_var()
                s += " == %s) {\n" % c.value
                s += c._c_str(tab + 1)
                s += "  " * tab + "} else "
            s += "{}\n"
        return s

    def c_str(self):
        return self._c_str(0)

    def __str__(self):
        return self.printme(0)


def gini(labels):
    if len(labels) == 0:
        return 0
    hist = bincount(labels) / float(len(labels))
    return 1.0 - square(hist).sum()


def cart_(data, labels, attributes):
    lbls = unique(labels);
    if len(lbls) == 1:
        root = Node()
        root.label = labels[0]
        return root

    assert( len(attributes) > 0 ) # We can't descriminate anymore

    # Get the best attribute
    G = empty( (len(attributes)) )
    for i, dim in enumerate(attributes):
        idx1 = flatnonzero(data[:, dim])
        idx2 = flatnonzero(logical_not(data[:, dim]))
        G[i] = (gini(labels[idx1]) + gini(labels[idx2])) / 2
    A = attributes[G.argmin()]

    idx1 = flatnonzero(data[:, A])
    idx2 = flatnonzero(logical_not(data[:, A]))
    if len(idx1) == 0 or len(idx2) == 0:  # We can't descriminate anymore
        print "Erreur: les labels suivants sont les mêmes:"
        print map(lambda k: Node.lbl2char[k], labels)
        print "Vérifier que deux caractères identiques n'apparaissent pas deux fois dans les fichiers sous des labels (encodage) différents."
        assert(False)

    # Create node and run subsets
    attr = copy.deepcopy(attributes)
    attr.remove(A)
    root = Node()
    root.dim = A
    lchild = cart_(data[idx1, :], labels[idx1], attr)
    rchild = cart_(data[idx2, :], labels[idx2], attr)
    lchild.value = 1
    rchild.value = 0
    root.children.append(lchild)
    root.children.append(rchild)
    return root

def cart(dim, data, labels):
    lbls = unique(labels);
    if len(lbls) == 1:
        root = Node()
        root.label = labels[0]
        return root

    values = unique(map(lambda x: x[dim], data))
    tree = Node()
    for v in values:
        (idx, ) = where(map(lambda x: x[dim] == v, data))
        if dim < 2:
            ndata = data[idx]
            nlabels = labels[idx]
            noeud = cart(dim + 1, ndata, nlabels)
            noeud.dim = dim + 1
        else:
            sz = len(data[idx[0]])
            ndata = empty( (len(idx), sz), dtype=int)
            nlabels = labels[idx]
            attributes = range(3, sz)
            for i, j in enumerate(idx):
                ndata[i, :] = data[j]
            noeud = cart_(ndata, nlabels, attributes)
        noeud.value = v
        tree.children.append(noeud)
    return tree


if __name__ == "__main__":
    data = array([ array([15, 5, 5, 0, 0, 0, 1]),
                   array([28, 5, 5, 0, 1, 0]),
                   array([28, 5, 6, 0, 0]),
                   array([28, 5, 5, 0, 0, 0]),
                   array([28, 5, 5, 0, 1, 1]) ], dtype=object)
    labels = arange(len(data))
    lbl2char = ["a", "ï", "ô", "ù", "&" ]
    tree = cart(0, data, labels)
    tree.value = "root"
    tree.dim = 0
    print tree.c_str()
