/**
* @file
* @copyright    Precitec Vision GmbH & Co. KG
* @author       MM
* @date         2021
* @brief        Usefull enums for poreClassifierOutput and poreClassifierOutputTriple
*/

#pragma once

namespace precitec
{
namespace filter
{
    /**
    * @brief Set of pore candidate features.
    */
    enum FeatureType {
        eSize = 0,
        eBoundingBoxDX,
        eBoundingBoxDY,
        ePcRatio,
        eGradient,
        eSurface,
        eNbFeatureTypes = eSurface + 1
    };

    /**
    * @brief Set of pore classes.
    */
    enum PoreClassType {
        ePore = 0,          // all blob features lie within parameter range
        eNoPore,            // at leat one blob feature does not lie within parameter range
        ePoreIfScaled,      // all blob features lie within scaled parameter range, was not a pore without scaling
        eNoPoreIfScaled,    // all blob features lie within scaled parameter range, was a pore without scaling
        ePoreClassTypeMin = ePore,
        ePoreClassTypeMax = eNoPoreIfScaled
    };
}
}
