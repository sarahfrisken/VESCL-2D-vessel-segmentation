//
// Controller.cpp
// Implementation of Controller.
// 

#include "Controller.h"
#include "ExportDialog.h"
#include "RenderState.h"
#include "../View/GL_View.h"
#include "../View/GL_Exporter.h"
#include "../Model/Model.h"
#include "../Model/ImageFilterer.h"
#include "../Model/ImageConverter.h"

#include<iostream>
#include<fstream>

// 
// Public
//
Controller::Controller(QWidget* parent) :
	QMainWindow(parent),
	m_model(nullptr),
	m_view(nullptr) 
{
	try {
		// Create the model and the view 
		m_model = new Model(&m_renderState);
		m_view = new GL_View(m_model, &m_renderState);

		// Create the main window for the controller
		createMainWindow(m_view);

		// Set initial colors for the contour and cursor
		QColor contorColor(253, 223, 103, 150);
		setContourColors(contorColor);
	}
	catch (const std::exception& e) {
		std::cout << "Exception " << "Error creating main window." << e.what() << std::endl;
	}
}
Controller::~Controller()
{
	delete m_view;
	delete m_model;
}

void Controller::createMainWindow(GL_View* view)
{
	setWindowTitle(tr("2D Vessel Contouring"));
	resize(600, 600);

	// Main window components and layouts
	QWidget* centralWidget = new QWidget;
	setCentralWidget(centralWidget);

	// OpenGL window
	QVBoxLayout* centralLayout = new QVBoxLayout(centralWidget);
	centralLayout->addWidget(m_view);

	// Menu
	QMenuBar* menuBar = new QMenuBar;
	setMenuBar(menuBar);

	// File menu
	QMenu* menuFile = menuBar->addMenu(tr("&File"));
	m_newAction.setText(tr("New"));
	m_openAction.setText(tr("Open"));
	m_saveAction.setText(tr("Save"));
	m_exportAction.setText(tr("Export"));
	m_exitAction.setText(tr("Exit"));
	m_newAction.setShortcut(QKeySequence::New);
	m_openAction.setShortcut(QKeySequence::Open);
	m_saveAction.setShortcut(QKeySequence::Save);
	m_exitAction.setShortcut(QKeySequence(Qt::Key_Escape));
	menuFile->addAction(&m_newAction);
	menuFile->addAction(&m_openAction);
	menuFile->addSeparator();
	menuFile->addAction(&m_saveAction);
	menuFile->addAction(&m_exportAction);
	menuFile->addSeparator();
	menuFile->addAction(&m_exitAction);
	connect(&m_newAction, &QAction::triggered, this, &Controller::onNew);
	connect(&m_openAction, &QAction::triggered, this, &Controller::onOpen);
	connect(&m_saveAction, &QAction::triggered, this, &Controller::onSave);
	connect(&m_exportAction, &QAction::triggered, this, &Controller::onExport);
	connect(&m_exitAction, &QAction::triggered, this, &Controller::onExit);

	// Image menu
	QMenu* menuImage = menuBar->addMenu(tr("&Image"));
	m_resetWindowingAction.setText(tr("Reset brightness and contrast"));
	m_toggleImageInterpolationAction.setCheckable(true);
	m_toggleImageInterpolationAction.setChecked(false);
	m_toggleImageInterpolationAction.setText(tr("Image smoothing"));
	m_toggleImageFormatAction.setCheckable(true);
	m_toggleImageFormatAction.setChecked(false);
	m_toggleImageFormatAction.setText(tr("Look for light vessels"));
	menuImage->addAction(&m_resetWindowingAction);
	menuImage->addAction(&m_toggleImageInterpolationAction);
	menuImage->addSeparator();
	menuImage->addAction(&m_toggleImageFormatAction);
	connect(&m_resetWindowingAction, &QAction::triggered, this, &Controller::onResetWindowing);
	connect(&m_toggleImageInterpolationAction, &QAction::triggered, this, &Controller::onToggleImageInterpolation);
	connect(&m_toggleImageFormatAction, &QAction::triggered, this, &Controller::onToggleImageFormat);

	// Contour menu
	QMenu* menuContour = menuBar->addMenu(tr("&Contour"));
	m_deselectAction.setText(tr("Deselect"));
	m_deleteSelectedAction.setText(tr("Delete selected"));
	m_clearContourAction.setText(tr("Delete all"));
	m_setContourColorAction.setText(tr("Set contour color"));
	m_toggleVisibilityAction.setCheckable(true);
	m_toggleVisibilityAction.setChecked(true);
	m_toggleVisibilityAction.setText(tr("Visibility on"));
	m_fitSelectedToVesselAction.setText(tr("Fit selected to vessel"));
	m_fitWidthOfSelectedAction.setText(tr("Set width of selected"));
	m_deselectAction.setShortcut(QKeySequence(Qt::Key_D));
	m_deleteSelectedAction.setShortcut(QKeySequence::Delete);
	m_toggleVisibilityAction.setShortcut(QKeySequence(Qt::Key_V));
	m_fitSelectedToVesselAction.setShortcut(QKeySequence(Qt::Key_F));
	m_fitWidthOfSelectedAction.setShortcut(QKeySequence(Qt::Key_W));
	menuContour->addAction(&m_deselectAction);
	menuContour->addAction(&m_deleteSelectedAction);
	menuContour->addAction(&m_clearContourAction);
	menuContour->addSeparator();
	menuContour->addAction(&m_setContourColorAction);
	menuContour->addAction(&m_toggleVisibilityAction);
	menuContour->addSeparator();
	menuContour->addAction(&m_fitSelectedToVesselAction);
	menuContour->addAction(&m_fitWidthOfSelectedAction);
	connect(&m_deselectAction, &QAction::triggered, this, &Controller::onDeselect);
	connect(&m_deleteSelectedAction, &QAction::triggered, this, &Controller::onDeleteSelected);
	connect(&m_clearContourAction, &QAction::triggered, this, &Controller::onClearContour);
	connect(&m_setContourColorAction, &QAction::triggered, this, &Controller::onSetContourColor);
	connect(&m_toggleVisibilityAction, &QAction::triggered, this, &Controller::onToggleContourVisibility);
	connect(&m_fitSelectedToVesselAction, &QAction::triggered, this, &Controller::onFitSelectedToVessel);
	connect(&m_fitWidthOfSelectedAction, &QAction::triggered, this, &Controller::onFitWidthOfSelected);

	// View menu
	QMenu* menuView = menuBar->addMenu(tr("&View"));
	m_resetViewAction.setText(tr("Reset view"));
	m_resetViewAction.setShortcut(QKeySequence(Qt::Key_R));
	menuView->addAction(&m_resetViewAction);
	connect(&m_resetViewAction, &QAction::triggered, this, &Controller::onResetView);

	// Help menu
	QMenu* menuHelp = menuBar->addMenu(tr("&Help"));
	m_DisplayShortcutKeysAction.setText(tr("Display shortcuts"));
	menuHelp->addAction(&m_DisplayShortcutKeysAction);
	connect(&m_DisplayShortcutKeysAction, &QAction::triggered, this, &Controller::onDisplayShortcutKeys);
}

