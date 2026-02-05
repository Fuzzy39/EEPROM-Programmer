#include <stdio.h>
#include <time.h>
#include <stdlib.h>


void main(void)
{
    printf("Hello!\n");

    FILE* fptr = fopen("rom.bin", "w");
    int size = 8192;
    srand(time(NULL));   // Initialization, should only be called once.
    for(int i=0;i<size/sizeof(int); i++)
    {
        char data = rand();
        fwrite(&data, sizeof(char),1,fptr);
    }
    // Write some text to the file
   
    // Close the file
    fclose(fptr); 
}