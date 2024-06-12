//
// ExportDialog.cpp
// Implementation of ExportDialog.
// 

#include "ExportDialog.h"
#include "../View/GL_Exporter.h"

// 
// Public
//
ExportDialog::ExportDialog(int width, int height) :
	m_imageWidth(width),
	m_imageHeight(height),
	m_width(width),
	m_height(height),
	m_scale(1),
	m_maxFileDimension(2048),
	m_minScale(0.1), 
	m_maxScale(100)
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	QGroupBox* sizeGroup = new QGroupBox(tr("Export image size"));
	QFormLayout* sizeLayout = new QFormLayout(sizeGroup);
	m_getWidth = new QLineEdit();
	m_getWidth->setValidator(new QIntValidator(1, m_maxFileDimension, this));
	m_getWidth->setText(QString::number(m_imageWidth));
	m_widthLabel = new QLabel(tr("Width: "));
	m_getHeight = new QLineEdit();
	m_getHeight->setValidator(new QIntValidator(1, m_maxFileDimension, this));
	m_getHeight->setText(QString::number(m_imageHeight));
	m_heightLabel = new QLabel(tr("Height: "));
	m_getScale = new QLineEdit();
	m_getScale->setText(QString::number(m_scale));
	m_getScale->setValidator(new QDoubleValidator(m_minScale, m_maxScale, 2, this));
	m_scaleLabel = new QLabel(tr("Scale: "));

	sizeLayout->insertRow(0, m_widthLabel, m_getWidth);
	sizeLayout->insertRow(1, m_heightLabel, m_getHeight);
	sizeLayout->insertRow(2, m_scaleLabel, m_getScale);
	mainLayout->addWidget(sizeGroup);

	connect(m_getWidth, &QLineEdit::textEdited, this, &ExportDialog::onSetWidth);
	connect(m_getHeight, &QLineEdit::textEdited, this, &ExportDialog::onSetHeight);
	connect(m_getScale, &QLineEdit::textEdited, this, &ExportDialog::onSetScale);

	QGroupBox* typeGroup = new QGroupBox(tr("Export type"));
	QFormLayout* typeLayout = new QFormLayout(typeGroup);
	m_getExportType = new QComboBox();
	m_getExportType->addItem("Binary", (int)GL_Exporter::ExportType::Binary);
	m_getExportType->addItem("Distance to vessel", (int)GL_Exporter::ExportType::DistToVesselEdge);
	m_getExportType->addItem("Distance to centerline", (int)GL_Exporter::ExportType::DistToCenterline);
	m_getExportType->addItem("Source image", (int)GL_Exporter::ExportType::SourceImage);
	m_exportTypeLabel = new QLabel(tr("Export type: "));

	typeLayout->insertRow(0, m_exportTypeLabel, m_getExportType);
	mainLayout->addWidget(typeGroup);

	QGroupBox* fileGroup = new QGroupBox(tr("Export file"));
	QHBoxLayout* filenameLayout = new QHBoxLayout(fileGroup);
	m_filenameLabel = new QLabel(tr("Filename: "));
	m_getFilename = new QLineEdit(tr("output.png"));
	m_browseFilename = new QPushButton(tr("Browse"));

	filenameLayout->addWidget(m_filenameLabel);
	filenameLayout->addWidget(m_getFilename);
	filenameLayout->addWidget(m_browseFilename);
	mainLayout->addWidget(fileGroup);

	connect(m_browseFilename, &QPushButton::clicked, this, &ExportDialog::onBrowse);

	QDialogButtonBox::StandardButtons buttonBox = QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
	m_dialogButtons = new QDialogButtonBox(buttonBox);

	mainLayout->addWidget(m_dialogButtons);

	connect(m_dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
ExportDialog::~ExportDialog()
{
	delete m_getWidth;
	delete m_widthLabel;
	delete m_getHeight;
	delete m_heightLabel;
	delete m_getScale;
	delete m_scaleLabel;

	delete m_filenameLabel;
	delete m_getFilename;
	delete m_browseFilename;

	delete m_getExportType;
	delete m_exportTypeLabel;
	delete m_dialogButtons;
}

float ExportDialog::imageToExportScale()  const
{ 
	return m_getScale->text().toFloat();
}
QString ExportDialog::filename()  const
{ 
	return m_getFilename->text();
}
GL_Exporter::ExportType ExportDialog::exportType()  const
{
	return (GL_Exporter::ExportType)m_getExportType->currentData().toInt();
}

void ExportDialog::onSetWidth()
{
	float newWidth = m_getWidth->text().toFloat();
	float newScale = newWidth / m_imageWidth;
	m_getScale->setText(QString::number(newScale));
	m_getHeight->setText(QString::number((int)(newScale * m_imageHeight)));
}
void ExportDialog::onSetHeight()
{
	float newHeight = m_getHeight->text().toFloat();
	float newScale = newHeight / m_imageHeight;
	m_getScale->setText(QString::number(newScale));
	m_getWidth->setText(QString::number((int)(newScale * m_imageWidth)));
}
void ExportDialog::onSetScale()
{
	float newScale = m_getScale->text().toFloat();
	m_getWidth->setText(QString::number((int)(newScale * m_imageWidth)));
	m_getHeight->setText(QString::number((int)(newScale * m_imageHeight)));
}
void ExportDialog::onSetFilename()
{
	QString filename = m_getFilename->text();
	QFileInfo info(filename);
	QString ext = info.completeSuffix();
	if (!(ext == ".jpg" || ext == ".jpeg" || ext == ".png")) {
		filename = info.path() + "/" + info.completeBaseName() + ".png";
	}
}
void ExportDialog::onBrowse()
{
	QString filename = QFileDialog::getSaveFileName(0, ("Export File"), QDir::currentPath(),
		tr("JPEG (*.jpg *.jpeg);;PNG (*.png)"));
	m_getFilename->setText(filename);
}

//
// Private
//
void ExportDialog::updateWidth()
{
	m_width = (int)(m_imageWidth* m_scale);
	assert(m_width > 0);
}
void ExportDialog::updateHeight()
{
	m_height = (int)(m_imageHeight * m_scale);
	assert(m_height > 0);
}