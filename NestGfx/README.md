## Nest Graphics Demos
This is the code repository used to create my Nest graphics demos used for display at BlackHat 2014.

Note, this assumes you have a rooted Nest thermostat generation 1 and that you have dropbear installed and running on the system. Keep in mind, the code for this was written in 2014 and hasn't been tested since.

### Videos

* Nest Thermostat 3D Graphics and Fractal Demo - [![Nest Thermostat 3D Graphics and Fractal Demo](https://img.youtube.com/vi/UpQynNvkrDI/0.jpg)](https://www.youtube.com/watch?v=UpQynNvkrDI)
* Nest Thermostat 3D Graphics - Twister - [![Nest Thermostat 3D Graphics - Twister](https://img.youtube.com/vi/uPLLFvKcikk/0.jpg)](https://www.youtube.com/watch?v=uPLLFvKcikk)

### Code Layout

* `lib/` - Core libraries for graphics and input on the Nest
* `NestConfig` - The half-way start of a UI demo for the nest (not interesting)
* `NestDemo` - All of the graphics demos and the main `demo.cpp` file
* `notes` - Any notes when trying to get this work on the nest. Doesn't contain much
* `old` - Old files and Orlando's rewrite of the toolkit in C
* `testing` - Some old testing files before I made a toolkit
* `tools` -  Various scripts that really helped during my Nest development and testing
