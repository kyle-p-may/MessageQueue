#!/usr/bin/env python2
from __future__ import print_function

import subprocess

cxx="g++ "

cmdR=cxx + "-std=c++11 -g -Iinc/ tst/recv.cpp -o recv.exe -lrt"
cmdS=cxx + "-std=c++11 -g -Iinc/ tst/send.cpp -o send.exe -lrt"

subprocess.call(cmdR, shell=True)
subprocess.call(cmdS, shell=True)

print ("Compilation successful ...")
