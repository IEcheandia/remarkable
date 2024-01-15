#ifndef TRIGGERCONTEXT_H_
#define TRIGGERCONTEXT_H_

#include "system/templates.h"
#include "system/timer.h"
#include "message/messageBuffer.h"
#include "message/serializer.h"
#include "Poco/Mutex.h"

namespace precitec
{
namespace interface
{
	using system::message::Serializable;
	using system::message::MessageBuffer;
	using system::Timer;

	class TriggerContext : public Serializable {
		public:
			TriggerContext() : 
				imageNumber_(0),
				relTime_(0),
				m_Seam(0),
				m_SeamSeries(0),
				m_Product(0),
				HW_ROI_x0(0),
				HW_ROI_y0(0),
				HW_ROI_dx0(0),
				HW_ROI_dy0(0),
				isSingleShot_(false)
				{}
				
			// for calibration: single shot, NO inspection!
			TriggerContext(bool m_oSingleShot) : 
				imageNumber_(0),
				relTime_(0),
				m_Seam(0),
				m_SeamSeries(0),
				m_Product(0),
				HW_ROI_x0(1),
				HW_ROI_y0(0),
				HW_ROI_dx0(0),
				HW_ROI_dy0(0),
				isSingleShot_(m_oSingleShot)
				{}
				
			TriggerContext(TriggerContext const&tc, int imageNumber, Timer::Time time) :
				imageNumber_(imageNumber),
				relTime_(time),
				m_Seam(0),
				m_SeamSeries(0),
				m_Product(0),
				HW_ROI_x0(tc.HW_ROI_x0),
				HW_ROI_y0(tc.HW_ROI_y0),
				HW_ROI_dx0(tc.HW_ROI_dx0),
				HW_ROI_dy0(tc.HW_ROI_dy0),
				isSingleShot_(tc.isSingleShot_)
				{}

			/// die Zeit wird erst beim Bild-Ausloesen gesetzt
			TriggerContext(int imageNumber,int hwroix,int hwroiy) :
				imageNumber_(imageNumber),
				relTime_(0),
				m_Seam(0),
				m_SeamSeries(0),
				m_Product(0),
				HW_ROI_x0(hwroix),
				HW_ROI_y0(hwroiy),
				isSingleShot_(false)
				{}
				
			/// die Zeit wird erst beim Bild-Ausloesen gesetzt
			TriggerContext(int imageNumber,int hwroix,int hwroiy, int _productNumber, int _seamseriesNumber, int _seamNumber) :
				imageNumber_(imageNumber),
				relTime_(0),
				m_Seam(_seamNumber),
				m_SeamSeries(_seamseriesNumber),
				m_Product(_productNumber),
				HW_ROI_x0(hwroix),
				HW_ROI_y0(hwroiy),
				isSingleShot_(false)
				{}

            TriggerContext(const Poco::UUID &product, int serialNumber, int seamSeriesNumber, int seamNumber, int imageNumber)
                : imageNumber_(imageNumber)
                , relTime_(0)
                , m_Seam(seamNumber)
                , m_SeamSeries(seamSeriesNumber)
                , m_Product(serialNumber)
                , m_productInstance(product)
                , HW_ROI_x0(0)
                , HW_ROI_y0(0)
                , isSingleShot_(true)
            {
            }
				
			/// deserialisierungs-CTor
			// Deserialize/demarshal constructor
			TriggerContext(system::message::MessageBuffer const& buffer)
			{
				deserialize(buffer);
			}

			~TriggerContext() {}

			// wg Serializer abgeleiteter Klassen
			void swap(TriggerContext &tc);
			void serialize ( system::message::MessageBuffer &buffer ) const;
			void deserialize(system::message::MessageBuffer const&buffer);

			void sync(Timer::Time time) 				{ relTime_ = time; 		}		// Realtiver Timer neu setzen
			void setImageNumber (int image) 	{ imageNumber_ = image;	}
			void increaseImageNumber()           {imageNumber_++;}
			int imageNumber() 	const { return imageNumber_;	}
			
			void setSeamNumber(int _newValue)
			{
				m_Seam=_newValue;
			}
			
			int getSeamNumber() const
			{
				return m_Seam;
			}
			
			void setSeamSeriesNumber(int _newValue)
			{
				m_SeamSeries=_newValue;
			}
			
			int getSeamSeriesNumber() const
			{
				return m_SeamSeries;
			}
			
			void setProductNumber(int _newValue)
			{
				m_Product=_newValue;
			}
			
			int getProductNumber() const
			{
				return m_Product;
			}
			
