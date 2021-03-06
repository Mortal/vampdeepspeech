VAMP plugin to run DeepSpeech
=============================

Mozilla's DeepSpeech project enables free and open source developers to incorporate high-quality voice recognition into software.

This project is a VAMP plugin suitable for loading into Audacity
that runs DeepSpeech on a selected range of audio.

Prerequisites
-------------

* The VAMP SDK in `/usr/include/vamp-sdk`.

* DeepSpeech C++ library files `lib{deepspeech,deepspeech_utils,tensorflow_cc,tensorflow_framework}.so`
  extracted into a subdirectory named `deepspeech-native-client` of this repository checkout.

* DeepSpeech `native_client` header file `deepspeech.h`
  extracted into a subdirectory named `DeepSpeech/native_client` of this repository checkout.

* The DeepSpeech model files
  (`output_graph.pb`, `alphabet.txt`, `lm.binary`, `trie`)
  in a subdirectory named `models` of this repository checkout.

Installation
------------

* Run `make` to build `vampdeepspeech.so`

* Run `make install` to copy `vampdeepspeech.so` into the directory `~/vamp`
  and remove Audacity's plugin registry.

* In Audacity, go to *Analyze* → *Add / Remove Plug-ins...*
  and enable *DeepSpeech* in the list.

* Convert your audio track to 16 KHz
  by setting the Project Rate to 16000
  (lower left corner of main Audacity window)
  and converting the track in *Tracks* → *Resample...*.

TODO
----

This is very pre-alpha quality.

* Customizable silence detection

* Auto resample non-16KHz input

* Check that model files exist and give proper error message if they don't
