//
// RenderState.cpp
// Implementation of RenderState.
//

#include "RenderState.h"

#include <QImage>
#include <QPixmap>

#include <algorithm>

// 
// Public
//
RenderState::RenderState() :
	m_imageRendererNeedsUpdate(false),
	m_contourRendererNeedsUpdate(false),
	m_activeCurveRendererNeedsUpdate(false),
	m_allNeedUpdate(true),
	m_contourColor(Qt::lightGray),
	m_activeCurveColor(Qt::white),
	m_isContourVisible(true),
	m_maxPointSpacingInWindowPixels(1),
	m_imageMinValue(0),
	m_imageMaxValue(1),
	m_windowingWidth(1),
	m_windowingLevel(0.5),
	m_interpolateImage(false),
	m_winWidth(1),
	m_winHeight(1),
	m_viewportToWindowScale(1),
	m_windowToWorldScale(1),
	m_windowToImageScale(1)
{
}
RenderState::~RenderState()
{
}

void RenderState::setWindowingRange(float normalizedMinValue, float normalizedMaxValue)
{
	// Normalized values represent min and max image values scaled to [0,1], where 0
	// represents an intensity of 0 and 1 represents the max intensity for the image 
	// format (e.g., 255 for 8-bit unsigned chars).
	if (normalizedMinValue > normalizedMaxValue) {
		std::swap(normalizedMinValue, normalizedMaxValue);
	}
	m_imageMinValue = std::max(0.0f, normalizedMinValue);
	m_imageMaxValue = std::min(1.0f, normalizedMaxValue);
}
void RenderState::setDefaultWindowing() {
	m_windowingLevel = 0.5 * (double(m_imageMinValue) + double(m_imageMaxValue));
	m_windowingWidth = double(m_imageMaxValue) - double(m_imageMinValue);
}
void RenderState::incrementWindowing(float brightnessIncr, float contrastIncr)
{
	float levelIncr = 0.5f * (m_imageMaxValue - m_imageMinValue);
	m_windowingLevel += levelIncr * brightnessIncr;
	m_windowingWidth -= levelIncr * contrastIncr;

	m_windowingWidth = std::max(0.0f, m_windowingWidth);
	if ((m_windowingLevel - 0.5 * m_windowingWidth) < m_imageMinValue) {
		m_windowingWidth = m_windowingLevel + 0.5 * m_windowingWidth - m_imageMinValue;
		m_windowingLevel = m_imageMinValue + 0.5 * m_windowingWidth;
	}
	if ((m_windowingLevel + 0.5 * m_windowingWidth) > m_imageMaxValue) {
		m_windowingWidth = m_imageMaxValue - (m_windowingLevel - 0.5 * m_windowingWidth);
		m_windowingLevel = m_imageMaxValue - 0.5 * m_windowingWidth;
	}
}

QMatrix4x4 RenderState::worldToView() const
{
	return m_worldToView;
}
void RenderState::setWorldToView(QMatrix4x4 worldToView)
{
	// Set worldToView and dependent transforms
	m_worldToView = worldToView;
	m_imageToView = m_worldToView * m_imageToWorld;
	setWorldToViewport(m_viewToViewport * m_worldToView);
}
QVector3D RenderState::convertWindowToWorld(QVector3D windowPoint)
{
	return m_windowToWorld * windowPoint;
}
void RenderState::resetWorldToView()
{
	QMatrix4x4 identity;
	setWorldToView(identity);
}
void RenderState::centerImageInViewport(int w, int h)
{
	float maxD = (w > h) ? w : h;
	maxD = std::max(1.0f, maxD);
	m_imageToWorld.setToIdentity();
	m_imageToWorld.scale(2.0 / maxD, -2.0 / maxD, 1.0);
	m_imageToWorld.translate(-0.5 * w, -0.5 * h, 0.0);
	resetWorldToView(); 
}
void RenderState::resetProjectionMatrix(int w, int h)
{
	m_winWidth = w;
	m_winHeight = h;
	float maxD = (w > h) ? w : h;
	maxD = std::max(1.0f, maxD);
	float W = w / maxD;
	float H = h / maxD;
	m_viewToViewport.setToIdentity();
	m_viewToViewport.ortho(-W, W, -H, H, -1.0, 1.0);

	m_viewportToWindow.setToIdentity();
	m_viewportToWindow.translate(0.5 * w, 0.5 * h, 0);
	m_viewportToWindowScale = 0.5 * maxD;
	m_viewportToWindow.scale(m_viewportToWindowScale, -m_viewportToWindowScale, 1);

	setWorldToViewport(m_viewToViewport * m_worldToView);
}
QVector3D RenderState::convertWindowToImage(QVector3D windowPoint)
{
	return windowPoint.unproject(m_imageToView, m_viewToViewport, 
		QRect(0, 0, m_winWidth, m_winHeight));
}
QMatrix4x4 RenderState::mvpMatrix() const
{
	return m_imageToViewport;
}


// 
// Private
//
// Transforms
void RenderState::setWorldToViewport(QMatrix4x4 transform)
{
	// Set worldToViewport and dependent transforms
	m_worldToViewport = transform;
	m_windowToWorld = (m_viewportToWindow * m_worldToViewport).inverted();
	m_windowToWorldScale = getScale(m_windowToWorld);

	// Model view projection matrix
	m_imageToViewport = m_worldToViewport * m_imageToWorld;
	float imageToViewportScale = getScale(m_imageToViewport);
	float imageToWindowScale = m_viewportToWindowScale * imageToViewportScale;
	m_windowToImageScale = 1.0f / std::max(std::numeric_limits<float>::epsilon(), imageToWindowScale);
}

float RenderState::getScale(QMatrix4x4 transform)
{
	// This assumes that the matrix was composed from uniform scaling, rotation and 
	// translation only
	float scaleSqr = fabs(transform.determinant());
	return (sqrt(scaleSqr));
}
