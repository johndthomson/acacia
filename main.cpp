/*
    Copyright 2016 Oleksandr Murashko, John Thomson.
    This file is part of ACACIA Image Processing Tool.

    ACACIA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ACACIA is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ACACIA. If not, see <http://www.gnu.org/licenses/>.
*/


#include <QApplication>
#include <QDesktopWidget>
#include <QDateTime>
#include <QFile>
#include <QImage>
#include <QTextStream>

#include "math.h"

#include "mainwindow.h"
#include "featureextractor.h"
#include "optimizer.h"
#include "encoder.h"

int main(int argc, char *argv[])
{
    // set program version
    const QString version = "0.17";

    // run GUI version if no arguments
    if (argc == 1)
    {
        // for GUI version we need QApplication, not QCoreApplication
        QApplication app(argc, argv);
        MainWindow w;
        w.setFixedSize(w.size());
        w.move(QApplication::desktop()->screenGeometry().center() - w.rect().center());
        w.setWindowTitle("ACACIA v" + version);
        w.show();
        return app.exec();
    }

    // start console version otherwise
    const unsigned long long int appStartTime = QDateTime::currentMSecsSinceEpoch();
    const QString appName = "acacia";
    const QString msgPref = "[" + appName + "] ";

    // prepare for parsing arguments
    // note that QCoreApplication object is used only for argument parsing (no event loops in this console version)
    const QStringList arguments = QCoreApplication(argc, argv).arguments();

    // list of input parameters
    bool    isjpeg         = true;
    int     targetFileSize = -1;
    double  targetYMSSIM   = -1;
    double  targetYPSNR    = -1;
    QString inFileName;
    QString outFileName;
    bool    silent         = false;

    // argument presence flags
    bool formatOk      = 0;
    int  sizeOk        = 0;
    int  mssimOk       = 0;
    int  psnrOk        = 0;
    bool inFileNameOk  = false;
    bool outFileNameOk = false;

    // loop over the app's arguments
    for (int i = 1;  i < arguments.size();  i++)
    {
        QString currentArgument = arguments.at(i);

        if (currentArgument == "-h" || currentArgument == "--help") {
            QTextStream(stdout, QIODevice::WriteOnly) << "ACACIA image compression tool, version " << version << ".\n"
                                                      << "Program will run in GUI mode if no arguments specified.\n"
                                                      << "Command line usage: " << appName << " -jpeg|-webp -size|-mssim|-psnr <target_value> -i <input_image> -o <output_image> [-silent]\n"
                                                      << "Options:\n"
                                                      << "  -h, --help        this information;\n"
                                                      << "  -jpeg             compress to JPEG format;\n"
                                                      << "  -webp             compress to WebP format;\n"
                                                      << "  -size <value>     target file size in bytes;\n"
                                                      << "  -mssim <value>    target Y-MSSIM (mean SSIM for luminance channel);\n"
                                                      << "  -psnr <value>     target Y-PSNR;\n"
                                                      << "  -i <path>         path to input image;\n"
                                                      << "  -o <path>         path to compressed image;\n"
                                                      << "  -silent           do not print anything to stdout and disable quality comparison.\n";
            return 0;
        } else if (currentArgument == "-jpeg") {
            isjpeg = true;
            formatOk = true;
        } else if (currentArgument == "-webp") {
            isjpeg = false;
            formatOk = true;
        } else if (currentArgument == "-size") {
            i++;
            if (i == argc) {
                QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: missing target file size\n";
                return -1;
            }
            bool ok;
            targetFileSize = arguments.at(i).toInt(&ok);
            if (!ok || targetFileSize < 0) {
                QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: invalid target file size\n";
                return -1;
            }
            sizeOk = 1;    // true
        } else if (currentArgument == "-mssim") {
            i++;
            if (i == argc) {
                QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: missing target Y-MSSIM value\n";
                return -1;
            }
            bool ok;
            targetYMSSIM = arguments.at(i).toDouble(&ok);
            if (!ok || targetYMSSIM > 1.0 || targetYMSSIM < -1.0) {
                QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: invalid target Y-MSSIM value\n";
                return -1;
            }
            mssimOk = 1;    // true
        } else if (currentArgument == "-psnr") {
            i++;
            if (i == argc) {
                QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: missing target Y-PSNR value\n";
                return -1;
            }
            bool ok;
            targetYMSSIM = arguments.at(i).toDouble(&ok);
            if (!ok || targetYPSNR < 0) {
                QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: invalid target Y-PSNR value\n";
                return -1;
            }
            psnrOk = 1;   // true
        } else if (currentArgument == "-i") {
            i++;
            if (i == argc) {
                QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: missing input image\n";
                return -1;
            }
            inFileName = arguments.at(i);
            inFileNameOk = true;
        } else if (currentArgument == "-o") {
            i++;
            if (i == argc) {
                QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: missing output image\n";
                return -1;
            }
            outFileName = arguments.at(i);
            outFileNameOk = true;
        } else if (currentArgument == "-silent") {
            silent = true;
        } else {
            QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: unknown option \"" + currentArgument + "\"\n";
            return -1;
        }
    }

    // now checking if all necessary parameters were supplied as arguments
    if (!formatOk) {
        QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: missing target image format\n";
        return -1;
    }
    if (!sizeOk && !mssimOk && !psnrOk) {
        QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: missing target restriction\n";
        return -1;
    }
    if ((sizeOk + mssimOk + psnrOk) > 1) {
        QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: only one target restriction can be used\n";
        return -1;
    }
    if (!inFileNameOk) {
        QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: missing input image\n";
        return -1;
    }
    if (!outFileNameOk) {
        QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: missing output image\n";
        return -1;
    }

    // arguments are checked, time to open input image
    QImage inputImage(inFileName);
    if (inputImage.isNull()) {
        QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: can't open input image\n";
        return -1;
    }

    // open file for saving compressed image
    QFile encodedImage(outFileName);
    if (!encodedImage.open(QFile::WriteOnly)) {
        QTextStream(stderr, QIODevice::WriteOnly) << msgPref << "error: can't open output file for writing\n";
        return -1;
    }

    // --------------------------------------------------------------------------------------------
    // we don't perform error checks beyond this point to simplify program
    // --------------------------------------------------------------------------------------------

    // check if image is not 24 bpp, e.g. grayscale, and convert it to 24 bpp
    if (inputImage.format() != QImage::Format_RGB32) inputImage = inputImage.convertToFormat(QImage::Format_RGB32);

    // get uncompressed image data (each pixel has format xBGR)
    const int w = inputImage.width();
    const int h = inputImage.height();
    const unsigned char *inputImageData = inputImage.constBits();

    // now image is located in memory, so we measure time from this point
    // there are 3 main stages to compress image: feature extraction, choosing optimal quality factor and actual compression

    // stage 1 - feature extraction
    const unsigned long long int featureExtractionStartTime = QDateTime::currentMSecsSinceEpoch();

    // prepare MLP input vector, which consists of 10 content features, image size and quality factor (in this order); 12 elements in total
    const int inputVectorSize = 12;
    double inputVector [inputVectorSize];

    // actual function that calculated 10 image features from uncompressed data
    FeatureExtractor::calculateFeatures((const unsigned int *) inputImageData, w, h, inputVector);

    // calculate 11-th input (image size)
    inputVector[10] = log(w * h / 1000000.0);

    // stage 2 - search for optimal parameters (quality factor)
    const unsigned long long int optimizationStartTime = QDateTime::currentMSecsSinceEpoch();

    // we need to chose optimal QF - last 12-th input, which gives us the closest prediction to the target value
    // firstly, we perform multiplexing of the objective type and target value
    const char targetObjective = sizeOk ? 's'            : (mssimOk ? 'm'          : 'p');
    const double targetValue   = sizeOk ? targetFileSize : (mssimOk ? targetYMSSIM : targetYPSNR);

    // secondly, we call a universal function that performs search using a repective regression model
    const int qualityFactor = Optimizer::findQualityFactor(isjpeg, targetObjective, targetValue, inputVector);

    // stage 3 - compression
    const unsigned long long int compressionStartTime = QDateTime::currentMSecsSinceEpoch();

    // call respective function depending on a target image format
    unsigned long long int compressedBufferSize = 0;
    const unsigned char *compressedImageBuffer = isjpeg ? Encoder::compressToJpeg(inputImageData, w, h, qualityFactor, &compressedBufferSize) :
                                                          Encoder::compressToWebp(inputImageData, w, h, qualityFactor, &compressedBufferSize);

    // save compressed image to file
    encodedImage.write((const char *) compressedImageBuffer, compressedBufferSize);
    encodedImage.close();
    delete [] compressedImageBuffer;

    // compression finished
    const unsigned long long int finishTime = QDateTime::currentMSecsSinceEpoch();

    // print some information
    if (!silent) {
        QTextStream(stdout, QIODevice::WriteOnly) << msgPref << "image reading time: "          << (featureExtractionStartTime - appStartTime)          << " ms\n"
                                                  << msgPref << "feature extraction time: "     << (optimizationStartTime - featureExtractionStartTime) << " ms\n"
                                                  << msgPref << "parameter optimization time: " << (compressionStartTime - optimizationStartTime)       << " ms\n"
                                                  << msgPref << "actual compression time: "     << (finishTime - compressionStartTime)                  << " ms\n"
                                                  << msgPref << "compressed size: "             << compressedBufferSize << " bytes\n"
                                                  << msgPref << "quality factor used: "         << qualityFactor << '\n';
    }

    return 0;
}
