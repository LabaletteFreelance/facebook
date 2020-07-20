#!/bin/bash
if [ $# -ne 3 ]; then
    echo "Usage: $0 basedir test_name test_url" >&2
    exit 1
fi

base="$1"
name="$2"
url="$3"
res_file="expected_result_${2}.dat"

if ! [ -d "$base" ]; then
    mkdir "$base"
    echo -e "#name\turl\texpected_result" > "$base/tests.txt"
fi

server_url=http://localhost:8080

full_url="$server_url/$url"
curl --no-progress-meter $full_url |grep -v "<p>Connection counter:" > "$base/$res_file"
echo -e "$name\t$url\t$res_file" >> "$base/tests.txt"