//
// Private slots
//
void Controller::onNew()
{
	// Import the new image
	QString filename = QFileDialog::getOpenFileName(0, ("New Image"), ("*.jpg;;*.jpeg;;*.png"), QDir::currentPath());
	if (!filename.isEmpty() && !filename.isNull()) {
		std::ifstream file(filename.toStdString().c_str(), std::ios::binary);
		prepareForLoad();
		QImage inputImage(filename);
		ImageConverter ic;
		ic.imageFromQImage(m_model->image(), inputImage);
		updateForLoaded();
	}
}
void Controller::onOpen()
{
	// Load a saved VESCL file
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);
	QString filename = QFileDialog::getOpenFileName(0, ("Open"), QDir::currentPath(), tr("*.vscl"));
	if (!filename.isEmpty() && !filename.isNull()) {
		std::ifstream file(filename.toStdString().c_str(), std::ios::binary);
		prepareForLoad();
		m_model->load(file);
		updateForLoaded();
	}
}
void Controller::onSave()
{
	// Finalize any curve drawing or editing before saving
	m_model->deselect();

	// Save the image and contour in a VESCL file
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::AnyFile);
	QString filename = QFileDialog::getSaveFileName(0, ("Save"), QDir::currentPath(), tr("*.vscl"));
	if (!filename.isEmpty() && !filename.isNull()) {
		if (QFileInfo(filename).suffix() != tr("vscl")) filename.append(".vscl");
		std::ofstream file(filename.toStdString().c_str(), std::ios::binary);
		m_model->save(file);
	}
}
void Controller::onExport()
{
	// Finalize any curve drawing or editing before exporting
	m_model->deselect();

	// Get export parameters and export the selected format
	ExportDialog exportDialog(m_model->imageWidth(), m_model->imageHeight());
	exportDialog.exec();
	if (exportDialog.result() == QDialog::Accepted) {
		float imageToExportScale = exportDialog.imageToExportScale();
		QString filename = exportDialog.filename();
		GL_Exporter::ExportType exportType = exportDialog.exportType();
		GL_Exporter exporter(m_model);
		exporter.exportSegmentation(filename.toStdString().c_str(), imageToExportScale, exportType);
	}
}
void Controller::onExit()
{
	close();
}

