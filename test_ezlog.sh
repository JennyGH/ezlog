#!/bin/bash
EXE="`pwd`"/built/linux/x86_64/Release/bin/ezlog_test
BUFF_SIZE=$[1024*1024]
ROLL_SIZE=$[1024*1024*10]
OUTPUT_DIR="`pwd`"/test_logs

function do_test() {
    rm -f "$OUTPUT_DIR"/*.log
    "$EXE" -output_dir="$OUTPUT_DIR" -thread_count=$(nproc) -async_buffer_size=$1 -roll_size=$2 >> test_report_linux.txt
}

bash build-linux.sh

if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir "$OUTPUT_DIR"
fi

rm -f "`pwd`"/test_report_linux.txt

for i in {0..1}; do
    for j in {0..1}; do
        do_test $[i*BUFF_SIZE] $[j*ROLL_SIZE]
    done
done