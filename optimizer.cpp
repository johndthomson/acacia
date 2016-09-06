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


#include "optimizer.h"
#include "jpegmodels.h"
#include "webpmodels.h"

#include "math.h"

int Optimizer::findQualityFactor(bool isjpeg, char targetObjective, double targetValue, const double *inputVector)
{
    // z-score standardization information
    const double meanValues [] = {1.67857871738869, 1.84940743362902, 2.03573723917577, 4.32308662342086, 4.77520341534466, 5.18429697435992, 1.07480309472305, 1.19925725148692, 0.785210507241508, 0.344762186269797, 0.875428671146136, (isjpeg ? 52.5 : 50.0)};
    const double sdValues [] = {0.570680274424306, 0.570547669481748, 0.551873365309837, 1.28965887561942, 1.18708941771867, 1.04983787410284, 0.498536202400926, 0.520063844702872, 0.404995922117509, 0.22549143978714, 1.34954012555432, (isjpeg ? 27.8567765543682 : 29.3001706479672)};

    // perform standardization of the first 11 inputs except quality factor (it's not known yet)
    const int inputVectorSize = 12;
    double standardizedInputVector [inputVectorSize];
    for (int i = 0;  i < 11;  i++) standardizedInputVector[i] = (inputVector[i] - meanValues[i]) / sdValues[i];

    // chose estimating function and regression model
    double (*estimatingFunction)(const double *, const double *) = nullptr;
    double *mlpModel = nullptr;

    switch (targetObjective) {
    case 's':    // size
        estimatingFunction = &estimateFileSize;
        mlpModel = isjpeg ? jpeg_fsize_model : webp_fsize_model;
        break;
    case 'm':    // mssim
        estimatingFunction = &estimateYMSSIM;
        mlpModel = isjpeg ? jpeg_ymssim_model : webp_ymssim_model;
        break;
    case 'p':    // psnr
        estimatingFunction = &estimateYPSNR;
        mlpModel = isjpeg ? jpeg_ypsnr_model : webp_ypsnr_model;
        break;
    }

    // for JPEG minimal useful QF is set to 5
    const int minQF = isjpeg ? 5 : 0;

    // the following search with a break may cause problems in some rare cases because
    // it assumes that the regression function is strictly monotonic,
    // but at least MSSIM model has some minor deviations from this trend,
    // which can lead to early exit from the loop with a wrong result
    //
    // the binary search may also be affected by this (although in a much smaller extent)
    //
    // therefore, for the sake of avoiding bugs in an easy way
    // we exaustively enumerate all QFs and choose the smallest
    //
    // old version of the loop will remain here for the record

    /*
    // start search using a simple approach:
    // changing quality factor from 5 to 100 we check when the difference between predicted and target values is minimal
    // obviously, this approach can be improved with binary search, but it's already fast enough (< 1 ms)
    double minDifference = 1000000000;    // just a big number
    int qualityFactor;
    for (qualityFactor = minQF;  qualityFactor <= 100;  qualityFactor++)    // 100 is not included as it's the maximum possible QF (101 is invalid)
    {
        standardizedInputVector[11] = (qualityFactor - meanValues[11]) / sdValues[11];
        const double predictedValue = estimatingFunction(mlpModel, standardizedInputVector);
        double difference = predictedValue - targetValue;
        if (difference < 0) difference = -difference;
        if (difference >= minDifference) {
            // qualityFactor decrement was moved outside the loop when to fix a bug
            break;
        } else {
            minDifference = difference;
        }
    }

    // this decrement has 2 functions: prevents QF = 101 and makes a step back after jumping over the best solution
    qualityFactor--;
    */

    double minDifference = 1000000000;    // just a big number
    int bestQualityFactor = -1;           // solution
    for (int qualityFactor = minQF;  qualityFactor <= 100;  qualityFactor++)
    {
        standardizedInputVector[11] = (qualityFactor - meanValues[11]) / sdValues[11];
        const double predictedValue = estimatingFunction(mlpModel, standardizedInputVector);
        double difference = predictedValue - targetValue;
        if (difference < 0) difference = -difference;
        if (difference < minDifference) {
            bestQualityFactor = qualityFactor;
            minDifference = difference;
        }
    }

    return bestQualityFactor;
}

double Optimizer::estimateFileSize(bool isjpeg, const double *inputVector)
{
    // reserve space
    const int inputVectorSize = 12;
    double standardizedInputVector [inputVectorSize];

    // standardization
    standardizeInput(isjpeg, inputVector, standardizedInputVector);

    // call overloaded method, which requires more low-level input
    const double *mlpModel = isjpeg ? jpeg_fsize_model : webp_fsize_model;
    return estimateFileSize(mlpModel, standardizedInputVector);
}

