MumbleEchoSpatializer
=====================

Mumble spatial audio plugin for Echo VR.

Installation
------------
Copy the 32- or 64-bit DLL into `%AppData%/Mumble/Plugins`. Restart Mumble. Go
to Configure -> Settings -> Plugins and very that "Echo VR (latest)" appears in
the list.

Usage
-----
Audio will be spatialized while a match is in progress and actively being
played. Launch countdown, post-score and other states will not have spatialized
audio.


Contributing
------------
Only unlicensed contributions are accepted.


Building
--------
Clone the project and import it with VS2015 (no other version will work). All
dependencies are included. Build in release mode only; Mumble does not expect
debugging symbols in its plugin DLLs and they will corrupt Mumble address
space.
