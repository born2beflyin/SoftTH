SoftTH
======

Software TripleHead originally by Keijo "Kegetys" Ruotsalainen, http://www.kegetys.fi.

Updated and maintained by born2beflyin, http://www.softth.net

Licensed under GNU GPL v3 (see LICENSE).

SoftTH is an open source software triplehead/multihead gaming solution for the PC. It performs the same job as AMD Eyefinity, nVidia Surround, and Matrox2go, only it supports any number and/or combination of non-identical or identical screens at any resolution. No special hardware is needed. You simply need your monitors attached to PCI Express video cards. Only one card does all the rendering work, so the secondary+ card(s) can be low end; only Direct3D support is required.

SoftTH works by rendering the whole scene on one display adapter, which then gets split into multiple parts to be displayed on each monitor. While it is technically possible to use it on an AGP/PCI card, only PCI Express has the required bandwidth to get good framerates.

======

(As of Jan. 2014)

Compiles with Code::Blocks 13.12: http://www.codeblocks.org using the Microsoft VC++ 2010 compiler.

Requires:
(Static libs included with SoftTH)
- distorm disassembler library: http://code.google.com/p/distorm/ (Currently linking to distorm v3.3)
- zlib compression library: http://www.zlib.net/ (Currently linking to v1.2.7)
(NOT included with SoftTH)
- DirectX SDK June 2010: http://www.microsoft.com/en-us/download/details.aspx?id=6812
- Windows SDK v7.1: http://www.microsoft.com/en-us/download/details.aspx?id=8279
(You will not need the following if you have MS Visual Studio 2010 Pro, but you will have to modify the include/lib search folders)
- Windows DDK v7.1.0: http://www.microsoft.com/en-us/download/details.aspx?id=11800

Modify the project include/lib search directories accordingly.

If you get a link error about "failure during conversion to COFF," it may be related to the issue here:
http://stackoverflow.com/questions/10888391/error-link-fatal-error-lnk1123-failure-during-conversion-to-coff-file-inval

If a project target isn't yet in the list, i.e. "Release DX11 Win x64", it's in the works.  It may take a while.

SoftTH contains incomplete code for Direct3D 10 (DXGI) and Direct3D 10.1 support. For Direct3D 11, most functionality is missing, but is under development.

======

From Kegetys on December 09, 2012 - the initial source code release: (http://www.kegetys.fi/forum/index.php?topic=3176.0)

"I wanted to write some notes of the source but I havent found the time to do it so it is here in a bit raw form. Its a VS 2005 project, see readme for requirements. The configurations are a bit broken, you need to select the matching configuration for the architecture (Release for Win32 and Release x64 for x64).

It is looking quite likely I wont be developing SoftTH much further personally at least in major ways, so hopefully the source will find a new home. Feel free to upload it to github or similar services for easier collaboration.

As for the reasons why development has been almost at a halt is mostly lack of time and interest - It has been over 6 years since the start of the project now and the "I wonder if it would be possible" has been answered. SoftTH's concept is proven but supporting it is quite labor intensive: New games come out which do unexpected things, and new Direct3D versions come and go. Also since I have done professional software development for a few years now I tend to want to find something else to do on my free time. Trying new experimental things and making something nobody else has done before is still as interesting as ever but I think SoftTH is mostly past that stage.

For a bit of history the birth of SoftTH happend in the Live for Speed forums here and here. The Matrox Triplehead2go gave me the original idea of rendering on a single card and copying the side monitor images to other video cards as I thought it should be possible to do what the th2go did with software. Back then I only had an AGP+PCI system and I knew from the numbers that its not going to work - PCI Express would be needed but motherboards with two PCI Express slots were very expensive back then. Regardless I decided to skimp on other components when doing a system upgrade and got myself a dual slot motherboard so I could test the concept. Turns out it worked quite well so the money was well spent.

I originally kind of hoped that major video card manufacturers would pick the triple monitor feature up and implement it without needing the likes of SoftTH. To some extent it has happened but still not well enough in my opinion, very obvious and easy features are still missing. Hopefully they'll get there eventually. :)"
