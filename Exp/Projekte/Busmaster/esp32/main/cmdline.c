#include "freertos/FreeRTOS.h"
#include "soc/usb_serial_jtag_struct.h"
#include "wlan.h"
#include "nvs.h"
#include "string.h"
#include "xapp058/micro.h"
#include <driver/gpio.h>
#include <unistd.h>
#include "mms16_master_spi.h"
#include <ctype.h>
#include "cmd_scp.h"

uint32_t menulevel = 0;
char * buffer = NULL;
uint8_t * xsvfdata = NULL;
uint32_t xsvfoffset = 0;
uint8_t base64ptr = 0;


// Char an USB-Serial ausgeben, wird direkt in den Sendebuffer geschrieben
void printChar(char ch)
{
    uint32_t i = 0;
    while (i<100)
    {
        if (USB_SERIAL_JTAG.ep1_conf.serial_in_ep_data_free) // Ausgabe nur wenn Platz im Buffer ist
        {
            USB_SERIAL_JTAG.ep1.rdwr_byte = (uint8_t)ch; // byte ausgeben
            USB_SERIAL_JTAG.ep1_conf.wr_done=1; // Buffer senden
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
        uint32_t j = 0;
        while (j<100)
        {
            if (USB_SERIAL_JTAG.ep1_conf.serial_in_ep_data_free) // Ausgabe nur wenn Platz im Buffer ist
            {
                USB_SERIAL_JTAG.ep1.rdwr_byte = (uint8_t)text[i]; // byte ausgeben
                break;
            }
            j++;
            esp_rom_delay_us(100);
        }
    }
    USB_SERIAL_JTAG.ep1_conf.wr_done=1; // Buffer senden
}

uint8_t base64toByte(uint8_t b64)
{
    if (b64>='A' && b64<='Z') return b64-'A';
    if (b64>='a' && b64<='z') return b64-'a'+26;
    if (b64>='0' && b64<='9') return b64-'0'+52;
    if (b64=='+') return 62;
    if (b64=='/') return 63;
    return 255;
}

uint32_t parseAddress(char *str) 
{
    char *colon = strchr(str, ':');
    if (colon) 
    {
        *colon = '\0';
        uint32_t seg = (uint32_t)strtoul(str, NULL, 16);
        uint32_t off = (uint32_t)strtoul(colon + 1, NULL, 16);
        return (seg << 4) + off;
    }
    return (uint32_t)strtoul(str, NULL, 16);
}

#include <stdio.h>

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
                    printf("4: Wlan aus\n");
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
                case '5':
                    xsvfdata = heap_caps_malloc(1024*1024, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM);
                    xsvfoffset = 0;
                    base64ptr = 0;
                    memset(xsvfdata,0,1024*1024);
                    memset(buffer,0,64);
                    printString("Base64-String empfangen... (ESC:Abbruch, =:Ende)\n",64);
                    menulevel = 4;
                    return;
                case '6':
                    printf("MMS16-Debugger\n?:kurze Hilfe\n");
                    memset(buffer,0,64);
                    menulevel = 5;
                    return;
                case '7':
                    printf("SCP Tools\n1. SCP-Kernel laden\n2. RAM-Disk laden\n3. RAM-Disk speichern\n4. RAM-Disk Dir anzeigen\n");
                    menulevel = 6;
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
                case '4':
                    wlan_mode = 0;
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
            break;
        case 4:
            uint8_t in = base64toByte(input);
            if (in<64)
            {
                switch (base64ptr)
                {
                    case 0:
                        xsvfdata[xsvfoffset] = in<<2;
                        break;
                    case 1:
                        xsvfdata[xsvfoffset] |= (in & 0x30)>>4;
                        xsvfoffset++;
                        xsvfdata[xsvfoffset] |= (in & 0x0f)<<4;
                        break;
                    case 2: 
                        xsvfdata[xsvfoffset] |= in>>2;
                        xsvfoffset++;
                        xsvfdata[xsvfoffset] |= (in & 3) << 6;
                        break;
                    case 3:
                        xsvfdata[xsvfoffset] |= in;
                        xsvfoffset++;
                        break;
                }
                base64ptr = (base64ptr+1) & 3;
            }
            else if (input=='=')
            {
                printf("\nXSVF Flash gestartet (%ld bytes).\n",xsvfoffset);

                uint8_t r = xsvfExecute(xsvfdata, xsvfoffset);
                if (r==0)
                {
                    printf("XSVF Flash erfolgreich.\n");
                }
                else
                {
                    printf("XSVF Flash Fehlgeschlagen, fehler %d!\n",r);
                }
                free(xsvfdata);
                menulevel = 0;
            }
            else if (input == 27)
            {
                printf("XSVF Flash abgebrochen.\n");
                free(xsvfdata);
                menulevel = 0;
            }
            else if (input == '?')
            {
                for (uint32_t a=0;a<xsvfoffset;a++)
                {
                    printf("%02X ",xsvfdata[a]);
                    if ((a & 31)==31)
                    {
                        printf("    \"");
                        for (uint32_t b=0;b<32;b++)
                        {
                            if (xsvfdata[a-31+b]>32)
                            {
                                printf("%c",xsvfdata[a-31+b]);
                            }
                            else
                            {
                                printf(".");
                            }
                        }
                        printf("\"\n");
                    }
                }
                printf("\n");
            }
            return;
        case 5:
            switch(input)
            {
                case 13: // Enter
                case 10:
                    char *cmd = strtok(buffer, " ");
                    char *addr_str = NULL;
                    char *val_str = NULL;
                    if (!cmd) break;

                    uint32_t addr;
                    uint32_t val = 0;
                    char action = tolower(cmd[0]);

                    switch (action)
                    {
                        case 'd':
                            addr_str = strtok(NULL, " ");
                            addr = parseAddress(addr_str);
                            val = 64;
                            if ((val_str = strtok(NULL, " ")) != NULL)
                            {
                                val = strtoul(val_str, NULL,16);
                            }
                            for (int i = 0; i < val; i++) 
                            {
                                if (i % 16 == 0) printf("\n%05lX: ", addr + i);
                                uint8_t v;
                                if (readMemByte(addr + i,&v) < 0)
                                    printf("** ");
                                else
                                    printf("%02X ", v);
                            }
                            printf("\nOK\n");
                            break;
                        case 'e':
                            uint8_t a = 0;
                            addr_str = strtok(NULL, " ");
                            addr = parseAddress(addr_str);
                            while ((val_str = strtok(NULL, " ")) != NULL) 
                            {
                                if (writeMemByte(addr, (uint8_t)strtoul(val_str, NULL, 16))<0) 
                                    printf("MMS16 Timeout at %05lX\n",addr);
                                else
                                    a++;
                                addr++;
                            }
                            if (a>0) printf("\nOK\n");
                            break;
                        case 'i':
                            addr_str = strtok(NULL, " ");
                            addr = parseAddress(addr_str);
                            uint8_t v;
                            if (readIoByte(addr,&v)<0)
                                printf("MMS16 Timeout\n");
                            else
                                printf("\n%02X\nOK\n", v);
                            break;
                        case 'o':
                            addr_str = strtok(NULL, " ");
                            addr = parseAddress(addr_str);
                            if ((val_str = strtok(NULL, " ")) != NULL)
                            {
                                if (writeIoByte(addr++, (uint8_t)strtoul(val_str, NULL, 16))<0)
                                    printf("MMS16 Timeout\n");
                                else
                                    printf("\nOK\n");
                            }
                            break;
                        case 'q':
                            printf("exit\n");
                            menulevel = 0;
                            break;
                        case '?':
                            printf("\nSpeicher anzeigen: d <addr> (anzahl)\nSpeicher schreiben: e <addr> <wert> (...werte)\nIO-Eingaben: i <addr>\n");
                            printf("IO-Ausgeben: o <addr> <wert>\nQuit: q\n");
                            break;
                    }
                    memset(buffer,0,64);
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
            return;
        case 6: // SCP Tools
            switch (input)
            {
                case '1': // SCP-Kernel und RAM-Disk laden
                    scp_kernel_to_mms();
                    return;
                case '2': // RAM-Disk laden
                    flash_to_mms();
                    return;
                case '3': // RAM-Disk speichern
                    mms_to_flash();
                    return;
                case '4': // RAM-Disk Dir anzeigen
                    scp_dir_mms();
                    return;
                case 27: // Escape
                    printf("\nAbbruch\n");
                    menulevel = 0;
                    break;
            }
            printf("SCP Tools\n1. SCP-Kernel laden\n2. RAM-Disk laden\n3. RAM-Disk speichern\n4. RAM-Disk Dir anzeigen\n");
            return;
    }
    printf("1: Wlan-Modus\n2: Wlan-SSID\n3: Wlan-Passwort\n4: alles speichern\n5: base64 encodierte .xsvf Datei empfangen.\n6: MMS-16-Debugger\n");
    printf("7: SCP-Funktionen\n");
}

void run_cmdline()
{
    if (buffer == NULL)
    {
        buffer = heap_caps_malloc(64, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM);
    }
    while (USB_SERIAL_JTAG.ep1_conf.serial_out_ep_data_avail) // Byte im USB-Serial Empfangspuffer ?
    {
        processInput((char)USB_SERIAL_JTAG.ep1.rdwr_byte); // Byte aus Buffer lesen und verarbeiten
    }
}

