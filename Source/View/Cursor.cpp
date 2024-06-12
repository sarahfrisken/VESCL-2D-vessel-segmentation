//
// Cursor.cpp
// Implementation of Cursor.
//

#include "Cursor.h"

#include <QImage>
#include <QPixmap>

// 
// Public
//
Cursor::Cursor() :
	m_color(Qt::lightGray),
	m_radius(4),
	m_pixmapSize(32),
	m_cursor(nullptr)
{
	init();
	update();
}
Cursor::~Cursor()
{
	delete m_cursor;
}

void Cursor::init()
{
	m_color = Qt::lightGray;
	m_radius = 4.0;
	update();
}
void Cursor::setColor(QColor color)
{
	m_color = color;
	update();
};
void Cursor::setRadius(float radius)
{
	float minRadius = 0.25;
	float maxRadius = 0.5 * m_pixmapSize - 1.0;
	m_radius = (radius < minRadius) ? minRadius : (radius > maxRadius) ? maxRadius : radius;
	update();
}

//
// Private
//
void Cursor::update()
{
	// Set the pixmap image
	float radius = m_radius;
	int size = m_pixmapSize;
	int centerX = size / 2;
	int centerY = size / 2;
	float lineRadius = 1.5;
	float filterWidth = 0.8;
	float filteredLineRadius = lineRadius + filterWidth;
	if (radius > size / 2) radius = size / 2;
	float minRad = (radius > filteredLineRadius) ? radius - filteredLineRadius : 0;
	float minRadSqr = minRad * minRad;
	float maxRadSqr = (radius + filteredLineRadius) * (radius + filteredLineRadius);
	QImage image(size, size, QImage::Format_ARGB32);
	for (int j = 0; j < size; j++) {
		QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(j));
		for (int i = 0; i < size; i++) {
			float distSqr = (centerX - i) * (centerX - i) + (centerY - j) * (centerY - j);
			double fDensity = 0.0;
			if ((distSqr >= minRadSqr) && (distSqr < maxRadSqr)) {
				fDensity = 1.0 - fabs((sqrt(distSqr) - (double)radius)) / filteredLineRadius;
				fDensity = std::min(1.0, std::max(0.0, fDensity));
			}

			uchar redF = (uchar)(m_color.red() * fDensity);
			uchar greenF = (uchar)(m_color.green() * fDensity);
			uchar blueF = (uchar)(m_color.blue() * fDensity);
			uchar alphaF = (uchar)(255.0 * fDensity);
			QRgb& rgb = line[i];
			rgb = qRgba(redF, greenF, blueF, alphaF);
		}
	}

	// Create the new cursor
	QPixmap pixmap(QPixmap::fromImage(image));
	if (m_cursor) delete m_cursor;
	m_cursor = new QCursor(pixmap, centerX, centerY);
}