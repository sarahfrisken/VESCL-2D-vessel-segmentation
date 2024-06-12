//
// Image.cpp
// Implementation of Image.
//

#include "Image.h"

#include <sstream>
#include <string> 
#include <assert.h>

// 
// Public
//
Image::Image() :
    m_width(0),
    m_height(0),
    m_dataFormat(DataFormat::UChar),
    m_data(nullptr),
    m_stats({ false, 0, 0 })
{
}
Image::Image(int width, int height, DataFormat format, unsigned char* data) :
	m_width(width),
	m_height(height),
	m_dataFormat(format),
	m_data(nullptr),
	m_stats({ false, 0, 0 }) 
{
	try {
		int numBytesPerPixel = (m_dataFormat == DataFormat::UChar) ? 1 : 2;
		size_t size = numBytesPerPixel * (size_t)m_width * (size_t)m_height;
		m_data = new unsigned char[size];
		memcpy(m_data, data, size);
	}
	catch (std::bad_alloc& e) {
		std::cout << "Memory Allocation " << "Error allocating image." << e.what() << std::endl;
	}
}

Image::~Image()
{
	clear();
}

float Image::normalizedMinValue()
{
	if (!m_stats.isValid) computeStats();
	return m_stats.minValue; 
};
float Image::normalizedMaxValue()
{ 
	if (!m_stats.isValid) computeStats();
	return m_stats.maxValue;
};

// 
// Public
//
void Image::clear()
{
	m_width = 0;
	m_height = 0;
	m_dataFormat = DataFormat::UChar;
	delete m_data;
	m_data = nullptr;
	m_stats.isValid = false;
}
bool Image::readFromFile(std::ifstream& fstream)
{
	try {
		clear();

		// Read image parameters
		std::string line;
		std::getline(fstream, line);
		if (line != "key_image") {
			throw std::runtime_error("Error reading image data.");
		}

		std::getline(fstream, line);
		m_width = std::stoi(line);
		std::getline(fstream, line);
		m_height = std::stoi(line);
		std::getline(fstream, line);
		int format = std::stoi(line);
		switch (format) {
		case 1:
			m_dataFormat = DataFormat::UChar;
			break;
		case 2:
			m_dataFormat = DataFormat::UShort;
			break;
		default :
			throw std::runtime_error("Image format not supported.");
		}

		// Read image data
		int numBytesPerPixel = (m_dataFormat == DataFormat::UChar) ? 1 : 2;
		int size = numBytesPerPixel * m_width * m_height;
		m_data = new unsigned char[size];

		if (!fstream.read((char*)m_data, size)) {
			throw std::runtime_error("Error reading image data.");
		}

		computeStats();
	}
	catch (std::bad_alloc& e) {
		std::cout << "Memory Allocation " << "Error allocating image." << e.what() << std::endl;
		clear();
		return false;
	}
	catch (const std::exception& e) {
		std::cout << "Exception " << e.what() << std::endl;
		return false;
	}
	return true;
}
bool Image::writeToFile(std::ofstream& fstream) const
{
	try {
		if (!(m_dataFormat == DataFormat::UChar || m_dataFormat == DataFormat::UShort)) {
			throw std::runtime_error("Image format not supported.");
		}

		std::string key = "key_image";
		fstream << key << std::endl;
		fstream << m_width << std::endl;
		fstream << m_height << std::endl;
		int format = (m_dataFormat == DataFormat::UChar) ? 1 : ((m_dataFormat == DataFormat::UShort) ? 2 : 0);
		fstream << format << std::endl;

		// Write image data
		int numBytesPerPixel = (m_dataFormat == DataFormat::UChar) ? 1 : 2;
		int size = numBytesPerPixel * m_width * m_height;
		fstream.write((char*)m_data, size);
	}
	catch (const std::exception& e) {
		std::cout << "Exception " << e.what() << std::endl;
		return false;
	}
	return true;
}

//
// Private
//
void Image::computeStats()
{
	m_stats = { false, 0, 0 };
	float max = 0;
	float min = std::numeric_limits<int>::max();
	int numVals = m_width * m_height;

	assert(m_dataFormat == DataFormat::UChar || m_dataFormat == DataFormat::UShort);
	switch (m_dataFormat) {
	case DataFormat::UShort:
	{
		unsigned short* pVal = (unsigned short*)m_data;
		for (int i = 0; i < numVals; i++) {
			float val = (float)*pVal++;
			if (val < min) min = val;
			if (val > max) max = val;
		}
		min /= 65535.0;
		max /= 65535.0;
		break;
	}
	case DataFormat::UChar:
	{
		unsigned char* pVal = m_data;
		for (int i = 0; i < numVals; i++) {
			float val = (float)*pVal++;
			if (val < min) min = val;
			if (val > max) max = val;
		}
		min /= 255.0;
		max /= 255.0;
		break;
	}
	}

	m_stats.minValue = min;
	m_stats.maxValue = max;
	m_stats.isValid = true;
}
