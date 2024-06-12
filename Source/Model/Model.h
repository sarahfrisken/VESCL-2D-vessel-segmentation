//
// Model.h
// Specific to this application. Contains a contour and its reference image.
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

#include "Image.h"
#include "Contour.h"
#include "ImageFilterer.h"
#include "../Controller/RenderState.h"

class Model
{
public:
    Model(RenderState* renderState);
    ~Model();

    void clear();
    void save(std::ofstream& file);
    void load(std::ifstream& file);

    Contour* contour() { return &m_contour; };
    void startDraw(CurvePoint pStart);
    void updateDraw(CurvePoint point);
    void endDraw(CurvePoint point);

    bool select(float pos[2]);
    void deselect();
    void deleteSelected();
    void fitSelectedToNearestVessel(float expectedRadius);
    void fitSelectedVesselWidth(float expectedRadius);

    Image& image() { return m_image; };
    bool imageIsValid() const { return m_image.isValid(); };
    int imageWidth() const { return m_image.width(); };
    int imageHeight() const { return m_image.height(); };
    float imageNormalizedMin() { return m_image.normalizedMinValue(); };
    float imageNormalizedMax() { return m_image.normalizedMaxValue(); };
    ImageFilterer::VesselContrastType vesselContrast() const;
    void setVesselContrast(ImageFilterer::VesselContrastType contrastType);

    // Make non-copyable
    Model(Model const&) = delete;
    void operator=(Model const&) = delete;

private:
    Image m_image;
    Contour m_contour;
    RenderState* m_renderState; 
    ImageFilterer* m_imageFilterer;

    // Version number for saving and loading
    std::string m_version = "VSCL0001";

    // Drawing and editing
    bool m_isDrawing;
    float m_minSeparationInWindowPixels;
    float m_selectionRadiusInWindowPixels;
};