			Timer::Time relativeTime()	const { return relTime_;		}
			friend  std::ostream &operator <<(std::ostream &os, TriggerContext const& v) {
				os << v.imageNumber_  <<" "<<v.relTime_; return os;
			}
			bool operator == (TriggerContext const& rhs) const {
				return (imageNumber_==rhs.imageNumber_)&& (HW_ROI_x0==rhs.HW_ROI_x0)&& (HW_ROI_y0==rhs.HW_ROI_y0) &&
						(HW_ROI_dx0==rhs.HW_ROI_dx0)&& (HW_ROI_dy0==rhs.HW_ROI_dy0) &&
						(m_cycleCount == rhs.cycleCount()) &&
						(relTime_==rhs.relTime_) && (isSingleShot_ == rhs.isSingleShot_);
			}
			TriggerContext& operator=(TriggerContext const &rhs)
			{
				imageNumber_ = rhs.imageNumber_;
				relTime_ = rhs.relTime_;
				HW_ROI_x0 = rhs.HW_ROI_x0;
				HW_ROI_y0 = rhs.HW_ROI_y0;
				HW_ROI_dx0 = rhs.HW_ROI_dx0;
				HW_ROI_dy0 = rhs.HW_ROI_dy0;
				isSingleShot_ = rhs.isSingleShot_;
				m_Seam = rhs.getSeamNumber();
				m_SeamSeries = rhs.getSeamSeriesNumber();
				m_Product = rhs.getProductNumber();
				m_cycleCount = rhs.cycleCount();
				return *this;
			}
			bool isSingleShot() const
			{
				return isSingleShot_;
			}
			void setSingleShot(const bool m_oSingleShot)
			{
				isSingleShot_ = m_oSingleShot;
			}

            Poco::UUID productInstance() const
            {
                return m_productInstance;
            }

            void setProductInstance(const Poco::UUID &id)
            {
                m_productInstance = id;
            }

            uint32_t cycleCount() const
            {
                return m_cycleCount;
            }
            void setCycleCount(uint32_t cycleCount)
            {
                m_cycleCount = cycleCount;
            }

		private:
			int imageNumber_;
			/// Zeit ab Taskstart
			Timer::Time relTime_;
			int m_Seam;
			int m_SeamSeries;
			int m_Product;
            Poco::UUID m_productInstance;
            uint32_t m_cycleCount = 0;
			
		public:
			/// Hardware Roi wird hier mitgeschleppt - muss hier noch weg
			int HW_ROI_x0;
			int HW_ROI_y0;
			int HW_ROI_dx0;
			int HW_ROI_dy0;
			bool isSingleShot_;  // should be true for single shots and NO inspection in cases such as calibrations!
	};

	inline void TriggerContext::serialize ( system::message::MessageBuffer &buffer ) const
	{
		int oSingleShot = (int)isSingleShot_;
		marshal(buffer, imageNumber_);
		marshal(buffer, oSingleShot);
		marshal(buffer, HW_ROI_x0);
		marshal(buffer, HW_ROI_y0);
		marshal(buffer, HW_ROI_dx0);
		marshal(buffer, HW_ROI_dy0);
		Serializable::marshal<Timer::Time>(buffer, relTime_);
		marshal(buffer, m_Seam);
		marshal(buffer, m_SeamSeries);
		marshal(buffer, m_Product);
        marshal(buffer, m_productInstance);
        marshal(buffer, m_cycleCount);
	}

	inline void TriggerContext::deserialize(system::message::MessageBuffer const&buffer)
	{
		isSingleShot_ = false;
		int oSingleShot = 0;
		deMarshal(buffer, imageNumber_);
		deMarshal(buffer, oSingleShot);
		deMarshal(buffer, HW_ROI_x0);
		deMarshal(buffer, HW_ROI_y0);
		deMarshal(buffer, HW_ROI_dx0);
		deMarshal(buffer, HW_ROI_dy0);
		if (oSingleShot)
		{
			isSingleShot_ = true;
		}
		relTime_ = Serializable::deMarshal<Timer::Time>(buffer);
		deMarshal(buffer, m_Seam);
		deMarshal(buffer, m_SeamSeries);
		deMarshal(buffer, m_Product);
        deMarshal(buffer, m_productInstance);
        deMarshal(buffer, m_cycleCount);
	}

	inline void TriggerContext::swap(TriggerContext &rhs)
	{
		std::swap(imageNumber_, rhs.imageNumber_);
		std::swap(relTime_, rhs.relTime_);
		std::swap(HW_ROI_x0, rhs.HW_ROI_x0);
		std::swap(HW_ROI_y0, rhs.HW_ROI_y0);
		std::swap(HW_ROI_dx0, rhs.HW_ROI_dx0);
		std::swap(HW_ROI_dy0, rhs.HW_ROI_dy0);
		std::swap(isSingleShot_, rhs.isSingleShot_);
		int iSave = rhs.getSeamNumber();
		rhs.setSeamNumber(m_Seam);
		m_Seam=iSave;
		iSave = rhs.getSeamSeriesNumber();
		rhs.setSeamSeriesNumber(m_SeamSeries);
		m_SeamSeries=iSave;
		iSave = rhs.getProductNumber();
		rhs.setProductNumber(m_Product);
		m_Product=iSave;
        const auto temp = rhs.cycleCount();
        rhs.setCycleCount(cycleCount());
        setCycleCount(temp);
	}

}
}
#endif /*TRIGGERCONTEXT_H_*/
