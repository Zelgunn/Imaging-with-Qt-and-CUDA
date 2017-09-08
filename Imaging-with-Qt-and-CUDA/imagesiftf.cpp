#include "imagesiftf.h"

ImageSiftF::ImageSiftF()
{
    m_baseImage = Q_NULLPTR;
    m_hasBeenProcessed = false;
}

ImageSiftF::ImageSiftF(const QImage &image, bool processNow)
{
    m_baseImage = new QImage(image);
    m_hasBeenProcessed = false;

    // /!\ TO DO /!\ *
    if(processNow) m_hasBeenProcessed = true;
}

QImage *ImageSiftF::baseImage() const
{
    return m_baseImage;
}

void ImageSiftF::setBaseImage(QImage *baseImage)
{
    m_baseImage = baseImage;
    m_hasBeenProcessed = false;
}

void ImageSiftF::setBaseImage(float *array, int width, int height)
{
    setBaseImage(arrayToImage(array, width, height));
}

void ImageSiftF::computeKeyPoints()
{
    computeDoGPyramid();

    QList<QImage *> tmpOctavList;
    QImage *image, *prevImage, *nextImage;
    int width, height;

    for(int octav=0; octav<OCTAV_CNT; octav++)
    {
        tmpOctavList = m_DoGPyramid.at(octav);
        width = tmpOctavList.first()->width();
        height = tmpOctavList.first()->height();
        for(int scale=0; scale<SCALE_CNT; scale++)
        {
            prevImage   = tmpOctavList.at(scale);
            image       = tmpOctavList.at(scale + 1);
            nextImage   = tmpOctavList.at(scale + 2);
            for(int i=1; i<width-1; i++)
            {
                for(int j=1; j<height-1; j++)
                {
                    // Fouille 3D
                    if(isAPotentialKeyPoint(i,j,prevImage, image, nextImage))
                    {
                        m_descriptors.append(SiftDescriptor(i, j, octav, scale));
                    }
                }
            }
        }
    }

    qDebug() << "Nombre de points clés trouvés\t" << endl << m_descriptors.size();
    removeLowContrastKeyPoints();
    qDebug() << "Nombre de points clés restants après suppression des points à bas contraste\t\t" << endl << m_descriptors.size();
    removeEdgesKeyPoints();
    qDebug() << "Nombre de points clés restants après suppression des points à haut courbure\t\t" << endl << m_descriptors.size();
    computeKeyPointsOrientation();
    qDebug() << "Nombre de points clés restants après création des histogrammes\t\t" << endl << m_descriptors.size();
}

float *ImageSiftF::imageToArray(const QImage *image)
{
    int h = image->height();
    int w = image->width();
    float* res = (float*)malloc(h*w*sizeof(float));

    for(int i=0; i<h; i++)
    {
        for(int j=0; j<w; j++)
        {
            res[j + i*w] = grayValue(image, j, i);
        }
    }

    return res;
}

QImage *ImageSiftF::arrayToImage(float *array, int width, int height)
{
    int v;
    QImage *res = new QImage(width, height, QImage::Format_Grayscale8);

    for(int i=0; i<height; i++)
    {
        for(int j=0; j<width; j++)
        {
            v = (int)(array[i*width+j] * 255.0);
            if(v<0) v = 0;
            if(v>255)v = 255;
            res->setPixel(j,i,qRgb(v,v,v));
        }
    }

    return res;
}

QImage *ImageSiftF::blurImage(const QImage *image)
{
    float *input = imageToArray(image);
    float *output = gaussianBlur(input, image->width(), image->height());

    QImage *res = arrayToImage(output, image->width(), image->height());
    free(input);
    free(output);

    return res;
}

QImage *ImageSiftF::subImages(const QImage *image1, const QImage *image2)
{
    QImage *tmpImage = new QImage(image2->scaled(image1->size()));

    float *input1 = imageToArray(image1);
    float *input2 = imageToArray(tmpImage);
    float *output = pixelSubtraction(input1, input2, image1->width(), image1->height());

    QImage *res = arrayToImage(output, image1->width(), image1->height());
    free(input1);
    free(input2);
    free(output);
    delete tmpImage;

    return res;
}

