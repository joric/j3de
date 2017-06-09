j3de
====

Originally written in 2001 as a softrender engine for 64k intros (see oldsrc folder).
Uses Clax keyframing library by Borzom.

[![2ndreal](http://img.youtube.com/vi/81AnuDFmbEM/0.jpg)](https://www.youtube.com/watch?v=81AnuDFmbEM)

License
-------

Public domain, I guess. Clax may be linked as dll. You may not mention me (preferably).
Stencil shadows do not use z-fail because it is patented (there is no z-pass either).

Controls
--------

* Tab - switch cameras
* Up/Down - toggle rendering mode (wireframe/solid/textured)
* F or Enter - toggle full screen
* Left/Right - rewind scene (back and forward)
* Space - pause scene animation
* S - toggle shadows (Second Reality scene only)
* PgUp/PgDn - load previous/next scene


Scenes
------

The engine uses good old 3ds scenes with spline animation, played with Clax. The best editor for those scenes, without a doubt,
3ds Max 1.0, released in 1996 (haven't changed much after that, except getting overweight, first version was only 20 Mb).
Works just fine with a new 64-bit Sentinel Driver (you have to download it separately).
In case if camera animation is broken or camera target disappears after reimport, try to create a new camera and copy animation tracks from the old one.
In case if you got used to Blender, note that it doesn't yet import or export 3ds animation at all (checked at 2.75).


3-rd party code
---------------

### Clax

Note that original Clax does not support:

* Instances
* References
* Smoothing groups
* Probably a ton of other things

OpenGL version uses (tries to use) vanilla Clax 0.10, so mind that scene won't load if it contains
instances or referenced objects, so you have to make a full-blown copies of everything.
This is terrible sizewise, but nevertheless, it works just fine without referencing (toolsmedia style).
If anything, you can write your own handler, see the metaballs object for example ("Fx01" in 1.3ds).
And the file can be well compressed anyway.

### ~~Meatballs~~ Metaballs

One scene uses pretty fast and robust 3-rd party metaballs code,
based on Crystal Space Metaballs Demo (1999) by Denis Dmitriev.
The code recursively travels through adjacent cells and ought to (must to) be better than O(n^3).

### JPEG Loader

The old version used my own DCT/iDCT converter and raw DCT bitmaps compressed along with the executable,
but real JPEG's are just so much more convenient.
Current version uses baseline (not progressive) JPEG loader by Frustum.
I haven't stumbled upon any loading errors yet, despite the loader is quite small
(16 Kb of plain C) and doesn't use external libraries.

