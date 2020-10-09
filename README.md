# Technical Exercise for Software Developers

JSON (JavaScript Object Notation) is a popular text-based format for representing structured
data. More details about JSON can be found at https://www.json.org/
JSON documents must conform to various syntax rules in order to be successfully parsed. For
example, string literals must be enclosed in double quotes and contained double quotes must
be escaped using the escape sequence \”.
Sometimes malformed JSON documents are created by mistake, often due to buggy software.
Such malformed documents violate JSON’s syntax rules in some way, causing JSON parsers to
reject them. This can lead to a loss of data until the software producing the JSON is fixed.
Your task is to write a program that:
1. Accepts lines containing either valid or invalid JSON from stdin (you can assume that
each valid/invalid JSON document is separated by a newline character - there will be no
newlines within the documents themselves)
2. For each line, determines whether or not it is a valid JSON document
3. For each invalid JSON document, determines which of the following reasons cause it to
be invalid:
a. Unescaped double quote in string (e.g. “he said “hello””)
b. String enclosed by single quote instead of double quote (e.g. ‘hello’ instead
of “hello”)
c. Floating point number missing leading zero (e.g. .35 instead of 0.35)
d. Missing field value (e.g. {“field_name1” : , “field_name2” :
“field_value2”})
e. Other
4. For input JSON that is invalid for reasons a-d, produces a valid JSON document
(preserving as much information from the invalid document as possible) and writes this
to stdout
5. For input JSON that is valid, writes this to stdout
6. Writes any error messages to stderr

#### note that step d. Missing field value (e.g. {“field_name1” : , “field_name2” :“field_value2”}), is not implemented, 

#### to run the executable you need to pass an input file

#### running without output file will give the user the following output:
#### Usage: /tmp/foo <infile>
  
#### there are 2 json files attached to the project for unit testing 

#### in.json contains a valid json which will output the following:
#### /tmp/foo in.json
#### line #1, contains valid json
#### line #2, contains valid json
#### line #3, contains valid json

#### in2.json contains 4 cases as requested:
#### /tmp/foo in2.json
#### line #1, contains, #1 errors, atempting to fix them.
#### Missing field value, at: 34
#### {"tag1": true,"tAir": [9865, 9868,0.55 ]}
#### line #2, contains, #1 errors, atempting to fix them.
#### Unescaped double quote in a string, at: 14
#### {"tag1": true,"tAir": [9865, 9868,0.55 ]}
#### line #3, contains, #1 errors, atempting to fix them.
#### String enclosed by single quote instead of double quote, at: 14
#### {"tag1": true,"tAir": [9865, 9868,0.55 ]}
#### line #4, contains invalid json
