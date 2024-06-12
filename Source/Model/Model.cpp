//
// Model.cpp
// Implementation of Model.
//

#include "Model.h"
#include "Image.h"
#include "Math.h"

// 
// Public
//
Model::Model(RenderState* renderState) :
    m_renderState(renderState),
    m_isDrawing(false),
    m_selectionRadiusInWindowPixels(3),
    m_minSeparationInWindowPixels(2)
{
    m_imageFilterer = new ImageFilterer(&m_image);
}
Model::~Model()
{
    delete m_imageFilterer;
}

// 
// Public
//
void Model::clear()
{
    m_image.clear();
    m_contour.clear();
}
void Model::save(std::ofstream& file)
{
    try {
        if (!m_image.isValid()) {
            throw std::runtime_error("Image not valid.");
        }
        if (!file.is_open()) {
            throw std::runtime_error("File not open.");
        }

        file << m_version << std::endl;
        if (!m_image.writeToFile(file)) {
            throw std::runtime_error("Write image failed.");
        }
        if (!m_contour.writeToFile(file)) {
            throw std::runtime_error("Write contour failed.");
        }
    }
    catch (const std::exception& e) {
        std::cout << "Exception " << e.what() << endl;
    }

    file.close();
}
void Model::load(std::ifstream& file)
{
    try {
        if (!file.is_open()) {
            throw std::runtime_error("File not open.");
        }

        // Read version identifier
        std::string line;
        std::getline(file, line);
        if (line != m_version) {
            throw std::runtime_error("Unsupported file.");
        }

        // Read data
        if (!m_image.readFromFile(file)) {
            throw std::runtime_error("Read image failed.");
        }
        if (!m_contour.readFromFile(file)) {
            throw std::runtime_error("Read contour failed.");
        }
    }
    catch (const std::exception& e) {
    std::cout << "Exception " << e.what() << endl;
    }

    file.close();
}

void Model::startDraw(CurvePoint pStart)
{
    m_isDrawing = false;

    int idActiveCurve = m_contour.idActiveCurve();
    float minSeparation = m_minSeparationInWindowPixels * m_renderState->windowToContourScale();
    if (idActiveCurve >= 0) {

        // Begin overdrawing the selected curve
        m_isDrawing = m_contour.curve(idActiveCurve)->startDrawing(pStart, minSeparation);
    }
    if (!m_isDrawing) {

        // Create a new curve and select it to make it active
        deselect();
        idActiveCurve = m_contour.addCurve();

        // Begin drawing the new curve
        m_isDrawing = m_contour.curve(idActiveCurve)->startDrawing(pStart, minSeparation);
    }

    m_renderState->setContourNeedsUpdate(true);
    m_renderState->setActiveCurveNeedsUpdate(true);
}
void Model::updateDraw(CurvePoint point)
{
    if (!m_isDrawing) return;
    int idActiveCurve = m_contour.idActiveCurve();
    m_contour.curve(idActiveCurve)->addPoint(point);
    m_renderState->setActiveCurveNeedsUpdate(true);
}
void Model::endDraw(CurvePoint point)
{
    updateDraw(point);
    int idActiveCurve = m_contour.idActiveCurve();
    m_contour.curve(idActiveCurve)->endDrawing();
    m_renderState->setActiveCurveNeedsUpdate(true);
    m_isDrawing = false;
}

bool Model::select(float pos[2])
{
    float selectionRadius = m_selectionRadiusInWindowPixels * m_renderState->windowToContourScale();
    m_contour.selectCurve(Math::Vec2D(pos[0], pos[1]), selectionRadius);
    m_renderState->setContourNeedsUpdate(true);
    m_renderState->setActiveCurveNeedsUpdate(true);
    if (m_contour.idActiveCurve() >= 0) return true;
    else return false;
}
void Model::deselect()
{
    int idActiveCurve = m_contour.idActiveCurve();
    if (idActiveCurve >= 0) {
        m_contour.deselectCurve();
        m_renderState->setContourNeedsUpdate(true);
        m_renderState->setActiveCurveNeedsUpdate(true);
    }
}
void Model::deleteSelected()
{
    int idActiveCurve = m_contour.idActiveCurve();
    if (idActiveCurve >= 0) {
        m_contour.removeCurve(idActiveCurve);
        m_renderState->setContourNeedsUpdate(true);
        m_renderState->setActiveCurveNeedsUpdate(true);
    }
}

