
module Main(
    inout [23:0] MMS_ADR, inout [15:0] MMS_DATA,  inout [7:0] MMS_INT, inout MMS_INH1,MMS_INH2,
	 inout MMS_XACK, inout MMS_INIT, inout MMS_BHEN, inout MMS_BCLK, inout MMS_CBRQ, inout MMS_MREQ,
	 inout MMS_BAO, inout MMS_BUSY, output MMS_BPRO, inout MMS_BREQ, inout MMS_CCLK, inout MMS_LOCK,
	 input MMS_BPRN, inout MMS_INTA, inout MMS_MWTC, inout MMS_IOWC, inout MMS_MRDC, inout MMS_IORC,
	 inout [7:0] ESP_DATA, inout ESP_CTRL0, inout ESP_CTRL1, inout ESP_CTRL2, inout ESP_CTRL3, inout ESP_CTRL4,
	 inout [18:0] MISC_PIN, output [15:0] DEBUG_PIN
	 );

reg [47:0] spi_reg_in;
reg [25:0] spi_reg_out;
reg [3:0] step_o;
reg [3:0] step_i;

wire spi_clk = ESP_DATA[0] || (step_o==7);
wire spi_mosi = ESP_DATA[1];
wire spi_cs0 = ESP_DATA[2];
wire spi_cs1 = ESP_DATA[3];
reg spi_miso;
assign ESP_DATA[4] = spi_miso;
wire esp_start = ESP_DATA[5];
wire esp_idle = (step_o==0);
assign ESP_DATA[6] = esp_idle;
wire esp_done = (step_o==15);
assign ESP_DATA[7] = esp_done;

reg [9:0] timer;

always @ (posedge spi_clk)
begin
  if (!spi_cs0)
    begin
      spi_reg_in[47:0] <= {spi_reg_in[46:0],spi_mosi};
	 end
  else if (!spi_cs1)
    begin
      spi_reg_out[25:0] <= {spi_reg_out[24:0],1'b0};
	 end
  else if (step_o==7) 
    begin
	   spi_reg_out[9:0] <= timer;
		spi_reg_out[17:10] <= ~MMS_DATA[15:8];
		spi_reg_out[25:18] <= ~MMS_DATA[7:0];
	 end
end

always @ (negedge spi_clk)
begin
  spi_miso = spi_reg_out[25];
end

always @ (negedge MMS_BCLK)
begin
  if (!MMS_INIT || esp_start)
    begin
      step_o <= 0;
      timer <= 0;
    end
  else
    begin
      case (step_i)
		  // idle
        0: if (MMS_CBRQ) step_o <= 1;
		  // Wunsch nach Busmaster anmelden, Auf Freigabe des Bus warten
        1: if (!MMS_BPRN && MMS_BUSY && MMS_XACK) step_o <= 4;
		  // Bus Übernehmen
        4: if (!MMS_BUSY) step_o <= 5;
        // Transfer Starten: Daten/Adressbus
		  5: step_o <= 6;
        // Transfer starten: Steuersignale, auf XACK warten
        6: if (!MMS_XACK || timer==1023) 
             step_o <= 7;
           else
             timer <= timer + 1;
		  // XACK verlängern
        7: step_o <= 8;
        8: step_o <= 9;
		  // Steuersignale loslassen
        9: step_o <= 10;
		  // Adress/Datenbus loslassen
        10: step_o <= 11;
        11: step_o <= 12;
		  // Bus abgeben
        12: step_o <= 15;
        13: step_o <= 15;
        14: step_o <= 15;
		  // Auf datenabnahme von ESP32 warten
        15: step_o <= 15;
      endcase
	 end
end

always @ (posedge MMS_BCLK)
begin
  step_i <= step_o;
end

// unseren Wunsch nach Busmaster signalisieren
assign MMS_CBRQ = (step_o>=1 && step_o<=3) ? 1'b0 : 1'bz;

// Bus übernehmen
assign MMS_BUSY = (step_o>=4 && step_o<=12) ? 1'b0 : 1'bz;  

// Bus Priorität
assign MMS_BPRO = MMS_BPRN || (step_o>=1 && step_o<=8);

wire trans1 = (step_o>=5) && (step_o<=9) && !esp_start;
wire trans2 = (step_o>=6) && (step_o<=8) && !esp_start;

assign MMS_ADR[23:0] = trans1 ? ~{spi_reg_in[23:16],spi_reg_in[31:24],spi_reg_in[39:32]} : 24'hzzzzzz;
assign MMS_DATA[15:0] = (trans1 && (!spi_reg_in[41] || !spi_reg_in[42])) ? ~{spi_reg_in[7:0],spi_reg_in[15:8]} : 16'hzzzz;
assign MMS_BHEN = trans1 ? spi_reg_in[40] : 1'bz;
assign MMS_MWTC = trans2 ? spi_reg_in[41] : 1'bz;
assign MMS_IOWC = trans2 ? spi_reg_in[42] : 1'bz;
assign MMS_MRDC = trans2 ? spi_reg_in[43] : 1'bz;
assign MMS_IORC = trans2 ? spi_reg_in[44] : 1'bz;

assign DEBUG_PIN[3:0] = step_o;

/*assign DEBUG_PIN[0] = esp_done;
assign DEBUG_PIN[1] = spi_clk;
assign DEBUG_PIN[2] = spi_mosi;
assign DEBUG_PIN[3] = spi_miso;*/
assign DEBUG_PIN[4] = !trans1;
assign DEBUG_PIN[5] = !trans2;
assign DEBUG_PIN[6] = spi_cs0;
assign DEBUG_PIN[7] = spi_cs1;
assign DEBUG_PIN[8] = MMS_BCLK;
assign DEBUG_PIN[9] = MMS_MRDC;
assign DEBUG_PIN[10] = esp_start;
assign DEBUG_PIN[11] = MMS_BPRO;
assign DEBUG_PIN[12] = MMS_BUSY;
assign DEBUG_PIN[13] = MMS_CBRQ;
assign DEBUG_PIN[14] = MMS_BPRN;
assign DEBUG_PIN[15] = MMS_IOWC;
//assign DEBUG_PIN[15:8] = ~MMS_ADR[7:0];//spi_reg_in[39:32];//~MMS_DATA[7:0];

endmodule
