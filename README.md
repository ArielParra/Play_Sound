# Play_Sound

Play_Sound is a reimplementation of the legacy WINAPI PlaySound()/PlaySoundA() functions, improving them by using a background process to run .wav sound files asynchronously

- less than 350 lines of code covering all platforms.
- Compatibility with most UNIX based Systems including with Linux running under WSL 
- WINAPIs ThreadFunction and process handlers to avoid the direct use of system() in Windows.
- Secure fork() and exec() calls to avoid the unsecure system() calls in UNIX based systems.
- Uses PlaySound() WINAPI function on Windows, aplay on Linux, afplay on macOS, aucat in OpenBSD.
- Alternative options to use sox or ffmpeg on all platforms, by default sox is used on not recognized UNIX based systems.

The function API usage is as follows:

You can use it as the legacy WINAPI form: 
```c
//PlaySound()/PlaySoundA() To play sound from the pszSound file
bool PlaySoundA(LPCTSTR pszSound,HMODULE hmod,DWORD fdwSound); 
//PlaySound()/PlaySoundA() To stop sound 
bool PlaySound(NULL, 0, 0);
```
Or you can use the new Play_Sound and Stop_Sound:
```c
//To Play sound from the file
int Play_Sound(char* file); 
//To Stop sound
int Stop_Sound();
````

By default both implementations `return 1` if there was an error or `return 0` if the call was succesfull

Microsoft PlaySound documentation: https://learn.microsoft.com/en-us/previous-versions/dd743680(v=vs.85)


# Dependencies
- It may need alsa-utils package for Linux.
- Windows needs a C compiler like GCC which can be found in MSYS2.
- Windows needs -lwinmm as compiler argument
- This project have been only tested with GCC and Glibc, compatibility with Compilers like LLVM/CLANG and/or other C libraries like ulibc, musl or bionic is unkown and most likely may not work as intended.
- The GNU make is used for the Makefile, it isn't compatibile with BSD Make.

# Warning
- Functions are defined in the header file which may lead to ODR violations