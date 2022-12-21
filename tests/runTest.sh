#!/usr/bin/env bash
set -euo pipefail
PORT=4242

# if [ "$#" -ne 1 ]; then
#   echo "Usage: $(basename "$0") <path/to/file.csv>"
#   exit 1
# fi
# test -f "$1" || (echo "\"$1\": No such file or directory" && exit 1)

# Prepare the data
# data/splitCSV.sh "$1"

# Spawn the coordinator process
cmake-build-debug/coordinator "filelist.csv" $PORT &

# Spawn some workers
for _ in {1..4}; do
  cmake-build-debug/worker "127.0.0.1" "$PORT" &
done

sleep 1

echo "a" > fifo

# And wait for completion
time wait
