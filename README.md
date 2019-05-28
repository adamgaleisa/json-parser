# senseon
Senseon Technical Exercise for Software Developers

to build the project run the following steps:
method 1 run:
./build.sh

method 2 run:
g++ -o /tmp/foo -std=c++14  main.cpp reader.cpp  -Wall -O3

note that step d. Missing field value (e.g. {“field_name1” : , “field_name2” :“field_value2”}), is not implemented, 

to run the executable you need to pass an input file

running with output file will give the user the following output:
Usage: /tmp/foo <infile>
  
there are 2 json files attached to the project for unit testing 

in.json contains a valid json which will output the following:
/tmp/foo in.json
line #1, contains valid json
line #2, contains valid json
line #3, contains valid json

in2.json contains 4 cases as requested:
/tmp/foo in2.json
line #1, contains, #1 errors, atempting to fix them.
Missing field value, at: 34
{"tag1": true,"tAir": [9865, 9868,0.55 ]}
line #2, contains, #1 errors, atempting to fix them.
Unescaped double quote in a string, at: 14
{"tag1": true,"tAir": [9865, 9868,0.55 ]}
line #3, contains, #1 errors, atempting to fix them.
String enclosed by single quote instead of double quote, at: 14
{"tag1": true,"tAir": [9865, 9868,0.55 ]}
line #4, contains invalid json


