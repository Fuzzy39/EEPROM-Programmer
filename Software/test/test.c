#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>

#define size 0x2000

void main(void)
{
    printf("Hello!\n");

    FILE* fptr = fopen("rom.bin", "wb"); // need to tell windows it's binary, evidently.
    uint8_t* data = (uint8_t*)malloc(size);

    srand(time(NULL));   // Initialization, should only be called once.
    size_t i = 0;
    for(; i<size; i++)
    {

        data[i] = (uint8_t)rand();
      
    }
    fwrite(data, 1,size,fptr);

    // Write some text to the file
    //printf("Wrote %d bytes.\n", i);
    // Close the file
    fclose(fptr); 
}