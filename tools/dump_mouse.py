import sys

with open(sys.argv[1], "rb") as f:
    line = "!"*16
    lg = 0
    while len(line) == 16:
        line = f.read(16)
        lg += len(line)
        print '"%s"' % ("".join([r"\x%0.2x" % ord(x) for x in line]))
    print ", %d" % lg
