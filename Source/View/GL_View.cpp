//
// GL_View.cpp
// Implementation of GL_View.
//

#include "GL_View.h"
#include "GL_BltRenderer.h"
#include "GL_ImageRenderer.h"
#include "GL_ContourRenderer.h"
#include "../Model/Model.h"
#include "../Controller/RenderState.h"

#include <QMouseEvent>
#include <QScreen>
#include <QVector3D>

// 
// Public
//
GL_View::GL_View(Model* model, RenderState* renderState) :
	m_model(model),
	m_renderState(renderState),
	m_currentAction(MouseAction::None),
	m_mouseDownWinLevel(0.5), 
	m_mouseDownWinWidth(1),
	m_mouseDownCursorSize(8),
	m_bltRenderer(nullptr),
	m_contourRenderer(nullptr),
	m_activeCurveRenderer(nullptr),
	m_imageRenderer(nullptr)
{
}
GL_View::~GL_View()
{
	delete m_imageRenderer;
	delete m_contourRenderer;
	delete m_activeCurveRenderer;
	delete m_bltRenderer;
}

void GL_View::setImage(const Image& image)
{
	m_imageRenderer->setImage(image);
}

void GL_View::initCursor()
{
	m_cursor.init();
}
float GL_View::cursorRadius() const
{
	return m_cursor.radius();
}
void GL_View::setCursorRadius(float radius)
{
	m_cursor.setRadius(radius);
	setCursor(*m_cursor.qCursor());
}
void GL_View::setCursorColor(QColor color)
{
	m_cursor.setColor(color);
	setCursor(*m_cursor.qCursor());
}

