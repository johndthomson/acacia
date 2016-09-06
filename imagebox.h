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


#ifndef IMAGEBOX_H
#define IMAGEBOX_H

#include <QImage>
#include <QWidget>

class ImageBox : public QWidget
{
public:
    ImageBox(QWidget *parent);
    QImage getImage();
    void setImage(QImage image);

protected:
    QImage image;
    void paintEvent(QPaintEvent *event);
};

#endif // IMAGEBOX_H
