# AsFigo Instance Extraction Utility (`$afPrHier`)

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

AsFigo Instance Extraction Utility (`$afPrHier`) is a Verilog Procedural Interface (VPI) application designed to extract and
log instance hierarchy information of modules in a Verilog simulation. 

## Key Features

- **Module Instance Extraction**: Gathers information for each module in the design.
- **CSV Output**: Saves the extracted information in `output_hier_info.csv` with the following columns:
  - Module Name
  - Instance Name
- **VPI-Based Application**: Integrates with simulation environments that support IEEE 1364 PLI/VPI, allowing it to operate on a wide range of simulators.
- **Start of Simulation Callback**: Automatically initiates at the start of simulation, providing a brief utility header.

## File Structure

- **`afPrHierCompiletf`**: Verifies that `$afPrHier` is called without arguments.
- **`afPrHierCalltf`**: Main function that iterates through top-level modules, writes information to the CSV, and handles errors.
- **`afPrHierTraverse`**: Recursively traverses through the module hierarchy.
- **`afPrHierCurNode`**: Retrieves the information for a single module.
- **`afPrHierStartOfSim`**: Start-of-simulation callback, prints a utility message.

## Building and Installation

1. **Prerequisites**: Ensure you have a Verilog simulator that supports VPI (e.g., Icaurs Verilog, QuestaSim, Xcelium, VCS, etc.).
2. **Compile the VPI Library**: Build the shared library (e.g., `.so` for Linux, `.dll` for Windows) for your simulator:
    ```sh
    gcc -shared -fPIC  -I<simulator_include_path> ../vpi_src/afVpiPrHier.cc -o afPrHierVpi.so
    ```
    Replace `<simulator_include_path>` with the path to your simulator's VPI headers.

3. **Simulator Setup**: Load the compiled VPI library in your simulator. Typically, this can be done by adding a command line argument to your simulation command:
    ```sh
    vsim -pli afPrHierVpi.so <your_verilog_testbench>
    ```
3. **Fully working example**: A simple example is provided with Makefile to run this app with opensoruce Icarus Verilog simulator. To run this example:
    ```sh
    cd exec_dir
    make
    ```
## Usage

Invoke the `$afPrHier` system task in your Verilog testbench to trigger the tree walk.
The results will be saved in a CSV file (`output_hier_info.csv`) in the simulation directory.

### Example Verilog Usage

```verilog
module testbench;
  initial begin
    $afPrHier; // Trigger the tree walk
  end
endmodule
```

## Output Format

The generated CSV file, `output_hier_info.csv`, contains the following columns:

| Module        | Instance Name |
|---------------|---------------|
| `module_name` | `inst name `  |

## Error Handling

Errors encountered during execution (e.g., file I/O issues or argument misuse) will be reported to the simulator console. 
Common errors include:
- **`ERR_FILE_OPEN`**: Unable to open CSV file for writing.
- **`ERR_MOD_ITERATION`**: Unable to iterate over modules.
- **`ERR_SYSTF_CALL`**: Failure to acquire a handle for `$afPrHier` task.
- **`ERR_SYSTF_USAGE`**: `$afPrHier` was called with invalid arguments.

## License

This project is licensed under the MIT License. See the `SPDX-License-Identifier: MIT` statement 
in the source code headers for more information.

## Credits
Original version of this app was developed in early 2000's and we derived from that example: https://vseenu.tripod.com/

