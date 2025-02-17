#!/bin/bash

set -e

filename="posix_tz_db.h"

printf "struct { const char* key; const char* value; } posix_tz_db[] = {\n" > $filename

cat $(dirname $0)/zones.csv | while read line 
do
    printf "    { %s },\n" $line >> $filename
done

printf "};\n" >> $filename
