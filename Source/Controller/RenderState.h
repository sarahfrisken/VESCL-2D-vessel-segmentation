//
// RenderState.h
// Holds rendering data, e.g., update needs, contour colors, image windowing, transforms.
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

#include <QColor>
#include <QMatrix4x4>

class RenderState
{
public:
	RenderState();
	~RenderState();

	// Render update status
	bool imageNeedsUpdate() const { return m_imageRendererNeedsUpdate; };
	void setImageNeedsUpdate(bool needsUpdate) { m_imageRendererNeedsUpdate = needsUpdate; };
	bool contourNeedsUpdate() const { return m_contourRendererNeedsUpdate; };
	void setContourNeedsUpdate(bool needsUpdate) { m_contourRendererNeedsUpdate = needsUpdate; };
	bool activeCurveNeedsUpdate() const { return m_activeCurveRendererNeedsUpdate; };
	void setActiveCurveNeedsUpdate(bool needsUpdate) { m_activeCurveRendererNeedsUpdate = needsUpdate; };
	bool needsFullUpdate() const { return m_allNeedUpdate; };
	void setNeedsFullUpdate(bool needsUpdate) { m_allNeedUpdate = needsUpdate; };

	// Contour color and visibility for rendering
	QColor contourColor() const { return m_contourColor; };
	void setContourColor(QColor color) { m_contourColor = color; };
	QColor activeCurveColor() const { return m_activeCurveColor; };
	void setActiveCurveColor(QColor color) { m_activeCurveColor = color; };
	bool isContourVisible() const { return m_isContourVisible; };
	void setContourVisible(bool isContourVisible) { m_isContourVisible = isContourVisible; };

	// Point spacing in window pixels for interpolating curves prior to rendering
	float maxPointSpacing() const { return m_maxPointSpacingInWindowPixels; };

	// Image interpolation and windowing for rendering
	void setImageInterpolation(bool doInterpolate) { m_interpolateImage = doInterpolate; };
	bool imageInterpolation() const { return m_interpolateImage; };
	void setWindowingRange(float normalizedMinValue, float normalizedMaxValue);
	void setWindowing(float windowingLevel, float windowingWidth) {
		m_windowingLevel = windowingLevel;
		m_windowingWidth = windowingWidth;
	}
	void setDefaultWindowing();
	float windowingWidth() const { return m_windowingWidth; };
	float windowingLevel() const { return m_windowingLevel; };
	void incrementWindowing(float brightnessIncr, float contrastIncr);

	// Transforms
	QMatrix4x4 worldToView() const;
	void setWorldToView(QMatrix4x4 worldToView);
	void resetWorldToView();
	void centerImageInViewport(int w, int h);
	void resetProjectionMatrix(int w, int h);
	QVector3D convertWindowToWorld(QVector3D windowPoint);
	QVector3D convertWindowToImage(QVector3D windowPoint);
	float windowToWorldScale() const { return m_windowToWorldScale; };
	float windowToContourScale() const { return m_windowToImageScale; };
	QMatrix4x4 mvpMatrix() const;

private:
	// General render state
	bool m_imageRendererNeedsUpdate;
	bool m_contourRendererNeedsUpdate;
	bool m_activeCurveRendererNeedsUpdate;
	bool m_allNeedUpdate;

	// Contour render state
	QColor m_contourColor;
	QColor m_activeCurveColor;
	bool m_isContourVisible;
	float m_maxPointSpacingInWindowPixels;

	// Image rendering state
	float m_imageMinValue;
	float m_imageMaxValue;
	float m_windowingWidth;
	float m_windowingLevel;
	bool m_interpolateImage;

	// Transforms
	// The viewport is the OpenGLWidget display screen
	// View coordinates are [-1,-1] to [1,1] with [-1,-1] the bottom left corner
	// Window coordinates are [0,0] to [winW,winH] with [0,0] the top left corner
	// Image coordinates are [0,0] to [imgW,imgH] with [0,0] the top left corner
	QMatrix4x4 m_viewToViewport;	// Projection matrix
	QMatrix4x4 m_worldToView;		// Initialized to identity. Modified by zooming and panning.
	QMatrix4x4 m_worldToViewport;
	int m_winWidth;
	int m_winHeight;
	void setWorldToViewport(QMatrix4x4 transform);

	QMatrix4x4 m_imageToWorld;		
	QMatrix4x4 m_imageToView;
	QMatrix4x4 m_imageToViewport;	// Model view projection matrix

	// For converting mouse positions for camera transformations and selection
	QMatrix4x4 m_viewportToWindow;
	QMatrix4x4 m_windowToWorld;
	float m_viewportToWindowScale;
	float m_windowToWorldScale;
	float m_windowToImageScale;
	float getScale(QMatrix4x4 transform);
};