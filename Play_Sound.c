#include<stdio.h>//printf()
#include<stdlib.h>//atoi()
#include<string.h>//strlen()
#include "Play_Sound.h"//Play_Sound()

#if defined(_WIN32) || defined(_CYGWIN_) // Windows
    #include <synchapi.h> //Sleep()
#else
    #include <unistd.h> // usleep()
    void Sleep(unsigned long int milliseconds) { usleep(milliseconds * 1000); }
#endif

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <seconds> <file Path>\n", argv[0]);
        return 1;
    }    
    int seconds = atoi(argv[1]);
    if (seconds <= 0) {
        printf("Please provide a valid positive integer for the duration.\n");
        return 1;
    }
    
    char* file = argv[2];
    if(file == NULL){
        printf("Please provide a valid file path.\n");
        return 1;
    }
    char format[4];// the last 4 characters of the file most be .wav
    format[0]= file[strlen(file)-4];
    format[1]= file[strlen(file)-3];
    format[2]= file[strlen(file)-2];
    format[3]= file[strlen(file)-1];

    if(strcmp (format, ".vaw") != 1 ){
        printf("Please provide a valid .wav file\n");
        return 1;
    }

    //Play_Sound() function to play sound
    Play_Sound(file);
    printf("Playing '%s' file for %d seconds\n",file, seconds);
    for (int i = 0; i<seconds; i++){
        printf(" Seconds remaining %2d\r", seconds - i); //display the remaining time
        fflush(stdout); 
        Sleep(1000);
    }
    printf("\n");

    //Stop_Sound() function to stop sound
    Stop_Sound();

    return 0;
}