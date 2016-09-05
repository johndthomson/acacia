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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

#include "math.h"

#include "featureextractor.h"
#include "optimizer.h"
#include "encoder.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    imageBox = new ImageBox(ui->centralWidget);
    imageBox->setGeometry(QRect(10, 40, 530, 300));

    labelsList = new QLabel* [6];
    labelsList[0] = ui->labelSize_1;
    labelsList[1] = ui->labelSize_2;
    labelsList[2] = ui->labelSize_3;
    labelsList[3] = ui->labelSize_4;
    labelsList[4] = ui->labelSize_5;
    labelsList[5] = ui->labelSize_6;

    inputImage = QImage();
    isjpeg = ui->radioButtonJPEG->isChecked();
    numPositions = 1024;
    ui->sliderMSSIM->setMaximum(numPositions);
    ui->sliderFileSize->setMaximum(numPositions);

    const int inputVectorSize = 12;
    inputVector = new double [inputVectorSize];  // 10 features + megapixels + qf

    lnMinSize = 0;
    lnMaxSize = 0;

    // flags indicating that spinBox or slider update is triggered by changes in another control
    autoUpdateFlag0 = false;
    autoUpdateFlag1 = false;
    autoUpdateFlag2 = false;

    minDisplayMSSIM = 0.6;    // anything smaller is clipped to this value
    minImageMSSIM = 0;        // minimal estimated MSSIM for current image
}

MainWindow::~MainWindow()
{
    delete ui;
    delete labelsList;
    delete inputVector;
}

// ------------------------------------------------------------------------------------------------

QString MainWindow::formatFileSize(unsigned int size)
{
    // format predicted file size in megabytes, kilobytes to 3 decimal digits
    if (size >= 9995000) {                                 // 9.995 MB  ->  >10 MB
        double normSize = size / 1000000.0;
        return QString::number(normSize, 'f', 1) + " MB";
    } else if (size >= 999500) {                           // 0.9995 MB ->  >1 MB
        double normSize = size / 1000000.0;
        return QString::number(normSize, 'f', 2) + " MB";
    } else if (size >= 99950) {                            // 99.950 kB ->  >100 kB
        double normSize = size / 1000.0;
        return QString::number(normSize, 'f', 0) + " kB";
    } else if (size >= 9995) {                             // 9.995 kB  ->  >10 kB
        double normSize = size / 1000.0;
        return QString::number(normSize, 'f', 1) + " kB";
    } else {
        double normSize = size / 1000.0;
        return QString::number(normSize, 'f', 2) + " kB";
    }
}

void MainWindow::updateProcedure(int triggerId, int newQF)
{
    // this function is called when QF value or any of 2 slider positions is changed
    // triggerId = 0 for QF, = 1 for quality and = 2 for file size slider
    // value of the respective GUI control, whose trigger called this function, is not modified

    // special attention should be paid to the case, when current QF is the same as new QF
    if ((triggerId != 0) && (newQF == ui->spinBoxQF->value())) return;

    // check if image is opened
    if (inputImage.isNull()) return;

    // assign new QF to the input vector
    inputVector[11] = newQF;

    // calculate expected Y-PSNR, Y-MSSIM and file size with new QF
    const double expectedYPSNR    = Optimizer::estimateYPSNR(isjpeg, inputVector);
    const double expectedYMSSIM   = Optimizer::estimateYMSSIM(isjpeg, inputVector);
    const double expectedFileSize = Optimizer::estimateFileSize(isjpeg, inputVector);

    // display PSNR, MSSIM and file size
    ui->editPSNR->setText(QString::number(expectedYPSNR, 'f', 1));
    ui->editMSSIM->setText(QString::number(expectedYMSSIM, 'f', 3));
    ui->editFileSize->setText(formatFileSize((unsigned int) expectedFileSize));

    // now check which control has triggered this update
    if (triggerId == 0) {           // spinBoxQF
        moveQualitySlider(expectedYMSSIM);
        moveSizeSlider(expectedFileSize);
    } else if (triggerId == 1) {    // sliderMSSIM
        autoUpdateFlag0 = true;
        ui->spinBoxQF->setValue(newQF);
        moveSizeSlider(expectedFileSize);
    } else if (triggerId == 2) {    // sliderFileSize
        autoUpdateFlag0 = true;
        ui->spinBoxQF->setValue(newQF);
        moveQualitySlider(expectedYMSSIM);
    }
}

