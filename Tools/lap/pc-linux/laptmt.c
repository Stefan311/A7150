#include <sys/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>

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

// Datenleitungen so vertauschen, dass die am Empfänger in der richtigen Reihenfolge sind
unsigned char cross(unsigned char i)
{
  return ((i&1) ? 16 : 0) | (i&2) | ((i&4) ? 1 : 0) | ((i&8) ? 4 : 0) | ((i&16) ? 8 : 0);
}

// Nibble empfangen
unsigned char in()
{
  return ((inb(DATAPORT) & 0x78) >> 3) & 0x0f;
}

// Nibbele und Strobe senden
void out(unsigned char b, bool t)
{
  outb(cross((b & 0x0f) | (t ? 0x10 : 0x00)),BASEPORT);
}

// auf Strobe warten
void wait(bool t)
{
  while ((inb(DATAPORT) & 0x80)==(t ? 0x80 : 0x00));
}

// Byte senden
void send(unsigned char o)
{
  out(o, true);
  wait(true);
  out(o >> 4, false);
  wait(false);
}

// gleichzeitig Byte senden und empfangen
unsigned char xchange(unsigned char o)
{
  out(o, true);
  wait(true);
  unsigned char c = in();
  out(o >> 4, false);
  wait(false);
  c = c | (in() << 4);
  return c & 255;
}

int main(int argc, char *argv[])
{
  if (argc < 2) return 0;
  unsigned char c[128];
	
  if (ioperm(BASEPORT, 8, 1)) 
  { 
    printf("ioperm error!\n"); 
    return(1); 
  }

  // Datei öffnen, größe ermitteln
  FILE *ifile;
  ifile = fopen(argv[1],"rb");
  if (ifile == NULL)
  {
    fprintf(stderr, "\nFehler: Datei '%s' konnte nicht geoeffnet werden!\n", argv[1]);
    return 1;
  }
  fseek(ifile, 0L, SEEK_END);
  long s = ftell(ifile);
  rewind(ifile);

  // strobe passend zur empfangsrichtung setzen, um die Kommunikation Gang zu bringen
  if ((inb(DATAPORT) & 0x80)==0x00) out(0,true); else out(0,false);

  // Synchronisieren
  while (xchange(SymSSync1)!=SymRSync1);
  printf("#");
  fflush(stdout);
  while (xchange(SymSSync2)!=SymRSync2);

  // Dateiname senden
  char *fn = strdup(argv[argc-1]);
  char *f = basename(fn);
  uint e = strlen(f);
  if (e>15) e=15;
  send(e);
  for (uint a=0; a<e; a++) send(f[a]);
  free(fn);

  // größe senden
  send(s & 0xff);
  send((s>>8) & 0xff);
  send((s>>16) & 0xff);
  send((s>>24) & 0xff);

  printf("[");
  fflush(stdout);

  while (1)
  {
    bool d=feof(ifile);
    unsigned char a;

    // der Gegenseite sagen ob noch ein neuer Block kommt
    if (d)
      a=xchange(SymSStop);
    else
      a=xchange(SymSSend);

    if (a==SymRReady) // die Gegenseite hat den alten Block übernommen
    {
      if (d) // Dateiende --> Programmende
      {
        outb(8,BASEPORT); // Druckerport auf Ruhe-Level setzen, es gibt sonst Probleme beim Selbsttest im A7100
        printf("]\n");
        fclose(ifile);
        return(0);
      }
      else  // nächsten Block aus Datei laden
      {
        for (uint i=0;i<128;i++) c[i]=0x1a;
        a=fread(c,128,1,ifile);
        printf(".");
        fflush(stdout);
      }
    }
    else if (a==SymRRepeat) // die Gegenseite hat den Block nicht angenommen und möchte eine Wiederholung
    {
        printf("!");
        fflush(stdout);
    }
    else // Häää? Wir sind out of sync!
    {
        outb(8,BASEPORT);
        printf("Protocoll Error! (%d)\n",a);
        fclose(ifile);
        return(1);
    }

    // Datenblock senden
    uint e=0;
    for (uint a=0;a<128;a++)
    {
      send(c[a]);
      e=e ^ (c[a]<<8);
      for (int b=0;b<8;b++)
      {
        if ((e & 0x8000) != 0)
        {
          e=(e<<1) ^ 0x1021;
        }
        else
        {
          e=e<<1;
        }
      }
    }
    // CRC senden
    send(e & 0xff);
    send(e >> 8);
  }
}
