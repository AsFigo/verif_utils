 `timescale 1 us / 1 ps;
   
 module top;

 wire q;
	 
 block1 u1 (q, clk, a, b);
 block2 u2 (out_val, clk, a, q);

 initial
   $afPrTs;
endmodule
`timescale 1 ns / 10 ps;
module block1 (q, clk, a, b);
output q;
input clk, a,b;
// Description..
endmodule
`timescale 1 ps / 10 fs;
module block2 (q, clk, a, b);
output q;
input clk, a,b;
// Description..
endmodule
