#!/bin/bash

datasets=("uk-2007-05@100000")  # List of datasets
#"uk_1000000" "in-2004" "eu-2005" "indochina" "arabic-2005" "sk-2005" "it-2004" "uk-2007-05" "gsh-2015-host"
batch_nums=("1" "5" "8" "10" "12" "15" "20")  # List of batch_nums

# Function to stop the monitor script
stop_monitor() {
    echo "Stopping monitor script"
    kill $monitor_pid $pid
    exit
}

# Trap the SIGINT signal (Ctrl+C) to stop the monitor script
trap stop_monitor SIGINT

# Iterate over datasets
for dataset in "${datasets[@]}"; do
    # Iterate over batch_nums
    for part_num in "${batch_nums[@]}"; do
        echo "Compress for dataset: $dataset, part_num: $part_num"

        # Start the Laconic_serial_compression.py script in the background and get the process ID (PID)
        python3 Laconic_serial_compression.py "$dataset" --part_num "$part_num" &
        pid=$!

        # Monitor the VmRSS every second and output it to a text file
        while true; do
            # Check if the Laconic_serial_compression.py script is still running
            if ! ps -p $pid > /dev/null; then
                echo "Laconic_serial_compression.py has exited"
                break
            fi

            timestamp=$(date "+%Y-%m-%d %H:%M:%S")
            vmrss=$(ps -o pid=,rss= -p $pid | awk '{print $2}')
            echo "$timestamp Dataset: $dataset, Part_num: $part_num, VmRSS: $vmrss kB" >> ../../benchmarks/$dataset/peak_memory/$part_num+rule.txt
            sleep 0.0005
        done

        echo "Finished compress.py for dataset: $dataset, part_num: $part_num"
        sleep 5
    done
done