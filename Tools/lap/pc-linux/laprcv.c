#include <sys/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define BASEPORT 0x378
#define DATAPORT (BASEPORT+1)
#define SymSSync1 0x23
#define SymSSync2 0x76
#define SymSSend 0x42
#define SymSStop 0x84
#define SymRSync1 0x83
#define SymRSync2 0x37
#define SymRReady 0x47
#define SymRRepeat 0x68

unsigned char cross(unsigned char i)
{
    return ((i&1) ? 16 : 0) | (i&2) | ((i&4) ? 1 : 0) | ((i&8) ? 4 : 0) | ((i&16) ? 8 : 0);
}

unsigned char in()
{
    return ((inb(DATAPORT) & 0x78) >> 3) & 0x0f;
}

void out(unsigned char b, bool t)
{
    outb(cross((b & 0x0f) | (t ? 0x10 : 0x00)), BASEPORT);
}

void wait(bool t)
{
    while ((inb(DATAPORT) & 0x80) == (t ? 0x80 : 0x00));
}

unsigned char recive()
{
    wait(true);
    unsigned char c = in();
    out(0, true);
    wait(false);
    c = c | (in() << 4);
    out(0, false);
    return (c & 255);
}

unsigned char xchange(unsigned char o)
{
    wait(true);
    unsigned char c = in();
    out(o, true);
    wait(false);
    c = c | (in() << 4);
    out(o >> 4, false);
    return c & 255;
}

int main(int argc, char *argv[])
{
    unsigned char c[128];
    char fname[17] = {0};

    if (ioperm(BASEPORT, 8, 1))
    {
        printf("ioperm error!\n");
        return(1);
    }

    // Strobe passend zur Empfangsrichtung setzen
    if ((inb(DATAPORT) & 0x80) == 0x00) out(0, true); else out(0, false);

    // Synchronisieren
    while (xchange(SymRSync1) != SymSSync1);
    printf("#"); fflush(stdout);
    while (xchange(SymRSync2) != SymSSync2);

    // Dateinamen empfangen
    unsigned char fn_len = recive() & 0x0f;
    if (fn_len > 15) fn_len = 15;
    for (int a = 0; a < fn_len; a++)
    {
        fname[a] = recive();
    }
    fname[fn_len] = '\0';

    // Falls auf der CLI ein eigener Name übergeben wurde, diesen verwenden
    char *out_filename = fname;
    if (argc >= 2)
    {
        out_filename = argv[1];
    }
    printf("%s\n", out_filename);

    // Dateigröße empfangen (32-Bit Unsigned Long)
    unsigned long filesize = 0;
    filesize |= ((unsigned long)recive());
    filesize |= ((unsigned long)recive() << 8);
    filesize |= ((unsigned long)recive() << 16);
    filesize |= ((unsigned long)recive() << 24);

    FILE *ofile = fopen(out_filename, "wb");
    if (!ofile)
    {
        fprintf(stderr, "Fehler beim Öffnen der Zieldatei!\n");
        return 1;
    }

    printf("["); fflush(stdout);

    unsigned char e = SymRReady;
    while ((xchange(e) == SymSSend) || (e == SymRRepeat))
    {
        uint b = 0;
        uint d = 0;
        for (int a = 0; a < 128; a++)
        {
            b = recive();
            c[a] = b;
            d = d ^ (b << 8);
            for (int bit = 0; bit < 8; bit++)
            {
                if ((d & 0x8000) != 0)
                {
                    d = (d << 1) ^ 0x1021;
                }
                else
                {
                    d = d << 1;
                }
            }
        }
        d = d & 0xffff;
        b = (recive() & 255) | ((recive() << 8) & 0xff00);
        if (b == d)
        {
            fwrite(c, 128, 1, ofile);
            printf("."); fflush(stdout);
            e = SymRReady;
        }
        else
        {
            printf("!"); fflush(stdout);
            e = SymRRepeat;
        }
    }

    sleep(1);
    out(0, true);
    printf("]\n");

    // Falls Dateigröße übermittelt wurde, Datei auf exakte Länge ftruncate-n
    if (filesize > 0)
    {
        fflush(ofile);
        int fd = fileno(ofile);
        ftruncate(fd, filesize);
    }

    fclose(ofile);
    return(0);
}
