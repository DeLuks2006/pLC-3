<h1 align=center>pLC-3</h1>
<p align=center>
    A simple VM/emulator for the LC-3 architecture written in C.
</p>

This is a simple VM/emulator for the LC-3 architecture written 
fully in C by following the tutorial on 
[jmeiners.com](https://www.jmeiners.com/lc3-vm/).

## TODO:

- [ ] Use function pointers instead of giant switch-case
- [x] Hardcode program to run
- [ ] Add crypter (polymorphic?)
- [ ] Swap opcodes with same functionality
- [ ] Insert trash opcodes

## Changes: (security wise)

...

## Reversing:

This is a little section to place a list of techniques we can use to 
dump the payload/file being ran by the VM and then disassemble it.

- find the image array and dump it LOL (gotta fix this)
- symbolic execution
