#!/usr/bin/env python3

import sys
if sys.version_info < (3, 0):
    sys.stdout.write("The script requires Python 3.x, not Python 2.x\n")
    print (sys.version_info)
    sys.exit(1)


def Main():
    print("hello")

if __name__ == "__main__":
    Main()