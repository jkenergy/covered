/*
 Name:        timescale4.15.v
 Author:      Trevor Williams  (phase1geo@gmail.com)
 Date:        10/24/2008
 Purpose:     Verifies that real delays work properly when timescale is 100 s / 100 fs
*/

`timescale 100 s / 1 ps

module main;

reg a, b;

inner foo( b );

initial begin
	a = 1'b0;
	b = 1'b0;
	#(2.123_456_789_987_654_321);
	b = 1'b1;
	a = ($time == 2);
end

initial begin
`ifdef DUMP
        $dumpfile( "timescale4.15.vcd" );
        $dumpvars( 0, main );
`endif
        #(10);
        $finish;
end

endmodule

`timescale 1 ps / 1 ps

module inner(
  input b
);

reg a;

initial begin
        a = 1'b0;
        @(posedge b);
        a = ($time == 64'd2123456789987654);
end

endmodule

