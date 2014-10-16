#include "imageoperator.h"

namespace ImageOperator
{

bool gradientMagnitudeAndOrientation_CPU(GrayScaleImage& src, GrayScaleImage& grd, GrayScaleImage& ore, float& gradMax)
{
    int width = src.width();
    int height = src.height();

    GrayScalePixel* gradPixels = new GrayScalePixel[width * height];
    if(gradPixels == 0) return false;
    GrayScalePixel* orientPixels = new GrayScalePixel[width * height];
    if(orientPixels == 0) return false;

    gradMax = -1.0;
    for (int y=0;y<height;y++)
    {
        int rowOffset = y * width;
        for (int x=0;x<width;x++)
        {
            GrayScalePixel right = src.getPixel(x+1, y);
            GrayScalePixel down = src.getPixel(x, y+1);
            GrayScalePixel pixel = src.getPixel(x, y);

            double diffx = pixel - right;
            double diffy = down - pixel;
            double gradValue = sqrt(diffx * diffx + diffy * diffy);
            if (gradValue > gradMax) gradMax = gradValue;
            gradPixels[rowOffset + x] = gradValue;
            orientPixels[rowOffset + x] = atan2(diffy, diffx);
        }
    }

    grd = GrayScaleImage(gradPixels, width, height);
    ore = GrayScaleImage(orientPixels, width, height);
    delete[] gradPixels;
    delete[] orientPixels;

    return true;
}

//! output orientation value [-PI, PI]
bool gradientMagnitudeAndOrientation_CPU(GrayScaleImage& src, GrayScaleImage& grd, GrayScaleImage& grdX, GrayScaleImage& grdY, GrayScaleImage& ore, float& gradMax)
{
    int width = src.width();
    int height = src.height();

    GrayScalePixel* gradPixels = new GrayScalePixel[width * height];
    if(gradPixels == 0) return false;
    GrayScalePixel* gradXPixels = new GrayScalePixel[width * height];
    if(gradXPixels == 0) return false;
    GrayScalePixel* gradYPixels = new GrayScalePixel[width * height];
    if(gradYPixels == 0) return false;
    GrayScalePixel* orientPixels = new GrayScalePixel[width * height];
    if(orientPixels == 0) return false;

    gradMax = -1.0;
    for (int y=0;y<height;y++)
    {
        int rowOffset = y * width;
        for (int x=0;x<width;x++)
        {
            GrayScalePixel right = src.getPixel(x+1, y);
            GrayScalePixel down = src.getPixel(x, y+1);
            GrayScalePixel pixel = src.getPixel(x, y);

            double diffx = pixel - right;
            double diffy = down - pixel;
            double gradValue = sqrt(diffx * diffx + diffy * diffy);
            if (gradValue > gradMax) gradMax = gradValue;
            gradPixels[rowOffset + x] = gradValue;
            gradXPixels[rowOffset + x] = diffx;
            gradYPixels[rowOffset + x] = diffy;
            orientPixels[rowOffset + x] = atan2(diffy, diffx);
        }
    }

    grd = GrayScaleImage(gradPixels, width, height);
    grdX = GrayScaleImage(gradXPixels, width, height);
    grdY = GrayScaleImage(gradYPixels, width, height);
    ore = GrayScaleImage(orientPixels, width, height);
    delete[] gradPixels;
    delete[] gradXPixels;
    delete[] gradYPixels;
    delete[] orientPixels;

    return true;
}

bool gradientMagnitudeAndOrientation_CPU(GrayScaleImage& src, GrayScaleImage& grd, GrayScaleImage& ore)
{
    int width = src.width();
    int height = src.height();

    GrayScalePixel* gradPixels = new GrayScalePixel[width * height];
    if(gradPixels == 0) return false;
    GrayScalePixel* orientPixels = new GrayScalePixel[width * height];
    if(orientPixels == 0) return false;

    for (int y=0;y<height;y++)
    {
        int rowOffset = y * width;
        for (int x=0;x<width;x++)
        {
            GrayScalePixel right = src.getPixel(x+1, y);
            GrayScalePixel down = src.getPixel(x, y+1);
            GrayScalePixel pixel = src.getPixel(x, y);

            double diffx = pixel - right;
            double diffy = down - pixel;
            double gradValue = sqrt(diffx * diffx + diffy * diffy);
            gradPixels[rowOffset + x] = gradValue;
            orientPixels[rowOffset + x] = atan2(diffy, diffx);
        }
    }

    grd = GrayScaleImage(gradPixels, width, height);
    ore = GrayScaleImage(orientPixels, width, height);
    delete[] gradPixels;
    delete[] orientPixels;

    return true;
}

bool gradientMagnitudeAndOrientation_CPU(GrayScaleImage& src, GrayScaleImage& grd, GrayScaleImage& grdX, GrayScaleImage& grdY, GrayScaleImage& ore)
{
    int width = src.width();
    int height = src.height();

    GrayScalePixel* gradPixels = new GrayScalePixel[width * height];
    if(gradPixels == 0) return false;

    GrayScalePixel* gradXPixels = new GrayScalePixel[width * height];
    if(gradXPixels == 0) return false;

    GrayScalePixel* gradYPixels = new GrayScalePixel[width * height];
    if(gradYPixels == 0) return false;

    GrayScalePixel* orientPixels = new GrayScalePixel[width * height];
    if(orientPixels == 0) return false;

    for (int y=0;y<height;y++)
    {
        int rowOffset = y * width;
        for (int x=0;x<width;x++)
        {
            GrayScalePixel right = src.getPixel(x+1, y);
            GrayScalePixel down = src.getPixel(x, y+1);
            GrayScalePixel pixel = src.getPixel(x, y);

            double diffx = pixel - right;
            double diffy = down - pixel;
            double gradValue = sqrt(diffx * diffx + diffy * diffy);
            gradPixels[rowOffset + x] = gradValue;
            gradXPixels[rowOffset + x] = diffx;
            gradYPixels[rowOffset + x] = diffy;
            orientPixels[rowOffset + x] = atan2(diffy, diffx);
        }
    }

    grd = GrayScaleImage(gradPixels, width, height);
    grdX = GrayScaleImage(gradXPixels, width, height);
    grdY = GrayScaleImage(gradYPixels, width, height);
    ore = GrayScaleImage(orientPixels, width, height);
    delete[] gradPixels;
    delete[] gradXPixels;
    delete[] gradYPixels;
    delete[] orientPixels;

    return true;
}
//==============================================================================
//==============================================================================
RGBAImage extrema_CPU(GrayScaleImage& img)
{
    int width = img.width();
    int height = img.height();

    double* pixelArray = new double[width * height * 4];

    for (int i=0;i<height;i++)
    {
        for (int j=0;j<width;j++)
        {
            double pixels[9];
            for (int k=-1;k<=1;k++)
            {
                int rowIdx = k + 1;
                for (int l=-1;l<=1;l++)
                {
                    int colIdx = l + 1;
                    pixels[rowIdx * 3 + colIdx] = img.getPixel(j + l, i + k);
                }
            }

            bool isMaxima = true, isMinima = true;
            for (int m=0;m<9;m++)
            {
                if (m==4) continue;
                if (pixels[m] >= pixels[4]) isMaxima = false;
                if (pixels[m] <= pixels[4]) isMinima = false;
            }

            RGBAPixel p;
            if (isMaxima) p.r = 1.0;
            else p.r = 0.0;
            if (isMinima) p.g = 1.0;
            else p.g = 0.0;
            p.b = 0.0;
            p.a = 1.0;

            if (!isMaxima && !isMinima)
            {
                p.r = p.g = p.b = pixels[4];
                p.a = 1.0;
            }
            int offset = (i * img.width() + j) * 4;
            pixelArray[offset + 0] = p.r;
            pixelArray[offset + 1] = p.g;
            pixelArray[offset + 2] = p.b;
            pixelArray[offset + 3] = p.a;
        }
    }

    RGBAImage resultImage = RGBAImage(pixelArray, width, height);

    delete[] pixelArray;
    return resultImage;
}

GrayScaleImage gaussianFilter_bidirectional_CPU(GrayScaleImage& src, const double& sigma)
{
    int kernelSize = ceil(4.0 * sigma);
    double* kernel = new double[kernelSize];
    double inverseSigma = 1.0 / sigma;
    double inverseTwoSigmaSquare = 0.5 * inverseSigma * inverseSigma;
    for (int i=0;i<kernelSize;i++)
    {
        int x = i - (kernelSize - 1) / 2;
        kernel[i] = 1.0 / sqrt(2.0 * PI) * inverseSigma * exp( - x * x * inverseTwoSigmaSquare);
    }

    Utils::normalizeVector<double>(kernel, kernelSize);

    int width = src.width(), height = src.height();
    int size = width * height;
    GrayScalePixel* pixelArray = new GrayScalePixel[size];
    memset(pixelArray, 0, sizeof(GrayScalePixel)*size);

    // first pass, horizotal
#pragma omp parallel for
    for (int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            int offset = y * width + x;
            for (int i=0;i<kernelSize;i++)
            {
                int refOffset = i - (kernelSize - 1) / 2;
                pixelArray[offset] += src.getPixel(x + refOffset, y) * kernel[i];
            }
        }
    }

