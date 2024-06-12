//
// GL_ImageRenderer.cpp
// Implementation of ImageRenderer.
//

#include "GL_ImageRenderer.h"

#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLPixelTransferOptions>

#include <assert.h>

// 
// Public
//
GL_ImageRenderer::GL_ImageRenderer() :
	m_imageData(nullptr),
	m_imageWidth(0),
	m_imageHeight(0),
	m_imageDataFormat(Image::DataFormat::NotSupported),
	m_imageTexture(nullptr),
	m_fboWidth(0),
	m_fboHeight(0),
	m_fboDoInterpolate(false),
	m_fbo(nullptr),
	m_shaderProgram(nullptr)
{
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
GL_ImageRenderer::~GL_ImageRenderer()
{
	m_vertexBuffer.destroy();
	delete m_imageTexture;
	delete m_shaderProgram;
	delete m_fbo;
}

// Get the texture ID of the fbo's color buffer. Return's false if the fbo is invalid.
bool GL_ImageRenderer::textureID(GLuint* textureID)
{
	if (m_fbo) {
		*textureID = m_fbo->texture();
		return true;
	}
	return false;
}
QImage GL_ImageRenderer::renderedImage()
{
	return(m_fbo->toImage(true));
}

void GL_ImageRenderer::setImage(const Image& image)
{
	if (!image.isValid()) return;
	m_imageWidth = image.width();
	m_imageHeight = image.height();
	m_imageDataFormat = image.dataFormat();
	m_imageData = image.data();
	setVertexBuffer();

	// Create a new texture to hold the image data for rendering
	if (m_imageTexture) {
		delete m_imageTexture;
		m_imageTexture = nullptr;
	}

	QOpenGLTexture::TextureFormat texFormat;
	QOpenGLTexture::PixelType pixelType;
	switch (m_imageDataFormat) {
	case Image::DataFormat::UChar:
		texFormat = QOpenGLTexture::R8_UNorm;
		pixelType = QOpenGLTexture::UInt8;
		break;

	case Image::DataFormat::UShort:
		texFormat = QOpenGLTexture::R16_UNorm;
		pixelType = QOpenGLTexture::UInt16;
		break;

	default:
		return;
	}

	m_imageTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	m_imageTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
	m_imageTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
	m_imageTexture->setSize(m_imageWidth, m_imageHeight);
	m_imageTexture->setAutoMipMapGenerationEnabled(false);
	m_imageTexture->setFormat(texFormat);
	m_imageTexture->allocateStorage();

	QOpenGLPixelTransferOptions options;
	options.setAlignment(1);
	m_imageTexture->setData(QOpenGLTexture::Red, pixelType, m_imageData, &options);
}

// Updates the fbo's color buffer 
void GL_ImageRenderer::update(int winWidth, int winHeight, RenderState* renderState)
{
	// Check for required data
	if (!m_shaderProgram) return;
	if (!m_imageData || !m_imageTexture) return;

	// Update the FBO if necessary
	if (winWidth != m_fboWidth || winHeight != m_fboHeight) {
		delete m_fbo;
		m_fboWidth = winWidth;
		m_fboHeight = winHeight;
		m_fbo = new QOpenGLFramebufferObject(m_fboWidth, m_fboHeight, GL_TEXTURE_2D);
	}
	if (renderState->imageInterpolation() != m_fboDoInterpolate) {
		m_fboDoInterpolate = renderState->imageInterpolation();
		if (m_imageTexture) {
			if (m_fboDoInterpolate) m_imageTexture->setMagnificationFilter(QOpenGLTexture::Linear);
			else m_imageTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
		}
	}

	m_fbo->bind();
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Set shader data
	m_shaderProgram->bind();
	int mvpLocation = m_shaderProgram->uniformLocation("u_mvpMatrix");
	assert(mvpLocation != -1);
	m_shaderProgram->setUniformValue(mvpLocation, renderState->mvpMatrix());

	float windowingLevel;
	float windowingWidth;
	int levelLocation = m_shaderProgram->uniformLocation("u_winLevel");
	int widthLocation = m_shaderProgram->uniformLocation("u_winWidth");
	assert(levelLocation != -1);
	assert(widthLocation != -1);
	m_shaderProgram->setUniformValue(levelLocation, renderState->windowingLevel());
	m_shaderProgram->setUniformValue(widthLocation, renderState->windowingWidth());

	// Tell OpenGL programmable pipeline how to locate the vertex data
	m_vertexBuffer.bind();
	quintptr offset = 0;
	int numFloatsPerVertex = 3;
	int numFloatsPerTexCoord = 2;
	int stride = (numFloatsPerVertex + numFloatsPerTexCoord) * sizeof(GLfloat);

	int posLocation = m_shaderProgram->attributeLocation("a_position");
	assert(posLocation != -1);
	m_shaderProgram->enableAttributeArray(posLocation);
	m_shaderProgram->setAttributeBuffer(posLocation, GL_FLOAT, offset, numFloatsPerVertex, stride);

	offset += numFloatsPerVertex * sizeof(GLfloat);
	int texLocation = m_shaderProgram->attributeLocation("a_texCoord");
	assert(texLocation != -1);
	m_shaderProgram->enableAttributeArray(texLocation);
	m_shaderProgram->setAttributeBuffer(texLocation, GL_FLOAT, offset, numFloatsPerTexCoord, stride);

	// Perform the rendering
	m_imageTexture->bind();
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	m_imageTexture->release();
	m_vertexBuffer.release();
	m_shaderProgram->release();
	m_fbo->release();
}

// 
// Private
//
void GL_ImageRenderer::setVertexBuffer()
{
	// Vertex data in image coordinates. Format is (x,y,x,s,t), position and texture coords.
	if (!m_imageData) return;
	float w = m_imageWidth;
	float h = m_imageHeight;

	GLfloat vertData[]{
		0, 0, 0, 0, 0,
		w, 0, 0, 1, 0,
		w, h, 0, 1, 1,
		0, h, 0, 0, 1
	};
	if (!m_vertexBuffer.isCreated()) {
		m_vertexBuffer.create();
	}
	m_vertexBuffer.bind();
	m_vertexBuffer.allocate(vertData, sizeof(vertData));
	m_vertexBuffer.release();
}