#!/usr/bin/env python3
import argparse
import os
import re
import ntpath
import glob
from tabulate import tabulate


def Average(lst): 
    return sum(lst) / len(lst) 



parser = argparse.ArgumentParser(description='SQLite Speedtest1 parser')
parser.add_argument("path", help="directory with files")

args = parser.parse_args()

if not os.path.exists(args.path):
    print("Path '"+args.path +"' does not exist")
    exit(1)



srclist = [
"01_vanila.txt",
"02_unikraft.txt",
"03_genode_3.txt",
"04_genode_4.txt",
"05_cubicle_3.txt",
"06_cubicle_4.txt",
"07_cubicle_7.txt",
"08_sel4_3.txt",
"09_sel4_4.txt",
"10_foc_3.txt",
"11_foc_4.txt",
"12_nova_3.txt",
"13_nova_4.txt",
]
total = 13

data = []
norm = []

fn = 0
for file in srclist:
    f = open(args.path+'/'+file, "r")
    fstr = []
    for line in f:
#        print(line)
        fstr.append(float(str.split(line.split(". ")[1],'s')[0]))

    data.append(fstr)

for i in range(0, total):
    j = 0
    t = []
    a = 0.0
    for n in data[i]:
        t.append(n / data[0][j] )
        j = j + 1
    norm.append(t)


#for i in range(0, total):
#    for n in norm[i]:
#    for n in data[i]:
#        print("%f" % (n) )
#    print("\n")

print("\n Raw results compared to Linux:")

pavg = [[Average(norm[0]), Average(norm[1]), Average(norm[2]), Average(norm[3]), Average(norm[4]), Average(norm[5]),
Average(norm[6]), Average(norm[7]), Average(norm[8]), Average(norm[9]), Average(norm[10]), Average(norm[11]),Average(norm[12]),
]]

print (tabulate(pavg, headers=["Linux", "Unikraft", "G-3", "G-4", "COS-3", "COS-4", "COS-7", "SeL4-3", "SeL4-4", "FOC-3", "FOC-4", "NOVA-3", "NOVA-4"]))

print("\n 10a: Compared to Linux")

fig10a = [[1, Average(norm[1]),Average(norm[2]),Average(norm[3]),Average(norm[4]), Average(norm[5])]]
print (tabulate(fig10a, headers=["Linux", "Unikraft", "Genode-3", "Genode-4", "CubicleOS-3", "CubicleOS-4"]))

print("\n 10b: Compared to 3 components")

fig10b = [[
	Average([y / x for x, y in zip(data[7], data[8])]),
	Average([y / x for x, y in zip(data[9], data[10])]),
	Average([y / x for x, y in zip(data[11], data[12])]),
	Average([y / x for x, y in zip(data[2], data[3])]),
	Average([y / x for x, y in zip(data[4], data[5])]),
]]

print (tabulate(fig10b, headers=["SeL4", "Fiasco.OC", "NOVA", "Linux", "CubicleOS"]))


