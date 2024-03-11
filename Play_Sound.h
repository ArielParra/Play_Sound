#ifndef Play_Sound_h 
#define Play_Sound_h 

#ifdef __cplusplus
extern "C" {
#endif

/*OS detection*/
#if defined(_WIN32) || defined(_CYGWIN_) // Windows

    #include <windows.h>//PlaySound()

    #define USE_WINMM_PLAYSOUND_ASYNC //uncomment this line to use sox or ffplay insted
    
    #ifdef USE_WINMM_PLAYSOUND_ASYNC
    
    //#pragma comment(lib, "winmm") //Microsoft Visual C++ (MSVC) specific macro

    // Normal implentation wihout running in the background
    /*
    int Play_Sound(char* file){
        if(PlaySound(TEXT(file),NULL,SND_ASYNC)){
            return 0;
        } 
        return 1;
    }//PlaySound() WILL stop while using getch()
    */
    #warning "PlaySound() needs -lwinmm as compiler argument"
    DWORD WINAPI ThreadFunction(LPVOID lpParameter){
        char* file = (char*)lpParameter;
        PlaySoundA(file, NULL, SND_ASYNC);
        return 0; 
    }
    #include<stdio.h>//fprintf(),stderr
    int Play_Sound(char* file){
        HANDLE hThread = CreateThread(NULL, 0, ThreadFunction, (LPVOID)file, 0, NULL);
        if (!hThread) {
            fprintf(stderr, "Failed to create Play_Sound() thread.\n");
            return 1;
        }
        return 0;
    }//Play_Sound() will NOT stop while using getch()
    int Stop_Sound(){ 
        if(PlaySound(NULL, 0, 0)){
            return 0;
        }
        return 1;
    }
    #else //Alternatively, using sox or ffplay (ffmpeg) programs:
    /* 
        sox.exe needs 14 dlls to work and only uses 4.96MB in total, 
        ffplay.exe is a standalon exe without dlls, but it is 77.2MB in size,
        You can get sox portable from: https://sourceforge.net/projects/sox/files/sox/14.4.2/sox-14.4.2-win32.zip/download
        You can get ffmpeg portable from: https://www.gyan.dev/ffmpeg/builds/ffmpeg-git-essentials.7z
    */

    #include <string.h> //strlen(), sprintf()
    #include <tlhelp32.h> //CreateToolhelp32Snapshot(), Process32First(), Process32Next(), PROCESSENTRY32
    //#include <windows.h> 
    /* CreateProcess(), PROCESS_INFORMATION, STARTUPINFO,  MAX_PATH ZeroMemory(), 
        CloseHandle(), TerminateProcess(), OpenProcess(), Process32First(),
        Process32Next(), CreateToolhelp32Snapshot(), etc. */


    #define USE_SOX //comment this line to use ffplay

    #ifdef USE_SOX
        const char* SoundCommand = ".\\sox.exe";
        const char* SoundArgument = "-t waveaudio -d";
        const char* processName = "sox.exe";
    #else //ffplay
        const char* processName = "ffplay.exe";
        const char* SoundCommand = ".\\ffplay.exe";
        const char* SoundArgument = "-nodisp";
    #endif

    //similar to system("start /b .\sox.exe file -t waveaudio -d >NUL 2>&1 &");
    int Play_Sound(char* file) {
        // Ensure the file is using Windows Routes with backward slashes '\'
        for (int i = 0; i < strlen(file); i++){
            if (file[i] == '/') file[i] = '\\';
        }

        // Create process parameters
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Construct the command 
        const int blankSpaces = 2; 
        char command[(strlen(SoundCommand) + strlen(SoundArgument) + strlen(file) + blankSpaces) % MAX_PATH]; //limit the maximum lenght
        
        if(strlen(SoundCommand)==9){
            sprintf(command, "%s %s %s",SoundCommand, file, SoundArgument); //sox syntaxis
        } else {
            sprintf(command, "%s %s %s",SoundCommand, SoundArgument ,file); //ffplay syntaxis
        }

        // Create the process
        if (!CreateProcess(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            fprintf(stderr,"Error creating process: %lu\n", GetLastError());
            return 1; 
        }

        // Close process and thread handles (do not wait for the process to finish)
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return 0;
    }

    //similar to system("taskkill /f /im processName >NUL 2>&1 &");
    int Stop_Sound() {
        HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hProcess, &pe32)) {
            do {
                if (strcmp(pe32.szExeFile, processName) == 0) {
                    HANDLE hProcessTerminate = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                    if (hProcessTerminate != NULL) {
                        if (TerminateProcess(hProcessTerminate, 0)) {
                            CloseHandle(hProcessTerminate);
                            CloseHandle(hProcess);
                            return 0; 
                        } else {
                            CloseHandle(hProcessTerminate);
                            CloseHandle(hProcess);
                            return 1; 
                        }
                    } else {
                        CloseHandle(hProcess);
                        return 1;
                    }
                }
            } while (Process32Next(hProcess, &pe32));
        }

        CloseHandle(hProcess);
        return 1;
    }
    #endif // USE_WINMM_PLAYSOUND_ASYNC
    
