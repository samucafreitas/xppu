## xppu
Xppu is a utility I wrote in C for learning purposes.

## Prerequisites
    xlib  - Standard Xlib library
    glib - Gnome library

## Install
```sh
$ sudo make all install
```

## Usage
Option           | Description
-----------------|-------------
`-resolution`    | print out display resolution
`-mouseposition` | returns the current mouse position
`-mousemove x y` | move the mouse to the x,y coordinates
`-selectwin`     | returns (id, class, etc...) of the selected window
`-wintitle`      | returns the title of the current focused window
