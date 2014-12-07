DirectXAmbi
===========

This is a Ambilight driver for windows which uses DirectX9.

It uses the WS2812 library from: https://github.com/cpldcpu/light_ws2812 
Because I've never done assembly before ;)
Later on I'm thinking of writing my own replacement for it, that maybe is even more efficient.

You can use an Arduino uno for this software. Connect the WS2812B data line to pin 8 on the arduino.


You need to install the DirectX SDK to compile it. 
http://www.microsoft.com/en-us/download/details.aspx?id=6812

It works right now. There are no known errors or bugs.
If you find any please report them.

When capturing Netflix use GDI. If it cant capture your game when using D3D then try the game in fullscreen windowed, or windowed mode.
Some games tend to flicker. If that happens try turning on vsync
GDI currently only captures your primary monitor. I'm working on a fix for this.
When playing bluray with PowerDVD 14, you can just play the stream files and it works fine.

At first startup it will start a wizard to help you configure the software.
It is all terminal based, because I hate to create an GUI.
Comments are all in dutch.

For any question please contact me at jobvdv@gmail.com.
For a precompiled version you also can contact me.

