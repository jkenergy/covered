module main;

reg a, b, c, d;

initial begin
	a = 1'b0;
	b = 1'b0;
	c = 1'b0;
	d = 1'b0;
	#5;
	a = -10 < -10;
	b = -10 <  10;
	c =  10 < -10;
	d =  10 <  10;
end

initial begin
        $dumpfile( "signed3.1.vcd" );
        $dumpvars( 0, main );
        #10;
        $finish;
end

endmodule