QImage *ImageSiftF::imageDoG(const QImage *image)
{
    float *input = imageToArray(image);
    float *output = differenceOfGaussian(input, image->width(), image->height());

    QImage *res = arrayToImage(output, image->width(), image->height());
    free(input);
    free(output);

    return res;
}

QList<QList<QImage *> > ImageSiftF::computeDoGPyramid()
{
    if(!m_DoGPyramid.isEmpty()) return m_DoGPyramid;

    QList<QImage *> tmpOctavList, tmpLoGList;
    QImage *tmpImage = new QImage(*m_baseImage);
    *tmpImage = tmpImage->convertToFormat(QImage::Format_Grayscale8);

    int width = m_baseImage->width(), height = m_baseImage->height();
    float *input, *inputPrev, *output, *tmp;
    inputPrev = imageToArray(tmpImage);

    for(int octav=0; octav<OCTAV_CNT; octav++)
    {
        for(int scale=0; scale<SCALE_CNT+2; scale++)
        {
            input = gaussianBlur(inputPrev, width, height);
            output = pixelSubtraction(inputPrev, input, width, height);
            // Sauvegarde
            tmpLoGList.append(arrayToImage(inputPrev, width, height));
            tmpOctavList.append(arrayToImage(output, width, height));
            free(output);
            // Passage
            free(inputPrev);
            inputPrev = input;
            // Preparation au sous-sampling
            if(scale == 1)
            {
                tmp = underSampling(input, width, height, 2.0);
            }
        }
        tmpLoGList.append(arrayToImage(input, width, height));
        m_LoGPyramid.append(tmpLoGList);
        m_DoGPyramid.append(tmpOctavList);

        // Passage au prochain octave
        if(octav < OCTAV_CNT -1)
        {
            tmpOctavList.clear();
            tmpLoGList.clear();
            width /= 2; height /= 2;
            free(inputPrev);            // inputPrev = input
            inputPrev = tmp;
        }
    }

    return m_DoGPyramid;
}

QList<SiftDescriptor> ImageSiftF::descriptors() const
{
    return m_descriptors;
}

float ImageSiftF::grayValue(const QImage *image, int i, int j)
{
    Q_ASSERT_X(image!=Q_NULLPTR, "grayValue","null qImage ptr");
    Q_ASSERT_X((i>=0)&&(j>=0)&&(i<image->width())&&(j<image->height()), "grayValue", "i,j : valeur(s) incorrecte(s)");

    QColor value(image->pixel(i,j));
    return (float)(value.blue() + value.green() + value.red())/765.0;
}

bool ImageSiftF::isAPotentialKeyPoint(int i, int j, const QImage *prevImage, const QImage *image, const QImage *nextImage) const
{
    Q_ASSERT_X(prevImage!=Q_NULLPTR, "isAPotentialKeyPoint", "null ptr : prevImage");
    Q_ASSERT_X(image!=Q_NULLPTR, "isAPotentialKeyPoint", "null ptr : image");
    Q_ASSERT_X(nextImage!=Q_NULLPTR, "isAPotentialKeyPoint", "null ptr : nextImage");

    int width = image->width(), height = image->height();
    Q_ASSERT_X((i>=0)&&(j>=0)&&(i<width)&&(j<height), "isAPotentialKeyPoint", "i,j : incorrect value");

    if((i<=0)||(i>=width-1)||(j<=0)||(j>=height-1))
        return false;

    float value = grayValue(image, i, j), tmp;
    const QImage *tmpImage;

    bool isHighest = true, isLowest = true;

    for(int z=0; z<3; z++)
    {
        switch (z) {
        case 0:
            tmpImage = prevImage;
            break;
        case 1:
            tmpImage = image;
            break;
        default:
            tmpImage = nextImage;
            break;
        }

        for(int y=-1; y<2; y++)
        {
            for(int x=-1; x<2; x++)
            {
                if(!((z==1)&&(y==0)&&(x==0)))
                {
                    tmp = grayValue(tmpImage, i+x, y+j);
                    if(tmp >= value)
                        isHighest = false;
                    if(tmp <= value)
                        isLowest = false;
                    if(!(isHighest||isLowest))
                        return false;
                }
            }
        }
    }
    return true;
}


