
module Main(
    inout [23:0] MMS_ADR, inout [15:0] MMS_DATA,  inout [7:0] MMS_INT, inout MMS_INH1,MMS_INH2,
	 inout MMS_XACK, inout MMS_INIT, inout MMS_BHEN, inout MMS_BCLK, inout MMS_CBRQ, inout MMS_MREQ,
	 inout MMS_BAO, inout MMS_BUSY, output MMS_BPRO, inout MMS_BREQ, inout MMS_CCLK, inout MMS_LOCK,
	 input MMS_BPRN, inout MMS_INTA, inout MMS_MWTC, inout MMS_IOWC, inout MMS_MRDC, inout MMS_IORC,
	 inout [7:0] ESP_DATA, inout ESP_CTRL0, inout ESP_CTRL1, inout ESP_CTRL2, inout ESP_CTRL3, inout ESP_CTRL4,
	 inout [18:0] MISC_PIN, inout [15:0] DEBUG_PIN
	 );

reg [7:0] ioreg;
reg [15:0] databus;
reg xack;

always @ (posedge MMS_BCLK) 
begin
  if (MMS_ADR==~24'h000400 && (!MMS_IORC || !MMS_IOWC))
    begin
	   if (!MMS_IOWC) ioreg[7:0] = MMS_DATA[7:0];
		if (!MMS_IORC) databus[7:0] = ioreg[7:0];
		xack = 1;
	 end
  else
    begin
	   databus = 16'bzzzzzzzzzzzzzzzz;
		xack = 0;
	 end
end

assign MMS_DATA = databus;
assign MMS_XACK = xack ? 1'b0 : 1'bz;
assign MMS_BPRO = MMS_BPRN;

assign DEBUG_PIN[7:0] = ~ioreg[7:0];
assign DEBUG_PIN[8] = MMS_ADR==~24'h000400;
assign DEBUG_PIN[9] = MMS_IORC;
assign DEBUG_PIN[10] = MMS_IOWC;
assign DEBUG_PIN[11] = xack;

endmodule
