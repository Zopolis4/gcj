/*
REQUIRED_ARGS: -d -lowmem -Jrunnable -preview=rvaluerefparam
EXTRA_FILES: xtest46.d
TEST_OUTPUT:
---
Boo!double
Boo!int
true
int
!! immutable(int)[]
int(int i, long j = 7L)
long
C10390(C10390(<recursion>))
tuple(height)
tuple(get, get)
tuple(clear)
tuple(draw, draw)
const(int)
string[]
double[]
double[]
{}
tuple("m")
true
TFunction1: extern (C) void function()
---
*/

mixin(import("xtest46.d"));
