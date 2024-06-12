//
// ImageConverter.h
// Conversion from standard image formats to the internal format used in this library.
// Currently supports QImage but others could be added in the future.
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

#include <QImage>

class ImageConverter
{
public:
	ImageConverter();
	~ImageConverter();

	void imageFromQImage(Image& dst, const QImage& src);

private:
};