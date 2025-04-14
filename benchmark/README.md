# NebulaStream Query Benchmarking

This directory contains tools for benchmarking NebulaStream query performance across different platforms.

## Overview

The benchmark script (`run_benchmarks.sh`) measures:
- Execution time
- Memory usage (peak and average)
- Throughput (messages per second)

For each of the query applications in the project (Query1 through Query5).

## Requirements

- Bash shell environment
- `ps` command for memory monitoring
- `bc` command for floating-point calculations (optional, will fallback to awk)
- The NebulaStream coordinator and worker must be running

## Running the Benchmarks

### Basic Usage

```bash
# Navigate to the benchmark directory
cd /path/to/nebulaQueryAPI/benchmark

# Make the script executable
chmod +x run_benchmarks.sh

# Run the benchmarks
./run_benchmarks.sh
```

### Configuration Options

You can customize the benchmark behavior with environment variables:

```bash
# Set the project root directory
PROJECT_ROOT=/custom/path/to/nebulaQueryAPI \
# Use a specific build directory
BUILD_DIR=/custom/path/to/build \
# Custom results directory
RESULTS_DIR=/custom/path/to/results \
# Duration to run each query (seconds)
QUERY_DURATION=15 \
# Skip the build step if already built
SKIP_BUILD=true \
# Copy results to a shared location
SHARED_RESULTS_DIR=/path/to/shared/results \
./run_benchmarks.sh
```

## Running on Different Platforms

### Mac/Desktop

```bash
./run_benchmarks.sh
```

### Raspberry Pi

```bash
# Set paths appropriate for your Raspberry Pi setup
PROJECT_ROOT=/home/pi/nebulaQueryAPI \
BUILD_DIR=/home/pi/nebulaQueryAPI/build \
RESULTS_DIR=/home/pi/benchmark_results \
./run_benchmarks.sh
```

### NVIDIA Jetson

```bash
# Set paths appropriate for your Jetson setup
PROJECT_ROOT=/home/nvidia/nebulaQueryAPI \
BUILD_DIR=/home/nvidia/nebulaQueryAPI/build \
RESULTS_DIR=/home/nvidia/benchmark_results \
./run_benchmarks.sh
```

## Collecting Results from Multiple Platforms

You can use a shared directory to collect results from different platforms:

1. Mount a shared directory on all platforms (e.g., NFS, Samba)
2. Run the benchmark on each platform with `SHARED_RESULTS_DIR` set to this shared location
3. Compare results across platforms

## Result Format

The benchmark generates:
1. A CSV summary file: `benchmark_summary_[platform].csv`
2. Individual log files for each query's output and memory usage
3. A console report at the end of the run

## Troubleshooting

- **Build Failures**: Check CMakeLists.txt for correct paths
- **Memory Monitoring Issues**: Ensure the `ps` command works on your platform
- **Query Failures**: Ensure the NebulaStream coordinator and worker are running
- **Path Issues**: Use absolute paths with the environment variables 