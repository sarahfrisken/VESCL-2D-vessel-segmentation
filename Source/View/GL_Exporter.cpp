//
// GL_Exporter.cpp
// Implementation of GL_Exporter.
//

#include "GL_Exporter.h"

#include "GL_ImageRenderer.h"
#include "GL_ContourRenderer.h"
#include "../Model/Model.h"
#include "../Controller/RenderState.h"

#include <QScreen>
#include <QImage>
#include <QOpenGLFramebufferObject>

#include <iostream>

// 
// Public
//
GL_Exporter::GL_Exporter(Model* model) :
	m_model(model),
	m_isGLSetup(false)
{
}
GL_Exporter::~GL_Exporter()
{
}

void GL_Exporter::exportSegmentation(const char* filename, float imageToExportScale, ExportType type)
{
	if (!m_isGLSetup) {
		if (!setupGL()) return;
	}
	m_isGLSetup = true;
	
	// Clear the offscreen surface viewport
	int imageWidth = m_model->imageWidth();
	int imageHeight = m_model->imageHeight();
	int exportWidth = (int) imageWidth * imageToExportScale;
	int  exportHeight = (int) imageHeight * imageToExportScale;
	m_context.makeCurrent(&m_surface);
	glViewport(0, 0, exportWidth, exportHeight);

	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Set renderstate for export
	RenderState renderState;
	renderState.resetProjectionMatrix(exportWidth, exportHeight);
	renderState.centerImageInViewport(imageWidth, imageHeight);

		// Render the image to the appropriate renderer
	QImage renderedImage;
	if (type == ExportType::SourceImage) {
		GL_ImageRenderer renderer;
		renderer.setImage(m_model->image());
		renderer.update(exportWidth, exportHeight, &renderState);
		renderedImage = renderer.renderedImage();
	}
	else {
		GL_ContourRenderer::RendererType renderType;
		switch (type)
		{
		case ExportType::DistToCenterline:
			renderType = GL_ContourRenderer::RendererType::DistToCenterline;
			break;
		case ExportType::DistToVesselEdge:
			renderType = GL_ContourRenderer::RendererType::DistToContour;
			break;
		default:
		case ExportType::Binary:
			renderType = GL_ContourRenderer::RendererType::Binary;
			break;

		}
		GL_ContourRenderer renderer(renderType);
		std::list<Curve*>* curves = m_model->contour()->curves();
		std::list<Curve::PointVector> curvePoints;
		float maxPointSpacing = renderState.maxPointSpacing() * renderState.windowToContourScale();
		for (std::list<Curve*>::iterator it = curves->begin(); it != curves->end(); it++) {
			Curve::PointVector pv = (*it)->getSampledCurvePoints(maxPointSpacing);
			curvePoints.push_back(pv);
		}
		renderer.update(curvePoints, Qt::white, exportWidth, exportHeight, 
			renderState.mvpMatrix(), renderState.windowToContourScale());
		renderedImage = renderer.renderedImage();
	}

	for (int y = 0; y < renderedImage.height(); ++y) {
		QRgb* ptr = reinterpret_cast<QRgb*>(renderedImage.scanLine(y));
		for (int x = 0; x < renderedImage.width(); ++x) {
			float alpha = ((float)qAlpha(*ptr)) / 255.0;
			float red = ((float)qRed(*ptr)) * alpha;
			float green = ((float)qGreen(*ptr)) * alpha;
			float blue = ((float)qBlue(*ptr)) * alpha;
			*ptr++ = qRgba((int)red, (int)green, (int)blue, 255);
		}
	}

	try {
		if (!renderedImage.save(filename, nullptr, -1)) {
			throw std::runtime_error("Can't save image to export file.");
		}
	}
	catch (const std::exception& e) {
		std::cout << "Exception " << e.what() << std::endl;
	}
	m_context.doneCurrent();
}

// 
// Private
//
bool GL_Exporter::setupGL()
{
	try {
		if (!m_context.create()) {
			throw std::runtime_error("Can't create GL context.");
		}
		m_surface.setFormat(m_context.format());
		m_surface.create();
		if (!m_surface.isValid()) {
			throw std::runtime_error("GL surface not valid.");
		}
		if (!m_context.makeCurrent(&m_surface)) {
			throw std::runtime_error("Can't make GL context current.");
		}
	}
	catch (const std::exception& e) {
		std::cout << "Exception " << e.what() << std::endl;
		return false;
	}

	initializeOpenGLFunctions();
	return true;
}
