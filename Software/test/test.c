#include <stdio.h>

void main(void)
{
    printf("Hello!\n");

    FILE* fptr = fopen("rom.bin", "w");
    int size = 8192;
    int i = 0;
    for(unsigned char j = 0;i<256; i++)
    {
        fwrite(&j, 1, 1, fptr);
        j++;
    }

    for(;i<size; i++)
    {
        int data = 0;
        fwrite(&data, 1,1,fptr);
    }
    // Write some text to the file
   
    // Close the file
    fclose(fptr); 
}