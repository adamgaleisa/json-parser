#!/bin/bash
g++ -o /tmp/foo -std=c++14  main.cpp reader.cpp  -Wall -O3
/tmp/foo in2.json
