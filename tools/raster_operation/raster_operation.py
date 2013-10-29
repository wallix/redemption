# -*- coding: utf-8 -*-
import unittest
import re

binop = {
    'a': lambda a, b: "(%s & %s)" % (a , b),
    'o': lambda a, b: "(%s | %s)" % (a , b),
    'x': lambda a, b: "(%s ^ %s)" % (a , b)
}

unaryop = {
    'n': lambda a: "~" + a
}

letter = {
    'P' : "pen",
    'D' : "dest",
    'S' : "source",
    '0' : "0x00",
    '1' : "0xFF"
}

def polish_eval(exp):
    pile = []
    for e in exp:
        if e in binop:
            b = pile.pop()
            a = pile.pop()
            pile.append(binop[e](a, b))
        elif e in unaryop:
            a = pile.pop()
            pile.append(unaryop[e](a))
        elif e in letter:
            pile.append(letter[e])
    return pile[0]



class TerRop:
    def __init__(self, hexa, rpn):
        self.hexa = hexa
        self.rpn = rpn

    def print_functor(self):
        expr = polish_eval(self.rpn)
        result = """struct Op3_%s  // %s
{
    uint8_t operator()(uint8_t dest, uint8_t source, uint8_t pen)
    {
        return %s;
    }
};
""" % (self.hexa, self.rpn, expr)
        return result

    def print_case(self, name, args):
        argstring = "(" + ", ".join(args) + ")"
        result = """case %s:
    this->%s<Op3_%s>%s;
    break;""" % (self.hexa, name, self.hexa, argstring)
        return result


def print_switch(ropl, name, args):
    cases = []
    for rop in ropl:
        cases.append(rop.print_case(name, args))
    result = """switch(rop) {
%s
default:
    break;
}
""" % '\n'.join(cases)
    return result


def write_rop3(ropl, output_path, fname, args):
    f = open(output_path, "w")
    for rop in ropl:
        f.write(rop.print_functor())
    f.write(print_switch(ropl, fname, args))
    f.close()

def read_rop3(file_path):
    f = open(file_path)
    g = f.read().split('\n')
    hexalist = []
    oplist = []
    for line in g:
        if re.match(r'^0x', line):
            hexalist.append(line)
        elif re.match(r'^RPN', line):
            oplist.append(line.split(" ")[1])
    f.close()
    return map(TerRop, hexalist, oplist)

class TestPolishEval(unittest.TestCase):
    def test1(self):
        self.assertEqual("~(dest | (pen | source))", polish_eval('DPSoon'))
        self.assertEqual("~(pen | source)", polish_eval('PSon'))
        self.assertEqual("(source & ~(dest | pen))", polish_eval('SDPona'))
    def test2(self):
        self.assertEqual("~(pen | (dest ^ source))", polish_eval('PDSxon'))
        self.assertEqual("~(pen | (source & ~dest))", polish_eval('PSDnaon'))

if __name__ == '__main__':
    rop_list = read_rop3("rop3.txt")
    write_rop3(rop_list, "rop3.hpp", "draw_ellipse", ["el", "fill", "fore_color", "back_color"])
    # unittest.main()
