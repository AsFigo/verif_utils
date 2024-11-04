ivl:
	iverilog-vpi ../vpi_src/afVpiPrHier.cc
	iverilog ../tests/simple_hier.v -o afPrHier
	vvp  -M . -mafVpiPrHier afPrHier 

ts:
	iverilog-vpi ../vpi_src/afVpiPrTs.cc
	iverilog ../tests/simple_3.v -o afPrTs
	vvp  -M . -mafVpiPrTs afPrTs 

clean:
	rm -fr afPr* *.log *.vpi *.o *.csv

vlt:
	gcc -DVERILATOR -I/home/ajeetha/tools/git_vlt/verilator/include -shared -fPIC ../vpi_src/afVpiPrHier.cc -c -lverilated -lverilated_vcd -o afPrHier.so
	verilator --cc --exe --build --vpi afPrHier.so ../tests/simple_hier.v