// 
// Protected
//
void GL_View::mousePressEvent(QMouseEvent* e)
{
	if (e->buttons() & Qt::LeftButton && e->modifiers()) {

		// Modify view transforms
		m_mouseDownPos = QVector2D(e->pos());
		QVector3D worldPoint = m_renderState->convertWindowToWorld(QVector3D(m_mouseDownPos, 0));
		m_mouseDownWorld = QVector2D(worldPoint);
		m_mouseDownWorldToView = m_renderState->worldToView();
		m_mouseDownWinLevel = m_renderState->windowingLevel();
		m_mouseDownWinWidth = m_renderState->windowingWidth();
		m_mouseDownCursorSize = m_cursor.radius();
		if (e->modifiers() & Qt::ShiftModifier) {
			m_currentAction = MouseAction::Pan;
		}
		else if (e->modifiers() & Qt::ControlModifier) {
			m_currentAction = MouseAction::Zoom;
		}
		else if (e->modifiers() & Qt::AltModifier) {
			m_currentAction = MouseAction::Windowing;
		}
	}
	else {

		// Interact with model
		if (e->buttons() & Qt::LeftButton) {
			m_currentAction = MouseAction::Draw;
			QSize windowSize = size();
			QVector3D pWindow((float)e->pos().x(), (float)(windowSize.height() - e->pos().y()), 0);
			QVector3D pImage = m_renderState->convertWindowToImage(pWindow);
			Math::Vec2D p(pImage[0], pImage[1]);
			float curveRadius = m_cursor.radius() * m_renderState->windowToContourScale();
			m_model->startDraw(CurvePoint(p, curveRadius));
			update();
		}
		else if (e->buttons() & Qt::RightButton) {
			m_currentAction = MouseAction::None;
			QSize windowSize = size();
			QVector3D pWindow((float)e->pos().x(), (float)(windowSize.height() - e->pos().y()), 0);
			QVector3D pImage = m_renderState->convertWindowToImage(pWindow);
			float p[2] = { pImage[0], pImage[1] };
			m_model->select(p);
			update();
		}
	}
}
void GL_View::mouseReleaseEvent(QMouseEvent* e)
{
	if (m_currentAction == MouseAction::Draw) {
		QSize windowSize = size();
		QVector3D pWindow((float)e->pos().x(), (float)(windowSize.height() - e->pos().y()), 0);
		QVector3D pImage = m_renderState->convertWindowToImage(pWindow);
		Math::Vec2D p(pImage[0], pImage[1]);
		float curveRadius = m_cursor.radius() * m_renderState->windowToContourScale();
		m_model->endDraw(CurvePoint(p, curveRadius));
		update();
	}
	m_currentAction = MouseAction::None;
}
void GL_View::mouseMoveEvent(QMouseEvent* e)
{
	switch (m_currentAction) {
	case MouseAction::Draw:
	{
		QSize windowSize = size();
		QVector3D pWindow((float)e->pos().x(), (float)(windowSize.height() - e->pos().y()), 0);
		QVector3D pImage = m_renderState->convertWindowToImage(pWindow);
		Math::Vec2D p(pImage[0], pImage[1]);
		float curveRadius = m_cursor.radius() * m_renderState->windowToContourScale();
		m_model->updateDraw(CurvePoint(p, curveRadius));
		update();
		break;
	}
	case MouseAction::Pan:
	{
		int dx = e->x() - m_mouseDownPos[0];
		int dy = m_mouseDownPos[1] - e->y();
		float x = dx * m_renderState->windowToWorldScale();
		float y = dy * m_renderState->windowToWorldScale();
		QMatrix4x4 worldToView = m_mouseDownWorldToView;
		worldToView.translate(x, y, 0);
		m_renderState->setWorldToView(worldToView);
		m_renderState->setNeedsFullUpdate(true);
		update();
		break;
	}
	case MouseAction::Zoom:
	{
		// Zoom about the mouse down point
		QSize windowSize = size();
		float minDim = windowSize.width() < windowSize.height() ? (float)windowSize.width() : (float)windowSize.height();
		float dy = e->y() - m_mouseDownPos[1];
		float scale = 1.0 - (float)dy / (float)(std::max(1, windowSize.height()));
		float minScale = 0.1f;
		float maxScale = 100.0f;
		if (scale < minScale) scale = minScale;
		if (scale > maxScale) scale = maxScale;
		QMatrix4x4 worldToView = m_mouseDownWorldToView;
		worldToView.translate(m_mouseDownWorld[0], m_mouseDownWorld[1], 0);
		worldToView.scale(scale, scale, 1);
		worldToView.translate(-m_mouseDownWorld[0], -m_mouseDownWorld[1], 0);
		m_renderState->setWorldToView(worldToView);
		m_renderState->setNeedsFullUpdate(true);
		m_cursor.setRadius(m_mouseDownCursorSize * scale);
		setCursor(*m_cursor.qCursor());
		update();
		break;
	}
	case MouseAction::Windowing:
	{
		// Change windowing about the mouse down point
		QSize windowSize = size();
		float minDim = windowSize.width() < windowSize.height() ? (float)windowSize.width() : (float)windowSize.height();
		int dx = e->x() - m_mouseDownPos[0];
		int dy = e->y() - m_mouseDownPos[1];
		float contrastIncr = float(dx) / (float)(std::max(1, windowSize.width()));
		float brightnessIncr = float(dy) / (float)(std::max(1, windowSize.height()));
		m_renderState->setWindowing(m_mouseDownWinLevel, m_mouseDownWinWidth);
		m_renderState->incrementWindowing(brightnessIncr, contrastIncr);
		m_renderState->setImageNeedsUpdate(true);
		update();
		break;
	}
	}
}
void GL_View::wheelEvent(QWheelEvent* e)
{
	QPoint numPixels = e->pixelDelta();
	QPoint numDegrees = e->angleDelta() / 8;
	int numSteps = 0;
	if (!numPixels.isNull()) {
		numSteps = numPixels.y();
	}
	else if (!numDegrees.isNull()) {
		numSteps = numDegrees.y() / 15;
	}
	if (!numSteps) return;
	m_cursor.setRadius(m_cursor.radius() + numSteps);
	setCursor(*m_cursor.qCursor());
}

