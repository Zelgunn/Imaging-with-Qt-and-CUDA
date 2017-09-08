#ifndef IMAGESIFTF_H
#define IMAGESIFTF_H

#include <QImage>
#include <QTime>
#include <QColor>
#include <QList>
#include <QtCore/qmath.h>
#include "siftdescriptor.h"

#include <QDebug>

#define OCTAV_CNT 3
#define SCALE_CNT 2

class ImageSiftF
{
public:
    ImageSiftF();
    ImageSiftF(const QImage &image, bool processNow = false);

    QImage *baseImage() const;
    void setBaseImage(QImage *baseImage);
    void setBaseImage(float* array, int width, int height);
    void computeKeyPoints();

    static float *imageToArray(const QImage *image);
    static QImage *arrayToImage(float* array, int width, int height);
    static QImage *blurImage(const QImage *image);
    static QImage *subImages(const QImage *image1, const QImage *image2);
    static QImage *imageDoG(const QImage *image);
    QList<QList<QImage *> > computeDoGPyramid();

    QList<SiftDescriptor> descriptors() const;

    QList<QList<QImage *> > LoGPyramid() const;
    QList<QList<QImage *> > DoGPyramid() const;

protected:
    static float grayValue(const QImage *image, int i, int j);
    bool isAPotentialKeyPoint(int i, int j, const QImage *prevImage,
                              const QImage *image, const QImage *nextImage) const;

    void removeLowContrastKeyPoints();
    void removeEdgesKeyPoints();
    void computeKeyPointsOrientation();

private:
    QImage *m_baseImage;
    QList<SiftDescriptor> m_descriptors;
    QList<QList<QImage *> > m_LoGPyramid;
    QList<QList<QImage *> > m_DoGPyramid;
    bool m_hasBeenProcessed;
};

#endif // IMAGESIFTF_H
