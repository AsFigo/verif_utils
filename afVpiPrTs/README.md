# AsFigo Timescale Extraction Utility (`$afPrTs`)

## Table of Contents
- [Overview](#overview)
- [Key Features](#key-features)
- [File Structure](#file-structure)
- [Building and Installation](#building-and-installation)
- [Usage](#usage)
- [Output Format](#output-format)
- [Error Handling](#error-handling)
- [License](#license)
- [Credits](#credits)

## Overview

The AsFigo Timescale Extraction Utility (`$afPrTs`) is a Verilog Procedural Interface (VPI) application designed to extract and
log timescale information of modules in a Verilog simulation. 
This app provides insight into the time units and precisions used across different modules, 
helping developers verify consistency and catch potential timing discrepancies in their designs.

## Key Features

- **Module Timescale Extraction**: Gathers timescale units and precisions for each module in the design.
- **CSV Output**: Saves the extracted information in `output_tscale_info.csv` with the following columns:
  - Module Name
  - Time Unit
  - Time Precision
- **VPI-Based Application**: Integrates with simulation environments that support IEEE 1364 PLI/VPI, allowing it to operate on a wide range of simulators.
- **Start of Simulation Callback**: Automatically initiates at the start of simulation, providing a brief utility header.

## File Structure

- **`afPrTsCompiletf`**: Verifies that `$afPrTs` is called without arguments.
- **`afPrTsCalltf`**: Main function that iterates through top-level modules, writes timescale information to the CSV, and handles errors.
- **`afPrTsTraverse`**: Recursively traverses through the module hierarchy.
- **`afPrTsCurNode`**: Retrieves the time unit and precision for a single module.
- **`tu2Str`**: Converts numerical timescale values to human-readable strings (e.g., `-12` to `"ps"`).
- **`afPrTsStartOfSim`**: Start-of-simulation callback, prints a utility message.

## Building and Installation

1. **Prerequisites**: Ensure you have a Verilog simulator that supports VPI (e.g., Icaurs Verilog, QuestaSim, Xcelium, VCS, etc.).
2. **Compile the VPI Library**: Build the shared library (e.g., `.so` for Linux, `.dll` for Windows) for your simulator:
    ```sh
    gcc -shared -fPIC  -I<simulator_include_path> ../vpi_src/afVpiPrTs.cc -o afPrTsVpi.so
    ```
    Replace `<simulator_include_path>` with the path to your simulator's VPI headers.

3. **Simulator Setup**: Load the compiled VPI library in your simulator. Typically, this can be done by adding a command line argument to your simulation command:
    ```sh
    vsim -pli afPrTsVpi.so <your_verilog_testbench>
    ```
3. **Fully working example**: A simple example is provided with Makefile to run this app with opensoruce Icarus Verilog simulator. To run this example:
    ```sh
    cd exec_dir
    make
    ```
## Usage

Invoke the `$afPrTs` system task in your Verilog testbench to trigger the timescale extraction.  
The results will be saved in a CSV file (`output_tscale_info.csv`) in the simulation directory.

### Example Verilog Usage

```verilog
module testbench;
  initial begin
    $afPrTs; // Trigger the timescale extraction
  end
endmodule
```

## Output Format

The generated CSV file, `output_tscale_info.csv`, contains the following columns:

| Module       | Time Unit | Time Precision |
|--------------|-----------|----------------|
| `module_name` | `1 ns`     | `100 ps`       |

## Error Handling

Errors encountered during execution (e.g., file I/O issues or argument misuse) will be reported to the simulator console. 
Common errors include:
- **`ERR_FILE_OPEN`**: Unable to open CSV file for writing.
- **`ERR_MOD_ITERATION`**: Unable to iterate over modules.
- **`ERR_SYSTF_CALL`**: Failure to acquire a handle for `$afPrTs` task.
- **`ERR_SYSTF_USAGE`**: `$afPrTs` was called with invalid arguments.

## License

This project is licensed under the MIT License. See the `SPDX-License-Identifier: MIT` statement 
in the source code headers for more information.

## Credits
Original version of this app was developed in early 2000's and we derived from that example: https://vseenu.tripod.com/

