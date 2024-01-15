/**
 *  @file       selectLayerRoi.h
 *  @ingroup    Filter_Utility
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		LB
 *  @date		2020
 *  @brief		Generate x,y coordinates and Z values accordig to the current seam number
 */

#include  "scanmasterFocusPoint.h"
#include <filter/productData.h>
#include "common/systemConfiguration.h"
#include <overlay/overlayCanvas.h>
#include "overlay/overlayPrimitive.h"
#include <fliplib/TypeToDataTypeImpl.h>

#include "module/moduleLogger.h"

namespace precitec {
using fliplib::SynchronePipe;
using fliplib::PipeEventArgs;
using fliplib::Parameter;
using interface::GeoDoublearray;
using geo2d::Doublearray;

namespace filter {

    ScanmasterFocusPoint::ScanmasterFocusPoint()
    : TransformFilter("ScanmasterFocusPoint", Poco::UUID{"3DA8DF53-C12F-4894-B678-B050D19EA408"}),
    m_pPipeInImageFrame( nullptr ),
    m_oPipeOutStep(this,"Step"),
    m_oPipeOutRelativePosition (this,"ParametricPosition"),
    m_oPipeOutX1(this, "x1"),
    m_oPipeOutY1(this, "y1"),
    m_oPipeOutX2(this, "x2"),
    m_oPipeOutY2(this, "y2"),
    m_oPipeOutZ(this, "z")
    {
        parameters_.add("NumIncrements", Parameter::TYPE_uint, m_numIncrementsPerSide);
        parameters_.add("DeltaX", Parameter::TYPE_double, m_deltaXmm);
        parameters_.add("DeltaY", Parameter::TYPE_double, m_deltaYmm);
        parameters_.add("DeltaZ", Parameter::TYPE_double, m_deltaZmm);
        parameters_.add("LengthX", Parameter::TYPE_double, m_lengthXmm);
        parameters_.add("LengthY", Parameter::TYPE_double, m_lengthYmm);
        parameters_.add("FeatureStartAndMiddle", Parameter::TYPE_bool, m_featureStartAndMiddle);        

        setInPipeConnectors({{Poco::UUID("FC3E4F23-AEDE-4292-A74E-7EBE5B94A13D"),m_pPipeInImageFrame,"Image", 0, "image"}});
        setOutPipeConnectors({
            {Poco::UUID("600191BA-3259-4A0C-B3D6-96B1B8896B93"),&m_oPipeOutStep, "Step", 0, ""},
            {Poco::UUID("25951DCC-D3C2-4F37-B731-A1CF83DB8339"),&m_oPipeOutRelativePosition, "ParametricPosition"},
            {Poco::UUID("4B99E8B5-A7E0-46B1-A785-108792C7ECA2"),&m_oPipeOutX1, "x1"},
            {Poco::UUID("A935CD3A-4D9C-4F1F-92E9-33D43475A913"),&m_oPipeOutY1, "y1"},
            {Poco::UUID("351BB559-BD1C-4BE4-ABCD-F2CA2FE722C3"),&m_oPipeOutX2, "x2"},
            {Poco::UUID("EBE17873-A0DA-48D1-8231-F241D3150DF8"),&m_oPipeOutY2, "y2"},
            {Poco::UUID("AEFBF03F-1C1E-4ED1-9337-ED4ABFAB41D0"),&m_oPipeOutZ, "z"}
        });
        setVariantID(Poco::UUID("DA1B290A-47D9-4586-994E-57C57BD17788"));
    }
    
    ScanmasterFocusPoint::~ScanmasterFocusPoint()
    {
        
    }
    
    void ScanmasterFocusPoint::setParameter()
    {
        TransformFilter::setParameter();

        m_numIncrementsPerSide = parameters_.getParameter("NumIncrements").convert<unsigned int>();
        m_deltaXmm = parameters_.getParameter("DeltaX").convert<double>();
        m_deltaYmm = parameters_.getParameter("DeltaY").convert<double>();
        m_deltaZmm = parameters_.getParameter("DeltaZ").convert<double>();
        m_lengthXmm = parameters_.getParameter("LengthX").convert<double>();
        m_lengthYmm = parameters_.getParameter("LengthY").convert<double>();
        m_featureStartAndMiddle = parameters_.getParameter("FeatureStartAndMiddle").convert<bool>();
        
        m_numberOfSteps = 2*m_numIncrementsPerSide + 1;
        bool isSCANMASTER_ThreeStepInterface = interface::SystemConfiguration::instance().getBool("SCANMASTER_ThreeStepInterface", false);
        
        m_seamOffset = isSCANMASTER_ThreeStepInterface ?  2 * m_numberOfSteps : 0;
        m_currentStep = -1;
    }
    
