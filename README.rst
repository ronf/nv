Network Video tool
==================

This is an archive of the source code and binaries of one of the very
first videoconferencing tools available on the Internet. It was primarily
written by Ron Frederick, but also includes contributions from a number
of others to adapt it to support various video capture devices and
compression algorithms. See the individual source files for more detailed
author information.

Copyright and License
---------------------

The majority of this code is copyright Xerox Corporation and is released
until the following license:

    Copyright (c) Xerox Corporation 1992. All rights reserved.

    License is granted to copy, to use, and to make and to use derivative
    works for research and evaluation purposes, provided that Xerox is
    acknowledged in all documentation pertaining to any such copy or derivative
    work. Xerox grants no other licenses expressed or implied. The Xerox trade
    name should not be used in any advertising without its written permission.

    XEROX CORPORATION MAKES NO REPRESENTATIONS CONCERNING EITHER THE
    MERCHANTABILITY OF THIS SOFTWARE OR THE SUITABILITY OF THIS SOFTWARE
    FOR ANY PARTICULAR PURPOSE.  The software is provided "as is" without
    express or implied warranty of any kind.

    These notices must be retained in any copies of any part of this software.

Some individual source files are copyright Canon Information Systems, Digital
Equipment Corporation, Sun Microsystems, the Naval Research Laboratory, and
the University of Southern California. See the individual source files for
full copyright and license information.

Background (by Ron Frederick)
-----------------------------

In October of 1992, I began to experiment with the Sun VideoPix frame grabber
card, with the idea of writing a network videoconferencing tool based upon IP
multicast. It would be modeled after "vat" -- an audioconferencing tool
developed at LBL, in that it would use a similar lightweight session protocol
for users joining into conferences, where you simply sent data to a particular
multicast group and watched that group for any traffic from other group
members.

In order for the program to really be successful, it needed to compress the
video data before putting it out on the network. A goal I chose was to make
an acceptable looking stream of data that would fit in about 128kbps, or the
bandwidth available on a standard home ISDN line. I also hoped to produce
something that was still watchable that fit in half this bandwidth. This
meant I needed approximately a factor of 20 in compression for the particular
image size and frame rate I was working with. I was able to achieve this
compression and filed for a patent on the techniques I used, later granted
as patent `US5485212A`__: Software video compression for teleconferencing.

__ https://patents.google.com/patent/US5485212A

At the beginning of November, I released the videoconferencing tool "nv" (in
binary form) to the Internet community. After some initial testing, it was used
to videocast parts of the November Internet Engineering Task Force all around
the world. Approximately 200 subnets in 15 countries were capable of
receiving this broadcast, and approximately 50-100 people received video using
"nv" at some point in the week.

Over the next couple of months, three other workshops and some smaller
meetings have used "nv" to broadcast to the Internet at large, including the
Australian NetWorkshop, the MCNC Packet Audio and Video workshop, and the
MultiG workshop on distributed virtual realities in Sweden.

A source code release of "nv" followed in February of 1993, and in March I
released a new version of the tool where I introduced a new wavelet-based
compression scheme. In May of 1993, I added support for color video.

The network protocol used for "nv" and other Internet conferencing tools
became the basis of the Realtime Transport Protocol (RTP), standardized
through the Internet Engineering Task Force (IETF), first published in
RFCs `1889`__-`1890`__ and later revised in RFCs `3550`__-`3551`__, along
with various other RFCs that covered profiles for carrying specific
formats of audio and video.

__ https://tools.ietf.org/html/rfc1889
__ https://tools.ietf.org/html/rfc1890
__ https://tools.ietf.org/html/rfc3550
__ https://tools.ietf.org/html/rfc3551

Over the next couple of years, work contined on "nv", porting the tool
to a number of additional hardware platforms and video capture devices.
It continued to be used as one of the primary tools for broadcasting
conferences on the Internet at the time, including being selected by NASA
to broadcast live coverage of shuttle missions online.

In 1994, I added support in "nv" for supporting video compression algorithms 
developed by others, including some hardware compression schemes such as
the CellB format supported by the SunVideo video capture card. This also
allowed "nv" to send video in CUSeeMe format, to send video to users
running CUSeeMe on Macs and PCs.

The last publicly released version of "nv" was version 3.3beta, released
in July of 1994. I was working on a "4.0alpha" release that was intended
to migrate "nv" over to version 2 of the RTP protocol, but this work was
never completed. A copy of the 4.0 alpha code is included in this archive
for completeness, but it is unfinished and there are known issues with it,
particularly in the incomplete RTPv2 support.

The framework provided in "nv" later went on to become the basis of video
conferencing in the "Jupiter multi-media MOO" project at Xerox PARC, which
eventually became the basis for a spin-off company "PlaceWare", later
acquired by Microsoft. It was also used as the basis for a number of
hardware video conferencing projects that allowed sending of full NTSC
broadcast quality video over high-bandwidth Ethernet and ATM networks.
I also later used some of this code as the basis for "Mediastore", which
was a network-based video recording and playback service.
