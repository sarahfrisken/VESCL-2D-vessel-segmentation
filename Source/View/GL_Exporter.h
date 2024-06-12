//
// GL_Exporter.h
// QOpenGL context and functionality for rendering and exporting segmentations.
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

#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>

class Model;

class GL_Exporter : protected QOpenGLFunctions
{
public:
    GL_Exporter(Model* model);
    ~GL_Exporter();

    enum class ExportType { SourceImage, Binary, DistToCenterline, DistToVesselEdge };
    void exportSegmentation(const char* filename, float imageToExportScale, ExportType type);

private:
    Model* m_model;

    bool m_isGLSetup;
    bool setupGL();
    QOpenGLContext m_context;
    QOffscreenSurface m_surface;
};