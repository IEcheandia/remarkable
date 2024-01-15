/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 *  @brief			Component-wide definitions
 */

#include "seamSearch.h"

#include "geo/array.h"			///< rank enum

#include "system/types.h"		///< typedefs


namespace precitec {
namespace filter {
	using geo2d::Doublearray;
	using interface::GeoDoublearray;


void calcRank(		
	Doublearray	&p_rSeamPosL, 
	Doublearray	&p_rSeamPosR, 
	int			p_oProfileSize, 
	int			p_oBoundaryLength) {

	poco_assert_dbg(p_rSeamPosL.size() == p_rSeamPosR.size());
	const unsigned int NbSeamPos	= p_rSeamPosL.size();

	for (unsigned int oSeamPosN = 0; oSeamPosN != NbSeamPos; ++oSeamPosN) {

		const double oSeamPosLeft	= p_rSeamPosL.getData()[oSeamPosN];
		const double oSeamPosRight	= p_rSeamPosR.getData()[oSeamPosN];

		auto&	rRankLeft	= p_rSeamPosL.getRank()[oSeamPosN];
		auto&	rRankRight	= p_rSeamPosR.getRank()[oSeamPosN]; 

		rRankLeft	= eRankMax; // init with max rank
		rRankRight	= eRankMax; // init with max rank 

		// check if index lies on boundary region
		const bool		oPosLeftIsBoundary	= (oSeamPosLeft < p_oBoundaryLength)	|| ( oSeamPosLeft >= p_oProfileSize - p_oBoundaryLength);
		const bool		oPosRightIsBoundary	= (oSeamPosRight < p_oBoundaryLength)	|| ( oSeamPosRight >= p_oProfileSize - p_oBoundaryLength);
		const bool		oPosLeftIsOnBorder	= (oSeamPosLeft <= 1)	|| ( oSeamPosLeft >= p_oProfileSize-1);
		const bool		oPosRightIsOnBorder	= (oSeamPosRight <= 1)	|| ( oSeamPosRight >= p_oProfileSize-1);

		if (oPosLeftIsBoundary) {
			rRankLeft /= 2; // divide by 2 if on boundary
		}

		if (oPosRightIsBoundary) {
			rRankRight /= 2; // divide by 2 if on boundary
		}

		// check if left index greater than right index
		const bool		oLIsGreaterThanR	= oSeamPosLeft > oSeamPosRight;

		if (oLIsGreaterThanR) {
			rRankLeft /= 3; // divide by 3 if left index greater than right index
			rRankRight /= 3; // divide by 3 if left index greater than right index
		}

		// check if indices are equal
		const bool		oPositionsEqual		= oSeamPosLeft == oSeamPosRight;

		if (oPositionsEqual) {
			rRankLeft /= 2; 
			rRankRight /= 2;
		}
		
		if (oPosLeftIsOnBorder) {
   			rRankLeft = eRankMin; // found nothing
        }

		if (oPosRightIsOnBorder) {
   			rRankRight = eRankMin; // found nothing
        }
        
	} // for

} // calcRank



void enforceIntegrity (
	Doublearray	&p_rContourL,
	Doublearray	&p_rContourR,
	int			p_oImgWidth,
	int			p_oDefaultSeamWidth/* = 100*/
) {
	poco_assert_dbg(p_rContourL.size() == p_rContourR.size());
	const unsigned int NbSeamPos	= p_rContourL.size();

	for (unsigned int oSeamPosN = 0; oSeamPosN != NbSeamPos; ++oSeamPosN) {

		double oSeamPosL	= p_rContourL.getData()[oSeamPosN];
		double oSeamPosR	= p_rContourR.getData()[oSeamPosN];

		int &rRankL	= p_rContourL.getRank()[oSeamPosN];
		int &rRankR	= p_rContourR.getRank()[oSeamPosN]; 

		if (oSeamPosR < oSeamPosL) {
			//std::cout << "oSeamPosL: " << oSeamPosL << std::endl;
			//std::cout << "oSeamPosR: " << oSeamPosR << std::endl;
			if (oSeamPosR < p_oImgWidth / 2) { // if right pos within left img half, move right pos
				oSeamPosR = oSeamPosL + p_oDefaultSeamWidth;
				rRankL /= 2; // divide by 2
				rRankR /= 3; // divide by 3
				//poco_stdout_dbg("Right position smaller than left position. Moved right pos to left plus default seam width."); // TODO invoke logger
			}
			else { // if right pos within right img half, move left pos
				oSeamPosL = oSeamPosR - p_oDefaultSeamWidth;
				rRankL /= 3; // divide by 3
				rRankR /= 2; // divide by 2
				//poco_stdout_dbg("Right position smaller than left position. Moved left pos to right minus default seam width."); // TODO invoke logger
			}
		}


		/// check for negative values and clip

		const auto oDefaultSeamWidthClipped =	std::min(p_oImgWidth - 1, p_oDefaultSeamWidth);

		if (oSeamPosL < 0) {
			oSeamPosL = 0;
			rRankL = eRankMin;
			//poco_stdout_dbg("Left data negative. Clipped to zero."); // TODO invoke logger
		}
		if (oSeamPosR < 0) {
			oSeamPosR = oDefaultSeamWidthClipped;
			rRankR = eRankMin;
			//poco_stdout_dbg("Right data negative. Clipped to default seam width."); // TODO invoke logger
		}


		/// check for too big and clip

		if (oSeamPosL > p_oImgWidth) {
			oSeamPosL = p_oImgWidth - oDefaultSeamWidthClipped;
			rRankL = eRankMin;
			//poco_stdout_dbg("Left data bigger than image width. Clipped to image width minus default seam width."); // TODO invoke logger
		}
		if (oSeamPosR > p_oImgWidth) {
			oSeamPosR = p_oImgWidth;
			rRankR = eRankMin;
			//poco_stdout_dbg("Right data negative. Clipped to zero."); // TODO invoke logger
		}

	}  // for

} // enforceIntegrity



} // namespace filter
} // namespace precitec