    bool ScanmasterFocusPoint::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
    {
        
        m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < interface::ImageFrame >* >(&p_rPipe);
        return BaseFilter::subscribe( p_rPipe, p_oGroup );
        
    }
    
    void ScanmasterFocusPoint::proceed(const void * sender, fliplib::PipeEventArgs & e)
    {
        poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
        
        const interface::ImageFrame &rFrame (m_pPipeInImageFrame->read(m_oCounter));
        
        const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
        auto & oCurrentSeam = pProductData->m_oSeam;
        
        auto createSingleValueGeoDoubleArray = [&](double value, bool valid)
        {
            return GeoDoublearray(rFrame.context(), geo2d::Doublearray(1, value, valid ? eRankMax : eRankMin), rFrame.analysisResult(), 1.0);
        };
        
        if (oCurrentSeam < m_seamOffset || oCurrentSeam >= int(m_seamOffset + m_numberOfSteps) || m_numberOfSteps < 1)
        {
            m_currentStep = -1.0;
            wmLog(eDebug, "Seam %d: no welding (not in %d - %d) \n", oCurrentSeam, m_seamOffset,(m_seamOffset + m_numberOfSteps -1));
            GeoDoublearray oNullDoubleArray = createSingleValueGeoDoubleArray (0.0, false);
            
            preSignalAction();
            m_oPipeOutStep.signal( createSingleValueGeoDoubleArray (m_currentStep, false));
            m_oPipeOutRelativePosition.signal(oNullDoubleArray);
            m_oPipeOutX1.signal(oNullDoubleArray);
            m_oPipeOutY1.signal(oNullDoubleArray);
            m_oPipeOutX2.signal(oNullDoubleArray);
            m_oPipeOutY2.signal(oNullDoubleArray);
            m_oPipeOutZ.signal(oNullDoubleArray);
            return;
        }
        
        
        //interpolate values according to current seam value
        
        m_currentStep = oCurrentSeam - m_seamOffset;
        m_currentRelativePosition = (m_numberOfSteps <= 1) ?  0.0 : 2 * m_currentStep / double(m_numberOfSteps - 1) -1.0;
        assert(m_currentRelativePosition >= (-1.0 - 1e-7) || m_currentRelativePosition <= (-1.0 + 1e-7));
        
        const auto & k = m_currentRelativePosition * (m_numberOfSteps - 1)/2;
        
        m_currentZ = k * m_deltaZmm;
        
        //compute center segment coordinates

        double x = k * m_deltaXmm;
        double y = k * m_deltaYmm;
        
        double halfLengthX = m_lengthXmm / 2.0;
        double halfLengthY = m_lengthYmm / 2.0;
        if (m_featureStartAndMiddle)
        {
            if (m_currentStep == 0)
            {   // first segment 10% longer 
                halfLengthX *= 1.1;
                halfLengthY *= 1.1;
                
            }
            if (m_currentStep == int(m_numIncrementsPerSide))
            {
                // mid segment 20% longer   
                halfLengthX *= 1.2;
                halfLengthY *= 1.2;
            }            
        }
        m_currentSegmentCoordinates[0].x = x - halfLengthX;
        m_currentSegmentCoordinates[0].y = y - halfLengthY;
        m_currentSegmentCoordinates[1].x = x + halfLengthX;
        m_currentSegmentCoordinates[1].y = y + halfLengthY;
        
        //send hw result only on the first image
        m_validHWResult = (m_oCounter == 0);
        
        auto oOutStep = createSingleValueGeoDoubleArray (m_currentStep, true);
        auto oOutRelativePosition = createSingleValueGeoDoubleArray(m_currentRelativePosition, true);
        auto oOutX1 = createSingleValueGeoDoubleArray(m_currentSegmentCoordinates[0].x, m_validHWResult);
        auto oOutY1 = createSingleValueGeoDoubleArray(m_currentSegmentCoordinates[0].y, m_validHWResult);
        auto oOutX2 = createSingleValueGeoDoubleArray(m_currentSegmentCoordinates[1].x, m_validHWResult);
        auto oOutY2 = createSingleValueGeoDoubleArray(m_currentSegmentCoordinates[1].y, m_validHWResult);
        auto oOutZ = createSingleValueGeoDoubleArray(m_currentZ, m_validHWResult);
        
        preSignalAction();
        m_oPipeOutStep.signal(oOutStep);
        m_oPipeOutRelativePosition.signal( oOutRelativePosition );
        m_oPipeOutX1.signal(oOutX1);
        m_oPipeOutY1.signal(oOutY1);
        m_oPipeOutX2.signal(oOutX2);
        m_oPipeOutY2.signal(oOutY2);
        m_oPipeOutZ.signal(oOutZ);
        
    }
    
