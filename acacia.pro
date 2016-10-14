# -------------------------------------------------------------------------------------------------
#
# ACACIA (Advanced content-adaptive compressor of images) is an image compression tool, which
# encodes previously not compressed images aiming to fit user requirements.
#
# This application provides both GUI and console interface for image compression with constraints.
# It uses Qt4 library as well as libjpeg-turbo and libwebp libraries for JPEG and WebP formats.
#
# Created July 2016.
#
# -------------------------------------------------------------------------------------------------


QT += core
QT += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = acacia

# this option is important only for Windows
# you may want to comment it if you plan to use program only in GUI mode
# it's enabled by default, but in Windows a command line window will appear alongside GUI
CONFIG  += console

TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    imagebox.cpp \
    featureextractor.cpp \
    optimizer.cpp \
    encoder.cpp

HEADERS += \
    mainwindow.h \
    imagebox.h \
    featureextractor.h \
    optimizer.h \
    jpegmodels.h \
    webpmodels.h \
    encoder.h

FORMS += \
    mainwindow.ui

# To allow constant class members and nullptr
QMAKE_CXXFLAGS += -std=c++11

# To enable AVX instructions
QMAKE_CXXFLAGS += -mavx -mavx2

# To make GCC unroll loops in the feature extraction functions
QMAKE_CXXFLAGS_RELEASE += -O3



# JPEG library path
# In Linux a system library can be used.
# If libjpeg is used, then comment USE_LIBJPEG_TURBO in encoder.cpp
# A custom path to JPEG library can be specified below:

# - for Windows
#INCLUDEPATH += "c:/SomeUserPath/Applications/Codecs/libjpeg-turbo-1.5.0-gcc64/include"
#QMAKE_LIBDIR += "c:/SomeUserPath/Applications/Codecs/libjpeg-turbo-1.5.0-gcc64/lib"

# - for custom Linux installation
#INCLUDEPATH += "/opt/libjpeg-turbo/include"
#QMAKE_LIBDIR += "/opt/libjpeg-turbo/lib64"

LIBS += -ljpeg

# Enable static linking for libjpeg. Disabled by default as it may cause conflicts with libjpeg integrated into Qt.
#QMAKE_LFLAGS += -static



# WebP library path:

# - for Windows
#INCLUDEPATH += "c:/SomeUserPath/Applications/Codecs/libwebp-0.5.0-windows-x64-no-wic/include"
#QMAKE_LIBDIR += "c:/SomeUserPath/Applications/Codecs/libwebp-0.5.0-windows-x64-no-wic/lib"

# - for Linux and OSX
#INCLUDEPATH += "/.../libwebp-0.5.0-mac-10.9/include"
#QMAKE_LIBDIR += "/.../libwebp-0.5.0-mac-10.9/lib/"

LIBS += -lwebp