    GrayScaleImage tmpImg(pixelArray, width, height);
    memset(pixelArray, 0, sizeof(GrayScalePixel)*size);

    // second pass, vertical
#pragma omp parallel for
    for (int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            int offset = y * width + x;
            for (int i=0;i<kernelSize;i++)
            {
                int refOffset = i - (kernelSize - 1) / 2;
                pixelArray[offset] += tmpImg.getPixel(x, y + refOffset) * kernel[i];
            }
        }
    }

    GrayScaleImage result(pixelArray, width, height);

    delete[] pixelArray;
    delete[] kernel;

    return result;
}

GrayScaleImage difference_CPU(GrayScaleImage& img1, GrayScaleImage& img2)
{
    if ((img1.width() != img2.width())
            || (img1.height() != img2.height()) )
        return GrayScaleImage();

    int width = img1.width();
    int height = img1.height();
    GrayScalePixel* pixelArray = new GrayScalePixel[width * height];

#pragma omp parallel for
    for (int y=0;y<height;y++)
        for (int x =0;x<width;x++)
        {
            double diffValue = img1.getPixel(x,y) - img2.getPixel(x, y);
            pixelArray[y * width + x] = diffValue;
        }

    GrayScaleImage diffimg = GrayScaleImage(pixelArray, width, height);
    delete[] pixelArray;
    return diffimg;
}

