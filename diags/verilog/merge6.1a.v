/*
 Name:        merge6.1a.v
 Author:      Trevor Williams  (phase1geo@gmail.com)
 Date:        09/15/2008
 Purpose:     See ../regress/merge6.1.pl for details.
*/

module main;

wire a;
reg  b, c;

dut_and a(
  .a(a),
  .b(b),
  .c(c)
);

initial begin
`ifdef DUMP
        $dumpfile( "merge6.1a.vcd" );
        $dumpvars( 0, main );
`endif
`ifdef TEST1
	b = 1'b0;
	c = 1'b0;
`else
	b = 1'b0;
	c = 1'b1;
`endif
        #10;
        $finish;
end

endmodule
