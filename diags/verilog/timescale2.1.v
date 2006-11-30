/*
 Name:     timescale2.1.v
 Author:   Trevor Williams  (trevorw@charter.net)
 Date:     11/25/2006
 Purpose:  Verifies multiple timescale specifications within the design when
           top module timescale is 10 s / 1 s.
*/

`timescale 10 s / 1 s

module main;

ts_module tsm();

initial begin
`ifndef VPI
        $dumpfile( "timescale2.1.vcd" );
        $dumpvars( 0, main );
`endif
        #10;
        $finish;
end

endmodule