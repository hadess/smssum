/*
   Master System checksum fixer
   Originally written between 14/04/2017 10:02:01 - 19/04/2017 07:43:55 by
   Daniel Hall (Dandaman955) <Dandaman95@googlemail.com>

   Copyright (c) 2017 Daniel Hall
   Copyright (c) 2019 Bastien Nocera

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License version 3 as published by
   the Free Software Foundation.
*/

#include <stdio.h>
#include <stdlib.h>

unsigned short Checksum(unsigned char *buffer, unsigned short CC_Last, unsigned short ChecksumRange, int i);

int main(int argc, char *argv[])
{
    FILE *fp;
    short TMRValues[3] = {0x1FF0, 0x3FF0, 0x7FF0};
    unsigned char ChecksumRanges[9] = {0x1F, 0x3F, 0x7F, 0xBF, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F};
    unsigned char ROMPages[4] = {0x02, 0x06, 0x0E, 0x1E};
    unsigned char TMR[8] = "TMR SEGA";
    unsigned char *buffer;

    puts("Welcome to the Master System Checksum Fixer!");

    if(argc<2)
    {
        fprintf(stderr,"\nNot enough arguments specified.\n");
        exit(1);
    }

    // If there's no file...

    if((fp = fopen(argv[1],"rb")) == NULL)
    {
        fprintf(stderr,"\nCould not open the file (%s).\n",argv[1]);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    int fsize = ftell(fp);
    rewind(fp);

    if((buffer = (unsigned char *) malloc(fsize)) == NULL)
    {
        fprintf(stderr,"\nCouldn't allocate memory for the file.\n");
        exit(1);
    }
    fread(buffer, sizeof(char), fsize, fp);
    fclose(fp);

    int i;
    int j;
    short TMRStart;
    int HeaderDetected = 0;

    // Detect the TMR_SEGA header at locations 0x1FF0, 0x3FF0 or 0x7FF0.

    for(j = 0; j < 3; j++)
    {
        TMRStart = TMRValues[j];
        for(i = 0; i < 8; i++)
        {
            if(buffer[TMRStart+i] != TMR[i])
            {
                TMRStart = TMRValues[j];  // Restore the correct address of the header.
                break;
            }
            else if(i == 7)
            {
                HeaderDetected = 1;
            }
        }
        if(HeaderDetected)
        {
            break;
        }
        else if(j == 2)
        {
            fprintf(stderr,"\nNo header specified!\n");
            free(buffer);
            exit(1);
        }
    }
    unsigned char ROMHeader = (buffer[TMRStart + 0x0F] - 0x0A) & 0x0F;
    unsigned short ChecksumRange = ((ChecksumRanges[ROMHeader] << 8) & 0xFF00) | 0xF0;
    unsigned short ComputedChecksum = 0;
    ComputedChecksum = Checksum(buffer, ComputedChecksum, ChecksumRange, 0);
    int ROMPage;
    if(ROMHeader > 3)
    {
        ROMPage = ROMPages[ROMHeader - 4] - 1;
        int i = 0x8000;
        while(1)
        {
            ComputedChecksum = Checksum(buffer, ComputedChecksum, 0x4000, i);
            if(ROMPage == 0)
            {
                break;
            }
            ROMPage--;
            i += 0x4000;
        }
    }

    // Check the checksum.

    if (buffer[TMRStart + 0x0A] != (ComputedChecksum & 0xFF) ||
        buffer[TMRStart + 0x0B] != ((ComputedChecksum >> 8) & 0xFF))
    {
        fprintf(stderr,"\nChecksum did not match\n");
        exit(1);
    }

    // Program-closing goodies.
    printf("\nChecksum verified successfully\n");

    free(buffer);
    return 0;
}

unsigned short Checksum(unsigned char *buffer, unsigned short CC_Last, unsigned short ChecksumRange, int i)
{
    unsigned char cs1 = (CC_Last >> 8) & 0xFF;
    unsigned char cs2 = CC_Last & 0xFF;
    unsigned char cs3 = 0;  // Artificial carry flag for adc emulation.
    unsigned char e = 0;    // a
    unsigned char ov1 = 0;
    unsigned char ov2 = 0;

    while(1)
    {
        e = cs2;             // ld a, e
        ov1 = e;             // Set first overflow check flag.
        e += buffer[i];      // add a, (hl)
        ov2 = e;             // Set the second overflow check flag.
        if(ov1 > ov2)        // Is the last value larger than the first (indicating overflow)?
        {
            cs3 = 1;         // If it is, set the carry for the adc instruction.
        }
        cs2 = e;             // ld e, a
        e = cs1;             // ld a, d
        e += cs3;            // adc a, $00
        cs3 = 0;             // Reset carry flag.
        cs1 = e;             // ld, d, a
        i++;                 // inc hl
        ChecksumRange--;     // dec bc
        if(ChecksumRange==0)
        {
            break;
        }
    }
    CC_Last = (cs1 << 8) & 0xFF00;
    CC_Last |= cs2;
    return CC_Last;
}
