//
// Image.h
// Reference image of vessels to be contoured.
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

#include<iostream>
#include<fstream>

class Image
{
public:
    enum class DataFormat { UChar, UShort, NotSupported };

    Image();
    Image(int width, int height, DataFormat format, unsigned char* data);
    ~Image();

    void clear();
    bool readFromFile(std::ifstream& fstream);
    bool writeToFile(std::ofstream& fstream) const;
    bool isValid() const { return m_data != nullptr; };

    int width() const { return m_width; };
    int height() const { return m_height; };
    DataFormat dataFormat() const { return m_dataFormat; };
    unsigned char* data() const { return m_data; };

    // Max and min values scaled to [0, 1], where 1 is max possible for data format
    float normalizedMinValue();
    float normalizedMaxValue();

    // Make non-copyable
    Image(Image const&) = delete;
    void operator=(Image const&) = delete;

    friend class ImageConverter;

private:
    int m_width;
    int m_height;
    DataFormat m_dataFormat;
    unsigned char* m_data;

    // Image intensity stats
    typedef struct {
        bool isValid;
        float minValue;
        float maxValue;
    } Stats;
    Stats m_stats;
    void computeStats();
};