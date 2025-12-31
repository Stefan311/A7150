#include "freertos/FreeRTOS.h"
#include "soc/usb_serial_jtag_struct.h"
#include "wlan.h"
#include "nvs.h"
#include "string.h"

uint32_t menulevel = 0;
char * buffer = NULL;

// Char an USB-Serial ausgeben, wird direkt in den Sendebuffer geschrieben
void printChar(char ch)
{
    uint32_t i = 0;
    while (i<100)
    {
        if (USB_SERIAL_JTAG.ep1_conf.serial_in_ep_data_free) // Ausgabe nur wenn Platz im Buffer ist
        {
            USB_SERIAL_JTAG.ep1.rdwr_byte = (uint8_t)ch; // byte ausgeben
            return;
        }
        i++;
        esp_rom_delay_us(100);
    }
}

// String direkt an USB-Serial ausgeben, im Gegensatz zu printf() muss das nicht mit \n abgeschlossen sein
void printString(char * text, uint32_t len) 
{
    for (int i=0;i<len && text[i]!=0; i++)
    {
        printChar(text[i]);
    }
    USB_SERIAL_JTAG.ep1_conf.wr_done=1; // Buffer senden
}

void processInput(char input)
{
    switch (menulevel)
    {
        case 0: 
            switch (input)
            {
                case '1':
                    printf("1: WPS-Modus (der ESP versucht die automatische Anmeldung an einem Accesspoint)\n");
                    printf("2: STA-Modus (der ESP verbindet sich mit einem Accesspoint)\n");
                    printf("3: AP-Modus (der ESP wird zum Accesspoint)\n");
                    menulevel = 1;
                    return;
                case '2':
                    memcpy(buffer,wlan_ssid,64);
                    int j;
                    for (j=0;j<64;j++) {if (buffer[j]==0) break;}
                    for (;j<64;j++) buffer[j]=0;
                    printString("SSID eingeben:",64);
                    printString(buffer,64);
                    menulevel = 2;
                    return;
                case '3':
                    memset(buffer,0,64);
                    printString("Passwort eingeben:",64);
                    menulevel = 3;
                    return;
                case '4':
                    write_settings();
                    return;
            }
            break;
        case 1:
            switch(input)
            {
                case '1':
                    wlan_mode = 1;
                    setup_wlan(wlan_mode);
                    menulevel = 0;
                    return;
                case '2':
                    wlan_mode = 2;
                    setup_wlan(wlan_mode);
                    menulevel = 0;
                    return;
                case '3':
                    wlan_mode = 3;
                    setup_wlan(wlan_mode);
                    menulevel = 0;
                    return;
                case 27: // Escape
                    printf("\nAbbruch\n");
                    menulevel = 0;
                    break;
            }
            break;
        case 2: 
        case 3:
            switch(input)
            {
                case 27: // Escape
                    printf("\nAbbruch\n");
                    menulevel = 0;
                    break;
                case 13: // Enter
                case 10:
                    printf("\nOk\n");
                    if (menulevel==1)
                    {
                        memcpy(wlan_ssid,buffer,64);
                    }
                    else
                    {
                        memcpy(wlan_passwd,buffer,64);
						setup_wlan(2);
                    }
                    menulevel = 0;
                    break;
                case 8: // Backspace
                    for (int j=63;j>=0;j--)
                    {
                        if (buffer[j]!=0)
                        {
                            buffer[j] = 0;
                            printChar(8);
                            printChar(32);
                            printChar(8);
                            USB_SERIAL_JTAG.ep1_conf.wr_done=1;
                            break;
                        }
                    }
                    return;
                default: // alles andere
                    for (int j=0;j<64;j++)
                    {
                        if (buffer[j]==0)
                        {
                            buffer[j] = input;
                            printChar(input);
                            USB_SERIAL_JTAG.ep1_conf.wr_done=1;
                            break;
                        }
                    }
                    return;
            }
    }
    printf("1: Wlan-Modus\n2: Wlan-SSID\n3: Wlan-Passwort\n4: alles speichern\n");
}

void run_cmdline()
{
    if (buffer == NULL)
    {
        buffer = heap_caps_malloc(64, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM);
    }
    if (USB_SERIAL_JTAG.ep1_conf.serial_out_ep_data_avail) // Byte im USB-Serial Empfangspuffer ?
    {
        processInput((char)USB_SERIAL_JTAG.ep1.rdwr_byte); // Byte aus Buffer lesen und verarbeiten
    }
}

