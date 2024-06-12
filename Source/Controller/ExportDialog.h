//
// ExportDialog.h
// Dialog for setting parameters required to export a segmentation.
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

#include <QtWidgets>

#include "../View/GL_Exporter.h"

class ExportDialog : public QDialog
{
	Q_OBJECT

public:
    ExportDialog(int width, int height);
	~ExportDialog();

    float imageToExportScale() const;
    QString filename() const;
    GL_Exporter::ExportType exportType() const;

private:
    float m_imageWidth;
    float m_imageHeight;
    float m_width;
    float m_height;
    float m_scale;
    float m_maxFileDimension;
    float m_minScale;
    float m_maxScale;
    QString m_filename;

    QLabel* m_widthLabel;
    QLabel* m_heightLabel;
    QLabel* m_scaleLabel;
    QLabel* m_exportTypeLabel;
    QLabel* m_filenameLabel;
    QLineEdit* m_getWidth;
    QLineEdit* m_getHeight;
    QLineEdit* m_getScale;
    QComboBox* m_getExportType;
    QLineEdit* m_getFilename;
    QPushButton* m_browseFilename;
    QDialogButtonBox* m_dialogButtons;

    void onSetWidth();
    void onSetHeight();
    void onSetScale();
    void onSetFilename();
    void onBrowse();

    void updateWidth();
    void updateHeight();
};