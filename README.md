
  libzvbi - VBI capture and decoding library
  ==========================================

  The zvbi/libzvbi repository was moved here from [Sourceforge](https://sourceforge.net/projects/zapping) in September 2022 for current development. The Zapping TV viewer (and other projects from the Sourceforge) have also been [archived in github](https://github.com/zapping-vbi/vbi-archive).



  Introduction
  ============

  The vertical blanking interval (VBI) is an interval in an analog
  television signal that temporarily suspends transmission of the
  signal for the electron gun to move back up to the first line of the
  television screen to trace the next screen field. The vertical
  blanking interval can be used to carry data, since anything sent
  during the VBI would naturally not be displayed; various test
  signals, closed captioning, and other digital data can be sent
  during this time period.

  The ZVBI library provides functions to read from Linux V4L, V4L2 and
  FreeBSD BKTR raw VBI capture devices, from Linux DVB devices and
  from a VBI proxy to share V4L and V4L2 VBI devices between multiple
  applications.

  It can demodulate raw to sliced VBI data in software, with support
  for a wide range of formats, has functions to decode several popular
  services including Teletext and Closed Caption, a Teletext cache
  with search function, various text export and rendering functions.

  Basically ZVBI offers all functions needed by VBI applications
  except for the user interface. The library was written for the
  [Zapping TV viewer](https://zapping.sourceforge.net).

  For details on using the library see the documentation in doc/html
  (built only if you have [Doxygen](https://www.doxygen.org/) installed)
  and the files in the test and examples directories.

  Download
  ========

  See Releases and Tags for the latest version of zvbi and libzvbi. The first release on this repo is 0.2.36. Previous tagged releases can be found [here](https://github.com/zapping-vbi/vbi-archive).

  Ileana Dumitrescu \<ileanadumitrescu95@gmail.com\> maintains the [Debian package](https://packages.debian.org/source/zvbi).

  Oliver Lehmann \<oliver@FreeBSD.org\> maintains a [FreeBSD port](https://www.freebsd.org/cgi/ports.cgi?query=libzvbi).

  For RPMs please see the repository of your distribution.


  Building and Installation
  =========================

  Obtain a local copy of this git repository by cloning:

    git clone https://github.com/zapping-vbi/zvbi.git
  
  This project uses GNU autotools. After cloning the repo, this command will create missing configuration files:
  
    cd zvbi && ./autogen.sh
  
  Next, to generate the Makefiles, run:

    ./configure
  
  Optionally, running `CONFIGURE=1 ./autogen.sh` will run the configure script immediately after the autogen script with a single command.
  
  After configuring, to compile and build, run:
  
    make
  
  If you want to run some tests, run:
  
    make check
  
  Then to install, run:
  
    make install

  The zvbi/libzvbi package produces the following binaries:
  
  * **zvbid** - proxy for VBI devices that forwards VBI data streams to one or more connected clients and manages channel change requests
  * **zvbi-atc-cc** - command-line utility that captures ATSC TV transmissions using a Linux DVB device and decodes the enclosed closed caption data
  * **zvbi-chains** - command-line wrapper that executes the VBI application while overloading several C library calls so that the application can be forced to access VBI devices via the VBI proxy instead of device files directly
  * **zvbi-ntsc-cc** - command-line utility for decoding and capturing closed captioning for NTSC and webtv
  * **libzvbi.[a, so, la]** - VBI decoding library
  
  If Doxygen is installed, `make` will also create library documentation.
  
  For additional instructions see the [INSTALL](https://github.com/zapping-vbi/zvbi/blob/main/INSTALL) file.

  The library has been tested on Linux and FreeBSD.

  Bindings
  ========

  Tom Zoerner wrote a Perl interface to libzvbi. The ZVBI Perl module
  covers all exported libzvbi functions. Most of the functions and
  parameters are exposed nearly identical, or with minor adaptions for
  the Perl idiom.

  Find out more at https://search.cpan.org/~TOMZO/Video-ZVBI

  Contributing
  ============

  Contributions are always welcome. Please feel free to write issues and patches, as well as create pull requests. When submitting pull requests, please create your own local branch first to merge into main.
  
  zvbi relevant bugs will be moved from [Sourceforge](https://sourceforge.net/p/zapping/bugs/) to github issues on this repo.

  zvbi patches from [Sourceforge](https://sourceforge.net/p/zapping/patches/) are being applied to the main branch.

  zvbi feature requests will also be moved from [Sourceforge](https://sourceforge.net/p/zapping/feature-requests/) to github issues on this repo.

  You can send an email to the maintainer directly with the tag \[zvbi\] in the subject line.

  You can browse the Sourceforge email list archives [here](http://sourceforge.net/mailarchive/forum.php?forum_name=zapping-misc).

  The current author and maintainer of this library is:

    Ileana Dumitrescu <ileanadumitrescu95@gmail.com>
  
  The previous author and maintainer of this library was:

    Michael H. Schimek <mschimek@users.sourceforge.net>

  Encrypted e-mail is welcome. You can find the authors' PGP/GPG keys in the
  AUTHORS file:

      gpg --import <AUTHORS


  Copyright and Licensing
  =======================

  Copyright (C) 2000-2008 Michael H. Schimek

  Copyright (C) 2000-2001 Iñaki García Etxebarria

  Copyright (C) 2003-2004, 2007 Tom Zoerner

  Copyright (C) 2022 Ileana Dumitrescu

  Originally based on AleVT 1.5.1 by Edgar Toernig

  The zvbi project is licensed under the [GNU General Public License version 2.0 (GPL-2+)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html). Other software licenses used throughout zvbi include:
  
  * [LGPL-2+](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
  * [MIT/Expat](https://opensource.org/licenses/MIT)
  * [BSD-2-Clause](https://opensource.org/licenses/BSD-2-Clause)
  * [BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)
  
  See [COPYING](https://github.com/zapping-vbi/zvbi/blob/main/COPYING.md) for full text of all software licenses.
