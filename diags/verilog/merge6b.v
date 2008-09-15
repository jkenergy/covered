/*
 Name:        merge6b.v
 Author:      Trevor Williams  (phase1geo@gmail.com)
 Date:        09/15/2008
 Purpose:     See ../regress/merge6.pl for details.
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
        $dumpfile( "merge6b.vcd" );
        $dumpvars( 0, main );
`endif
	b = 1'b0;
	c = 1'b1;
        #10;
        $finish;
end

endmodule
