#!/bin/bash

# Script to benchmark NebulaStream query performance across different platforms
# Measures: execution time, memory usage, and throughput

# Configuration (can be overridden by environment variables)
PROJECT_ROOT=${PROJECT_ROOT:-"$(cd "$(dirname "$0")/.." && pwd)"}
BUILD_DIR=${BUILD_DIR:-"${PROJECT_ROOT}/build"}
RESULTS_DIR=${RESULTS_DIR:-"${PROJECT_ROOT}/benchmark_results"}
QUERY_DURATION=${QUERY_DURATION:-10}  # How long to let each query run in seconds

# Detect system type
if [[ $(uname -m) == "aarch64" || $(uname -m) == "armv7l" ]]; then
    PLATFORM="raspberry_pi"
elif [[ -f "/etc/nv_tegra_release" ]]; then
    PLATFORM="nvidia_jetson"
elif [[ "$(uname)" == "Darwin" ]]; then
    PLATFORM="mac"
else
    PLATFORM="linux"
fi

echo "Detected platform: ${PLATFORM}"
echo "Using project root: ${PROJECT_ROOT}"
echo "Using build directory: ${BUILD_DIR}"

# Create results directory
mkdir -p $RESULTS_DIR

# Log file for all results
LOG_FILE="${RESULTS_DIR}/benchmark_summary_${PLATFORM}.csv"
echo "Platform,Query,Execution Time (s),Max Memory (MB),Avg Memory (MB),Throughput (msgs/s)" > $LOG_FILE

# Function to measure memory usage - platform specific
get_memory_usage() {
    local PID=$1
    
    case $PLATFORM in
        raspberry_pi|nvidia_jetson|linux)
            ps -o rss= -p $PID | awk '{print $1/1024}'
            ;;
        mac)
            ps -o rss= -p $PID | awk '{print $1/1024}'
            ;;
        *)
            echo "0"  # Default fallback
            ;;
    esac
}

