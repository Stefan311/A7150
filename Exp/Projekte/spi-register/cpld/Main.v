
module Main(
    inout [23:0] MMS_ADR, inout [15:0] MMS_DATA,  inout [7:0] MMS_INT, inout MMS_INH1,MMS_INH2,
	 inout MMS_XACK, inout MMS_INIT, inout MMS_BHEN, inout MMS_BCLK, inout MMS_CBRQ, inout MMS_MREQ,
	 inout MMS_BAO, inout MMS_BUSY, output MMS_BPRO, inout MMS_BREQ, inout MMS_CCLK, inout MMS_LOCK,
	 input MMS_BPRN, inout MMS_INTA, inout MMS_MWTC, inout MMS_IOWC, inout MMS_MRDC, inout MMS_IORC,
	 inout [7:0] ESP_DATA, inout ESP_CTRL0, inout ESP_CTRL1, inout ESP_CTRL2, inout ESP_CTRL3, inout ESP_CTRL4,
	 inout [18:0] MISC_PIN, inout [15:0] DEBUG_PIN
	 );

reg [7:0] databus;
reg xack;

reg [7:0] register [15:0];

reg [5:0] spictr;
reg [4:0] i;
reg spicmd;
reg spidir;

reg spi_clk;

always @ (negedge ESP_CTRL1 or posedge ESP_CTRL0)
begin
  if (ESP_CTRL0) spictr = 0;
  else spictr = spictr + 1;
end

assign ESP_DATA = (spicmd && spictr!=0) ? register[spictr-1] : 8'bzzzzzzzz;

reg [3:0] adr;
reg [7:0] dat;

always @ (ESP_CTRL1 or MMS_BCLK or ESP_CTRL0) 
begin
  adr[3:0] = ESP_CTRL0 ? ~MMS_ADR[3:0] : spictr[3:0]-1;
  dat[7:0] = ESP_CTRL0 ? ~MMS_DATA[7:0] : ESP_DATA;
  if ((!ESP_CTRL0 && !spicmd && ESP_CTRL1) || ((MMS_ADR[23:4]==~20'h00040) && !MMS_IOWC))
    begin
      register[adr] = dat;
	 end

  if (ESP_CTRL0)
    spicmd = 1;
  else if (spictr==0 && ESP_CTRL1)
    spicmd = ESP_DATA[0];

  xack = (MMS_ADR[23:4]==~20'h00040) && (!MMS_IORC || !MMS_IOWC) && ESP_CTRL0;
end

assign MMS_DATA[7:0] = (xack && !MMS_IORC) ? ~register[~MMS_ADR[3:0]] : 8'bzzzzzzzz;
assign MMS_XACK = xack ? 1'b0 : 1'bz;
assign MMS_BPRO = MMS_BPRN;

assign DEBUG_PIN[3:0] = spictr[3:0];
assign DEBUG_PIN[4] = spicmd;
assign DEBUG_PIN[5] = xack;
assign DEBUG_PIN[6] = ESP_CTRL0;
assign DEBUG_PIN[7] = ESP_CTRL1;
assign DEBUG_PIN[15:8] = ESP_DATA[7:0];

endmodule
