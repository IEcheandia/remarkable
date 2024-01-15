
#include "imageWrite.h"
#include "filter/algoStl.h"			///< stl algo
#include "module/moduleLogger.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <fliplib/TypeToDataTypeImpl.h>

#include <limits>					///< byte max value

#include "image/image.h"
#include "common/bitmap.h"
#include <fstream>
namespace precitec {
using namespace geo2d;
using namespace interface;
using namespace image;
namespace filter {

const std::string ImageWrite::m_oFilterName("ImageWrite");
const std::string ImageWrite::m_oPipeOutName("Image");


ImageWrite::ImageWrite() :
		TransformFilter	( ImageWrite::m_oFilterName, Poco::UUID{"73be4c0e-dcf3-46d5-983b-f1216062fa38"} ),
		m_pPipeInImageFrame(nullptr),
		m_oPipeImageFrame(this, ImageWrite::m_oPipeOutName),
		m_oOutputFolder("."),
		m_oOutputFilename(""),
		m_oOutputFolderN(0),
		m_oOutputFilenameN(0)
{
	parameters_.add("OutputFolder", fliplib::Parameter::TYPE_string, m_oOutputFolder);
	parameters_.add("OutputFilename", fliplib::Parameter::TYPE_string, m_oOutputFilename);
	parameters_.add("OutputFolderN", fliplib::Parameter::TYPE_uint, m_oOutputFolderN);
	parameters_.add("OutputFilenameN", fliplib::Parameter::TYPE_uint, m_oOutputFilenameN);

    setInPipeConnectors({{Poco::UUID("b55ce6be-4f18-4c1b-99bf-c43cd51fae03"), m_pPipeInImageFrame, "ImageIn", 1, ""}});
    setOutPipeConnectors({{Poco::UUID("857cbab6-2035-4943-937e-f62009480730"), &m_oPipeImageFrame, "ImageOut", 0, ""}});
    setVariantID(Poco::UUID("3abbe576-4d93-4e13-896b-627c4ccd86fc"));
} // CTor.



/*virtual*/ ImageWrite::~ImageWrite()
{

} // DTor.



void ImageWrite::setParameter()
{
	TransformFilter::setParameter();
	m_oOutputFolder = parameters_.getParameter("OutputFolder").convert<std::string>();
	m_oOutputFilename = parameters_.getParameter("OutputFilename").convert<std::string>();
	m_oOutputFolderN = parameters_.getParameter("OutputFolderN").convert<unsigned int>();
	m_oOutputFilenameN = parameters_.getParameter("OutputFilenameN").convert<unsigned int>();

} // SetParameter



bool ImageWrite::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe(p_rPipe, p_oGroup);
}
// subscribe



void ImageWrite::proceed(const void* sender, fliplib::PipeEventArgs&  e)
{
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	// Empfangenes Frame auslesen
	const ImageFrame	&rFrame = m_pPipeInImageFrame->read(m_oCounter);
	const BImage		&rImage = rFrame.data();
	ResultType	curAnalysisResult = rFrame.analysisResult();

	const unsigned int			oImgWidth = rImage.size().width;
	const unsigned int			oImgHeight = rImage.size().height;
	long	oImgNumber = rFrame.context().imageNumber();

	std::ostringstream oFilename, oSliceNum, oDestDirName;
	oDestDirName << m_oOutputFolder << m_oOutputFolderN;

	Poco::File	oDestDir(oDestDirName.str());
	if (!oDestDir.exists())
	{
		oDestDir.createDirectories();
		wmLog(eInfo, "Created output folder %s", oDestDirName.str().c_str());
	} // if

	oSliceNum << std::setfill('0') << std::setw(3) << oImgNumber+1;
	oFilename << oDestDirName.str() << "/" << m_oOutputFilename << m_oOutputFilenameN << oSliceNum.str() << ".bmp";
	if (oImgNumber == 0)
	{
		wmLog(eInfo, "Writing to %s" , oFilename.str().c_str());
	}
	fileio::Bitmap oBitmap(oFilename.str().c_str(), oImgWidth, oImgHeight);
	if (oBitmap.isValid())
	{
		BImage InputImage = BImage(Size2D(oImgWidth, oImgHeight));
        rImage.copyPixelsTo(InputImage);
		UNUSED bool oLocalRes = oBitmap.save(InputImage.data());
		poco_assert_dbg(oLocalRes);
	}
	//output is the same as input
	ImageFrame oNewFrame(rFrame.context(), rImage, curAnalysisResult);

	preSignalAction(); m_oPipeImageFrame.signal(oNewFrame);

} // proceedGroup


} // namespace filter
} // namespace precitec