# Function to run benchmark for a single query executable
run_benchmark() {
    QUERY_NAME=$1
    EXEC_PATH="${BUILD_DIR}/${QUERY_NAME}"
    
    echo "Running benchmark for ${QUERY_NAME} on ${PLATFORM}..."
    
    # Check if executable exists
    if [ ! -f "$EXEC_PATH" ]; then
        echo "Error: Executable not found at $EXEC_PATH"
        echo "${PLATFORM},${QUERY_NAME},Error: Executable not found,-,-,-" >> $LOG_FILE
        return
    fi
    
    local NUM_RUNS=5
    local TOTAL_EXEC_TIME=0
    local TOTAL_MAX_MEM=0
    local TOTAL_AVG_MEM_SUM=0 # Sum of average memories from each run
    local TOTAL_THROUGHPUT=0
    
    local SUCCESSFUL_EXEC_RUNS=0
    local SUCCESSFUL_MEM_RUNS=0
    local SUCCESSFUL_TP_RUNS=0

    echo "Running benchmark for ${QUERY_NAME} on ${PLATFORM} (${NUM_RUNS} iterations)..."

    for i in $(seq 1 $NUM_RUNS); do
        echo "  Iteration $i/$NUM_RUNS for ${QUERY_NAME}..."

        # Iteration specific log files
        MEM_LOG_ITER="${RESULTS_DIR}/${PLATFORM}_${QUERY_NAME}_mem_iter${i}.log"
        OUTPUT_LOG_ITER="${RESULTS_DIR}/${PLATFORM}_${QUERY_NAME}_output_iter${i}.log"
        > "$MEM_LOG_ITER" # Clear/create memory log for the iteration
        > "$OUTPUT_LOG_ITER" # Clear/create output log for the iteration

        START_TIME_ITER=$(date +%s.%N)
        
        # Start the query in background for the current iteration
        "$EXEC_PATH" > "$OUTPUT_LOG_ITER" 2>&1 &
        QUERY_PID=$!
        
        # Monitor memory usage for this iteration
        MONITOR_PID_ITER=""
        if [ -n "$QUERY_PID" ]; then
            while kill -0 $QUERY_PID 2>/dev/null; do
                get_memory_usage $QUERY_PID >> "$MEM_LOG_ITER"
                sleep 0.5 # Interval for memory sampling
            done &
            MONITOR_PID_ITER=$!
        fi
        
        # Let the query run for the specified duration
        sleep "$QUERY_DURATION"
        
        # Stop the query process if it's still running
        if kill -0 $QUERY_PID 2>/dev/null; then
            kill $QUERY_PID
        fi
        
        # Make sure memory monitoring for this iteration stops
        if [ -n "$MONITOR_PID_ITER" ] && kill -0 $MONITOR_PID_ITER 2>/dev/null; then
            kill $MONITOR_PID_ITER
        fi
        wait $QUERY_PID 2>/dev/null # Ensure query process is reaped

        END_TIME_ITER=$(date +%s.%N)
        
        # Calculate metrics for THIS iteration
        EXEC_TIME_ITER="N/A"
        if command -v bc &> /dev/null; then
            EXEC_TIME_ITER=$(echo "$END_TIME_ITER - $START_TIME_ITER" | bc)
        else
            EXEC_TIME_ITER=$(echo "$END_TIME_ITER - $START_TIME_ITER" | awk '{print $1}') # Fallback
        fi

        MAX_MEM_ITER="N/A"
        AVG_MEM_ITER="N/A"
        if [ -s "$MEM_LOG_ITER" ]; then # Check if memory log has content
            if command -v sort &> /dev/null; then
                MAX_MEM_ITER=$(sort -nr "$MEM_LOG_ITER" | head -n1)
            else
                MAX_MEM_ITER=$(awk 'BEGIN{max=0} {if($1>max) max=$1} END{print max}' "$MEM_LOG_ITER")
            fi
            AVG_MEM_ITER=$(awk '{ sum += $1; n++ } END { if (n > 0) print sum / n; else print 0; }' "$MEM_LOG_ITER")
        fi

        THROUGHPUT_ITER="N/A"
        if [ -f "$OUTPUT_LOG_ITER" ]; then
            TEMP_TP="N/A" # Use a temporary variable for throughput
            if grep -i "messages per second" "$OUTPUT_LOG_ITER" > /dev/null; then
                TEMP_TP=$(grep -i "messages per second" "$OUTPUT_LOG_ITER" | grep -o '[0-9]\+\.\?[0-9]*' | head -n1 || echo "N/A")
            elif grep -i "processed" "$OUTPUT_LOG_ITER" > /dev/null; then
                PROCESSED_MSGS=$(grep -i "processed" "$OUTPUT_LOG_ITER" | grep -o '[0-9]\+' | head -n1 || echo "0")
                if [ "$PROCESSED_MSGS" != "0" ] && [ "$EXEC_TIME_ITER" != "N/A" ] && command -v bc &> /dev/null && [ "$(echo "$EXEC_TIME_ITER > 0" | bc -l)" -eq 1 ]; then
                     TEMP_TP=$(echo "scale=2; $PROCESSED_MSGS / $EXEC_TIME_ITER" | bc)
                fi
            fi
            THROUGHPUT_ITER=$TEMP_TP
        fi

        echo "    Iter $i: Time=${EXEC_TIME_ITER}s, MaxMem=${MAX_MEM_ITER}MB, AvgMem=${AVG_MEM_ITER}MB, TP=${THROUGHPUT_ITER}msgs/s"

        # Accumulate results if they are numeric
        if [[ "$EXEC_TIME_ITER" != "N/A" ]] && [[ "$EXEC_TIME_ITER" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
            TOTAL_EXEC_TIME=$(echo "$TOTAL_EXEC_TIME + $EXEC_TIME_ITER" | bc)
            ((SUCCESSFUL_EXEC_RUNS++))
        fi
        if [[ "$MAX_MEM_ITER" != "N/A" ]] && [[ "$MAX_MEM_ITER" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
            TOTAL_MAX_MEM=$(echo "$TOTAL_MAX_MEM + $MAX_MEM_ITER" | bc)
            # If MAX_MEM is valid, AVG_MEM_ITER should also be processed if numeric
            if [[ "$AVG_MEM_ITER" != "N/A" ]] && [[ "$AVG_MEM_ITER" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
                TOTAL_AVG_MEM_SUM=$(echo "$TOTAL_AVG_MEM_SUM + $AVG_MEM_ITER" | bc)
            fi
            ((SUCCESSFUL_MEM_RUNS++)) # Count runs where memory data (at least max) was available
        fi
        if [[ "$THROUGHPUT_ITER" != "N/A" ]] && [[ "$THROUGHPUT_ITER" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
            TOTAL_THROUGHPUT=$(echo "$TOTAL_THROUGHPUT + $THROUGHPUT_ITER" | bc)
            ((SUCCESSFUL_TP_RUNS++))
        fi
        
        # Optional: clean up iteration logs if not needed for debugging
        # rm -f "$MEM_LOG_ITER" "$OUTPUT_LOG_ITER"
    done # End of iterations for a query

    # Calculate final averages
    AVG_EXEC_TIME_FINAL="N/A"
    AVG_MAX_MEM_FINAL="N/A"
    AVG_AVG_MEM_FINAL="N/A" # This will be the average of average memories
    AVG_THROUGHPUT_FINAL="N/A"

    if [ "$SUCCESSFUL_EXEC_RUNS" -gt 0 ]; then
        AVG_EXEC_TIME_FINAL=$(echo "scale=2; $TOTAL_EXEC_TIME / $SUCCESSFUL_EXEC_RUNS" | bc)
    fi
    if [ "$SUCCESSFUL_MEM_RUNS" -gt 0 ]; then
        AVG_MAX_MEM_FINAL=$(echo "scale=2; $TOTAL_MAX_MEM / $SUCCESSFUL_MEM_RUNS" | bc)
        AVG_AVG_MEM_FINAL=$(echo "scale=2; $TOTAL_AVG_MEM_SUM / $SUCCESSFUL_MEM_RUNS" | bc) # Average of the per-iteration averages
    fi
    if [ "$SUCCESSFUL_TP_RUNS" -gt 0 ]; then
        AVG_THROUGHPUT_FINAL=$(echo "scale=2; $TOTAL_THROUGHPUT / $SUCCESSFUL_TP_RUNS" | bc)
    fi

    # Log average results to the main CSV file
    echo "${PLATFORM},${QUERY_NAME},${AVG_EXEC_TIME_FINAL},${AVG_MAX_MEM_FINAL},${AVG_AVG_MEM_FINAL},${AVG_THROUGHPUT_FINAL}" >> $LOG_FILE
    
    echo "Benchmark STAGE completed for ${QUERY_NAME} on ${PLATFORM} (Averages over up to $NUM_RUNS runs)"
    echo "  Avg Execution time: ${AVG_EXEC_TIME_FINAL} seconds (from $SUCCESSFUL_EXEC_RUNS successful runs)"
    echo "  Avg Max memory: ${AVG_MAX_MEM_FINAL} MB (from $SUCCESSFUL_MEM_RUNS successful runs)"
    echo "  Avg Avg memory: ${AVG_AVG_MEM_FINAL} MB (from $SUCCESSFUL_MEM_RUNS successful runs)"
    echo "  Avg Throughput: ${AVG_THROUGHPUT_FINAL} msgs/s (from $SUCCESSFUL_TP_RUNS successful runs)"
    echo "----------------------------------------"
}

# Build all query executables if needed
if [ "$SKIP_BUILD" != "true" ]; then
    echo "Building query executables..."
    
    # Make sure build directory exists
    mkdir -p "$BUILD_DIR"
    
    # Navigate to build directory and build
    (cd "$BUILD_DIR" && cmake "$PROJECT_ROOT" && make -j$(nproc 2>/dev/null || echo 2))
    
    if [ $? -ne 0 ]; then
        echo "Error: Build failed!"
        exit 1
    fi
else
    echo "Skipping build step (SKIP_BUILD=true)"
fi

# Run benchmarks for each query
for QUERY in QueryTest Query1 Query1-NoMEOS Query2 Query2-NoMEOS Query3 Query3-NoMEOS Query4 Query4-NoMEOS Query5 Query5-NoMEOS; do
    run_benchmark $QUERY
done

echo "All benchmarks completed. Results saved to $LOG_FILE"
echo "Individual logs stored in $RESULTS_DIR"

# Generate a simple report
echo "
Benchmark Summary for ${PLATFORM}:
----------------------------------------"
if command -v column &> /dev/null; then
    column -t -s, $LOG_FILE
else
    cat $LOG_FILE | tr ',' '\t'
fi

# Optionally copy results to a shared location if specified
if [ -n "$SHARED_RESULTS_DIR" ] && [ -d "$SHARED_RESULTS_DIR" ]; then
    cp -r "$RESULTS_DIR"/* "$SHARED_RESULTS_DIR/"
    echo "Results copied to shared directory: $SHARED_RESULTS_DIR"
fi 