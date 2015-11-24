DirectXAmbi
===========

This is a Ambilight driver for windows which uses DirectX9 for windows 7 or Directx11 for windows 8.

It uses the WS2812 library from: https://github.com/cpldcpu/light_ws2812 
Because I've never done assembly before ;)
Later on I'm thinking of writing my own replacement for it, that maybe is even more efficient.

You can use an Arduino uno for this software. Connect the WS2812B data line to pin 8 on the arduino.

You need to install the DirectX SDK to compile it. 
http://www.microsoft.com/en-us/download/details.aspx?id=6812

It works right now. There are no known errors or bugs.
If you find any please report them.

If it cant capture your game when using D3D then try the game in fullscreen windowed, or windowed mode.
Some games tend to flicker. If that happens try turning on vsync
GDI currently only captures your primary monitor. I'm working on a fix for this.
When playing bluray with PowerDVD 14, you can just play the stream files and it works fine.

At first startup it will start a wizard to help you configure the software.
It is all terminal based, because I hate to create an GUI.
Comments are all in dutch.

It currently detects if there are black borders around the video feed and adjusts accordingly.
There is a brightness setting which uses the room created by using a BlackLevel.

Currently I am working on an GUI and I am trying to improve the performance. I also am thinking about adding an simalation mode which draws small rectangles on the edge of your screen which simulates the leds.

For any question please contact me at jobvdv@gmail.com.
For a precompiled version you also can contact me.
