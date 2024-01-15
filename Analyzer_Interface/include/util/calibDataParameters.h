//legend of array indexes 
//(or should it be a class to convert from/to array?)	
//enum class cannot be used for vector index ( not implicitly convertible to integers)

#ifndef CALIBDATAPARAMETERS_H
#define CALIBDATAPARAMETERS_H
#include <vector>
#include <sstream>

namespace precitec {
namespace calibration {

	enum  CalibrationResultHeader
	{
		eResultOK,
		eTopLayerHigher,
		eHighLeftX,
		eHighLeftY,
		eHighRightX,
		eHighRightY,
		eLowLeftX,
		eLowLeftY,
		eLowRightX,
		eLowRightY,
		eNumElementsHigherLayer,
		eNumElementsLowerLayer,
		eNumberOfComponentsOfHeader //12
	};


	inline std::string describeResultHeader( std::vector<double> p_rResHeader )
	{
		std::ostringstream msg;
		msg << "Higher surface in calibration piece corresponds to top layer in image? ";
		if ( p_rResHeader[eResultOK] > 0 )
		{
			msg << "y";
		}
		else
		{
			msg << "n";
		}
			msg << "\n"
			<< "Higher layer boundary coordinates:"
			<< "\n\tleft " << p_rResHeader[eHighLeftX] << " " << p_rResHeader[eHighLeftY] 
			<< "\tright " << p_rResHeader[eHighRightX] << " " << p_rResHeader[eHighRightY]
			<< "\n\tnum elements: " << p_rResHeader[eNumElementsHigherLayer] << "\n"
			<< "Lower layer boundary coordinates:" 
			<< "\n\tleft " << p_rResHeader[eLowLeftX] << " " << p_rResHeader[eLowLeftY]
			<< "\tright " << p_rResHeader[eLowRightX] << " " << p_rResHeader[eLowRightY]
			<< "\n\tnum elements " << p_rResHeader[eNumElementsLowerLayer] << "\n";
		return msg.str();
	}
}
}

#endif