GrayScaleImage bilinearSampling_CPU(GrayScaleImage& src, double scale)
{
    int width = src.width() * scale;
    int height = src.height() * scale;
    GrayScalePixel* pixelArray = new GrayScalePixel[width * height];
#pragma omp parallel for
    for (int y = 0; y < height; y++)
    {
        float yRatio = (float) y / (float) (height - 1);	// 0 ~ 1
        float yPos = yRatio * (src.height() - 1);		// 0 ~ src.height() - 1
        int up = floor(yPos);					//
        int down = ceil(yPos);
        float downRatio = yPos - up;
        float upRatio = 1.0 - downRatio;

        int rowOffset = y * width;
        for (int x = 0; x < width; x++)
        {
            float xRatio = (float) x / (float) (width - 1);
            float xPos = xRatio * (src.width() - 1);
            int left = floor(xPos);
            int right = ceil(xPos);
            float rightRatio = xPos - left;
            float leftRatio = 1.0 - rightRatio;
            pixelArray[rowOffset + x] = src.getPixel(left, up) * leftRatio * upRatio
                                        + src.getPixel(left, down) * leftRatio * downRatio
                                        + src.getPixel(right, up) * rightRatio * upRatio
                                        + src.getPixel(right, down) * rightRatio * downRatio;
        }
    }

    GrayScaleImage dst = GrayScaleImage(pixelArray, width, height);

    delete[] pixelArray;
    return dst;
}

GrayScaleImage grayscale_CPU(RGBAImage& src)
{
    int width = src.width(), height = src.height();
    int size = width * height;
    GrayScalePixel* pixels = new GrayScalePixel[size];

#pragma omp parallel for
    for (int y=0;y < height;y++)
    {
        for (int x=0; x<width;x++)
        {
            int pixelIdx = y * width + x;
            RGBAPixel p = src.getPixel(x, y);
            pixels[pixelIdx] = Utils::convertToGrayScaleValue(p.r, p.g, p.b);
            pixelIdx ++;
        }
    }
    return GrayScaleImage(pixels, width, height);
}

GrayScaleImage gradientMagnitude_CPU(GrayScaleImage& src)
{
    int width = src.width();
    int height = src.height();

    GrayScalePixel* pixelArray = new GrayScalePixel[width * height];
    for (int y=0;y<height;y++)
    {
        int rowOffset = y * width;
        for (int x=0;x<width;x++)
        {
            GrayScalePixel right = src.getPixel(x+1, y);
            GrayScalePixel down = src.getPixel(x, y+1);
            GrayScalePixel pixel = src.getPixel(x, y);

            double diffx = pixel - right;
            double diffy = pixel - down;
            pixelArray[rowOffset + x] = sqrt(diffx * diffx + diffy * diffy);
        }
    }

    GrayScaleImage grd = GrayScaleImage(pixelArray, width, height);
    delete[] pixelArray;
    return grd;
}

}
