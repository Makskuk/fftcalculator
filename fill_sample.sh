#!/bin/bash

i=1024
SAMPLE_FILE=sample.txt

if [ $1 ]; then
	i=$1
fi

echo "write $i strings to $SAMPLE_FILE"

if [ -e $SAMPLE_FILE ]; then
	rm $SAMPLE_FILE
fi

((i=i/13))

while [[ $i -ge 0 ]]; do
cat <<EOF >> $SAMPLE_FILE
0.1111
0.1112
0.1222
0.1278
0.1298
0.1335
0.1456
0.1335
0.1298
0.1278
0.1222
0.1112
0.1111
EOF
((i--))
done
