# Tate

Tate is a chess engine implementing parts of the xboard protocol.

## Portability

Tate should compile and work on all operating systems.  It does *not* work
under Windows, even if it compiles! Tate is currently relying on the
existence for "/dev/urandom" for high quality random data and that device
does not exist under Windows.  Patches are welcome!

## Description

The software contains of two parts. The library "libchi" is meant to implement
all kinds of functions needed for chess engines in general. The actual program
"tate" implements the engine, evaluation function and so on.

## Strength

Tate probably plays on the level of FruityMax. Note that the last versions are
actually weaker than previous ones. The strongest one seems to be version 0.3.2.
Check out the branch `branch-v0.3.2` for it.

## History

Tate was written in November and December 2002 as an experiment. The original
repository was on a CVS server that is now lost. But I have created a minimal
git history by committing release files.

## Copyright

Copyright (C) 2002-2019, Guido Flohr <guido.flohr@cantanea.com>.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
Written by Guido Flohr.
