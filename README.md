# Shapemaptool

This tool converts SVG files into TopoJSON format.

## Restrictions 

Objects in SVG must not overlap. If done in Inkscape, all objects must be defined as paths and
split with modify path extension to appropriate resolution. Negative scaling at any stage
will make generated map appear inversed (inside is outside and vice versa).

## Dependencies

- [TinyXML2](https://github.com/leethomason/tinyxml2).
- [CMake](https://cmake.org/)
- [JSON for modern C++](https://cmake.org/)

## Build instructions

```bash
 $ mkdir build
 $ cd build
 $ cmake ..
 $ make
```

## Usage

```bash
$ ./shapemaptool <svg file>  > output.json
```

Program writes json generated from svg file into stdout, redirect as you see fit.

## Example files

- kukkakartta.svg - SVG file used in .pbix file
- powerbee.bpix - Power BI Desktop project file where custom shape map is used.

## License

Copyright Anssi Gröhn anssi dot grohn at karelia dot fi (c) 2020-2021.
GPL-3.0. See LICENSE for details. 

