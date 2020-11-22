---------------------------------------------------------------------
How is Keen Dreams (a 2D platformer) related to the Catacombs (FPSes)
---------------------------------------------------------------------

A lot of the original Keen Dreams code base, with changes, was re-used by
id Software not only in the later Keen games from 1991 (i.e., Keen 4-6), but
also in Catacomb 3-D. This should explain why do Keen 4-6 and Catacomb 3-D seem
to share features like the control panel (menu), including the Paddle War game.

While the later Catacomb games may have the control panel and other features
removed or replaced, they're still clear derivatives of Catacomb 3-D.

The Wolfenstein 3D sources are also based on Catacomb 3-D, and again share
a lot of similarities.

------------------------------------
Comparisons to original DOS versions
------------------------------------

These source ports aim to reproduce behaviors of original executables
for DOS, including bugs, at least if it's not too difficult
(e.g., anything that greatly depends on the memory layout).
With the exception of The Catacomb Armageddon/Apocalypse, this also includes
compatibility with saved games for original DOS executables (done per version).
The Chocolate Doom source port can be considered an inspiration for that.

In fact, originally this codebase started as a Keen Dreams port titled
"Chocolate Keen Dreams". With the addition of support for The Catacomb Abyss,
and also in order to be a bit more original, the titles of "Ref Keen Dreams"
and "Ref Catacomb Abyss" were coined. Similar titles were later used for
the other Catacombs, with no specific name for the codebase. There are
chances some of these names are still found, here and there.

As of this release, the whole codebase has the title of "Reflection Keen".

------------------------------------------------------------------------
Differences from the original releases related to command line arguments
------------------------------------------------------------------------

