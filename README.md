# Commander Keen in Keen Dreams (Steam Release)

This repository contains (most of) the source for (an older version of) the Steam Release Commander Keen in Keen Dreams. 

It is released under the GNU GPLv2+. See LICENSE for more details.

The release of the source code does not affect the licensing of the game data files, which you must still legally acquire. Note that the data files for the Steam version do not quite match the data files for any of the previous versions.

*Note that the Steam version of Keen Dreams is no-longer available for purchase, so it may not be possible to acquire the data files if you don't already have them!*

# What does this contain?

This contains a version of the code from *before* Nightdive Studios took over, and hired the great Ryan C. Gordon to [fix and update the port](https://www.patreon.com/posts/15286078). It therefore does not contain any of the fixes within.

It does not contain any of the data files for the game, including graphics, sounds, story text, and the headers/compression dictionaries needed to load them.

It also is missing some of the later code from the Windows version, so it probably only builds on Linux at the moment. It also does not contain the actual Steam integration code, which required the Valve Steam SDK. There's a null backend for that in ``kd_stat_null.c``, originally the Steam backend was in ``kd_stat_steam.cpp``.

# How do I compile it?

Keen Dreams is built with CMake, and needs a C99-compatible compiler. It depends on [SDL 2.0](https://libsdl.org), OpenGL, and [GLEW](http://glew.sourceforge.net/).

Compiling it should be straightforward:
	mkdir build
	cd build
	cmake ..
	make

Once compiled, it needs to be in a directory with the datafiles from the Steam version:

* ``CONTEXT.KDR``
* ``EGADICT.CGA``
* ``EGADICT.KDR``
* ``EGAHEAD.CGA``
* ``EGAHEAD.KDR``
* ``GAMETEXT.KDR``
* ``KDREAMS.CGA``
* ``KDREAMS.CMP``
* ``KDREAMS.EGA``
* ``KDREAMS.MAP``
* ``KDREAMS.SND``
* ``MAPDICT.KDR``
* ``MAPHEAD.KDR``
* ``SOUNDDCT.KDR``
* ``SOUNDHHD.KDR``
* ``STORY.KDR``

There are various modding tools available which could be able to create or modify files in these formats: they're all compatible with the original DOS game, with the following exceptions:

* The CGA files are actually EGA files containing only the CGA colours.
* The ``KDREAMS.CMP`` file contains extra files. In addition to the ``TITLESCR.LBM`` file, there is also a CGA title screen: ``TITLECGA.LBM``, and two pages of credits: ``CREDITS1.LBM`` and ``CREDITS2.LBM``. These are all EGA Deluxepaint II files.
* The sound files are in a custom DPCM format. The code used to produce them is in the ``sound/`` directory.

# Credits

Keen Dreams was originally developed by id Software for Softdisk publishing.

This port was largely done by David Gow, with additional code by Emma Krantz and Braden Obrzut. 

Extra-special thanks must also go to NY00123, whose [RefKeen project](https://github.com/NY00123/refkeen) is also able to load the Steam version's files, and to Ryan C. Gordon for fixing many bugs in later versions. Thanks, too, to everyone who worked on the [Omnispeak](https://davidgow.net/keen/omnispeak.html), particularly NY00123 and lemm.

Copyright for Keen Dreams is held by Javier M. Chavez. Copyright for this port is held by its contributors.
Commander Keen is a registered trademark of id Software, LLC (A Zanimax Company).

The original Keen Dreams source release was made possible by a crowdfunding effort. 

It was brought to you by Javier M. Chavez and Chuck Naaden with additional support from:

* Dave Allen
* Kirill Illenseer
* Michael Jurich
* Tom Laermans
* Jeremy Newman
* Braden Obrzut
* Evan Ramos
* Sam Schultz
* Matt Stath
* Ian Williams
* Steven Zakulec
* et al