ImageFilterer::VesselContrastType Model::vesselContrast() const
{
    return m_imageFilterer->vesselContrastType();
}
void Model::setVesselContrast(ImageFilterer::VesselContrastType contrastType)
{
    m_imageFilterer->setVesselContrastType(contrastType);
}
void Model::fitSelectedToNearestVessel(float expectedRadius)
{
    try {
        if (!m_image.isValid()) {
            throw std::runtime_error("No image available for curve fitting.");
        }

        Curve* activeCurve = m_contour.curve(m_contour.idActiveCurve());
        if (!activeCurve) return;
        std::list<CurvePoint>& points = activeCurve->points();
        if (points.size() == 0) return;

        // Fit curve points to the nearest vessel. These constants were set by trial 
        // and error. For a better fit, consider more tries and a smaller moveConst. For
        // speed, consider fewer tries and a larger moveConst.
        int numTries = 10;
        float moveConst = 0.5;  // Set between 0 and 1
        Math::Vec2D* moveVecs = nullptr;
        moveVecs = new Math::Vec2D[points.size()];
        for (int i = 0; i < numTries; i++) {
            Math::Vec2D* pMoveVecs = moveVecs;
            for (std::list<CurvePoint>::iterator it = points.begin(); it != points.end(); it++) {
                // Compute move vector for each curve point
                *pMoveVecs++ = m_imageFilterer->getVecToClosestVessel(it->pos(), expectedRadius);
            }
            pMoveVecs = moveVecs;
            for (std::list<CurvePoint>::iterator it = points.begin(); it != points.end(); it++) {
                // Move curve points in direction of centerline
                it->setPos(it->pos() + moveConst * *pMoveVecs++);
            }
        }
        delete[] moveVecs;

        // Smooth the curve points. This helps prevent kinks in the fitted curve.
        activeCurve->applySmoothing(Curve::SmoothingType::Points);
    }
    catch (std::bad_alloc& e) {
        std::cout << "Memory Allocation " << "No memory for curve fitting." << e.what() << endl;
    }
    catch (std::exception & e) {
        std::cout << "Exception " << e.what() << endl;
    }
}
void Model::fitSelectedVesselWidth(float expectedRadius)
{
    try {
        if (!m_image.isValid()) {
            throw std::runtime_error("No image available for width detection.");
        }

        Curve* activeCurve = m_contour.curve(m_contour.idActiveCurve());
        if (!activeCurve) return;
        std::list<CurvePoint>& points = activeCurve->points();
        if (points.size() <= 1) return;

        for (std::list<CurvePoint>::iterator it = points.begin(); it != points.end(); it++) {
            std::list<CurvePoint>::iterator itPrev = (it == points.begin()) ? it : std::prev(it);
            std::list<CurvePoint>::iterator itNext = (std::next(it) == points.end()) ? it : std::next(it);
            Math::Vec2D predictedP;
            Math::Vec2D curveDir = itNext->pos() - itPrev->pos();
            curveDir.normalize();
            float width = m_imageFilterer->getWidthAtP(it->pos(), curveDir, expectedRadius);
            it->setRadius(0.5 * width);
        }

        // First/last widths may be bad due to bad tangents. Use the widths of adjacent 
        // points
        std::list<CurvePoint>::iterator it = std::prev(points.end());
        it->setRadius(std::prev(it)->radius());
        it = points.begin();
        it->setRadius(std::next(it)->radius());

        // Smooth the curve widths
        activeCurve->applySmoothing(Curve::SmoothingType::Widths);
    }
    catch (std::exception& e) {
        std::cout << "Exception " << e.what() << endl;
    }
}