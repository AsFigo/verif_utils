ivl:
	iverilog-vpi ../vpi_src/afVpiPrTs.cc
	iverilog ../tests/simple_3.v -o afPrTs
	vvp  -M . -mafVpiPrTs afPrTs 

clean:
	rm -fr afPr* *.log *.vpi *.o *.csv
