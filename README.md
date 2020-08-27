MumbleEchoSpatializer
=====================

Mumble spatial audio plugin for Echo VR. Using Mumble and this plugin allows
you to have private comms in Echo Arena while still hearing your teammates
relative to their position in the game, like you do when using the in-game VoIP.

Tested on Mumble 1.3.0.

Installation
------------
From the [Releases](https://github.com/qlyoung/MumbleEchoSpatializer/releases)
page, download both .dll files. Open an Explorer window and copy paste the
following into the address bar:
```
%AppData%/Mumble/Plugins
```
Press enter and you should be in an empty directory. Copy the .dll files into
this directory, then restart Mumble. In Mumble, go to Configure -> Settings ->
Plugins and verify that "Echo VR" appears in the list. Check "Link to Game and
Transmit Position" under "Options".  Go to "Audio Output" and check "Positional
Audio".

Make sure to set "Enable API Access" in the in-game settings menu to true (it
defaults to false), otherwise you will not have spatial audio.

Usage
-----
Audio will be spatialized while you are in a match with others using the plugin.
The lobby and other game areas (menus, tutorials etc) are not spatialized.

Mumble settings for positional audio will need to be adjusted for best
experience. Go to Configure -> Settings -> Audio Output and check the
"Positional Audio" box. In the Positional Audio section make the following
changes:

- Check "Headphones"
- Minimum Distance = 1.0m
- Maximum Distance >= 80.0m
- Minimum Volume <= 20%

You can adjust these to your preferences, but these are good defaults to start
from.

Once you have made these changes, join a Mumble channel with your teammates,
and join a match together in Echo Arena. You should be able to hear your
teammates' voices relative to their position in the game.

Troubleshooting
---------------
- Verify that Mumble is running
- Verify that Echo VR is running
- Verify that you have set "Enable API Access" to true in the in-game settings
- Join a match

In Mumble settings, under "Audio Output":
- Verify that the device is set to your Rift headphones
- Verify that the "Positional Audio" checkbox is [x] checked
- Under "Positional Audio", verify that the "Headphones" checkbox is [x]
  checked

In Mumble settings, under "Plugins":
- Under "Options", verify that the "Link to Game and Transmit Position"
  checkbox is [x] checked
- Under "Plugins", verify that "Echo VR" is present in the list
  - If it is not, you probably installed the wrong plugin version (32 or 64
    bit), or the plugin is missing; reinstall it
- Under "Plugins", verify that the checkbox next to "Echo VR" is [x] checked

Once you have checked all of the above:
- In the Mumble status pane (on the left), wait 30 seconds and ensure "Echo VR
  linked" appears. If it does not, start over from the beginning of this list.

If you are in the same match with someone and can't hear positional audio from
them, make sure you are both using the same version of the plugin. The version
number appears next to the plugin name in Settings -> Plugins.

If *none* of the above work:
- **While in a match**, go to http://localhost:6721/session in your browser, and
  verify that you get a text blob (page does not time out). If it is timing
  out, you either have a misconfigured firewall or something else on your computer
  is bound to port 6721 (unlikely). To check:
  - Open an administrator command prompt - Win+R, type "cmd", Ctrl+Shift+Enter
  - Run this command: `netstat -b -p TCP`
  - Look for "6721". If the program name in `[brackets]` is not `echovr.exe`, you
    need to close that program and restart Echo VR. After doing that:
  - Try going to http://localhost:6721/session in your browser again once you are
    in a match; if you get a text blob, the plugin should work.


Contributing
------------
Patches are welcome. Please use the following guidelines:
- Patches should use the line endings and style of the changed file
- Contributions must be public-domain, as specified in the
  [Unlicense](https://unlicense.org)


Building
--------
Clone the project and import it with VS2015 (no other version will work). All
dependencies are included. Build in release mode only; Mumble does not expect
debugging symbols in its plugin DLLs and they will corrupt Mumble address
space.
