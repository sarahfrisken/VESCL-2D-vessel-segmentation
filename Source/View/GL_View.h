//
// GL_View.h
// A QOpenGL widget that provides an interface for rendering and mouse input.
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

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector2D>
#include <QMatrix4x4>

#include "Cursor.h"
#include "../Controller/RenderState.h"
#include "../Model/Image.h"

class Model;
class GL_BltRenderer;
class GL_ImageRenderer;
class GL_ContourRenderer;
class RenderState;

class GL_View : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    GL_View(Model* model, RenderState* renderState);
    ~GL_View();

    void setImage(const Image& image);

    void initCursor();
    float cursorRadius() const;
    void setCursorRadius(float radius);
    void setCursorColor(QColor color);

protected:
    void mousePressEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* e) Q_DECL_OVERRIDE;

    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private:
	Model* m_model;
    RenderState* m_renderState;
    Cursor m_cursor;

    // Mouse input
    enum class MouseAction { None, Draw, Zoom, Pan, Windowing };
    MouseAction m_currentAction;
    QVector2D m_lastMousePos;
    QVector2D m_mouseDownPos;
    QVector2D m_mouseDownWorld;
    QMatrix4x4 m_mouseDownWorldToView;
    float m_mouseDownWinLevel;
    float m_mouseDownWinWidth;
    float m_mouseDownCursorSize;

    // Renderers take advantage of QOpenGLWidget functionality and thus 
    // are located in the view. Renderers can't be created until context is set (i.e., 
    // until OpenGL is initialzied).
    GL_BltRenderer* m_bltRenderer;
    GL_ImageRenderer* m_imageRenderer;
    GL_ContourRenderer* m_contourRenderer;
    GL_ContourRenderer* m_activeCurveRenderer;
};