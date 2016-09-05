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


#include <QPainter>
#include "imagebox.h"

ImageBox::ImageBox(QWidget *parent) : QWidget(parent) {}

void ImageBox::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    painter.fillRect(QRect(0, 0, this->width(), this->height()), Qt::gray);

    QPen pen(Qt::darkGray);
    pen.setWidth(4);
    painter.setPen(pen);
    for (int i = 0;  i < this->width() + this->height();  i += 12) painter.drawLine(i, 0, 0, i);

    if (!image.isNull()) painter.drawImage(0, 0, image.scaled(this->width(), this->height(), Qt::KeepAspectRatio));
}

QImage ImageBox::getImage()
{
    return image;
}

void ImageBox::setImage(QImage image)
{
    this->image = image;
    this->update();
}
