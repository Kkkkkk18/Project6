#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

using namespace std;

void applyGaussianFilter(vector<char>& imageBuffer, int width, int height, int bitsPerPixel, float sigma) {

    int kernelSize = ceil(6 * sigma) + 1;
    int halfKernelSize = kernelSize / 2;

    vector<float> kernel(kernelSize * kernelSize);

    float sum = 0.0;
    for (int y = -halfKernelSize; y <= halfKernelSize; y++) {
        for (int x = -halfKernelSize; x <= halfKernelSize; x++) {
            float value = exp(-(x * x + y * y) / (2 * sigma * sigma));
            kernel[(y + halfKernelSize) * kernelSize + (x + halfKernelSize)] = value;
            sum += value;
        }
    }

    for (int i = 0; i < kernelSize * kernelSize; i++) {
        kernel[i] /= sum;
    }

    vector<char> filteredImageBuffer(imageBuffer.size());

    for (int y = halfKernelSize; y < height - halfKernelSize; y++) {
        for (int x = halfKernelSize; x < width - halfKernelSize; x++) {
            int pixelIndex = (y * width + x) * bitsPerPixel / 8;

            float red = 0.0;
            float green = 0.0;
            float blue = 0.0;

            for (int ky = -halfKernelSize; ky <= halfKernelSize; ky++) {
                for (int kx = -halfKernelSize; kx <= halfKernelSize; kx++) {
                    int kernelIndex = (ky + halfKernelSize) * kernelSize + (kx + halfKernelSize);
                    int neighborPixelIndex = ((y + ky) * width + (x + kx)) * bitsPerPixel / 8;

                    red += kernel[kernelIndex] * static_cast<unsigned char>(imageBuffer[neighborPixelIndex]);
                    green += kernel[kernelIndex] * static_cast<unsigned char>(imageBuffer[neighborPixelIndex + 1]);
                    blue += kernel[kernelIndex] * static_cast<unsigned char>(imageBuffer[neighborPixelIndex + 2]);
                }
            }

            filteredImageBuffer[pixelIndex] = static_cast<char>(red);
            filteredImageBuffer[pixelIndex + 1] = static_cast<char>(green);
            filteredImageBuffer[pixelIndex + 2] = static_cast<char>(blue);
        }
    }

    imageBuffer = filteredImageBuffer;

}

int main() {
    float sigma = 6.0;
    string filename = "smi.bmp";

    ifstream file(filename, ios::binary);
    vector<char> imageBuffer;

    if (!file.is_open()) {
        cout << "Unable to open file: " << filename << endl;
        return 1;
    }

    char header[54];
    file.read(header, 54);

    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int offset = *(int*)&header[10];

    cout << "Image width: " << width << " bytes" << endl;
    cout << "Image height: " << height << " bytes" << endl;
    cout << "Image offset: " << offset << " bytes" << endl;

    char* garbage = new char[offset - 54];
    file.read(garbage, offset - 54);
    int bitsPerPixel = *(short*)&header[28];

    int imageSize = width * height * bitsPerPixel / 8;

    cout << "Image size: " << imageSize << " bytes" << endl;

    imageBuffer.resize(imageSize);

    file.read(imageBuffer.data(), imageSize);

    file.close();

    vector<char> rotatedRightImageBuffer(imageSize);

    int rotatedWidth = height;
    int rotatedHeight = width;
    *(int*)&header[18] = rotatedWidth;
    *(int*)&header[22] = rotatedHeight;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int rotatedX = y;
            int rotatedY = width - x - 1;
            int originalPixelIndex = (y * width + x) * bitsPerPixel / 8;
            int rotatedPixelIndex = (rotatedY * rotatedWidth + rotatedX) * bitsPerPixel / 8;

            for (int i = 0; i < bitsPerPixel / 8; i++) {
                rotatedRightImageBuffer[rotatedPixelIndex + i] = imageBuffer[originalPixelIndex + i];
            }
        }
    }

    string rotatedRightFilename = "image90r.bmp";
    ofstream rotatedRightFile(rotatedRightFilename, ios::binary);
    rotatedRightFile.write(header, 54);
    rotatedRightFile.write(garbage, offset - 54);
    rotatedRightFile.write(rotatedRightImageBuffer.data(), imageSize);
    rotatedRightFile.close();

    cout << "Rotated image saved as: " << rotatedRightFilename << endl;

    vector<char> rotatedLeftImageBuffer(imageSize);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int rotatedX = height - y - 1;
            int rotatedY = x;
            int originalPixelIndex = (y * width + x) * bitsPerPixel / 8;
            int rotatedPixelIndex = (rotatedY * rotatedWidth + rotatedX) * bitsPerPixel / 8;

            for (int i = 0; i < bitsPerPixel / 8; i++) {
                rotatedLeftImageBuffer[rotatedPixelIndex + i] = imageBuffer[originalPixelIndex + i];
            }
        }
    }

    string rotatedLeftFilename = "image90l.bmp";
    ofstream rotatedLeftFile(rotatedLeftFilename, ios::binary);
    rotatedLeftFile.write(header, 54);
    rotatedLeftFile.write(garbage, offset - 54);
    rotatedLeftFile.write(rotatedLeftImageBuffer.data(), imageSize);
    rotatedLeftFile.close();

    cout << "Rotated image saved as: " << rotatedLeftFilename << endl;

    applyGaussianFilter(rotatedRightImageBuffer, rotatedWidth, rotatedHeight, bitsPerPixel, sigma);

    ofstream rotatedRightGausFile("image90rGaus.bmp", ios::binary);
    rotatedRightGausFile.write(header, 54);
    rotatedRightGausFile.write(garbage, offset - 54);
    rotatedRightGausFile.write(rotatedRightImageBuffer.data(), imageSize);
    rotatedRightGausFile.close();

    cout << "Rotated image saved as: " << "image90rGaus.bmp" << endl;

    return 0;
}