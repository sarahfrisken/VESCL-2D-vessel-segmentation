//
// GL_ContourRenderer.cpp
// Implementation of GL_ContourRenderer.
//

#include "GL_ContourRenderer.h"
#include "../Model/Contour.h"
#include "../Model/Curve.h"
#include "../Model/CurvePoint.h"

#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>

#include <limits>
#include <assert.h>

// 
// Public
//
GL_ContourRenderer::GL_ContourRenderer(RendererType type) :
	m_type(type),
	m_antialiasingFilterWidth(0.8),
	m_centerlineRadius(2),
	m_maxDistFieldDistance(10),
	m_fboWidth(0),
	m_fboHeight(0),
	m_numVertices(0),
	m_fbo(nullptr),
	m_shaderProgram(nullptr)
{
	const char* vertexShader;
	const char* fragmentShader;
	switch (m_type) {
	case RendererType::Centerline:
		vertexShader = vertexShader_contourCenterline;
		fragmentShader = fragmentShader_contourCenterline;
		break;
	case RendererType::Binary:
		vertexShader = vertexShader_binaryContour;
		fragmentShader = fragmentShader_binaryContour;
		break;
	case RendererType::DistToContour:
		vertexShader = vertexShader_distToContour;
		fragmentShader = fragmentShader_distToContour;
		break;
	case RendererType::DistToCenterline:
		vertexShader = vertexShader_distToCenterline;
		fragmentShader = fragmentShader_distToCenterline;
		break;
	case RendererType::Antialiased:
	case RendererType::Default:
	default:
		vertexShader = vertexShader_antialiasedContour;
		fragmentShader = fragmentShader_antialiasedContour;
		break;
	}

	// Set up the shader programs
	try {
		m_shaderProgram = new QOpenGLShaderProgram;
		if (!m_shaderProgram) {
			throw std::runtime_error("Can't create GL shader program.");
		}
		m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
		m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);
		if (!m_shaderProgram->link()) {
			throw std::runtime_error("GL shader program failed to link.");
		}
	}
	catch (const std::exception& e) {
		std::cout << "Exception " << e.what() << endl;
		delete m_shaderProgram;
		m_shaderProgram = nullptr;
		return;
	}

	// Set up OpenGL
	initializeOpenGLFunctions();
}
GL_ContourRenderer::~GL_ContourRenderer()
{
	m_vertexBuffer.destroy();
	delete m_shaderProgram;
	delete m_fbo;
}

// Get the texture ID of the fbo's color buffer. Return's false if the fbo is invalid.
bool GL_ContourRenderer::textureID(GLuint* textureID)
{
	if (m_fbo) {
		*textureID = m_fbo->texture();
		return true;
	}
	return false;
}
QImage GL_ContourRenderer::renderedImage()
{
	return(m_fbo->toImage(true));
}

// Updates the fbo's color buffer 
void GL_ContourRenderer::update(std::list<Curve::PointVector> curvePoints, QColor contourColor,
	int winWidth, int winHeight, QMatrix4x4 mvpMatrix, float windowToContourScale)
{
	// Update the FBO if the requested size has changed
	if (winWidth != m_fboWidth || winHeight != m_fboHeight) {
		delete m_fbo;
		m_fboWidth = winWidth;
		m_fboHeight = winHeight;
		m_fbo = new QOpenGLFramebufferObject(m_fboWidth, m_fboHeight, GL_TEXTURE_2D);
	}

	m_fbo->bind();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Check for required data
	if (!m_shaderProgram) return;

	// Create the contour geometry
	setVertexBuffer(curvePoints, windowToContourScale);
	if (m_numVertices == 0) return;

	// Set the model view projection matrix
	m_shaderProgram->bind();
	int mvpLocation = m_shaderProgram->uniformLocation("u_mvpMatrix");
	assert(mvpLocation != -1);
	m_shaderProgram->setUniformValue(mvpLocation, mvpMatrix);

	// Set shader-specific data
	setShaderData(contourColor, windowToContourScale);

	// Tell OpenGL programmable pipeline how to locate the vertex data
	m_vertexBuffer.bind();
	quintptr offset = 0;
	int numFloatsPerPos = 2;
	int numFloatsPerData = 3;
	int stride = (numFloatsPerPos + numFloatsPerData) * sizeof(float);

	int posLocation = m_shaderProgram->attributeLocation("a_position");
	assert(posLocation != -1);
	m_shaderProgram->enableAttributeArray(posLocation);
	m_shaderProgram->setAttributeBuffer(posLocation, GL_FLOAT, offset, numFloatsPerPos, stride);

	offset += numFloatsPerPos * sizeof(float);
	int dataLocation = m_shaderProgram->attributeLocation("a_data");
	assert(dataLocation != -1);
	m_shaderProgram->enableAttributeArray(dataLocation);
	m_shaderProgram->setAttributeBuffer(dataLocation, GL_FLOAT, offset, numFloatsPerData, stride);

	// Perform the rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_ALPHA, GL_SRC_ALPHA);
	glBlendEquation(GL_MAX);
	glDrawArrays(GL_TRIANGLES, 0, m_numVertices);
	glDisable(GL_BLEND);
	m_vertexBuffer.release();
	m_shaderProgram->release();
	m_fbo->release();
}

