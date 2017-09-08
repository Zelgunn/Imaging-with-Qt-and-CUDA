#ifndef SIFTDESCRIPTOR_H
#define SIFTDESCRIPTOR_H

#include "kernel.cuh"
#include "QtCore/qmath.h"

#include <QDebug>

#define SIGMA 1.41421356237
#define PI 3.14159265358979

class SiftDescriptor
{
public:
    SiftDescriptor();
    SiftDescriptor(int x, int y, int octav, int scale);

    int x() const;
    void setX(int x);

    int y() const;
    void setY(int y);

    int octav() const;
    void setOctav(int octav);

    int scale() const;
    double scaleF() const;
    void setScale(int scale);

    void addMagnitudeToHistogram(double magnitude, double angle);

    int xInScale() const;
    int yInScale() const;
    double histogrammeAt(int i) const;

private:
    int m_x;
    int m_xInScale;
    int m_y;
    int m_yInScale;
    int m_octav;
    int m_scale;
    double m_histogramme[36];
};

#endif // SIFTDESCRIPTOR_H