void MainWindow::moveQualitySlider(double expectedYMSSIM)
{
    // move slider for MSSIM only if the new position is different from current
    const int sliderPosition = (expectedYMSSIM <= minDisplayMSSIM) ? 0 : (int) (numPositions * (expectedYMSSIM - minDisplayMSSIM) / (1 - minDisplayMSSIM) + 0.5);
    if (ui->sliderMSSIM->value() != sliderPosition) {
        autoUpdateFlag1 = true;
        ui->sliderMSSIM->setValue(sliderPosition);
    }
}

void MainWindow::moveSizeSlider(double expectedFileSize)
{
    const double lnExpectedFileSize = log(expectedFileSize);
    const int sliderPosition = (int) (numPositions * (lnExpectedFileSize - lnMinSize) / (lnMaxSize - lnMinSize) + 0.5);
    if (ui->sliderFileSize->value() != sliderPosition) {
        autoUpdateFlag2 = true;
        ui->sliderFileSize->setValue(sliderPosition);
    }
}

// ------------------------------------------------------------------------------------------------

void MainWindow::on_btnOpenImage_clicked()
{
    // user selects image file
    QFileDialog dialog(this, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Images (*.jpg *jpeg *.bmp *.png *.tif *.tiff)");
    dialog.setWindowTitle("Select image");
    if (!dialog.exec()) return;

    // open image
    QString imagePath = dialog.selectedFiles()[0];
    inputImage = QImage(imagePath);
    if (inputImage.isNull()) {
        QMessageBox::warning(this, "Error", "Can't open selected file as image", QMessageBox::Ok);
        return;
    }

    // image-related GUI update
    ui->editImagePath->setText(imagePath);
    imageBox->setImage(inputImage);
    ui->editCompressionTime->clear();

    // check if image is not 24 bpp, e.g. grayscale, and convert it to 24 bpp
    if (inputImage.format() != QImage::Format_RGB32) inputImage = inputImage.convertToFormat(QImage::Format_RGB32);

    // get uncompressed image data (each pixel has format xBGR)
    const int w = inputImage.width();
    const int h = inputImage.height();
    const unsigned char *inputImageData = inputImage.constBits();

    // stage 1 - feature extraction
    const unsigned long long int featureExtractionStartTime = QDateTime::currentMSecsSinceEpoch();

    // actual function that calculated 10 image features from uncompressed data
    FeatureExtractor::calculateFeatures((const unsigned int *) inputImageData, w, h, inputVector);

    // calculate 11-th input (image size)
    inputVector[10] = log(w * h / 1000000.0);

    // display feature extraction time
    ui->editExtractionTime->setText(QString::number(QDateTime::currentMSecsSinceEpoch() - featureExtractionStartTime));

    // before resetting QF to default value 75 we need to update file size scale
    updateSizeScale();
    resetQF();
}

void MainWindow::updateSizeScale()
{
    // min and max estimated file size
    inputVector[11] = isjpeg ? 5 : 0;
    lnMinSize = log(Optimizer::estimateFileSize(isjpeg, inputVector));
    inputVector[11] = 100;
    lnMaxSize = log(Optimizer::estimateFileSize(isjpeg, inputVector));

    // update file size grade labels according to the estimated min and max size
    for (int i = 0;  i < 6;  i++) {
        const double currentLabelSize = exp(lnMinSize + (lnMaxSize - lnMinSize) * (i / 5.0));
        labelsList[i]->setText(formatFileSize(currentLabelSize));
    }

    // here we also update the minimal possible MSSIM for current image
    inputVector[11] = isjpeg ? 5 : 0;
    minImageMSSIM = Optimizer::estimateYMSSIM(isjpeg, inputVector);
}

void MainWindow::resetQF()
{
    // prediction-related controls update in the following order:
    // - quality factor is set to 75 (it's essential to check if QF is already set to 75)
    // - this triggers updateProcedure() that updates locations of both sliders and estimated PSNR
    // - updateProcedure() sets autoUpdateFlags, which prevent recursive calls of updateProcedure()
    const int defaultQF = 75;
    if (ui->spinBoxQF->value() != defaultQF) {
        ui->spinBoxQF->setValue(defaultQF);
    } else {
        // if spinBoxQF is already set to 75, then set it to 76 to trigger updateProcedure()
        // this is a simple hack to avoid bugs
        ui->spinBoxQF->setValue(defaultQF + 1);
    }
}

// ------------------------------------------------------------------------------------------------

void MainWindow::on_spinBoxQF_valueChanged(int qualityFactor)
{
    if (autoUpdateFlag0) {
        autoUpdateFlag0 = false;
        return;
    }
    updateProcedure(0, qualityFactor);
}

void MainWindow::on_sliderMSSIM_valueChanged(int sliderPosition)
{
    // move "hovering edit" according to the new slider position
    // this change should be done regardless of the event, which caused it
    const int initialEditPosX = 10;
    const int initialEditPosY = 18;
    ui->editMSSIM->move((ui->sliderMSSIM->width() - 12) * sliderPosition / numPositions + initialEditPosX, initialEditPosY);

    // skip this update if it was caused by another control
    if (autoUpdateFlag1) {
        autoUpdateFlag1 = false;
        return;
    }

    // сalculate predicted value from slider position
    const double expectedYMSSIM = minDisplayMSSIM + (double) sliderPosition / numPositions * (1.0 - minDisplayMSSIM);

    // if user moves slider outside the allowed range [minImageMSSIM; 1.0], it has to be returned to minImageMSSIM
    // this will automatically move "hovering edit" too
    if (expectedYMSSIM < minImageMSSIM) moveQualitySlider(minImageMSSIM);

    // find the closet quality factor
    const int qualityFactor = Optimizer::findQualityFactor(isjpeg, 'm', expectedYMSSIM, inputVector);

    // update other GUI controls
    updateProcedure(1, qualityFactor);
}

void MainWindow::on_sliderFileSize_valueChanged(int sliderPosition)
{
    // move "hovering edit" according to the new slider position
    // this change should be done regardless of the event, which caused it
    const int initialEditPosX = 10;
    const int initialEditPosY = 18;
    ui->editFileSize->move((ui->sliderFileSize->width() - 12) * sliderPosition / numPositions + initialEditPosX, initialEditPosY);

    // skip this update if it was caused by another control
    if (autoUpdateFlag2) {
        autoUpdateFlag2 = false;
        return;
    }

    // сalculate predicted value from slider position
    const unsigned int expectedFileSize = (unsigned int) (exp(lnMinSize + (double) sliderPosition / numPositions * (lnMaxSize - lnMinSize)) + 0.5);

    // find the closet quality factor
    const int qualityFactor = Optimizer::findQualityFactor(isjpeg, 's', (double) expectedFileSize, inputVector);

    // update other GUI controls
    updateProcedure(2, qualityFactor);
}

void MainWindow::on_radioButtonJPEG_toggled(bool checked)
{
    if (checked && !inputImage.isNull()) { isjpeg = true; ui->spinBoxQF->setMinimum(5); updateSizeScale(); resetQF(); }
}

void MainWindow::on_radioButtonWebP_toggled(bool checked)
{
    if (checked && !inputImage.isNull()) { isjpeg = false; ui->spinBoxQF->setMinimum(0); updateSizeScale(); resetQF(); }
}

void MainWindow::on_btnCompress_clicked()
{
    // check if image is opened
    if (inputImage.isNull()) return;

    // in this function we don't need to do any predictions,
    // only compress an image with a QF, which was previously determined

    const int w = inputImage.width();
    const int h = inputImage.height();
    const unsigned char *inputImageData = inputImage.constBits();

    const int qualityFactor = ui->spinBoxQF->value();

    const unsigned long long int compressionStartTime = QDateTime::currentMSecsSinceEpoch();

    unsigned long long int compressedBufferSize = 0;
    const unsigned char *compressedImageBuffer = isjpeg ? Encoder::compressToJpeg(inputImageData, w, h, qualityFactor, &compressedBufferSize) :
                                                          Encoder::compressToWebp(inputImageData, w, h, qualityFactor, &compressedBufferSize);

    // display compression time
    ui->editExtractionTime->setText(QString::number(QDateTime::currentMSecsSinceEpoch() - compressionStartTime));

    // open dialog to save compressed image
    QString suffix = isjpeg ? "jpg" : "webp";
    QFileDialog dialog(this, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix(suffix);
    dialog.setNameFilter(isjpeg ? "JPEG images (*.jpg)" : "WebP images (*.webp)");
    dialog.setWindowTitle("Save image");
    if (!dialog.exec()) return;

    QString filePath = dialog.selectedFiles()[0];
    if (!filePath.endsWith("." + suffix)) filePath += "." + suffix;

    // save compressed image to file
    QFile f(filePath);
    if (!f.open(QFile::WriteOnly)) return;
    f.write((const char *) compressedImageBuffer, compressedBufferSize);
    f.close();

    delete [] compressedImageBuffer;
}
