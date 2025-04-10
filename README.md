# NebulaStream CSV Worker and Client Example

This project demonstrates how to set up a NebulaStream worker that serves a CSV file as a data source, and a client that can query this data.

## Prerequisites

- NebulaStream installed at `/media/psf/Home/Parallels/nebulastream`
- NebulaStream coordinator running at `127.0.0.1:8081`
- C++17 compatible compiler
- CMake 3.12 or higher

## Project Structure

- `nesworker.cpp` - The NebulaStream worker that serves a CSV file as a data source
- `query.cpp` - The NebulaStream client that queries data from the worker
- `CMakeLists.txt` - Build configuration

## CSV File Setup

Place your CSV file at:

```
data/selected_columns_df.csv
```

Make sure your CSV file has the following columns (or modify the schema in `nesworker.cpp` to match your data):

- timestamp (UINT64)
- id (UINT64)
- Vbat (FLOAT64)
- PCFA_mbar (FLOAT64)
- PCFF_mbar (FLOAT64)
- PCF1_mbar (FLOAT64)
- PCF2_mbar (FLOAT64)
- T1_mbar (FLOAT64)
- T2_mbar (FLOAT64)
- Code1 (UINT64)
- Code2 (UINT64)
- speed (FLOAT64)
- latitude (FLOAT64)
- longitude (FLOAT64)

## Building

```bash
mkdir -p build
cd build
cmake ..
make
```

This will build two executables:
- `NebulaStreamWorker` - The NebulaStream worker
- `NebulaStreamC++Example` - The NebulaStream client

## Running

1. First, make sure the NebulaStream coordinator is running at `127.0.0.1:8081`

2. Start the worker:

```bash
./NebulaStreamWorker
```

The worker will connect to the coordinator and register a physical source for the CSV file.

3. Run the client:

```bash
./NebulaStreamC++Example
```

The client will connect to the coordinator, execute a query on the CSV data, and print the results.

## Customization

- To modify the CSV data source, edit the schema and file path in `nesworker.cpp`
- To modify the query, edit the query logic in `query.cpp`

## Troubleshooting

- Check `nesworker.log` for worker logs
- Ensure the CSV file exists at the specified path
- Make sure the NebulaStream coordinator is running and accessible
- Verify that the schema in the worker matches the CSV file structure

## Note on Physical vs. Logical Sources

In NebulaStream:
- A logical source defines the schema of the data
- A physical source defines how to acquire the data

The worker registers a physical source (CSV file) that implements a logical source (schema).
The client queries the logical source by name ("gpsData"). 