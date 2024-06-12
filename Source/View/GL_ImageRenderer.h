//
// GL_ImageRenderer.h
// Renders image onto the texture of an OpenGL framebuffer object.
// 
// Copyright(C) 2024 Sarah F. Frisken, Brigham and Women's Hospital
// 
// This code is free software : you can redistribute it and /or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later version.
// 
// This code is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
// PARTICULAR PURPOSE. See the GNU General Public License for more details.
// 
// You may have received a copy of the GNU General Public License along with this 
// program. If not, see < http://www.gnu.org/licenses/>.
// 

#pragma once

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class QOpenGLTexture;
class QOpenGLFramebufferObject;
class QOpenGLShaderProgram;

#include "../Model/Image.h"
#include "../Controller/RenderState.h"

class GL_ImageRenderer : public QObject, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	GL_ImageRenderer();
	~GL_ImageRenderer();

	void setImage(const Image& image);

	// OpenGL context must be set prior to update
	void update(int winWidth, int winHeight, RenderState* renderState);
	bool textureID(GLuint* textureID);
	QImage renderedImage();

private:
	// Image data
	unsigned char* m_imageData;
	int m_imageWidth;
	int m_imageHeight;
	Image::DataFormat m_imageDataFormat;
	QOpenGLTexture* m_imageTexture;

	// Rendering
	int m_fboWidth;
	int m_fboHeight;
	bool m_fboDoInterpolate;
	QOpenGLFramebufferObject* m_fbo;
	QOpenGLShaderProgram* m_shaderProgram;
	QOpenGLBuffer m_vertexBuffer;
	void setVertexBuffer();

	// OpenGL shaders for rendering a grey-scale image with brightness and contrast
	// modulation (via windowing level and width)
	const char* const vertexShader =
		"attribute highp vec3 a_position;\n"
		"attribute mediump vec4 a_texCoord;\n"
		"varying mediump vec4 v_texCoord;\n"
		"uniform highp mat4 u_mvpMatrix;\n"
		"void main() {\n"
		"   gl_Position = u_mvpMatrix * vec4(a_position, 1.0);\n"
		"   v_texCoord = a_texCoord;\n"
		"}\n";
	const char* const fragmentShader =
		"uniform sampler2D texture;\n"
		"uniform mediump float u_winLevel;\n"
		"uniform mediump float u_winWidth;\n"
		"varying mediump vec4 v_texCoord;\n"
		"void main() {\n"
		"	vec4 texColor = texture2D(texture, v_texCoord.st);\n"
		"   texColor = clamp((texColor - u_winLevel) / u_winWidth + 0.5, 0.0, 1.0);\n"
		"   gl_FragColor = vec4(texColor.r, texColor.r, texColor.r, 1.0);\n"
		"}\n";
};