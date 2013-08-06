SoftTH
======

Software TripleHead originally by Kegetys (www.kegetys.fi).  Updated and maintained by born2beflyin.

Licensed under GNU GPL v3 (see LICENSE)

SoftTH is a software-only triplehead solution (three monitors) for DirectX 8/9 games (and in the future DirectX 10/11 and OpenGL.  It uses DirectX DLL hooks to draw the scene entirely on the primary video card and copy the appropriate portions of the scene to the various monitors.  SoftTH can be configured to work with any combination of monitor sizes/resolutions.

From Kegetys on December 09, 2012: (http://www.kegetys.fi/forum/index.php?topic=3176.0)

"I wanted to write some notes of the source but I havent found the time to do it so it is here in a bit raw form. Its a VS 2005 project, see readme for requirements. The configurations are a bit broken, you need to select the matching configuration for the architecture (Release for Win32 and Release x64 for x64).

It is looking quite likely I wont be developing SoftTH much further personally at least in major ways, so hopefully the source will find a new home. Feel free to upload it to github or similar services for easier collaboration.

As for the reasons why development has been almost at a halt is mostly lack of time and interest - It has been over 6 years since the start of the project now and the "I wonder if it would be possible" has been answered. SoftTH's concept is proven but supporting it is quite labor intensive: New games come out which do unexpected things, and new Direct3D versions come and go. Also since I have done professional software development for a few years now I tend to want to find something else to do on my free time. Trying new experimental things and making something nobody else has done before is still as interesting as ever but I think SoftTH is mostly past that stage.

For a bit of history the birth of SoftTH happend in the Live for Speed forums here and here. The Matrox Triplehead2go gave me the original idea of rendering on a single card and copying the side monitor images to other video cards as I thought it should be possible to do what the th2go did with software. Back then I only had an AGP+PCI system and I knew from the numbers that its not going to work - PCI Express would be needed but motherboards with two PCI Express slots were very expensive back then. Regardless I decided to skimp on other components when doing a system upgrade and got myself a dual slot motherboard so I could test the concept. Turns out it worked quite well so the money was well spent.

I originally kind of hoped that major video card manufacturers would pick the triple monitor feature up and implement it without needing the likes of SoftTH. To some extent it has happened but still not well enough in my opinion, very obvious and easy features are still missing. Hopefully they'll get there eventually. :)"
