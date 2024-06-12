//
// Controller.h
// Controller in Model-View-Controller architecture. Implements the main user interface.
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
#include "RenderState.h"

class Model;
class GL_View;

class Controller : public QMainWindow
{
	Q_OBJECT

public:
    Controller(QWidget* parent);
	~Controller();

private slots:
    // File menu
    void onNew();
    void onOpen();
    void onSave();
    void onExport();
    void onExit();

    // Image menu
    void onResetWindowing();
    void onToggleImageInterpolation(bool interpolate);
    void onToggleImageFormat();

    // Contour menu
    void onDeselect();
    void onDeleteSelected();
    void onClearContour();
    void onSetContourColor();
    void onToggleContourVisibility(bool visible);
    void onFitSelectedToVessel();
    void onFitWidthOfSelected();

    // View menu
    void onResetView();

    // Help menu
    void onDisplayShortcutKeys(); 

private:
    Model* m_model;
    GL_View* m_view;
    RenderState m_renderState;
    void createMainWindow(GL_View* view);

    // File menu
    QAction m_newAction;
    QAction m_openAction;
    QAction m_saveAction;
    QAction m_exportAction;
    QAction m_exitAction;
    void prepareForLoad();
    void updateForLoaded();
    QMessageBox::StandardButton saveQuery();

    // Image menu
    QAction m_resetWindowingAction;
    QAction m_toggleImageInterpolationAction;
    QAction m_toggleImageFormatAction;

    // Contour menu
    QAction m_deselectAction;
    QAction m_deleteSelectedAction;
    QAction m_clearContourAction;
    QAction m_setContourColorAction;
    QAction m_toggleVisibilityAction;
    QAction m_fitSelectedToVesselAction;
    QAction m_fitWidthOfSelectedAction;
    void setContourColors(QColor color);

    // View menu
    QAction m_resetViewAction;

    // Help menu
    QAction m_DisplayShortcutKeysAction;

    // Overrides
    void closeEvent(QCloseEvent* event);
};