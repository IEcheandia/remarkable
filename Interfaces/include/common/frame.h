#ifndef FRAME_H_
#define FRAME_H_

#ifndef FRAMEINFO_H_
#define FRAMEINFO_H_

#include "Poco/SharedPtr.h"
#include "image/image.h"
#include "common/geoContext.h"
#include "event/results.h"

/**
 * Informationen zu jedem Bild (allg. Messwert aus einem Sensor)
 */


namespace precitec
{
namespace interface
{
	/**
	 * Ein Frame ist ein bild und sein zeitlicher und 'urspruenglicher' Kontext
	 * d.h. die Aufnahmezeit und der Sensor (Kamera, ...) von dem die Aufnahme
	 * erfolgte
	 */
	template <class DataT>
	class Frame
	{
	public:

		Frame() : m_oAnalysisResult(AnalysisOK) {}

		Frame(int sensorId, ImageContext const& context, DataT const& data, interface::ResultType p_oAnalysisResult = interface::AnalysisOK)
		:  context_(context), data_(data), m_oAnalysisResult(p_oAnalysisResult), m_oSensorId(sensorId) {
			SmpTrafo trafo_(context_.trafo());
		}

        Frame(ImageContext const& context, DataT const& data, interface::ResultType p_oAnalysisResult = interface::AnalysisOK)
            : Frame(-1, context, data, p_oAnalysisResult)
        {
        }

		Frame(Frame const& frame)
		:  context_(frame.context_), data_(frame.data_), m_oAnalysisResult(frame.m_oAnalysisResult), m_oSensorId(frame.m_oSensorId) {
		}

		void operator = (Frame const& rhs)
		{
			context_ = rhs.context_;
			data_ = rhs.data_;
			m_oAnalysisResult = rhs.m_oAnalysisResult;
            m_oSensorId = rhs.m_oSensorId;
		}

		void setAnalysisResult(interface::ResultType p_oRes)
		{
			m_oAnalysisResult = p_oRes;
		}

		auto analysisResult() const -> interface::ResultType 
		{
			return m_oAnalysisResult;
		}

        void setSensorId(int sensor)
        {
            m_oSensorId = sensor;
        }

        int sensorId() const
        {
            return m_oSensorId;
        }

	public:
		// Accessoren
		DataT & data()  		{ return data_; }
		DataT const& data() const 		{ return data_; }
		ImageContext const&context() const 	{ return context_; } 
        ImageContext& context() { return context_; } 

	private:
		ImageContext	context_;
		DataT  			data_;
		interface::ResultType m_oAnalysisResult;
        int m_oSensorId = -1;
	};

	/// Bild aus Kamera
	typedef Frame< image::BImage > 		ImageFrame;
	/// einzelner Messwert e.g. aus Photodiode
	typedef Frame< image::Sample > 		SampleFrame;

	inline image::BImage const& getImage(ImageFrame const& f) { return f.data(); }
	inline image::Sample const& getSample(SampleFrame const& f) { return f.data(); }
} // interface
} // precitec

#endif /*FRAMEINFO_H_*/


#endif /*FRAME_H_*/
