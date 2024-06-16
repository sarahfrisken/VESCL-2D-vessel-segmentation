//
// GL_BltRenderer.cpp
// Implementation of BltRenderer.
//

#include "GL_BltRenderer.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLPixelTransferOptions>

#include <iostream>
#include <assert.h>

// 
// Public
//
GL_BltRenderer::GL_BltRenderer()
{
	// Set up the texture to screen rendering shader programs
	m_shaderProgram = new QOpenGLShaderProgram;
	m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
	m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);
	m_shaderProgram->link();

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
		std::cout << "Exception " << e.what() << std::endl;
		delete m_shaderProgram;
		m_shaderProgram = nullptr;
		return;
	}

	// Set up OpenGL
	initializeOpenGLFunctions();
	setVertexBuffer();
}
GL_BltRenderer::~GL_BltRenderer()
{
	m_vertexBuffer.destroy();
	delete m_shaderProgram;
}

void GL_BltRenderer::render(GLuint textureID, float opacity)
{
	// Check for required data
	if (!m_shaderProgram) return;

	// Set shader data
	m_shaderProgram->bind();
	m_shaderProgram->setUniformValue("u_opacity", opacity);

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
	glBindTexture(GL_TEXTURE_2D, textureID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
	m_vertexBuffer.release();
	m_shaderProgram->release();
}

// 
// Private
//
void GL_BltRenderer::setVertexBuffer()
{
	// Vertex data for blt'ing texture to the window. Format is (x,y,z,s,t), position 
	// and texture coords
	GLfloat vertData[]{
	  -1.0, -1.0, 0.0, 0.0, 0.0,
	  1.0, -1.0, 0.0, 1.0, 0.0,
	  1.0, 1.0, 0.0, 1.0, 1.0,
	  -1.0, 1.0, 0.0, 0.0, 1.0
	};
	if (!m_vertexBuffer.isCreated()) {
		m_vertexBuffer.create();
	}
	m_vertexBuffer.bind();
	m_vertexBuffer.allocate(vertData, sizeof(vertData));
	m_vertexBuffer.release();
}

