//
// ImageFilterer.cpp
// Implementation of ImageFilterer.
//

#include"ImageFilterer.h"

using Math::Vec2D;

// 
// Public
//
ImageFilterer::ImageFilterer(Image* image, VesselContrastType contrastType) :
	m_image(image),
	m_type(contrastType)
{
	try {
		if (!m_image->isValid()) {
			throw std::runtime_error("Image not valid.");
		}
	}
	catch (const std::exception& e) {
		std::cout << "Exception " << e.what() << std::endl;
	}
}
ImageFilterer::~ImageFilterer()
{
}

Vec2D ImageFilterer::getVecToClosestVessel(Vec2D pos, float expectedRadius)
{
	// Select a reasonable sigma that is narrow wrt the expected width
	float sigma = (expectedRadius <= 0.5) ? 0.5 : ((expectedRadius <= 1.5) ? 1 : 1.5);
	int filterRadius = (int)(3.0 * sigma);
	int filterWidth = 2 * filterRadius + 1;
	int filterSize = filterWidth * filterWidth;

	// Ignore points on the edge of the image to avoid complicated edge cases. This 
	// should be revisited if vessels lie close to the image edge
	int w = m_image->width();
	int h = m_image->height();
	int i = (int)pos[0];
	int j = (int)pos[1];
	if (i < filterRadius || i > w - filterRadius ||
		j < filterRadius || j > h - filterRadius) {
		return Vec2D(0, 0);
	}

	// Compute constants
	float sigmaInv = 1.0 / sigma;
	float filterScale = 1.0 / (sigma * sqrt(2.0 * 3.14159265359));

	// Convolve the image near the given point with Gaussian-blurred moment filters to 
	// compute the image moments and gradient at the given point. If this becomes a
	// limiting step, consider computing all the required image values with a single 
	// call so the interpolation constants aren't recomputed for every value
	float m00 = 0;
	float m10 = 0;
	float m01 = 0;
	Vec2D gradient(0, 0);
	float min = imageValueAtP(pos);
	for (int jj = -filterRadius; jj <= filterRadius; jj++) {
		float fy = jj;
		float fyy = fy * fy;
		for (int ii = -filterRadius; ii <= filterRadius; ii++) {
			float fx = ii;
			float fxx = fx * fx;
			float scaledExpXY = filterScale * exp(-0.5f * (fxx + fyy));
			Vec2D p = pos + Vec2D(ii, jj);
			float imgValue = imageValueAtP(p);
			if (imgValue < min) min = imgValue;
			m00 += imgValue;
			m10 += -imgValue * ii;
			m01 += -imgValue * jj;
			gradient += Vec2D(fx, fy) * imgValue * scaledExpXY * sigmaInv;
		}
	}

	// Compute center of mass (COM) corrected with image bias and contrast. The 
	// uncorrected COM is { m10 / m00, m01 / m00 }. 
	Vec2D com(0, 0);
	float correctedMean = m00 - min * filterSize;
	if (correctedMean > 0) {
		// This should always be true, unless the image values in the filter area are
		// all the same, in which case the COM is at the center of the image patch, 
		// i.e., (0, 0).
		com = Vec2D(m10 / correctedMean, m01 / correctedMean);
	}

	// Compute the image gradient at the given point. The image gradient computed 
	// above moves from light to dark.
	gradient.normalize();

	// Compute the move vector, i.e., the component of the vector from the given point
	// to the COM that lies in the direction of the gradient vector
	float moveMag = Vec2D::dotProduct(com, gradient);
	Vec2D moveVec;
	if (m_type == VesselContrastType::DarkOnLight) {
		moveVec = moveMag * gradient;
	}
	else {
		moveVec = -moveMag * gradient;
	}
	return moveVec;
}
float ImageFilterer::getWidthAtP(Vec2D curvePoint, Vec2D curveDir, float expectedRadius)
{
	// Filtering parameters dependent on the expected width. Use a Gaussian filter  
	// with standard deviation of sigma. Filter values outside filterRadius = 2*sigma
	// are small and can be ignored
	static float sigma = 0;				// Gaussian filter standard deviation; (min,max) = (0.5,1.5)
	static int filterRadius = 0;		// (int) (2.0 * sigma + 0.5); max: 3 
	static float samplesPerPixel = 10;	
	static int numFilterValues = 0;		// (2 * filterRadius + 1) * samplesPerPixel; max: 70
	static float filterValue[200];		// Big enough to hold max number of filter values

	// Filter is the 1st derivative of the Gaussian. When convolved with a vessel cross-section
	// expect a positive peak on one edge and a negative peak on the other edge.
	// Only compute new filter values if necessary
	float newSigma = (expectedRadius <= 0.5) ? 0.5 : ((expectedRadius <= 1.5) ? 1 : 1.5);
	if (sigma != newSigma) {
		// Recompute filter values
		sigma = newSigma;
		filterRadius = (int)(2.0 * sigma + 0.5);
		numFilterValues = 2 * filterRadius * samplesPerPixel + 1;
		float sampleSpacing = 1.0 / samplesPerPixel;
		for (int i = 0; i < numFilterValues; i++) {
			float x = (float)(i - numFilterValues / 2) * sampleSpacing;
			float scale = -x / ((double)sigma * sigma * sigma * sqrt(2 * 3.1415926));
			filterValue[i] = scale * exp(-(x * x) / (2.0 * sigma * sigma));
		}
	}

	// Sample the image along a line through the curve point & perpendicular to the curve. Create 
	// the array of sample points
	int samplesRadius = int((expectedRadius + (float)filterRadius) * samplesPerPixel + 0.5);
	int numSamplePoints = 2 * samplesRadius + 1;
	float* samples = new float[numSamplePoints];
	float* filtered = new float[numSamplePoints];
	Vec2D offsetVector(-curveDir[1], curveDir[0]);
	for (int i = 0; i < numSamplePoints; i++) {
		float distFromCenterPoint = (float)(i - numSamplePoints / 2) / samplesPerPixel;
		Vec2D p = curvePoint + distFromCenterPoint * offsetVector;
		samples[i] = imageValueAtP(p);
	}

	// Convolve sample point values with the filter to get filtered sample point values
	int filterCenterOffset = numFilterValues / 2;
	for (int i = 0; i < numSamplePoints; i++) {
		filtered[i] = 0;
		for (int j = 0; j < numFilterValues; j++)
		{
			int idxNeighbor = i + (j - filterCenterOffset);
			if (idxNeighbor < 0) {
				filtered[i] += filterValue[j] * samples[0];
			}
			else if (idxNeighbor >= numSamplePoints) {
				filtered[i] += filterValue[j] * samples[numSamplePoints - 1];
			}
			else {
				filtered[i] += filterValue[j] * samples[idxNeighbor];
			}
		}
	}

	// The filtered values should have peaks at the vessel edges. We expect one peak to
	// be positive and one to be negative. We expect the distance between peaks to be
	// similar to twice the expectedRadius, and we expect the curve centerline to be 
	// midway between the two points.
	float idxMinEdge = -1;
	float idxMaxEdge = -1;
	float min = 0;
	float max = 0;
	for (int i = 0; i < numSamplePoints; i++) {
		if (filtered[i] < min) {
			idxMinEdge = i;
			min = filtered[i];
		}
		if (filtered[i] > max) {
			idxMaxEdge = i;
			max = filtered[i];
		}
	}
	float width = fabs(idxMaxEdge - idxMinEdge) / samplesPerPixel;

	// Clean up memory and return
	delete[] samples;
	delete[] filtered;
	return width;
}

