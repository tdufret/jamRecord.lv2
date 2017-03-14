<h1>jamRecord</h1>

<p>! not yet functional !

<p>a simple Lv2 capture plugin</p>
-    idea is to record on file only the last x minutes before the save button was hit.
-    using libsndfile <a href="http://www.mega-nerd.com/libsndfile/">http://www.mega-nerd.com/libsndfile/</a>
-    save audio stream as wav or ogg files to ~/jamRecord/lv2_sessionX.wav/ogg were X is replaced by numbers
-    No Gui, the host need to provide the UI.

<p>interface:</p>
-    REC button: initialize a recording circular buffer, and start recording
                 when buffer is full, new data replace the oldest ones 
-    SAVE button: store the circular buffer content in a file

<p>build:</p>
-    no build dependency check, just make
-    libsndfile is needed

<p>install:</p>
-    as user make install will install to ~./lv2/
-    as root make install will install to /usr/lib/lv2/
