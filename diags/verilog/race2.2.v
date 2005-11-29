module main;

reg  x;
reg  a, b;

always @(x) a  =  x;
always @(x) b <= ~x;

initial begin
	$dumpfile( "race2.2.vcd" );
	$dumpvars( 0, main );
	x = 1'b0;
	#5;
	x = 1'b1;
	#5;
	x = 1'b0;
	#10;
	$finish;
end

endmodule