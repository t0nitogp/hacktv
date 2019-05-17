 -[ HackTV - Analogue TV transmitter for the HackRF ]-

This is a fork of https://github.com/fsphil/hacktv with some additional features
added. Most of them are those which I personally use, though not necessarily
warrant inclusions into original source.

I will try and keep this as up to date as possible with official releases.

2019-05-17:
Added Discret 11 scrambling in audience 7/free access mode. Use --d11 option.
  - Best results with sampling rate 10MHz or 20MHz (until I figure out how to delay properly)
Added PIC hex file for Syster and D11 when used in Syster decoder.

2019-04-30:
Holding screen for when position of the video is defined - can sometimes takes a few seconds.
Added xtea algo for Videocrypt 1. Use --key xtea option.
Added Funcard hex for Videocrypt 1.

2019-04-25:
Merged hardware support for Syster from original repository.

Changes and additions:
Default wss signalling has been changed to auto.
Control Word change for Videocrypt TAC mode - tested working on a real TAC card.
Widescreen/anamorphic videos are now appropriately letterboxed instead of being stretched vertically.

Extra options added:
  --position <value>  Set start position of video in minutes.
  --timestamp         Overlay video timestamp over video.
  --logo <path>       Overlay picture logo over video. Logos are kept in
                      resources/logos folder.

2019-04-07:
PAL FM deviation changed to 6000000 kHz and default signal level reduced to 0.8

Conditional Videocrypt mode requires --key sky|tac parameter. This specifies
which card you want to use to decode video. 

  sky = this works with an active Sky 11 series card (blue semi-oval logo) and 
        uses two fixed control words.
  tac = this works with an active The Adult Channel or Eurotica card and uses 
        randomly generated control words.

  Hex files for both modes are included in PIC directory to flash your own
  PIC16F84 based cards. Old pirate D2MAC cards based on this chip will work.
  These cards are also known as "Goldwafer" or "Multimac" - only PIC chip
  needs to be flashed. External EEPROM is not used.

**All credit for original code goes to author. Original README text is below.**

WHAT'S IT DO

Generates a PAL/NTSC video signal from a video file or test pattern.
Input is any file type or URL supported by ffmpeg.
Output can be to a file or directly to a HackRF.
Teletext support.
NICAM stereo audio.
Videocrypt I hardware support.
Partial Nagravision Syster hardware support.

WHAT'S IT NOT DO (yet)

There are no filters. Needed for proper audio and VSB modulation.
An optional notch filter for the colour subcarrier would be nice.
SECAM colour. More of my TVs support this than NTSC.
D-MAC / D2-MAC standards. Because SECAM isn't obscure enough.

WHAT IT WON'T DO

DVB or other pure digital signals.
Bring back Firefly. :(

REQUIREMENTS

Depends on libhackrf and various ffmpeg libraries.

* For Fedora (with rpmfusion)
yum install hackrf-devel ffmpeg-devel

* For Debian and related
apt-get update
apt-get install libhackrf-dev libavutil-dev libavdevice-dev libswresample-dev libswscale-dev libavformat-dev libavcodec-dev

INSTALL

make
make install

EXAMPLES

###### Generate a file containing a PAL baseband signal from a video
$ hacktv -o baseband.bin -m pal example.mkv

###### Transmit a test pattern on UHF channel 31 (PAL System I), 47dB TX gain
$ hacktv -f 551250000 -m i -g 47 test

###### Transmit a test pattern with teletext
$ hacktv -f 551250000 -m i -g 47 --teletext demo.tti test

###### Download and transmit teletext pages from the Teefax service
###### http://teastop.co.uk/teletext/
$ svn checkout http://teastop.plus.com/svn/teletext/ teefax

$ hacktv -f 551250000 -m i -g 47 --teletext teefax test

-Philip Heron <phil@sanslogic.co.uk>

