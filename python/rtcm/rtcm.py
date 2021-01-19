#!/usr/bin/env python3
if __name__ == "__main__":
    with open("./../../datasets/data.bin", 'rb') as file:
        byte = file.read(1)
        while byte:
            print(hex(byte[0])) # how to print hex instead of ascii?
            byte = file.read(1)