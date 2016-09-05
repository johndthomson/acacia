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


#ifndef OPTIMIZER_H
#define OPTIMIZER_H

class Optimizer
{
public:
    static int    findQualityFactor(bool isjpeg, char targetObjective, double targetValue, const double *inputVector);

    static double estimateFileSize(bool isjpeg, const double *inputVector);
    static double estimateYMSSIM(bool isjpeg, const double *inputVector);
    static double estimateYPSNR(bool isjpeg, const double *inputVector);

private:
    static void   standardizeInput(bool isjpeg, const double *inputVector, double *standardizedInputVector);

    static double estimateFileSize(const double *mlpModel, const double *standardizedInputVector);
    static double estimateYMSSIM(const double *mlpModel, const double *standardizedInputVector);
    static double estimateYPSNR(const double *mlpModel, const double *standardizedInputVector);
};

#endif // OPTIMIZER_H
