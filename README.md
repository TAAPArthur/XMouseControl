# XMouseControl

XMouseControl allows you to control your mouse from your keyboard. This differs from X's builtin [MouseKeys](https://www.x.org/releases/X11R7.7/doc/libX11/XKB/xkblib.html#Controls_for_Using_the_Mouse_from_the_Keyboard) in that is supports multiple directions at once. In addition, it does not grab your entire keyboard and supports multiple master keyboards (caveat see bugs)

## Requirements

Building XMouseControl requires:

* the following libraries are needed: -lxcb -lxcb-keysyms -lxcb-xinput -lxcb-xtest
* make
* a C99 compiler


## Configuration

Set keybindings in  `config.h`

## Bugs
Due to difficulty get the KeyMap for secondary keyboards, non default master keyboards with auto repeat enabled may miss a keyrelease when rapidly pressing keys. Simply repressing the key will fix the issue.

## Acknowledgements

XMouseControl was inspired by [ptrkeys](https://github.com/torbiak/ptrkeys/blob/master/ptrkeys.c) which is heavily influenced by [suckless.org's](http://suckless.org) [dwm](http://dwm.suckless.org/).

## License

MIT