void ImageSiftF::removeLowContrastKeyPoints()
{

    for(int i=0; i<m_descriptors.size(); i++)
    {
        SiftDescriptor tmp = m_descriptors.at(i);
        if(grayValue(m_baseImage, tmp.x(), tmp.y())<=0.03)
        {
            m_descriptors.removeAt(i);
            i--;
        }
    }
}


void ImageSiftF::removeEdgesKeyPoints()
{
    const int radius = 2;

    double dxx, dxy, dyy, dbase, value, trace, determinant;
    double sigma = 1.41421356237;
    double sigma2 = 2*qPow(sigma, 2);
    double pisigma4 = 2*PI*qPow(sigma, 4);
    double pi2sigma8 = 4*PI*PI*qPow(sigma, 8);//2*PI*qPow(sigma, 6);
    double scaleRatio;
    int x,y,xi,yj;
    SiftDescriptor keypoint;
    QImage *image;
    QList<SiftDescriptor> res;

    for(int i=0; i<m_descriptors.size(); i++)
    {
        keypoint = m_descriptors.at(i);
        image = m_DoGPyramid.at(keypoint.octav()).at(keypoint.scale());
        scaleRatio = 1/qPow(2, keypoint.scale());
        dxx = 0.0;
        dxy = 0.0;
        dyy = 0.0;
        trace = 0.0;
        determinant = 0.0;

        x = keypoint.x();
        y = keypoint.y();

        sigma = keypoint.scaleF();
        sigma2 = 2*qPow(sigma, 2);
        pisigma4 = 2*PI*qPow(sigma, 4);
        pi2sigma8 = 4*PI*PI*qPow(sigma, 8);
        for(int i=-radius; i<radius; i++)
        {
            xi = x + i;
            for(int j=-radius; j<radius; j++)
            {
                yj = y + j;
                if((xi>=0)&&(yj>=0)&&(xi<image->width())&&(yj<image->height()))
                {
                    dbase = qExp(-(i*i + j*j)/(sigma2));
                    value = grayValue(image, xi, yj)*dbase;
                    dxx = (i*i - pisigma4)*value/pi2sigma8;
                    dyy = (j*j - pisigma4)*value/pi2sigma8;
                    dxy = (i*j)/(pi2sigma8)*value;
                    trace += dxx + dyy;
                    determinant += dxx*dyy - dxy*dxy;
                }
            }
        }
        if((determinant > 0)&&((trace*trace/determinant) <= 12.1))
            res.append(keypoint);
    }
    m_descriptors = res;
}

void ImageSiftF::computeKeyPointsOrientation()
{
    double magnitude, orientation, deltax, deltay;
    int x, xi, y, yj, radius;
    QList<SiftDescriptor> updatedDescriptors;
    SiftDescriptor descriptor;
    QImage *image;

    for(int d=0; d<m_descriptors.size(); d++)
    {
        descriptor = m_descriptors.at(d);
        x = descriptor.xInScale();
        y = descriptor.yInScale();
        image = m_LoGPyramid.at(descriptor.octav()).at(descriptor.scale());
        radius = (int)(descriptor.scaleF() * 1.5);

        for(int i=-radius; i <= radius; i++)
        {
            xi = x + i;
            if((xi > 0)&&(xi < image->width() - 1))
            {
                for(int j=-radius; j <= radius; j++)
                {
                    yj = y + j;
                    if((yj > 0)&&(yj < image->height() - 1))
                    {
                        deltax = grayValue(image, xi + 1, yj) - grayValue(image, xi - 1, yj);
                        deltay = grayValue(image, xi, yj + 1) - grayValue(image, xi, yj - 1);

                        magnitude = qSqrt(qPow(deltax, 2) + qPow(deltay, 2));
                        if(deltax != 0)
                            orientation = qAtan(deltay/deltax);
                        else
                            orientation = PI/2;

                        if(deltay < 0)
                            orientation += PI;

                        if(orientation < 0)
                            orientation += 2*PI;

                        descriptor.addMagnitudeToHistogram(magnitude, orientation);
                    }
                }
            }
        }
        updatedDescriptors.append(descriptor);
    }
    m_descriptors = updatedDescriptors;
}

QList<QList<QImage *> > ImageSiftF::DoGPyramid() const
{
    return m_DoGPyramid;
}

QList<QList<QImage *> > ImageSiftF::LoGPyramid() const
{
    return m_LoGPyramid;
}