#elif defined(__unix__)|| defined(__APPLE__) || defined(__MACH__) //*NIX (UNIX based OS)

    #include <sys/types.h> // pid_t, setsid()
    #include <unistd.h> // fork(), execlp()
    
    #if defined(__linux__)

    #warning "To use Play_Sound you need the alsa-utils package"
    const char* SoundCommand = "aplay";
    const char* SoundArgument = NULL; 

    /* this is only needed if WSL is wanted,
        if WSL isn't needed then comment both 
        Play_Sound and Stop_Sound functions to use
        the default SoundCommand and SoundArgument constants
    */
    
    #warning "To use Play_Sound in WSL you need the ffmpeg package "
    #include <stdlib.h>//getenv()
    //defines to not use the *NIX Play_Sound and Stop_Sound common functions
    #define Play_Sound_Linux_WSL
    #define Stop_Sound_Linux_WSL

    int Play_Sound(char* file) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            // Child process

            // to do the same as  >/dev/null 2>&1
            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);
            setsid();

            if (getenv("WSL_DISTRO_NAME")) { // WSL
                // this is simlar to system("ffplay -nodisp file >/dev/null 2>&1 &");
                execlp("ffplay", "ffplay", "-nodisp", file, "&", (char *)NULL);
            } else { //normal Linux
                // this is simlar to system("aplay file >/dev/null 2>&1 &");
                execlp("aplay", "aplay", file, "&", (char *)NULL);
            }

            perror("execlp");
            return 1;
        } else {
            return 0;
        }
    }

    int Stop_Sound() {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            // Child process
            const char* aplay = "aplay";
            const char* ffplay = "ffplay";
            // to do the same as >/dev/null 2>&1
            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);
            setsid();
            
            // this is simlar to system("pkill ffplay >/dev/null 2>&1");
            if (getenv("WSL_DISTRO_NAME")) { 
                execlp("pkill", "pkill", ffplay, (char *)NULL); // WSL
            } else { 
                execlp("pkill", "pkill", aplay, (char *)NULL); // Normal Linux
            }

            perror("execlp");
            return 1;
        } else {
            return 0;
        }
    }

    #elif defined(__APPLE__) || defined(__MACH__)
        
        const char* SoundCommand = "afplay";
        const char* SoundArgument = NULL;  

    #elif defined(__OpenBSD__)

        const char* SoundCommand = "aucat";
        const char* SoundArgument = "-i";  

    #else //other *NIX
        
        /* Using the sox package to use the play command */
        #warning  "To use Play_Sound you need the sox package"
        const char* SoundCommand = "play";
        const char* SoundArgument = NULL;  

        /* Using the ffmpeg package to use the ffplay command */
        /*
        #warning  "To use Play_Sound you need the ffmpeg package"
        const char* SoundCommand = "ffmpeg";
        const char* SoundArgument = "-nodisp";   
        */
    #endif 

    #ifndef Play_Sound_Linux_WSL
        int Play_Sound(char* file) {
            pid_t pid = fork();

            if (pid == -1) {
                perror("fork");
                return 1;
            }

            if (pid == 0) {
                // Child process
                const char* aplay = "aplay";
                const char* ffplay = "ffplay";
                // to do the same as  >/dev/null 2>&1
                freopen("/dev/null", "w", stdout);
                freopen("/dev/null", "w", stderr);
                setsid();

                // this is simlar to system("SoundCommand SoundArgument file >/dev/null 2>&1 &");
                if (SoundArgument == NULL) {
                    execlp(SoundCommand, SoundCommand, file, "&", (char *)NULL); // no SoundArgument
                } else { 
                    execlp(SoundCommand, SoundCommand, SoundArgument, file, "&", (char *)NULL);
                }
                perror("execlp");
                return 1;
            } else {
                return 0;
            }
        }
    #endif//Play_Sound for all *NIX

    #ifndef Stop_Sound_Linux_WSL
        int Stop_Sound() {
            pid_t pid = fork();

            if (pid == -1) {
                perror("fork");
                return 1;
            }

            if (pid == 0) {
                // Child process

                // to do the same as  >/dev/null 2>&1
                freopen("/dev/null", "w", stdout);
                freopen("/dev/null", "w", stderr);
                setsid();
                
                // this is simlar to system("pkill SoundCommand >/dev/null 2>&1");
                execlp("pkill", "pkill", SoundCommand, (char *)NULL);

                perror("execlp");
                return 1;
            } else {
                return 0;
            }
        }
    #endif//Stop_Sound for all *NIX

#endif//OS detection 

// Windows PlaySound macro redefinition

#ifdef PlaySoundA
    #undef PlaySoundA
#endif

#define PlaySoundA(pszSound, hmod, fdwSound) do{ \
    if(pszSound==NULL) Stop_Sound();\
    else Play_Sound((char*)pszSound);\
    }while(0)

#ifdef PlaySound
    #undef PlaySound
#endif

#define PlaySound PlaySoundA

#ifdef __cplusplus
}
#endif//__cplusplus

#endif//Play_Sound_h 