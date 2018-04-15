## Nest Generation 1 UI Toolkit
This is the release of the toolkit used to create a patch for Google's Nest Thermostat to disable remote control and log file uploading. It has not been tested since 2014 and probably won't work on newer generation thermostats.
If you get this working, send me a picture and submit a patch!

It requires FreeType for font rendering and a working ARM toolchain for compilation. Until someone tests this code on a modern Nest, it will be for reference only. I [Grant] don't own a Nest right now, so I can't test this code.

If you are looking for code that does a lot more than display graphics, check out [Luke-Jr's FreeAbode](https://github.com/luke-jr/freeabode). It's a complete Nest frontend replacement.

### Layout

* `src/` - C code for a UI toolkit
* `NestGfx/` - C++ code for graphics demos and a toolkit (recommended)

### Authors

* Orlando Arias - UI toolkit creation
* Grant Hernandez - Toolchain management, graphics demos