    void ScanmasterFocusPoint::paint()
    {
        using namespace image;
        using namespace geo2d;
        
        if ((m_oVerbosity < eMedium)) 
        {
            return;
        }

        OverlayCanvas &rCanvas ( canvas<OverlayCanvas>(m_oCounter) );

        OverlayLayer &rLayerText ( rCanvas.getLayerText());
        const auto oTextFont = Font(16);
        const auto oTextSize = Size (500, 20);
        auto oTextPosition = Point(0,20);
        auto oColor = Color::Green();
        
        if ((m_oVerbosity >= eHigh)) 
        {   
            double n = static_cast<double>(m_numIncrementsPerSide);
            //seam numbers as in the productData, not visual numbers
            int firstSeam = m_seamOffset;
            int midSeam = m_seamOffset + m_numIncrementsPerSide;
            int lastSeam = m_seamOffset + 2*m_numIncrementsPerSide;
            //add + 1 to the seam number in the text (seam visualNumber)
            std::vector<std::string> oMsgList;
            {
                std::ostringstream oMsg;
                oMsg << "First Weld : center (" << - n * m_deltaXmm  << " , " << - n * m_deltaYmm << ") "
                    "Z " << - n * m_deltaZmm  << " mm (seam " << firstSeam + 1  << ")";
                oMsgList.push_back(oMsg.str());
            }
            {
                std::ostringstream oMsg;
                oMsg << "Mid Weld : center ( 0.0, 0.0) Z 0.0 mm (seam " << midSeam + 1 << ")";
                oMsgList.push_back(oMsg.str());
            }
            {
                std::ostringstream oMsg;
                oMsg << "Last Weld : center (" << n * m_deltaXmm  << " , " << n * m_deltaYmm << ") "
                    "Z " << n * m_deltaZmm  << " mm (seam " << lastSeam + 1 << ")";
                oMsgList.push_back(oMsg.str());
            }
            
        
            for (std::string & rMsg : oMsgList)
            {
                rLayerText.add<OverlayText>( rMsg, oTextFont, Rect(oTextPosition, oTextSize), oColor);
                oTextPosition.y += 30;
            }
        }
        oColor = m_validHWResult ? Color::Orange() : Color::Red();
        if (m_currentStep  == -1)
        {
            rLayerText.add<OverlayText>("No Segment To Weld", oTextFont, Rect(oTextPosition, oTextSize), oColor);
            return;
        }
        
        std::vector<std::string> oMsgList;
        {
            std::ostringstream oMsg;
            oMsg << "Current Step " << m_currentStep + 1 << " / " << m_numberOfSteps << " t = " << m_currentRelativePosition;
            oMsgList.push_back(oMsg.str());
        }
        oMsgList.push_back(std::string{"Z = " + std::to_string(m_currentZ)});
        for (auto & rPoint : m_currentSegmentCoordinates)
        {
            std::ostringstream oMsg;
            oMsg << " x: " <<rPoint.x << " y: " << rPoint.y;
            oMsgList.push_back(oMsg.str());
        }
        
        for (std::string & rMsg : oMsgList)
        {
            rLayerText.add<OverlayText>( rMsg, oTextFont, Rect(oTextPosition, oTextSize), oColor);
            oTextPosition.y += 30;
        }
        

    }
    
} //end namespace
}
