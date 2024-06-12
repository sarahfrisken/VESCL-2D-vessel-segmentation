//
// GL_BltRenderer.h
// Performs a Bit Blt of the framebuffer texture onto the screen of the current context.
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

class QOpenGLShaderProgram;

class GL_BltRenderer : public QObject, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	GL_BltRenderer();
	~GL_BltRenderer();

	// OpenGL context must be set prior to rendering
	void render(GLuint textureID, float opacity);

private:
	QOpenGLShaderProgram* m_shaderProgram = nullptr;
	QOpenGLBuffer m_vertexBuffer;
	void setVertexBuffer();

	// OpenGL shaders for bit blt'ing an RGBA texture map to the current context, while 
	// modulating its opacity
	const char* const vertexShader =
		"attribute highp vec3 a_position;\n"
		"attribute mediump vec4 a_texCoord;\n"
		"varying mediump vec4 v_texCoord;\n"
		"void main() {\n"
		"   gl_Position = vec4(a_position, 1.0);\n"
		"   v_texCoord = a_texCoord;\n"
		"}\n";
	const char* const fragmentShader =
		"uniform sampler2D u_texture;\n"
		"varying mediump vec4 v_texCoord;\n"
		"uniform float u_opacity;\n"
		"void main() {\n"
		"	vec4 texColor = texture2D(u_texture, v_texCoord.st);\n"
		"   gl_FragColor = vec4(texColor.r, texColor.g, texColor.b, u_opacity * texColor.a);\n"
		"}\n";
};