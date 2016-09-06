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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "imagebox.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnOpenImage_clicked();

private:
    void updateSizeScale();
    void resetQF();

private slots:
    void on_spinBoxQF_valueChanged(int qualityFactor);
    void on_sliderMSSIM_valueChanged(int sliderPosition);
    void on_sliderFileSize_valueChanged(int sliderPosition);
    void on_radioButtonJPEG_toggled(bool checked);
    void on_radioButtonWebP_toggled(bool checked);
    void on_btnCompress_clicked();

private:
    QString formatFileSize(unsigned int size);
    void updateProcedure(int triggerId, int newQF);
    void moveQualitySlider(double expectedYMSSIM);
    void moveSizeSlider(double expectedFileSize);

private:
    Ui::MainWindow *ui;
    ImageBox *imageBox;
    QLabel **labelsList;

    QImage inputImage;
    bool isjpeg;
    int numPositions;

    double *inputVector;

    double lnMinSize;
    double lnMaxSize;

    bool autoUpdateFlag0;
    bool autoUpdateFlag1;
    bool autoUpdateFlag2;

    double minDisplayMSSIM;
    double minImageMSSIM;
};

#endif // MAINWINDOW_H
