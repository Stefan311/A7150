#include "freertos/FreeRTOS.h"
#include "mms16_master_spi.h"
#include "esp_partition.h"
#include "cmdline.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

extern const uint8_t SCP_SYS_Data[] asm("SCP_SYS");
extern const uint32_t SCP_SYS_Len asm("SCP_SYS_length");

#define RAMDISK_SIZE_BYTES 0x4D000

uint32_t mms_ram_limit = 0;
const esp_partition_t *part = NULL;

void init_scp_function()
{
    if (mms_ram_limit == 0)
    {
        uint8_t v;
        while (mms_ram_limit < 0xf8000)
        {
            if (readMemByte(mms_ram_limit,&v) < 0) break;
            mms_ram_limit += 0x8000;
        }
        part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "ramdisk");
    }
}

void get_entry_details(unsigned int addr, char *name, char *ext, unsigned char *status) 
{
    unsigned char raw_byte;
    
    readMemByte(addr, status);
    if (*status == 0xE5) return;

    for (int n = 0; n < 8; n++) 
    {
        readMemByte(addr + 1 + n, &raw_byte);
        name[n] = (char)(raw_byte & 0x7F);
    }
    name[8] = '\0';

    for (int e = 0; e < 3; e++) 
    {
        readMemByte(addr + 9 + e, &raw_byte);
        ext[e] = (char)(raw_byte & 0x7F);
    }
    ext[3] = '\0';
}

void scp_dir_mms()
{
    init_scp_function();
    const int start_adresse = mms_ram_limit - RAMDISK_SIZE_BYTES;
    const int max_entries = 64;
    const int entry_size = 32;

    printf("\nUser Filename.Typ   Sektoren  Attribute\n");
    printf("---------------------------------------\n");

    for (int i = 0; i < max_entries; i++) 
    {
        unsigned int current_addr = start_adresse + (i * entry_size);
        unsigned char status;
        char name[9], ext[4];

        get_entry_details(current_addr, name, ext, &status);

        if (status == 0xE5 || status > 15) 
        {
            continue;
        }

        int already_processed = 0;
        for (int j = 0; j < i; j++) 
        {
            unsigned int prev_addr = start_adresse + (j * entry_size);
            unsigned char prev_status;
            char prev_name[9], prev_ext[4];
            
            get_entry_details(prev_addr, prev_name, prev_ext, &prev_status);
            
            if (prev_status == status && strcmp(prev_name, name) == 0 && strcmp(prev_ext, ext) == 0) 
            {
                already_processed = 1;
                break;
            }
        }

        if (already_processed) 
        {
            continue;
        }

        unsigned int max_sectors = 0;
        unsigned char t1_ro = 0, t2_sys = 0, t3_arc = 0;

        for (int j = i; j < max_entries; j++) 
        {
            unsigned int match_addr = start_adresse + (j * entry_size);
            unsigned char m_status;
            char m_name[9], m_ext[4];

            get_entry_details(match_addr, m_name, m_ext, &m_status);

            if (m_status == status && strcmp(m_name, name) == 0 && strcmp(m_ext, ext) == 0) 
            {
                unsigned char extent_counter = 0;
                unsigned char record_count = 0;

                readMemByte(match_addr + 12, &extent_counter);
                readMemByte(match_addr + 15, &record_count);

                unsigned int current_sectors = (extent_counter * 128) + record_count;
                
                if (current_sectors > max_sectors) 
                {
                    max_sectors = current_sectors;
                }

                unsigned char raw_attr;
                readMemByte(match_addr + 9, &raw_attr);  t1_ro = (raw_attr & 0x80) ? 1 : 0;
                readMemByte(match_addr + 10, &raw_attr); t2_sys = (raw_attr & 0x80) ? 1 : 0;
                readMemByte(match_addr + 11, &raw_attr); t3_arc = (raw_attr & 0x80) ? 1 : 0;
            }
        }

        printf("%2d   %-8s.%-3s   %8u  ", status, name, ext, max_sectors);
        printf("[%s %s %s]\n", t1_ro  ? "R/O" : "R/W", t2_sys ? "SYS" : "DIR", t3_arc ? "ARC" : "   ");
    }
}

void flash_to_mms()
{
    init_scp_function();
    printString("\nFlash-->Ram",13);
    if (part == NULL) return;
    uint16_t b[128];
    int o = 0;
    int a = mms_ram_limit - RAMDISK_SIZE_BYTES;
    while (o < RAMDISK_SIZE_BYTES) 
    {
        esp_partition_read(part, o, b, 256);
        for (int i = 0; i < 128; i++) 
        {
            writeMemWord(a, b[i]);
            a += 2;
        }
        o += 256;
        if ((o & 0xffff) == 0) printChar('.');
    }
    printf("\n");
}

void mms_to_flash()
{
    init_scp_function();
    printString("\nRam-->Flash",13);
    if (part == NULL) return;
    uint16_t b[128];
    esp_partition_erase_range(part, 0, part->size);
    int o = 0;
    int a = mms_ram_limit - RAMDISK_SIZE_BYTES;
    while (o < RAMDISK_SIZE_BYTES) 
    {
        for (int i = 0; i < 128; i++) 
        {
            uint16_t word_val;
            readMemWord(a, &word_val);
            b[i] = word_val;
            a += 2;
        }
        esp_partition_write(part, o, b, 256);
        o += 256;
        if ((o & 0xffff) == 0) printChar('.');
    }
    printf("\n");
}

void scp_kernel_to_mms()
{
    init_scp_function();
    for (uint32_t a = 0; a < SCP_SYS_Len-0x40; a++)
    {
        writeMemByte(a + 0x1000, SCP_SYS_Data[a+0x40]); 
    }
    writeMemByte(0x1000,0xb9); // MOV DX,4 (Start-Laufwerk für SCP)
    writeMemByte(0x1001,0x04);
    writeMemByte(0x1002,0x00);
    writeMemByte(0x1003,0xea); // JMP 0401:2500 (Starte SCP)
    writeMemByte(0x1004,0x00);
    writeMemByte(0x1005,0x25);
    writeMemByte(0x1006,0x04);
    writeMemByte(0x1007,0x01);
    printf("\nSCP.SYS geladen, start mit \"g 100:0\"\n");
}
