# ACACIA

ACACIA (Advanced Content-Adaptive Compressor of ImAges) is an image compression tool that aims to speed up encoding into JPEG and WebP formats with a given file size or quality level.

ACACIA is licensed under GPL3. It uses *Qt*, *libjpeg-turbo* (or *libjpeg*) and *libwebp* under their respective licenses.

Current version: 0.17.

<p>Contacts:</br>
- John Thomson - john.thomson@gmail.com</br>
- Oleksandr Murashko - om21@st-andrews.ac.uk</br>
</p>

This application has GUI and command line modes. If run without arguments it starts in GUI by default. Use "acacia --help" to see the list of command line options.

This version of the tool is designed to work with previously uncompressed images, obtained from a camera sensor, developed raw file, or from resizing a compressed image. If the supplied image has previously be compressed, it will still work, but might be less accurate. Support for previously compressed images is planned for later versions.

Alpha channel and images with high-contrast vector graphics are currently not supported.

To compile and run ACACIA you need Qt library (mainly for GUI) as well as libjpeg and libwebp. This version was tested with the following external libraries: qt-4.8.7, libjpeg-turbo-1.4.2, libwebp-0.4.3. Also this program uses AVX and AVX2 vector instructions and requires a respective CPU to run.

-----

To compile in Windows, Linux or OSX using Qt Creator:

1. Install Qt developer tools with Qt Creator.
2. Install external image compression libraries: libjpeg-turbo (may already be installed in Ubuntu) and libwebp.
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
