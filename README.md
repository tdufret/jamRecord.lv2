# jamRecord

! not yet functional !

A simple Lv2 capture plugin

- idea is to record on file only the last x minutes before the save button was hit.
- using libsndfile [http://www.mega-nerd.com/libsndfile/](http://www.mega-nerd.com/libsndfile/)
- save audio stream as wav or ogg files to ~/jamRecord/lv2_sessionX.wav/ogg were X is replaced by numbers
- No Gui, the host need to provide the UI.

## Interface:

- REC button: initialize a recording circular buffer, and start recording. When buffer is full, new data replace the oldest ones. 
- SAVE button: store the circular buffer content in a file

## Build:

- no build dependency check, just make
- libsndfile is needed

## Install:

- as user make install will install to ~./lv2/
- as root make install will install to /usr/lib/lv2/

