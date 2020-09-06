# mint-boy

Another gameboy advanced emulator, built for fun and leisure. I started this as a pet project and it turned into so much
more. As a gameboy advanced emulator mint-boy offers alot of customization and hooks into the actual emulator. You can
edit memory while the game is running, print current running statistics, add custom overlays full of useful information
and even control execution by controlling what instruction will be executed next. Useful and full of features for
hacking, mint-boy will help you  gain insight into a gameboy's hardware and ARM architecture.

## Details

I wrote mint-boy in C not C++ because I was familiar with C and C++ seemed like too much to learn at the time. I orginally wanted this to be cross platform but Microsoft doesnt directly support C18 using Visual Studio's C compiler. It it is still possible using WSL or perhaps MingW/Cygwin. For this reason I chose to use POSIX and SDL2 for cross platform support (since WSL, MingW or Cygwin can compile them). I used CMake to support the cross platform building. Windows specific support could be added by using different windows specific object files in the build. Finally, I used munit for the unit tests because it was portable and actually a pleasant suprise.

## Features

## Development

## Future

## Known Bugs