// Image menu
void Controller::onResetWindowing()
{
	m_renderState.setDefaultWindowing();
	m_renderState.setImageNeedsUpdate(true);
	m_view->update();
}
void Controller::onToggleImageInterpolation(bool interpolate)
{
	m_renderState.setImageInterpolation(interpolate);
	m_renderState.setImageNeedsUpdate(true);
	m_view->update();
}
void Controller::onToggleImageFormat()
{
	if (m_model->vesselContrast() == ImageFilterer::VesselContrastType::LightOnDark) {
		m_model->setVesselContrast(ImageFilterer::VesselContrastType::DarkOnLight);
		m_toggleImageFormatAction.setText(tr("Light vessels"));
	}
	else {
		m_model->setVesselContrast(ImageFilterer::VesselContrastType::LightOnDark);
		m_toggleImageFormatAction.setText(tr("Dark vessels"));
	}
}

// Contour menu
void Controller::onDeselect()
{
	m_model->deselect();
	m_renderState.setContourNeedsUpdate(true);
	m_view->update();
}
void Controller::onDeleteSelected()
{
	m_model->deleteSelected();
	m_renderState.setContourNeedsUpdate(true);
	m_view->update();
}
void Controller::onClearContour()
{
	m_model->contour()->clear();
	m_renderState.setContourNeedsUpdate(true);
	m_view->update();
}
void Controller::onSetContourColor()
{
	QColor color = QColorDialog::getColor(m_renderState.contourColor(),
		nullptr, "Contour Color", QColorDialog::ShowAlphaChannel);
	setContourColors(color);
}
void Controller::setContourColors(QColor color)
{
	// Use the given color for selected curves
	m_renderState.setActiveCurveColor(color);

	// Mute the color slightly for curves that are not selected
	QColor contourColor;
	contourColor.setHsv(color.hue(), color.saturation(), 0.8 * color.value());
	contourColor.setAlpha(color.alpha());
	m_renderState.setContourColor(contourColor);

	m_renderState.setContourNeedsUpdate(true);
	m_view->setCursorColor(contourColor);
	m_view->update();
}
void Controller::onToggleContourVisibility(bool visible)
{
	m_renderState.setContourVisible(visible);
	m_renderState.setContourNeedsUpdate(true);
	m_view->update();
}
void Controller::onFitSelectedToVessel()
{
	float expectedRadius = m_view->cursorRadius() * m_renderState.windowToContourScale();
	m_model->fitSelectedToNearestVessel(expectedRadius);
	m_renderState.setActiveCurveNeedsUpdate(true);
	m_view->update();
}
void Controller::onFitWidthOfSelected()
{
	float expectedRadius = m_view->cursorRadius() * m_renderState.windowToContourScale();
	m_model->fitSelectedVesselWidth(expectedRadius);
	m_renderState.setActiveCurveNeedsUpdate(true);
	m_view->update();
}

