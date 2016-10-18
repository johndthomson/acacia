# ACACIA

[![screenshot](https://github.com/johndthomson/acacia/blob/gh-pages/screenshot.png)](#ACACIA)


ACACIA (Advanced Content-Adaptive Compressor of ImAges) is an image compression tool which allows the users to target specific image quality or file size metrics when compressing an image with JPEG or WebP, with only minimal additional compression time. It does this by using machine learning to predict how an individual image will be compressed, and adjusts the aggressiveness of compression accordingly. 

ACACIA allows users to target compression to their file size or quality needs, significantly increasing the effectiveness of compression by adjusting to each individual image. It is available with a graphical interface, and with a CLI for batch processing.

ACACIA is licensed under GPL3. It uses *Qt*, *libjpeg-turbo* (or *libjpeg*) and *libwebp* under their respective licenses.

Current version: 0.17.

ACACIA is free, open source software. An important aspect of all research is measuring and understanding the impact of research. If you use ACACIA or even just like it, please let us know at j.thomson@st-andrews.ac.uk. This helps us greatly in funding future research.

ACACIA is not production software. Its primary purpose is to demonstrate research techniques.

If you would like to integrate ACACIA into your project (open source or otherwise) and would like to discuss a different licence, please get in touch.

If you use ACACIA in your research, please cite our paper describing the techniques used:

Predicting and Optimizing Image Compression, In the Proceedings of the 2016 ACM Conference on Multimedia, Pages 665-669
http://dl.acm.org/citation.cfm?doid=2964284.2967305

A preprint of the paper is available at http://jt.host.cs.st-andrews.ac.uk/

<p>Contacts:</br>
- John Thomson - j.thomson@st-andrews.ac.uk </br>
- Oleksandr Murashko - om21@st-andrews.ac.uk </br>
</p>

This application has GUI and command line modes. If run without arguments it starts in GUI by default. Use "acacia --help" to see the list of command line options.

Note: Windows version GUI doesn't scale properly on 4k screens just now. 

This version of the tool is designed to work with previously uncompressed images, obtained from a camera sensor, developed raw file, or from resizing a compressed image. If the supplied image has previously be compressed, it will still work, but might be less accurate. Support for previously compressed images is planned for later versions.

A test set of images can be downloaded at: http://om21.host.cs.st-andrews.ac.uk/60_test_images_CC0.zip
They are licensed under Creative Commons Zero - we are grateful to those generated them.

Alpha channel and images with high-contrast vector graphics are currently not supported.

To compile and run ACACIA you need Qt library (mainly for GUI) as well as libjpeg and libwebp. This version was tested with the following external libraries: qt-4.8.7, libjpeg-turbo-1.5.0, libwebp-0.5.0. Also this program uses AVX and AVX2 vector instructions and requires a respective CPU to run.

-----

To compile in Windows, Linux or OSX using Qt Creator:

1. Install Qt developer tools with Qt Creator.
2. Install external image compression libraries: [libjpeg-turbo](https://sourceforge.net/projects/libjpeg-turbo/files/) (may already be installed in your distribution) and [libwebp](https://developers.google.com/speed/webp/download).
3. Open project file "acacia.pro" in Qt Creator.
4. Change paths to image libraries in "acacia.pro" file according to your OS.
5. Build project.
6. Make sure all necessary external dynamic libraries can be located by application when it starts.

-----

To build in Windows you can also use Visual Studio with installed "Qt Addin" plugin. You may need to copy some external libraries (like Qt and libjpeg dll's) alongside the executable to run it - if they are not present in the Windows PATH.

-----

To build in Linux without Qt Creator go to the project directory, change paths to image libraries in "acacia.pro" file and execute:
```
qmake acacia.pro
make
```
[![Analytics](https://ga-beacon.appspot.com/UA-85883244-1/githubhome)](https://github.com/johndthomson/acacia/)