double Optimizer::estimateYMSSIM(bool isjpeg, const double *inputVector)
{
    const int inputVectorSize = 12;
    double standardizedInputVector [inputVectorSize];

    standardizeInput(isjpeg, inputVector, standardizedInputVector);

    const double *mlpModel = isjpeg ? jpeg_ymssim_model : webp_ymssim_model;
    return estimateYMSSIM(mlpModel, standardizedInputVector);
}

double Optimizer::estimateYPSNR(bool isjpeg, const double *inputVector)
{
    const int inputVectorSize = 12;
    double standardizedInputVector [inputVectorSize];

    standardizeInput(isjpeg, inputVector, standardizedInputVector);

    const double *mlpModel = isjpeg ? jpeg_ypsnr_model : webp_ypsnr_model;
    return estimateYPSNR(mlpModel, standardizedInputVector);
}

void Optimizer::standardizeInput(bool isjpeg, const double *inputVector, double *standardizedInputVector)
{
    // z-score standardization information
    const double meanValues [] = {1.67857871738869, 1.84940743362902, 2.03573723917577, 4.32308662342086, 4.77520341534466, 5.18429697435992, 1.07480309472305, 1.19925725148692, 0.785210507241508, 0.344762186269797, 0.875428671146136, (isjpeg ? 52.5 : 50.0)};
    const double sdValues [] = {0.570680274424306, 0.570547669481748, 0.551873365309837, 1.28965887561942, 1.18708941771867, 1.04983787410284, 0.498536202400926, 0.520063844702872, 0.404995922117509, 0.22549143978714, 1.34954012555432, (isjpeg ? 27.8567765543682 : 29.3001706479672)};

    // perform standardization of all 12 inputs including quality factor
    for (int i = 0;  i < 12;  i++) standardizedInputVector[i] = (inputVector[i] - meanValues[i]) / sdValues[i];
}

double Optimizer::estimateFileSize(const double *mlpModel, const double *standardizedInputVector)
{
    const int inputVectorSize = 12;
    const int numHiddenNeurons = 50;

    int counterHiddenLayer = 0;    // counters for NN coefficients
    int counterOutputNeuron = (1 + inputVectorSize) * numHiddenNeurons;

    double networkResult = mlpModel[counterOutputNeuron++];    // output bias

    for (int h = 0;  h < numHiddenNeurons;  h++)    // loop over hidden neurons
    {
        double linearCombination = mlpModel[counterHiddenLayer++];    // hidden neuron bias
        for (int k = 0;  k < inputVectorSize;  k++) linearCombination += standardizedInputVector[k] * mlpModel[counterHiddenLayer++];
        networkResult += mlpModel[counterOutputNeuron++] / (1 + exp(-linearCombination));
    }

    const unsigned int fileSize = (unsigned int) (exp(networkResult) + 0.5);    // exponentiate result as we predicted only logarithm of the file size
    return (double) fileSize;
}

double Optimizer::estimateYMSSIM(const double *mlpModel, const double *standardizedInputVector)
{
    const int inputVectorSize = 12;
    const int numHiddenNeurons = 50;

    int counterHiddenLayer = 0;    // counters for NN coefficients
    int counterOutputNeuron = (1 + inputVectorSize) * numHiddenNeurons;

    double networkResult = mlpModel[counterOutputNeuron++];    // output bias

    for (int h = 0;  h < numHiddenNeurons;  h++)    // loop over hidden neurons
    {
        double linearCombination = mlpModel[counterHiddenLayer++];    // hidden neuron bias
        for (int k = 0;  k < inputVectorSize;  k++) linearCombination += standardizedInputVector[k] * mlpModel[counterHiddenLayer++];
        networkResult += mlpModel[counterOutputNeuron++] / (1 + exp(-linearCombination));
    }

    if (networkResult > 1.0) networkResult = 1.0;    // SSIM cannot exceed 1

    return networkResult;
}

double Optimizer::estimateYPSNR(const double *mlpModel, const double *standardizedInputVector)
{
    const int inputVectorSize = 12;
    const int numHiddenNeurons = 50;

    int counterHiddenLayer = 0;    // counters for NN coefficients
    int counterOutputNeuron = (1 + inputVectorSize) * numHiddenNeurons;

    double networkResult = mlpModel[counterOutputNeuron++];    // output bias

    for (int h = 0;  h < numHiddenNeurons;  h++)    // loop over hidden neurons
    {
        double linearCombination = mlpModel[counterHiddenLayer++];    // hidden neuron bias
        for (int k = 0;  k < inputVectorSize;  k++) linearCombination += standardizedInputVector[k] * mlpModel[counterHiddenLayer++];
        networkResult += mlpModel[counterOutputNeuron++] / (1 + exp(-linearCombination));
    }

    return networkResult;
}
