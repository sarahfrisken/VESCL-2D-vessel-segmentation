//
// ImageFilterer.h
// Image filters used to move a point towards a vessel and detect vessel width.
// Additional filters could be added to improve curve fitting and width detection.
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

#include "Math.h"
#include "Image.h"

class ImageFilterer
{
public:
    enum class VesselContrastType { DarkOnLight, LightOnDark, NotSupported };

    ImageFilterer(Image* image, VesselContrastType contrastType = VesselContrastType::DarkOnLight);
	~ImageFilterer();

    VesselContrastType vesselContrastType() { return m_type; };
    void setVesselContrastType(VesselContrastType contrastType) { m_type = contrastType; };

    // Applies Gaussian filtering based on the expected radius to an image patch 
    // surrounding the given position and computes the moments of the filtered patch. 
    // Moves the curve point towards the center of mass of the filtered patch, which
    // is expected to lie on the vessel centerline.
    Math::Vec2D getVecToClosestVessel(Math::Vec2D pos, float expectedRadius);

    // Samples the image along a line perpendicular to the curve at the given point.
    // Uses 1D Canny edge detection to find the vessel edges and derive the vessel 
    // width at the point.
    float getWidthAtP(Math::Vec2D curvePoint, Math::Vec2D curveDir, float expectedRadius);

private:
    Image* m_image;
    VesselContrastType m_type;
    float imageValueAtP(Math::Vec2D p);
};