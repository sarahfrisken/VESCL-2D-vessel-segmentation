//
// GL_ContourRenderer.h
// Renders a contour onto the texture of an OpenGL framebuffer object.
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

#include "../Controller/RenderState.h"
#include "../Model/Contour.h"

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class QOpenGLTexture;
class QOpenGLFramebufferObject;
class QOpenGLShaderProgram;

class GL_ContourRenderer : public QObject, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	enum class RendererType { Default, Antialiased, Centerline, Binary, DistToContour, DistToCenterline };

	GL_ContourRenderer(RendererType type);
	~GL_ContourRenderer();

	// OpenGL context must be set prior to update
	void update(std::list<Curve::PointVector> curvePoints, QColor contourColor,
		int winWidth, int winHeight, QMatrix4x4 mvpMatrix, float windowToContourScale);
	bool textureID(GLuint* textureID);
	QImage renderedImage();

private:
	// Could consider sub-classing this renderer for different render types
	RendererType m_type;

	// Type specific state. Currently uses default values but could consider setting these.
	float m_antialiasingFilterWidth;
	float m_centerlineRadius;
	float m_maxDistFieldDistance;

	// OpenGL rendering
	int m_fboWidth;
	int m_fboHeight;
	QOpenGLFramebufferObject* m_fbo;
	QOpenGLShaderProgram* m_shaderProgram;
	QOpenGLBuffer m_vertexBuffer;
	int m_numVertices;
	void setShaderData(QColor contourColor, float windowToContourScale);
	void setVertexBuffer(std::list<Curve::PointVector> curvePoints, float windowToContourScale);

	// OpenGL shaders for rendering contours with antialiased edges
	const char* const vertexShader_antialiasedContour =
		"attribute vec2 a_position;\n"
		"attribute vec3 a_data;\n"
		"varying vec2 v_vecDist;\n"
		"varying float v_radius;\n"
		"uniform mat4 u_mvpMatrix;\n"
		"void main() {\n"
		"   gl_Position = u_mvpMatrix * vec4(a_position, 0.0, 1.0);\n"
		"   v_vecDist = vec2(a_data.x, a_data.y); \n"
		"   v_radius = a_data.z;\n"
		"}\n";
	const char* const fragmentShader_antialiasedContour =
		"varying vec2 v_vecDist;\n"
		"varying float v_radius;\n"
		"uniform vec4 u_color;\n"
		"uniform float u_filterWidth;\n"
		"void main() {\n"
		"   float distToEdge = v_radius - length(v_vecDist);\n"
		"	float profile = clamp(0.5 + distToEdge / u_filterWidth, 0.0, 1.0);\n"
		"	gl_FragColor = u_color;\n"
		" 	gl_FragColor.a *= profile;\n"
		"}\n";

	// OpenGL shaders for rendering contour centerlines 
	const char* const vertexShader_contourCenterline =
		"attribute vec2 a_position;\n"
		"attribute vec3 a_data;\n"
		"varying vec2 v_vecDist;\n"
		"uniform mat4 u_mvpMatrix;\n"
		"void main() {\n"
		"   gl_Position = u_mvpMatrix * vec4(a_position, 0.0, 1.0);\n"
		"   v_vecDist = vec2(a_data.x, a_data.y); \n"
		"}\n";
	const char* const fragmentShader_contourCenterline =
		"varying vec2 v_vecDist;\n"
		"uniform vec4 u_color;\n"
		"uniform float u_radius;\n"
		"uniform float u_filterWidth;\n"
		"void main() {\n"
		"   float distToEdge = u_radius - length(v_vecDist);\n"
		"	float profile = clamp(0.5 + distToEdge / u_filterWidth, 0.0, 1.0);\n"
		"	gl_FragColor = u_color;\n"
		" 	gl_FragColor.a *= profile;\n"
		"}\n";

	// OpenGL shaders for rendering as a binary segmentation
	const char* const vertexShader_binaryContour =
		"attribute vec2 a_position;\n"
		"attribute vec3 a_data;\n"
		"varying vec2 v_vecDist;\n"
		"varying float v_radius;\n"
		"uniform mat4 u_mvpMatrix;\n"
		"void main() {\n"
		"   gl_Position = u_mvpMatrix * vec4(a_position, 0.0, 1.0);\n"
		"   v_vecDist = vec2(a_data.x, a_data.y); \n"
		"   v_radius = a_data.z;\n"
		"}\n";
	const char* const fragmentShader_binaryContour =
		"varying vec2 v_vecDist;\n"
		"varying float v_radius;\n"
		"uniform vec4 u_color;\n"
		"void main() {\n"
		"   float distToEdge = v_radius - length(v_vecDist);\n"
		"	if (distToEdge >= 0) gl_FragColor = u_color;\n"
		"	else gl_FragColor = vec4(0, 0, 0, 0);\n"
		"}\n";

	// OpenGL shaders to encode signed distances to contour edges
	const char* const vertexShader_distToContour =
		"attribute vec2 a_position;\n"
		"attribute vec3 a_data;\n"
		"varying vec2 v_vecDist;\n"
		"varying float v_radius;\n"
		"uniform mat4 u_mvpMatrix;\n"
		"void main() {\n"
		"   gl_Position = u_mvpMatrix * vec4(a_position, 0.0, 1.0);\n"
		"   v_vecDist = vec2(a_data.x, a_data.y); \n"
		"   v_radius = a_data.z;\n"
		"}\n";
	const char* const fragmentShader_distToContour =
		"varying vec2 v_vecDist;\n"
		"varying float v_radius;\n"
		"uniform vec4 u_color;\n"
		"uniform float u_maxDist;\n"
		"void main() {\n"
		"   float distToEdge = v_radius - length(v_vecDist);\n"
		"	float profile = clamp(0.5 + 0.5 * distToEdge/u_maxDist, 0.0, 1.0);\n"
		"	gl_FragColor = u_color;\n"
		" 	gl_FragColor.a *= profile;\n"
		"}\n";

	// OpenGL shaders to encode distances to contours centerlines
	const char* const vertexShader_distToCenterline =
		"attribute vec2 a_position;\n"
		"attribute vec3 a_data;\n"
		"varying vec2 v_vecDist;\n"
		"varying float v_radius;\n"
		"uniform mat4 u_mvpMatrix;\n"
		"void main() {\n"
		"   gl_Position = u_mvpMatrix * vec4(a_position, 0.0, 1.0);\n"
		"   v_vecDist = vec2(a_data.x, a_data.y); \n"
		"   v_radius = a_data.z;\n"
		"}\n";
	const char* const fragmentShader_distToCenterline =
		"varying vec2 v_vecDist;\n"
		"varying float v_radius;\n"
		"uniform vec4 u_color;\n"
		"uniform float u_maxDist;\n"
		"void main() {\n"
		"   float distToCenterline = length(v_vecDist);\n"
		"	float scaledDist = clamp(1 - (distToCenterline/u_maxDist), 0.0, 1.0);\n"
		"	gl_FragColor = u_color;\n"
		"	gl_FragColor.a = scaledDist;\n"
		"}\n";
};