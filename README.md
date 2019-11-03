MumbleEchoSpatializer
=====================

Mumble spatial audio plugin for Echo VR.

Installation
------------
From the [Releases](https://github.com/qlyoung/MumbleEchoSpatializer/releases)
page, copy the 32- or 64-bit DLL - depending on which version of Mumble you
have installed - into `%AppData%/Mumble/Plugins`. Restart Mumble. Go to
Configure -> Settings -> Plugins and very that "Echo VR (latest)" appears in
the list. Check "Link to Game and Transmit Position".

This has only been tested on Mumble 1.3.0.

Usage
-----
Audio will be spatialized while a match is in progress and actively being
played. Launch countdown, post-score and other states will not have spatialized
audio.

Mumble settings for positional audio will need to be adjusted for best
experience. Go to Configure -> Settings -> Audio Output and check the
"Positional Audio" box. In the Positional Audio section make the following
changes:

- Check "Headphones"
- Minimum Distance = 1.0m
- Maximum Distance >= 80.0m
- Minimum Volume = 5%


Contributing
------------
Only unlicensed contributions are accepted.


Building
--------
Clone the project and import it with VS2015 (no other version will work). All
dependencies are included. Build in release mode only; Mumble does not expect
debugging symbols in its plugin DLLs and they will corrupt Mumble address
space.
