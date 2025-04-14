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
    
    # Create output files for this benchmark
    PERF_LOG="${RESULTS_DIR}/${PLATFORM}_${QUERY_NAME}_perf.log"
    MEM_LOG="${RESULTS_DIR}/${PLATFORM}_${QUERY_NAME}_mem.log"
    OUTPUT_LOG="${RESULTS_DIR}/${PLATFORM}_${QUERY_NAME}_output.log"
    
    # Clear any previous logs
    > $MEM_LOG
    
    # Run the query with time measurement
    START_TIME=$(date +%s.%N)
    
    # Start the query in background
    $EXEC_PATH > "$OUTPUT_LOG" 2>&1 &
    QUERY_PID=$!
    
    # Monitor memory usage
    while kill -0 $QUERY_PID 2>/dev/null; do
        get_memory_usage $QUERY_PID >> $MEM_LOG
        sleep 0.5
    done &
    MONITOR_PID=$!
    
    # Let the query run for the specified duration
    sleep $QUERY_DURATION
    
    # Stop the query process
    if kill -0 $QUERY_PID 2>/dev/null; then
        kill $QUERY_PID
    fi
    
    # Make sure memory monitoring stops
    if kill -0 $MONITOR_PID 2>/dev/null; then
        kill $MONITOR_PID
    fi
    
    # Calculate execution time
    END_TIME=$(date +%s.%N)
    if command -v bc &> /dev/null; then
        EXEC_TIME=$(echo "$END_TIME - $START_TIME" | bc)
    else
        # Fallback if bc is not available
        EXEC_TIME=$(echo "$END_TIME - $START_TIME" | awk '{print $1}')
    fi
    
    # Process memory logs
    if [ -s "$MEM_LOG" ]; then
        if command -v sort &> /dev/null; then
            MAX_MEM=$(sort -nr $MEM_LOG | head -n1)
        else
            MAX_MEM=$(awk 'BEGIN{max=0} {if($1>max) max=$1} END{print max}' $MEM_LOG)
        fi
        AVG_MEM=$(awk '{ sum += $1; n++ } END { if (n > 0) print sum / n; else print 0; }' $MEM_LOG)
    else
        MAX_MEM="N/A"
        AVG_MEM="N/A"
    fi
    
    # Extract throughput from the query output if available
    THROUGHPUT="N/A"
    if [ -f "$OUTPUT_LOG" ]; then
        if grep -i "messages per second" "$OUTPUT_LOG" > /dev/null; then
            THROUGHPUT=$(grep -i "messages per second" "$OUTPUT_LOG" | grep -o '[0-9]\+\.[0-9]\+' || echo "N/A")
        elif grep -i "processed" "$OUTPUT_LOG" > /dev/null; then
            PROCESSED_MSGS=$(grep -i "processed" "$OUTPUT_LOG" | grep -o '[0-9]\+' || echo "0")
            if [ "$PROCESSED_MSGS" != "0" ] && command -v bc &> /dev/null; then
                THROUGHPUT=$(echo "scale=2; $PROCESSED_MSGS / $EXEC_TIME" | bc)
            fi
        fi
    fi
    
    # Log results
    echo "${PLATFORM},${QUERY_NAME},$EXEC_TIME,$MAX_MEM,$AVG_MEM,$THROUGHPUT" >> $LOG_FILE
    
    echo "Benchmark completed for ${QUERY_NAME} on ${PLATFORM}"
    echo "  Execution time: ${EXEC_TIME} seconds"
    echo "  Max memory: ${MAX_MEM} MB"
    echo "  Avg memory: ${AVG_MEM} MB" 
    echo "  Throughput: ${THROUGHPUT} msgs/s"
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
for QUERY in Query1 Query2 Query3 Query4 Query5; do
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