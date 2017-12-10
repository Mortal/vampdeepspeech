VAMP plugin to run DeepSpeech
=============================

Mozilla's DeepSpeech project enables free and open source developers to incorporate high-quality voice recognition into software.

This project is a VAMP plugin suitable for loading into Audacity
that runs DeepSpeech on a selected range of audio.

Prerequisites
-------------

* The VAMP SDK in `/usr/include/vamp-sdk`.

* The DeepSpeech Python bindings

* The DeepSpeech model files
  (`output_graph.pb`, `alphabet.txt`, `lm.binary`, `trie`)
  in a subdirectory of this repository named `models`.

Installation
------------

* Run `make` to build `vampdeepspeech.so`

* Run `make install` to copy this into `~/vamp`
  and remove Audacity's plugin registry.

* In Audacity, go to *Analyze* → *Add / Remove Plug-ins...*
  and enable *DeepSpeech* in the list.

* Convert your audio track to 16 KHz
  by setting the Project Rate to 16000
  (lower left corner of main Audacity window)
  and converting the track in *Tracks* → *Resample...*.
