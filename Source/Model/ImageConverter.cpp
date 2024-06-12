//
// ImageConverter.cpp
// Implementation of ImageConverter.
//

#include "ImageConverter.h"

// 
// Public
//
ImageConverter::ImageConverter()
{
}
ImageConverter::~ImageConverter()
{
}

void ImageConverter::imageFromQImage(Image& dst, const QImage& src)
{
	try {

		// Convert the source image to greyscale
		QImage grayscaleImage;
		switch (src.format()) {
		case QImage::Format_Grayscale8:
		case QImage::Format_Grayscale16:
			grayscaleImage = src;
			break;
		case QImage::Format_RGBX64:
		case QImage::Format_RGBA64:
		case QImage::Format_RGBA64_Premultiplied:
			grayscaleImage = src.convertToFormat(QImage::Format_Grayscale16);
			break;
		default:
			grayscaleImage = src.convertToFormat(QImage::Format_Grayscale8);
			break;
		}

		// Determine the image format
		size_t bytesPerPixel = 1;
		Image::DataFormat format = Image::DataFormat::NotSupported;
		switch (grayscaleImage.format()) {
		case QImage::Format_Grayscale8:
		{
			format = Image::DataFormat::UChar;
			bytesPerPixel = 1;
			break;
		}
		case QImage::Format_Grayscale16:
		{
			format = Image::DataFormat::UShort;
			bytesPerPixel = 2;
			break;
		}
		default:
		{
			throw std::runtime_error("Input image format not supported.");
		}
		}

		// Allocate memory to store the image data
		dst.clear();
		size_t dataSize = (size_t)src.width() * src.height() * bytesPerPixel;
		dst.m_data = new unsigned char[dataSize];

		// Copy data into the new image
		if (bytesPerPixel == 1) {
			unsigned char* pData = dst.m_data;
			for (int y = 0; y < grayscaleImage.height(); ++y) {
				unsigned char* ptr = reinterpret_cast<unsigned char*>(grayscaleImage.scanLine(y));
				for (int x = 0; x < grayscaleImage.width(); ++x) {
					*pData++ = *ptr++;
				}
			}
		}
		else if (bytesPerPixel == 2) {
			unsigned short* pData = (unsigned short*)dst.m_data;
			for (int y = 0; y < grayscaleImage.height(); ++y) {
				unsigned short* ptr = reinterpret_cast<unsigned short*>(grayscaleImage.scanLine(y));
				for (int x = 0; x < grayscaleImage.width(); ++x) {
					*pData++ = *ptr++;
				}
			}
		}
		dst.m_dataFormat = format;
		dst.m_width = grayscaleImage.width();
		dst.m_height = grayscaleImage.height();
	}
	catch (std::bad_alloc& e) {
		std::cout << "Memory Allocation " << "Error allocating image." << e.what() << std::endl;
	}
	catch (const std::exception& e) {
		std::cout << "Exception " << e.what() << std::endl;
	}
}