#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Usage: $0 basedir [server_url]" >&2
    exit 1
fi

TEST_DIR=$1

if [ ! -f  "$TEST_DIR/tests.txt" ]; then
   echo "Error: $TEST_DIR/tests.txt does not exist!" >&2
   exit 1
fi
   
server_url=http://localhost:8080
if [ $# -eq 2 ]; then
    server_url=$2
fi

function check_reply() {
    local reply_file="$1"
    local expected_file="$2"
    if diff "$reply_file" "$expected_file" > /dev/null; then
	# files are identical
	return 0
    else
	return 1
    fi
    
}

function test_url() {
    local test_name="$1"
    local full_url="$server_url/$2"
    local expected="$3"
    local tmpfile=$(mktemp)

    echo -n "Testing $test_name (url: $full_url) ..."
    if ! curl --no-progress-meter $full_url |grep -v "<p>Connection counter:"  > $tmpfile ; then
	echo " -> FAILED (server crashed)"
	return 1
    fi
    if [ -n "$expected" ]; then
	check_reply $tmpfile $expected
	if [ "$?" -eq 0 ]; then
	    echo " -> SUCCESS"
	    rm $tmpfile
	    return 0
	else
	    echo " -> FAILED (bad reply: received: $tmpfile, expected: $expected)"
	    return 1
	fi
    fi
    rm $tmpfile
    echo " -> SUCCESS"
    return 0
}

while read line ; do
    [[ "$line" =~ ^#.*$ ]] && continue # ignore lines that start with #
    read name url expected_result <<< $( echo $line)
    test_url "$name" "$url" "$TEST_DIR/$expected_result"
done < "$TEST_DIR/tests.txt"
