#!/bin/bash

# ==============================================================================
# run_experiments.sh
#
# This script automates the process of running the cache simulator (csim)
# against multiple trace files and with various cache configurations.
# It outputs the results in a CSV format for easy analysis.
#
# Usage:
# 1. Make the script executable: chmod +x run_experiments.sh
# 2. Run it and redirect output to a file: ./run_experiments.sh > results.csv
# ==============================================================================

# Configuration

# Array of all trace files to be tested.
TRACE_FILES=(
    "gcc.trace"
    "read01.trace"
    "read02.trace"
    "read03.trace"
    "swim.trace"
    "write01.trace"
    "write02.trace"
)

# Array of different cache sizes (in bytes) to test.
CACHE_SIZES=(8192 16384 32768) # 8KB, 16KB, 32KB

# Array of different block sizes (in bytes) to test.
BLOCK_SIZES=(16 32 64)

# Array of different associativities (blocks per set) to test.
ASSOCIATIVITIES=(1 2 4 8) # 1 = Direct Mapped

# Array of eviction policies to test.
EVICTION_POLICIES=("lru" "fifo")

# Helper Function

# Function to check if a number is a power of 2
is_power_of_two() {
    local n=$1
    # A positive number n is a power of two if it has exactly one bit set.
    # In bash, ((...)) is used for arithmetic evaluation.
    (( n > 0 && (n & (n - 1)) == 0 ))
}


# Main Logic

# Print the CSV header row. This defines the columns of our output file.
''''''''''''''  
'///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////'
echo "Trace,TotalCacheSize,BlockSize,Associativity,NumSets,EvictionPolicy,Cycles,HitRate,MissRate,TotalLoads,TotalStores,LoadHits,LoadMisses,StoreHits,StoreMisses"

# Loop through each trace file.
for TRACE in "${TRACE_FILES[@]}"; do
    # Print progress to the console (stderr) so it doesn't interfere with CSV output.
    echo "Processing trace: $TRACE..." >&2

    # Loop through each desired total cache size.
    for CACHE_SIZE in "${CACHE_SIZES[@]}"; do
        # Loop through each block size.
        for BLOCK_SIZE in "${BLOCK_SIZES[@]}"; do
            # Loop through each associativity level.
            for ASSOC in "${ASSOCIATIVITIES[@]}"; do

                # Calculate the number of sets required for this configuration.
                # Bash only does integer arithmetic, which is fine here.
                NUM_SETS=$((CACHE_SIZE / (BLOCK_SIZE * ASSOC)))

                # Validation
                # Skip invalid configurations. The number of sets must be a positive power of 2.
                if ! is_power_of_two "$NUM_SETS"; then
                    continue # Skip to the next iteration of the loop.
                fi

                # Loop through each eviction policy.
                for EVICT in "${EVICTION_POLICIES[@]}"; do

                    # Construct the command to run the simulator.
                    # We will use a robust write-allocate/write-back policy for this analysis.
                    CMD="./csim $NUM_SETS $ASSOC $BLOCK_SIZE write-allocate write-back $EVICT < $TRACE"

                    # Run the command and capture its standard output into a variable.
                    STATS_OUTPUT=$(eval $CMD)

                    # Parse the Output
                    # Use grep to find the line and awk to extract the numeric value.
                    CYCLES=$(echo "$STATS_OUTPUT" | grep "Total cycles" | awk '{print $3}')
                    LOADS=$(echo "$STATS_OUTPUT" | grep "Total loads" | awk '{print $3}')
                    STORES=$(echo "$STATS_OUTPUT" | grep "Total stores" | awk '{print $3}')
                    LOAD_HITS=$(echo "$STATS_OUTPUT" | grep "Load hits" | awk '{print $3}')
                    LOAD_MISSES=$(echo "$STATS_OUTPUT" | grep "Load misses" | awk '{print $3}')
                    STORE_HITS=$(echo "$STATS_OUTPUT" | grep "Store hits" | awk '{print $3}')
                    STORE_MISSES=$(echo "$STATS_OUTPUT" | grep "Store misses" | awk '{print $3}')

                    # Calculate Derived Metrics
                    # Use 'bc' (basic calculator) for floating-point arithmetic.
                    TOTAL_HITS=$((LOAD_HITS + STORE_HITS))
                    TOTAL_ACCESSES=$((LOADS + STORES))

                    if [ "$TOTAL_ACCESSES" -gt 0 ]; then
                        HIT_RATE=$(echo "scale=6; $TOTAL_HITS / $TOTAL_ACCESSES" | bc)
                        MISS_RATE=$(echo "scale=6; 1 - $HIT_RATE" | bc)
                    else
                        HIT_RATE=0
                        MISS_RATE=0
                    fi

                    # Print the CSV Data Row
                    echo "$TRACE,$CACHE_SIZE,$BLOCK_SIZE,$ASSOC,$NUM_SETS,$EVICT,$CYCLES,$HIT_RATE,$MISS_RATE,$LOADS,$STORES,$LOAD_HITS,$LOAD_MISSES,$STORE_HITS,$STORE_MISSES"

                done # End eviction policy loop
            done # End associativity loop
        done # End block size loop
    done # End cache size loop
done # End trace file loop

echo "All experiments complete." >&2