void GL_View::initializeGL()
{
	makeCurrent();

	// Set up OpenGL
	initializeOpenGLFunctions();

	// Set the projection matrix
	QSize glWindowSize = size();
	resizeGL(glWindowSize.width(), glWindowSize.height());

	// Set up the renderers when the context is set and before paintGL is called
	m_bltRenderer = new GL_BltRenderer;
	m_imageRenderer = new GL_ImageRenderer;
	GL_ContourRenderer::RendererType type = GL_ContourRenderer::RendererType::Antialiased;
	m_contourRenderer = new GL_ContourRenderer(type);
	m_activeCurveRenderer = new GL_ContourRenderer(type);

	// Initialize the world to view transform
	m_renderState->resetWorldToView();
	m_renderState->setNeedsFullUpdate(true);

	// Initialize the cursor
	m_cursor.init();
	setCursor(*m_cursor.qCursor());
}
void GL_View::resizeGL(int w, int h)
{
	m_renderState->resetProjectionMatrix(w, h);
	m_renderState->setNeedsFullUpdate(true);
	update();
}
void GL_View::paintGL()
{
	// Clear the viewport
	makeCurrent();
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	QSize glViewportSize = this->size();
	//if (QT_VERSION < 0x060000) {   // Handle high def displays
		glViewportSize *= screen()->devicePixelRatio();
	//}

	// Render the image if required and blt to screen
	GLuint textureID;
	if (m_renderState->imageNeedsUpdate() || m_renderState->needsFullUpdate()) {
		m_imageRenderer->update(glViewportSize.width(), glViewportSize.height(), m_renderState);
		m_renderState->setImageNeedsUpdate(false);
	}
	if (m_imageRenderer->textureID(&textureID)) {
		if (m_bltRenderer) m_bltRenderer->render(textureID, 1);
	}

	// Render the contour if required and blt to screen
	if (m_renderState->isContourVisible())
	{
		std::list<Curve*>* curves = m_model->contour()->curves();
		int idActiveCurve = m_model->contour()->idActiveCurve();
		if (m_renderState->contourNeedsUpdate() || m_renderState->needsFullUpdate()) {

			// Get curve points to render
			std::list<Curve::PointVector> curvePoints;
			for (std::list<Curve*>::iterator it = curves->begin(); it != curves->end(); it++) {
				if ((*it)->id() == idActiveCurve) continue;
				float maxPointSpacing = m_renderState->maxPointSpacing() * m_renderState->windowToContourScale();
				Curve::PointVector pv = (*it)->getSampledCurvePoints(maxPointSpacing);
				curvePoints.push_back(pv);
			}

			m_contourRenderer->update(curvePoints, m_renderState->contourColor(),
				glViewportSize.width(), glViewportSize.height(),
				m_renderState->mvpMatrix(), m_renderState->windowToContourScale());
		}
		if (m_contourRenderer->textureID(&textureID)) {
			if (m_bltRenderer) m_bltRenderer->render(textureID, 1);
		}

		// Render the active curve if required and blt to screen
		if (m_renderState->activeCurveNeedsUpdate() ||
			m_renderState->contourNeedsUpdate() || m_renderState->needsFullUpdate()) {

			// Get curve points to render
			std::list<Curve::PointVector> activeCurvePoints;
			for (std::list<Curve*>::iterator it = curves->begin(); it != curves->end(); it++) {
				if ((*it)->id() == idActiveCurve) {
					float maxPointSpacing = m_renderState->maxPointSpacing() * m_renderState->windowToContourScale();
					Curve::PointVector pv = (*it)->getSampledCurvePoints(maxPointSpacing);
					activeCurvePoints.push_back(pv);
					break;
				}
			}

			m_activeCurveRenderer->update(activeCurvePoints, m_renderState->activeCurveColor(),
				glViewportSize.width(), glViewportSize.height(),
				m_renderState->mvpMatrix(), m_renderState->windowToContourScale());
		}
		if (m_activeCurveRenderer->textureID(&textureID)) {
			if (m_bltRenderer) m_bltRenderer->render(textureID, 1);
		}

		m_renderState->setActiveCurveNeedsUpdate(false);
		m_renderState->setContourNeedsUpdate(false);
	}
	m_renderState->setNeedsFullUpdate(false);
}