// 
// Private
//
float ImageFilterer::imageValueAtP(Vec2D p)
{
	// Get image value at p using bi-linear interpolation
	float x = p[0];
	float y = p[1];
	int i0 = x;
	int j0 = y;
	int i1 = x + 1;
	int j1 = y + 1;

	// Check bounds
	if (i0 < 0) i0 = 0;
	if (i1 < 0) i1 = 0;
	if (i0 > m_image->width() - 1) i0 = m_image->width() - 1;
	if (i1 > m_image->width() - 1) i1 = m_image->width() - 1;
	if (j0 < 0) j0 = 0;
	if (j1 < 0) j1 = 0;
	if (j0 > m_image->height() - 1) j0 = m_image->height() - 1;
	if (j1 > m_image->height() - 1) j1 = m_image->height() - 1;

	// Get the neigbor data values depending on image type
	float d[4];
	switch (m_image->dataFormat())
	{
	case Image::DataFormat::UShort:
	{
		unsigned short* data = (unsigned short*)m_image->data();
		d[0] = (float)data[i0 + j0 * m_image->width()];
		d[1] = (float)data[i1 + j0 * m_image->width()];
		d[2] = (float)data[i0 + j1 * m_image->width()];
		d[3] = (float)data[i1 + j1 * m_image->width()];
		break;
	}
	case Image::DataFormat::UChar:
	default:
	{
		unsigned char* data = m_image->data();
		d[0] = (float)data[i0 + j0 * m_image->width()];
		d[1] = (float)data[i1 + j0 * m_image->width()];
		d[2] = (float)data[i0 + j1 * m_image->width()];
		d[3] = (float)data[i1 + j1 * m_image->width()];
		break;
	}
	}
	float s = x - (float)i0;
	float t = y - (float)j0;
	float value = (1 - s) * (1 - t) * d[0] +
		s * (1 - t) * d[1] + (1 - s) * t * d[2] + s * t * d[3];
	return value;
}
