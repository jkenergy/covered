module main;

wire [1:0] a = return_two( 1'b0 );

initial begin
`ifdef DUMP
        $dumpfile( "func3.2.vcd" );
        $dumpvars( 0, main );
`endif
        #100;
        $finish;
end

function [1:0] return_two;
input a;
begin
  @(negedge a);
  return_two = 2'b10;
end

endfunction

endmodule