- There are some differences when it comes to the way command-line arguments
are parsed. The fact that a single EXE may support more than one version of
a game has an influence on that. See "Command line arguments" for more
information about supported command line arguments.
- In the original shareware releases of Keen Dreams, versions 1.13 and 1.20,
the /DETOUR command line argument should be passed in order to start the game.
This is not required in Reflection Keen Dreams, and in fact, leads to the same
behaviors as *omitting* this argument in any of these original DOS releases.
- The same applies to The Catacomb Adventure Series, regarding this command
line argument (yeah, that's no mistake): ^(a@&r`

----------------------------------------------------------------------------
Differences from the original releases related to usage of multiple DOS EXEs
----------------------------------------------------------------------------

There are some original game releases that have more than one EXE
(per release). There are a few technical differences.

- For one, no separate EXE is actually used in each Reflection Keen port.
As an example, in Keen Dreams v1.13, the behaviors of KDREAMS.EXE (the game)
and LOADSCN.EXE (showing ordering information on quit) are present
in a single Reflection Keen Dreams EXE.
- There are a few differences with the memory management, when it comes
to LOADSCN.EXE, INTRO.EXE and DEMOCAT.EXE from The Catacomb Abyss v1.13.
Basically, they may share some memory in the Reflection Catacomb Abyss port.
The same applies when it comes to different revisions of any of
these EXEs (e.g., HINTCAT.EXE from The Catacomb Abyss v1.24).
- Some of these additional EXEs lack original source codes as of
this release; So such sources had to be manually reconstructed.
See "Additional sources/data used for these ports"
below for details about these codes and more.

----------------------------------------------
One more difference from the original releases
----------------------------------------------

This one is specific to The Catacomb Armageddon/Apocalypse. If the
cheat code letting one cycle through ceiling/sky colors is used while there
is flashing (a lightning), this can lead to so-called "undefined behaviors". It
may seem harmless, but a crash and/or other unexpected side-effects may occur.

----------------
What is included
----------------

Reflection Keen consists of ports of Keen Dreams and all 3D Catacomb games.

Note that these ports do not come with a copy of any game, which is required.
Look below for mentions of "game data" to find possible sources for these.

-------------------
Supported platforms
-------------------

Reflection Keen builds for Windows, Linux and Android are available.
Thanks to external assistance, it was also made possible to
build Reflection Keen for OS X/macOS, but this isn't
well-tested due to lack of access to a Mac.

----------------------------------------------
Supported game data, described by DOS versions
----------------------------------------------

- Keen Dreams: Registered CGA+EGA v1.00, Shareware (EGA) v1.13,
CGA v1.05, Registered (EGA) v1.93 and Shareware (EGA) v1.20.
There's also partial support for the data from the (currently unavailable)
2015 re-release, previously offered on Steam and IndieGameStand. However,
this may fail to work with the game as downloaded via the Steam client
on Windows, due to minor differences in GAMETEXT.KDR
(although this was corrected at some point).
- Catacomb 3-D (The Descent): Versions 1.00 and 1.22.
- The Catacomb Abyss: Versions 1.13 (QA [0], Shareware) and 1.24 (rev 1).
- The Catacomb Armageddon: Version 1.02 (rev 1).
- The Catacomb Apocalypse: Version 1.01 (rev 1).

Note that Keen Dreams v1.20 hasn't been widely available so far, although
it is briefly mentioned in the Apogee FAQ. Furthermore, based on the source
codes release, the differences between v1.13, v1.93 and v1.20 are quite small,
and the two releases of v1.20 and v1.93 share the same static game data
(e.g., common EGA and map headers).

----------------------------------------------------------------------------
Where can I get compatible Keen Dreams game data (updated as of Jun 03 2020)
----------------------------------------------------------------------------

You can download a Shareware release of Keen Dreams, v1.13, from one of a few
places online, at the least. Links for reference:

- http://cd.textfiles.com/megagames2/GAMES/APOGEE/KEENDR13.ZIP
- (You want keendm.zip) https://dukeworld.duke4.net/classic%20dukeworld/share/

-------------------------------------
Partially compatible Keen Dreams data
-------------------------------------

2015 had a (since removed) re-release of Keen Dreams on Steam, IndieGameStand
and possibly also another store.
Note that while Reflection Keen is partially compatible with the 2015
data, it is still recommended to use data from any of the supported
DOS versions. In particular, the Reflection Keen Dreams codebase
is known to be internally different from the re-release's
codebase in many ways.

Also, this might *fail* to work with the 2015 re-release as originally
available from Steam on Windows, due to minor differences
in GAMETEXT.KDR. Updating the game via the Steam
client may assist with resolving this issue.

--------------------------------------------------------------------------
Where can I get compatible Catacombs game data (updated as of Jun 03 2020)
--------------------------------------------------------------------------

The Catacomb Abyss v1.13 can be downloaded as Shareware from the net. You can
try any of these links:

- http://cd.textfiles.com/maxx/tothemaxpcg/ARCADE/CATABS13.ZIP
- (You want v1.13) https://www.classicdosgames.com/game/The_Catacomb_Abyss.html

Copies of all games from the Catacomb series, including Catacomb 3-D and
The Catacomb Adventure Series, are currently available from GOG.com under
the "Catacombs Pack". On the Windows platform and under normal circumstances,
Reflection Keen can auto-detect and read the data from this pack
(assuming it was installed from setup_catacombs_pack_2.1.0.5.exe).

Link to the pack on GOG.com: https://www.gog.com/game/catacombs_pack

------------------------------------------------------------
Running any of the games using the corresponding source port
------------------------------------------------------------

If you used an installer for the games, creating shortcut/launcher icons
on the way, then selecting the correct icon should be sufficient.

Reflection Keen should consist of five executables, one per game, capable of
launching all supported versions of the given game:

- reflection-kdreams(.exe) for Keen Dreams.
- reflection-cat3d(.exe) for Catacomb 3-D (The Descent).
- reflection-catabyss(.exe) for The Catacomb Abyss.
- reflection-catarm(.exe) for The Catacomb Armageddon.
- reflection-catapoc(.exe) for The Catacomb Apocalypse.

If you want to manually start a game on Windows, make sure that SDL2.dll
and libspeexdsp-1.dll reside with the exe of choice.

Check above for "game data" in order to find a way to obtain compatible data.

----------------------------
The Reflection Keen launcher
----------------------------

When you start any of the Reflection Keen ports using the corresponding
shortcut/launcher icon or EXE file, you should get a window with an
internal Reflection Keen launcher. It should be possible to navigate
through that launcher with a keyboard, a mouse and/or a touchscreen.

It's also possible to use a game controller for navigation, although
there are good chances you won't be able to do so, at least initially.
See "Adding an unsupported game controller" below for more details.

Note that a US keyboard layout is currently assumed for the launcher
(mostly for consistency with the games themselves, including DOS versions).

You can use the keyboard's letter keys, as well as a few other keys, for
selecting menu items. This is probably mostly useful when you manually want
to add a game installation.

Similarly, the keyboard can be used to enter command line arguments for the
game (after selecting "Set arguments for game" in the main menu). Press on
the "Enter" key to confirm the changes to the arguments, or "Esc" to cancel.

It's also possible to show an on-screen keyboard, in case you're not using
an actual keyboard. Click/Touch on the "..." button on the top-right of the
launcher window in order to show it.

In case you're using a supported game controller with the Xbox 360 controller
layout, press on the Y button to show the on-screen keyboard. While the
on-screen keyboard is shown, press on the controller's X button to toggle
shifted keys on/off. While editing command-line arguments, select
the "Ent" key in the on-screen keyboard to confirm the changes.
To discard the changes, press on the controller's B or Back button.

--------------------------------------
Where is the configuration file stored
--------------------------------------

Each EXE should generate its own configuration file, having a filename ending
with ".cfg".

These are the default locations for the configuration file:

- Windows: %APPDATA%\reflection-keen.
- OS X/macOS (not well-tested): ~/Library/Application Support/reflection-keen
- Linux: $XDG_CONFIG_HOME/reflection-keen, if $XDG_CONFIG_HOME is defined
and filled. Otherwise, ~/.config/reflection-keen.
- Android: This is separated per installed Android app. For Keen Dreams,
this is the folder Android/data/net.duke4.ny.reflection_kdreams/files/appdata
within the shared storage, as often accessible when connecting an
Android-powered device to a PC/Mac using a USB cable. Similar folder
names are used for the other Reflection Keen apps (games).

The -cfgdir command line argument can be used to override this path, but
make sure to be consistent with your choice. Otherwise, you may get
unexpected behaviors.

------------------------------------------
Where are other newly written files stored
------------------------------------------

There are other files that may be written by each source port, like files
generated by the original DOS versions of the supported games. These are
generally split into separate directories, based on original versions
of the games (although there may be exceptions).

These are the default locations for the various files:

- Windows: %APPDATA%\reflection-keen.
- OS X/macOS (not well-tested): ~/Library/Application Support/reflection-keen
- Linux: $XDG_DATA_HOME/reflection-keen, if $XDG_DATA_HOME is defined
and filled. Otherwise, ~/.local/share/reflection-keen.
- Android: Same as the location of the configuration file.

The -datadir command line argument can be used to override this path, but
again ensure you're consistent with your choice. Otherwise,
you may get unexpected behaviors, as with the cfg dir.

------------------------------------------------------------------
A few configuration file settings not accessible from the launcher
------------------------------------------------------------------

- (Not available on all platforms) "fullres" can be edited to specify a
fullscreen resolution, with the form {width}x{height}, just like the value
of the "windowres" setting. Clearly, it's not necessary to pick a windowed
resolution from the launcher, in case the window is resizable. The reason
there's no separate menu item for the full screen resolution, is that using
any resolution other than the desktop's may lead to unexpected behaviors on
certain environments, like desktop icons getting messed up.
- "sndinterthreadbufferratio" specifies the sizes of a few misc. internal
audio buffers, relatively to another buffer (which is filled by SDL).
If you hear crackling sound, then increasing this value *might* resolve
the issue, although more memory will be consumed. Also, there's
no *guarantee* that this will solve any problem.

The following settings are not even written to the cfg file by default:

- "manualgamevermode" can be used if one wants to replace some game data.
By default, checksum and filesize verifications are done, so Reflection Keen
won't let you mistakenly start a game with modified data, even if the DOS EXE
or another original game file is OK. Given that none of the DOS versions
has such a verification (at least in general), "manualgamevermode" is an
unofficial path for doing the same here. Note that it is an UNOFFICIAL feature.
You may wish to make a full backup of the directory containing
"newly written files" as mentioned above before using this.
Also note that no automatic detection of game installations, other than what's
manually added, is done, in case you set "manualgamevermode" to "true".
- "farptrsegoffset" can be used for (some) level of saved game compatibility
in The Catacomb Armageddon/Apocalypse (doesn't apply to Abyss). Using just any
of the original DOS executables for one of these titles, it is possible that
a game saved at some point will fail to properly load later, due to
the location of a modified copy of the DOS EXE in the address space.
The value given to "farptrsegoffset" is a hexadecimal value that can
be used to control this location (as if a DOS executable were used).
If you have a saved game coming from a default setup of DOSBox 0.74, or some
official SVN build of DOSBox from 2014, chances are farptrsegoffset=1A2 is the
desired setting.

MOST IMPORTANT: If a Catacomb Armageddon/Apocalypse game is saved using the
source port, it will not properly load once farptrsegoffset is modified!

--------------------
There is some bug...
--------------------

Naturally, bugs tend to happen in the porting process, and also later.
But, if it's rather a bug found in an original game release which is
reproduced, most chances are it won't be fixed, since these ports aim
to preserve behaviors of original releases, including bugs.

------------------------------------------------------
Why are CGA graphics supported in the Keen Dreams port
------------------------------------------------------

Since there were originally releases compatible with the CGA, and we have the
source code(s) for these, then why not support these?
In fact, CGA graphics were supported before EGA in this port.

As a possible preparation for Commander Keen 4-6, some engine code considered
to be shared with these was originally ported from the Catacomb 3-D source code
(https://github.com/FlatRockSoft/Catacomb3D), with no actual testing.
CGA routines were preferred, given possible complications with the EGA
bit planes and multiple read/write modes. While Catacomb 3-D is EGA-only,
this assisted with the porting of Keen Dreams with CGA graphics later.

------------------------
Game controllers support
------------------------

There are two ways in which game controllers can be used in each game:
A classic controller scheme, and a modern one. The latter is currently
the default.

Following is a short explanation for the presence of these two schemes:

- The classic controller scheme is used to (somewhat) mimic the behaviors of
the original DOS versions of the games. These versions have basic support for
2-buttons joysticks, which are vastly different from a lot of the game
controllers in use these days, and have great limitations. In particular,
there is often the need to go through a process called "calibration", and
a separate keyboard is still required for various purposes (e.g., text input).
Other games, like Wolfenstein 3D and later releases of Keen 4-6,
also have support for all 4 buttons of the Gravis PC GamePad,
but there are still clear limits.
- The modern controller scheme is a better fit for various game controllers
used these days, with an emphasis on the Xbox 360 controller layout.
It makes it possible to exclusively use such a controller, with
no other input device (like a keyboard) required. There are still
some limitations, though. Look for "Limitations" below for details.

Note that with the modern scheme, each of the games will behave like there are
0 joysticks. Instead, keyboard/mouse emulation is used behind the scenes.

---------------------------------------
Classic controller scheme - Limitations
---------------------------------------

With a few exceptions in mind, any game controller that can be used in general
should be usable by Reflection Keen with that scheme. However, there isn't
really a way to configure/select features of the controller (like buttons).
Limitations from the original DOS versions apply, too. As stated above, this
includes the need to calibrate a joystick, as well as the fact that a keyboard
device (or another alternative) is still mandatory for various purposes.

--------------------------------------
Modern controller scheme - Limitations
--------------------------------------

Unfortunately, due to historical reasons, a lot of game controllers for
PCs lack a standard layout that requires zero configuration from the user.
You may wish to check out "Adding an unsupported game controller" if
your controller cannot be used. Checking if it is usable in
the launcher is a good way to verify this.

With the modern scheme in mind, it is currently assumed the controller has
the following features (although some of these are configurable).
The Xbox 360 controller layout should be covered.

- A d-pad (if there's no d-pad, maybe you can fake one using an analog stick).
- Four face buttons (A, B, X, Y on the Xbox 360 controller).
- Two shoulder/bumper buttons.
- Two trigger buttons (they don't have to be analog).
- Reserved Back button (Esc key replacement) and Start button (Pause key).

It's also possible to take advantage of clickable analog sticks.

However, you *cannot* take advantage of guide button clicks as
technically possible with the Xbox 360 controller.

The Reflection Keen launcher should be used for configuring in-game actions
for the controller. Any in-game configuration setting is irrelevant,
and it's actually better to not touch it. A possible issue with this
is described below.

As stated above, with the modern controller scheme, 0 joysticks are detected
in-game, and keyboard/mouse emulation is in use.

While it is still possible to configure the keyboard in-game, as in
the original DOS releases, this can lead to side effects. For instance, if you
configure 'Space' to be the key for shooting a fireball in the game Catacomb
3-D, it will be used not just for shooting, but also for drinking potions.
This is the case in the original DOS releases, as well as the source port.
While the controller is used, this can further lead to a similar effect
when any of the controller's features (e.g., a button) is configured
either for shooting or potion drinking.

There may still be cases where a keyboard is required, but hopefully there
aren't really. An on-screen keyboard should be displayed in-game for
text input, and it's also possible to show a keyboard
for entering debug keys (e.g., cheat codes).

---------------------------------------------------------------------------
Modern controller scheme / Launcher - Adding an unsupported game controller
---------------------------------------------------------------------------

Given the need to map layouts as described above, not all game controllers are
supported with this scheme out-of-the-box, even if any of them can be used
as a joystick while the classic controller scheme is in use. It is
expected that all XInput-capable controllers are supported
on Windows, along with a few more.

SDL2 should have a small built-in mapping table for other controllers and
non-Windows platforms. In addition, a file named gamecontrollerdb.txt can
be prepared in the location of the Reflection Keen cfg file
(see "Where is the configuration file stored" above for details).
This file can have additional controller mappings.

Such a mapping can be prepared for an unsupported game controller.
There are a few ways to do this:

- Use the SDL2 Gamepad Tool available from General Arcade as of Jun 03 2020:
https://generalarcade.com/gamepadtool/
After using it, simply copy the contents of the newly generated
gamecontrollerdb.txt file to the one bundled with any of the
Reflection Keen source ports (or use it as-is if one doesn't exist).
- Alternatively start Steam (https://store.steampowered.com/) in Big Picture
mode, then choose Settings -> Controllers and configure the mapping. Once
that's done, it's expected that you can optionally upload the mapping so other
users of the same controller can take advantage of it. Either way, though,
the directory where Steam is installed should have a config/config.vdf file,
with a new "SDL_GamepadBind" entry. The value of this entry (following the
mention of the string "SDL_GamepadBind") should be copied to the end of the
file gamecontrollerdb.txt bundled with one of the Reflection Keen ports.
- Use test/controllermap.c (and controllermap.bmp) from the SDL2
sources, on which the SDL2 Gamepad Tool above may possibly be
based (source code only, no EXE): https://hg.libsdl.org/SDL/
This should write output to a command prompt or similar, including a mapping
that can be added to gamecontrollerdb.txt,

--------------------------
Multi-touch screen support
--------------------------

Version 0.16.0 of Reflection Keen introduced support for multi-touch input,
along with support for Android.

- On Windows and Linux, multi-touch input is disabled by default, because there
are chances such input may arrive from a multi-touch trackpad
(at least on Macs).
- In fact, if you're running Reflection Keen on OS X/macOS
(not well-tested), then multi-touch input is not supported at all.
Two-finger scrolling should still work in the launcher, though.
- On Android, multi-touch input is auto-detected by default. This means
that touch controls should be shown while touch input is in use, and hidden
once you begin using a different input device (like a game controller).
There may be exceptions to this, e.g., pressing on an Android device's "Back"
button shouldn't immediately hide touch controls.

You may permanently disable, or permanently enable, multi-touch input,
in case you have issues. Note that if you permanently *enable* multi-touch
input, then a mouse will behave more-or-less like a finger.

As with the modern controller scheme, if you take advantage of multi-touch
input, then, keyboard/mouse emulation is in use.

Also, you probably do *not* want to mess with the in-game input settings
while using multi-touch input (say, any in-game keyboard configuration).

----------------------------------
Android - Granting access to files
----------------------------------

As stated above, on all supported versions of Android, the source ports
can write files to directories which are specific to the Android apps. For
Keen Dreams, this is Android/data/net.duke4.ny.reflection_kdreams/files/appdata
within the shared storage (as accessible from a PC/Mac, using a USB cable).
It should be similar for the other games.

However, read-only access to more directories is desired, so you have
the freedom to prepare a copy of a supported game (say Keen Dreams v1.13)
anywhere in the shared storage. You can then select this directory from
the Reflection Keen launcher, making it possible to play the game.

This is not an issue with versions of Android preceding 6.0. Either you grant
the app the permission to do so on installation (if required), or the app
isn't installed at all.

Beginning from Android v6.0, though, you don't have to grant this permission
on installation. You'll be asked to do so when you start the corresponding
Reflection Keen port, at least until you ask to permanently grant or deny
the permission. If it is granted, you can select a game directory as in
versions of Android preceding 6.0.

Otherwise, though, there are still app-specific directories
that may be accessed. Again assuming Keen Dreams for now, it's the
directory Android/data/net.duke4.ny.reflection_kdreams/files/user_gameinsts
within the shared storage.

You'll have to create this "user_gameinsts" directory (per app), though.

----------------------
Command line arguments
----------------------

Adding the single argument of -? should tell the details. For instance,
on Windows, in case reflection-kdreams.exe is chosen, the command
should look like this: "reflection-kdreams -?"

In case something goes wrong, these are the supported arguments common to
all games (i.e., all executables):

-gamever {VER}: Select game version supported by this executable.
-passorigargs {...}: Pass all following arguments to the original game port.
-datadir {...}: Specify an alternative path for game data (separated by ver.).
-cfgdir {...}: Specify an alternative path for new cfg files (not old CONFIG).
-fulllauncher: Show a fullscreen launcher window.
-softlauncher: Show a software-rendered launcher window (not fullscreen).

Te following arguments are specific to the Catacomb Adventure Series:

-skipintro: Skip what is found in the original intro EXE and start game.
-showslides: Show the electronic catalog or hint book.

Note: The path passed to -datadir or -cfgdir is assumed to exist.