// View menu
void Controller::onResetView()
{
	float prevWindowToContourScale = m_renderState.windowToContourScale();
	m_renderState.resetWorldToView();
	float divisor = std::max(std::numeric_limits<float>::epsilon(), m_renderState.windowToContourScale());
	float scaleDelta = prevWindowToContourScale / divisor;
	m_view->setCursorRadius(m_view->cursorRadius() * scaleDelta);
	m_renderState.setNeedsFullUpdate(true);
	m_view->update();
}

// Help menu
void Controller::onDisplayShortcutKeys()
{
	QString shortcutText;
	shortcutText.append("<b>File management</b> <br />");
	shortcutText.append("Ctrl+N:  Load new image <br />");
	shortcutText.append("Ctrl+O:  Open existing contouring file <br />");
	shortcutText.append("Ctrl+S:  Save current contouring file <br />");
	shortcutText.append("Esc key:  Exit <br />");
	shortcutText.append("<br />");

	shortcutText.append("<b>Contour drawing</b> <br />");
	shortcutText.append("LMB+drag:  Draw new curve/overdraw selected curve <br />");
	shortcutText.append("Mouse wheel:  Modify brush width<br />");
	shortcutText.append("<br />");

	shortcutText.append("<b>Contour selection</b> <br />");
	shortcutText.append("RMB click on curve centerline:  Select curve <br />");
	shortcutText.append("D:  Deselect currently selected curve <br />");
	shortcutText.append("Del key:  Delete selected curve <br />");
	shortcutText.append("<br />");

	shortcutText.append("<b>Contour editing</b> <br />");
	shortcutText.append("F:  Fit the selected curve to nearest vessel <br />");
	shortcutText.append("W:  Set vessel widths along selected curve <br />");
	shortcutText.append("V:  Togle contour visibility on/off <br />");
	shortcutText.append("<br />");

	shortcutText.append("<b>Image transforms and windowing</b> <br />");
	shortcutText.append("R:  Re-center image <br />");
	shortcutText.append("Shift+LMB+drag:  Translate image <br />");
	shortcutText.append("Ctrl+LMB+drag up/down:  Zoom image in/out<br />");
	shortcutText.append("Alt+LMB+drag:  Adjust brightness/contrast<br />");
	shortcutText.append("<br />");

	shortcutText.append("<b>Key:</b> <br />");
	shortcutText.append("LMB = left mouse button <br />");
	shortcutText.append("RMB = right mouse button <br />");

	QMessageBox helpSheet;
	helpSheet.setWindowTitle("Keyboard Shortcuts and Mouse Input");
	helpSheet.setTextFormat(Qt::RichText);
	helpSheet.setText(shortcutText);
	helpSheet.setStandardButtons(QMessageBox::Ok);
	helpSheet.exec();
}


//
// Private
//
void Controller::prepareForLoad()
{
	if (m_model->imageIsValid()) {
		QMessageBox::StandardButton save = saveQuery();
		if (save == QMessageBox::Cancel) {
			return;
		}
		else if (save == QMessageBox::Yes) {
			onSave();
		}
	}
	m_model->clear();
}
void Controller::updateForLoaded()
{
	// Reset image windowing for rendering
	float minValue = m_model->imageNormalizedMin();
	float maxValue = m_model->imageNormalizedMax();
	m_renderState.setWindowingRange(minValue, maxValue);
	m_renderState.setDefaultWindowing();

	// Set contour visibility on
	m_toggleVisibilityAction.setChecked(true);
	m_renderState.setContourVisible(true);

	// Initialize transforms to center the view in the viewport
	m_renderState.centerImageInViewport(m_model->imageWidth(), m_model->imageHeight());

	// Update view
	m_view->initCursor();
	m_view->setImage(m_model->image());
	m_renderState.setNeedsFullUpdate(true);
	m_view->update();
}
QMessageBox::StandardButton Controller::saveQuery()
{
	return 	QMessageBox::question(this, "VESCL",
		tr("Save contour?\n"),
		QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
		QMessageBox::Yes);
}

void Controller::closeEvent(QCloseEvent* event)
{
	// Save previous work?
	QMessageBox::StandardButton save = saveQuery();
	if (save == QMessageBox::Cancel) {
		event->ignore();
	}
	else if (save == QMessageBox::Yes) {
		onSave();
		event->accept();
	}
	else {
		event->accept();
	}
}