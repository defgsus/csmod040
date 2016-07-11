## CS Modular Editor

This is a fork of the CymaSonics Modular Editor v0.2.3

It's a modular audio-visual framework for all kinds of DSP, spatial audio and some basic drawing functions. It's mainly inspired by NI's Reaktor but uses a much simpler model internally. The only signal type is 'one float per sample' and each module introduces one sample of delay at minimum. You can wire up nodes in any way and create feedback-loops as you wish. Events are usually rising signals so you can have 22050 gates at a 44100 sampling rate. You can find a detailed overview of the CSMOD workings here: 

http://www.modular-audio-graphics.com/project/programming/cs-modular.html

The code is age-old (2010/11) and my first real C++ project. So it's quite terrible, beware! Almost everything i would do differently today. However, a few people exist on this planet that still use it for creative purposes and this repo is long overdue, i think. I'd be interested in re-doing the whole thing in a proper way, given enough time but alas.. 

### deps

CSMOD is cross-platform and mainly depends on FLTK, portaudio, fftw3.
This repo provides a .pro file for building with Qt (Qt is only used as IDE and for building)

### license
I've licensed the code under the GPL. It does not really matter, i guess, because it's really terrible code. 
