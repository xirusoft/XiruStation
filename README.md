# XiruStation
Yet Another NES/Famicom emulator in C written for both learning purposes and fun.

The main motivation behind it was nostalgic: be able to run & play decently most games, just like my old Famiclone did 30+ years ago. Therefore, it does not pass in several Test Roms, and may not be used as reference for accuracy. For this purpose I suggest more advanced emulators, such as Nintendulator and Mesen.

There is still no GUI, loading a game is made through command line eg:

$ ./XiruStation Castlevania



FEATURES
* high compatibility, with almost 700 games tested at present time, most working fairly well;
* Mapper suport (some partial) : 0, 1, 2, 3, 4, 5, 7, 9, 10, 16, 18, 19, 21, 22, 23, 24, 25, 26, 32, 32, 33, 34, 65, 66, 67, 68, 70, 72, 75, 78, 79, 80, 87, 88, 89, 91, 93, 94, 95, 97, 118, 119, 140, 152, 154, 180, 184, 206, 210;
* accurate 2A03 emulation including illegal opcodes required by some games;
* smooth pixelation using SDL/OpenGL;
* all 5 Audio channels emulated;


TODO
* audio expansion from Mappers: Namco 163, MMC5, VRC6/7, Sunsoft 5B, Jaleco D7756;
* enhance APU output mixer, especially for Square Channels;
* GUI;
* Debugger;
* Save/Load State;
* Famicom Disk System support.


BUILDING
* Make sure you have SDL2 developers package installed;
* place all files in same folder. For Linux system, command line would be something as:

$ g++ *.cpp -w -lSDL2 -o Xirustation


CONTROLS
* D-PAD = Keyboard arrow keys;
* Button A = Keyboard Z key;
* Button B = Keyboard X key;
* Button Select = Keyboard A key;
* Button Start = Keyboard S key.


ACKNOWLEDGEMENTS
* to all people on Nesdev Forum;
* Nesdev Wiki: simply the one-stop reference for everything related to NES programming.
* Other incredible emulators I compared the outputs for debugging purposes:
- Nintendulator
- Mesen
- Fceux.
* ... and very Special Thanks to Mr. Masayuki Uemura, the genius behind Famicom engineering, and his boss, Mr Hiroshi Yamauchi

