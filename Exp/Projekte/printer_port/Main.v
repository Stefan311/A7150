
module Main(
	inout [23:0] MMS_ADR, inout [15:0] MMS_DATA,  inout [7:0] MMS_INT, inout MMS_INH1,MMS_INH2,
	inout MMS_XACK, inout MMS_INIT, inout MMS_BHEN, inout MMS_BCLK, inout MMS_CBRQ, inout MMS_MREQ,
	inout MMS_BAO, inout MMS_BUSY, output MMS_BPRO, inout MMS_BREQ, inout MMS_CCLK, inout MMS_LOCK,
	input MMS_BPRN, inout MMS_INTA, inout MMS_MWTC, inout MMS_IOWC, inout MMS_MRDC, inout MMS_IORC,
	inout [7:0] ESP_DATA, inout ESP_CTRL0, inout ESP_CTRL1, inout ESP_CTRL2, inout ESP_CTRL3, inout ESP_CTRL4,
	inout [17:0] MISC_PIN, inout [15:0] DEBUG_PIN);

reg [7:0] databus;
reg xack;

reg [7:0] LPT_DATA;
reg LPT_AF;
reg LPT_INIT;
reg LPT_SEL_OUT;
reg LPT_STROBE;
reg irq_en;

assign MISC_PIN[7:0] = LPT_DATA;
wire LPT_ERR = MISC_PIN[8];
wire LPT_SEL_IN = MISC_PIN[9];
wire LPT_PE = MISC_PIN[10];
wire LPT_ACK = MISC_PIN[11];
wire LPT_BUSY = MISC_PIN[12];
assign MISC_PIN[13] = LPT_STROBE;
assign MISC_PIN[14] = LPT_AF;
assign MISC_PIN[15] = LPT_INIT;
assign MISC_PIN[16] = LPT_SEL_OUT;

always @ (*) 
begin
  if (!MMS_INIT) // System Reset
    begin
		LPT_DATA = 8'b00000000;
      LPT_AF = 1;
      LPT_INIT = 1;
      LPT_SEL_OUT = 0;
      LPT_STROBE = 1;
		xack = 0;
	 end
  else if (((MMS_ADR[15:4])==~12'h03b) && (!MMS_IORC || !MMS_IOWC)) // IO-Zugriff auf 03B0-03BF
    begin
	   case ({!MMS_IORC,!MMS_IOWC,~MMS_ADR[3:0]})
		  // Port 3B0-3B7: zur Ansteuerung des Druckers durch das A7150-Bios
		  6'b100000: databus = {4'b0000,LPT_PE,LPT_ERR,LPT_SEL_IN,LPT_BUSY}; // read 03b0: Status
		  6'b100010: databus = LPT_DATA;                               // read 03b2: Data
		  6'b010010: LPT_DATA = ~MMS_DATA[7:0];                        // write 03b2: Data
		  6'b100100: databus = {4'b0000,LPT_STROBE,LPT_ACK,2'b00};     // read 03b4: Status
		  6'b010110: LPT_STROBE = !MMS_DATA[0];                        // write 03b6: 8255 Richtungsregister, wird vom BIOS als Strobe benutzt
		  // Port 3BC-3BE: zur Ansteuerung durch Anwendungsprogramme nach IBM-Standard
		  6'b101100: databus = LPT_DATA;                               // read 03bc: Data
		  6'b011100: LPT_DATA = ~MMS_DATA[7:0];                        // write 03bc: Data
		  6'b101101: databus = {!LPT_BUSY,LPT_ACK,LPT_PE,LPT_SEL_IN,LPT_ERR,3'b000}; // read 03bd: Status
		  6'b101110: databus = {3'b000,irq_en,!LPT_SEL_OUT,LPT_INIT,!LPT_AF,!LPT_STROBE}; // read 03bc: Steuer
		  6'b011110: begin                                             // write 03bc: Steuer
                     irq_en = !MMS_DATA[4];
		               LPT_SEL_OUT = MMS_DATA[3];
						   LPT_INIT = !MMS_DATA[2]; 
						   LPT_AF = MMS_DATA[1]; 
						   LPT_STROBE = MMS_DATA[0];                       
						 end
		  default: xack = 0;
		endcase
		xack = 1;
	 end
  else
    xack = 0;
end

assign MMS_DATA[7:0] = (xack && !MMS_IORC) ? ~databus : 8'bzzzzzzzz;
assign MMS_DATA[15:8] = 8'bzzzzzzzz;
assign MMS_XACK = xack ? 1'b0 : 1'bz;
assign MMS_BPRO = MMS_BPRN;

/*
assign DEBUG_PIN[7:0] = xack ? MMS_DATA[7:0] : 8'bzzzzzzzz;
assign DEBUG_PIN[8] = xack;
assign DEBUG_PIN[9] = MMS_IOWC;
assign DEBUG_PIN[10] = MMS_IORC;
assign DEBUG_PIN[14:11] = MMS_ADR[3:0];
*/

endmodule
