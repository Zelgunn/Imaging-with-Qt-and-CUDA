#include "siftdescriptor.h"

SiftDescriptor::SiftDescriptor()
{
    // UNVALID CONSTRUCTOR
}

SiftDescriptor::SiftDescriptor(int x, int y, int octav, int scale)
{
    m_octav = octav;
    m_scale = scale;
    m_xInScale = x;
    m_yInScale = y;
    m_x = x * qPow(2, octav);
    m_y = y * qPow(2, octav);

    for(int i=0; i<36; i++)
        m_histogramme[i] = 0;
}
int SiftDescriptor::x() const
{
    return m_x;
}

void SiftDescriptor::setX(int x)
{
    m_x = x;
}
int SiftDescriptor::y() const
{
    return m_y;
}

void SiftDescriptor::setY(int y)
{
    m_y = y;
}
int SiftDescriptor::octav() const
{
    return m_octav;
}

void SiftDescriptor::setOctav(int octav)
{
    m_octav = octav;
}
int SiftDescriptor::scale() const
{
    return m_scale;
}

double SiftDescriptor::scaleF() const
{
    double scale = qPow(SIGMA, m_scale);
    scale *= qPow(2*SIGMA, m_octav);

    return scale;
}

void SiftDescriptor::setScale(int scale)
{
    m_scale = scale;
}

void SiftDescriptor::addMagnitudeToHistogram(double magnitude, double angle)
{
    int bin;
    bin = (int)((angle/(2 * PI)) * 36);

    m_histogramme[bin] += magnitude;
}

int SiftDescriptor::xInScale() const
{
    return m_xInScale;
}

int SiftDescriptor::yInScale() const
{
    return m_yInScale;
}

double SiftDescriptor::histogrammeAt(int i) const
{
    return m_histogramme[i];
}