// 
// Private
//
void GL_ContourRenderer::setShaderData(QColor contourColor, float windowToContourScale) {
	float filterWidth = m_antialiasingFilterWidth * windowToContourScale;
	switch (m_type) {
	case RendererType::Centerline:
	{
		m_shaderProgram->setUniformValue("u_color", contourColor);
		m_shaderProgram->setUniformValue("u_filterWidth", filterWidth);
		float centerlineRadius = m_centerlineRadius * windowToContourScale;
		m_shaderProgram->setUniformValue("u_radius", centerlineRadius);
		break;
	}
	case RendererType::Binary:
	{
		m_shaderProgram->setUniformValue("u_color", contourColor);
		break;
	}
	case RendererType::DistToContour:
	case RendererType::DistToCenterline:
	{
		m_shaderProgram->setUniformValue("u_color", contourColor);
		m_shaderProgram->setUniformValue("u_maxDist", m_maxDistFieldDistance);
		break;
	}
	case RendererType::Antialiased:
	case RendererType::Default:
	default:
	{
		m_shaderProgram->setUniformValue("u_color", contourColor);
		m_shaderProgram->setUniformValue("u_filterWidth", filterWidth);
		break;
	}
	}
}
void GL_ContourRenderer::setVertexBuffer(std::list<Curve::PointVector> curvePoints, 
	float windowToContourScale)
{
	// Make sure there is something to render
	m_numVertices = 0;
	if (curvePoints.size() == 0) return;

	// Determine render type dependent constants
	float radiusOffset = 0;
	switch (m_type) {
	case RendererType::Binary:
	{
		break;
	}
	case RendererType::DistToContour:
	case RendererType::DistToCenterline:
	{
		radiusOffset += m_maxDistFieldDistance;
		break;
	}
	case RendererType::Antialiased:
	case RendererType::Centerline:
	case RendererType::Default:
	default:
	{
		float filterWidth = m_antialiasingFilterWidth * windowToContourScale;
		radiusOffset += filterWidth;
		break;
	}
	}

	// Get the number of curve points for buffer allocation
	int numPoints = 0;
	for (std::list<Curve::PointVector>::iterator it = curvePoints.begin(); it != curvePoints.end(); it++) {
		numPoints += it->size();
	}

	// Allocate space for cell vertices. Each vertex has 5 components, (x, y, dx, dy, r), 
	// where (x, y) is the position of the vertex, (dx, dy) is the vector distance from 
	// the vertex to the stroke boundary and r is the stroke radius. Points and vertices 
	// are specified in contour coordinates. Vertex data is packed into a contiguous array. 
	// A curve is rendered as a set of line and corner cells. Corner cells are constructed
	// at each point along the curve. Line cells are constructed between each pair of 
	// points along the curve. Line and corner cells each have 2 triangles and 6 vertices.
	int numFloatsPerVertex = 5;
	int numVerticesPerCell = 6;
	int numFloatsPerCell = numFloatsPerVertex * numVerticesPerCell;
	int maxNumCells = 2 * numPoints;
	GLfloat* vertices = nullptr;
	try {
		vertices = new GLfloat[(double)maxNumCells * numFloatsPerCell];
	}
	catch (const std::bad_alloc& e) {
		std::cout << "Memory Allocation " << "failure: " << e.what() << endl;
		return;
	}

	// Compute cell vertices
	GLfloat* pV = vertices;
	int numCells = 0;

	for (std::list<Curve::PointVector>::iterator it = curvePoints.begin(); it != curvePoints.end(); it++) {
		if (it->size() < 1) continue;

		// Create the line cells. Line cells enclose the stroke between each pair of points. 
		Curve::PointVector::iterator itPoint = (*it).begin();
		float x0 = itPoint->pos()[0];
		float y0 = itPoint->pos()[1];
		float r0 = itPoint->radius();
		for (++itPoint; itPoint != (*it).end(); ++itPoint) {
			float x1 = itPoint->pos()[0];
			float y1 = itPoint->pos()[1];
			float r1 = itPoint->radius();
			float dir[2] = { x1 - x0, y1 - y0 };
			float len = dir[0] * dir[0] + dir[1] * dir[1];
			if (len <= std::numeric_limits<float>::epsilon()) continue;
			len = sqrt(len);
			float perpDir[2] = { -dir[1] / len, dir[0] / len };
			float x[6] = { x0, x1, x1, x0, x1, x0 };
			float y[6] = { y0, y1, y1, y0, y1, y0 };
			float r[6] = { r0, r1, r1, r0, r1, r0 };
			float dX[6] = { -perpDir[0], -perpDir[0], perpDir[0], -perpDir[0], perpDir[0], perpDir[0] };
			float dY[6] = { -perpDir[1], -perpDir[1], perpDir[1], -perpDir[1], perpDir[1], perpDir[1] };
			for (int idx = 0; idx < 6; idx++) {
				*pV++ = x[idx] + dX[idx] * (r[idx] + radiusOffset);	// Vertex x-component
				*pV++ = y[idx] + dY[idx] * (r[idx] + radiusOffset);	// Vertex y-component
				*pV++ = -dX[idx] * (r[idx] + radiusOffset);			// dx to curve centerline
				*pV++ = -dY[idx] * (r[idx] + radiusOffset);			// dy to curve centerline
				*pV++ = r[idx];										// Curve radius
			}
			numCells++;
			x0 = x1;
			y0 = y1;
			r0 = r1;
		}

		// Create point cells. Point cells are axis aligned squares that enclose each point.
		for (Curve::PointVector::iterator itPoint = (*it).begin(); itPoint != (*it).end(); ++itPoint) {
			float x = itPoint->pos()[0];
			float y = itPoint->pos()[1];
			float r = itPoint->radius();
			float dX[6] = { -1, 1, 1, -1, 1, -1 };
			float dY[6] = { -1, -1, 1, -1, 1, 1 };
			for (int idx = 0; idx < 6; idx++) {
				*pV++ = x + dX[idx] * (r + radiusOffset);	// Vertex x-component
				*pV++ = y + dY[idx] * (r + radiusOffset);	// Vertex y-component
				*pV++ = -dX[idx] * (r + radiusOffset);		// dx to point
				*pV++ = -dY[idx] * (r + radiusOffset);		// dy to point
				*pV++ = r;									// Curve radius
			}
			numCells++;
		}
	}

	// Create the vertex buffer
	int numStrokeVertices = numCells * numVerticesPerCell;
	int sizeVertices = sizeof(GLfloat) * numFloatsPerVertex * numStrokeVertices;
	if (!m_vertexBuffer.isCreated()) {
		m_vertexBuffer.create();
	}
	m_vertexBuffer.bind();
	m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_vertexBuffer.allocate(vertices, sizeVertices);
	m_vertexBuffer.release();
	delete[] vertices;
	m_numVertices = numStrokeVertices;
}