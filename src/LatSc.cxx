/**
 * @file LatSc.cxx
 * @brief Implementation of LatSc class.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/src/LatSc.cxx,v 1.8 2003/11/15 06:04:45 jchiang Exp $
 */

#include "astro/EarthCoordinate.h"
#include "astro/PointingTransform.h"
#include "astro/GPS.h"

#include "LatSc.h"

namespace observationSim {

astro::SkyDir LatSc::zAxis(double time) {
   HepRotation rotationMatrix = InstrumentToCelestial(time);
   return astro::SkyDir(rotationMatrix(Hep3Vector(0, 0, 1)),
                        astro::SkyDir::CELESTIAL);
}

astro::SkyDir LatSc::xAxis(double time) {
   HepRotation rotationMatrix = InstrumentToCelestial(time);
   return astro::SkyDir(rotationMatrix(Hep3Vector(1, 0, 0)),
                        astro::SkyDir::CELESTIAL);
}

double LatSc::EarthLon(double time) {
//   astro::GPS *gps = astro::GPS::instance();
   GPS *gps = GPS::instance();
   gps->getPointingCharacteristics(time);
   return gps->lon();
}

double LatSc::EarthLat(double time) {
//   astro::GPS *gps = astro::GPS::instance();
   GPS *gps = GPS::instance();
   gps->getPointingCharacteristics(time);
   return gps->lat();
}

HepRotation LatSc::InstrumentToCelestial(double time) {
// Get the rotation matrix from instrument to Galactic coordinates
// from GPS.
//    astro::GPS *gps = astro::GPS::instance();
//   GPS *gps = GPS::instance();
//    gps->getPointingCharacteristics(time);
//    HepRotation glastToGalactic(gps->transformGlastToGalactic(time));

// // Create astro::SkyDir objects for the instrument axes. 
//    astro::SkyDir xAxisCel(glastToGalactic(Hep3Vector(1, 0, 0)),
//                           astro::SkyDir::GALACTIC);
//    astro::SkyDir yAxisCel(glastToGalactic(Hep3Vector(0, 1, 0)),
//                           astro::SkyDir::GALACTIC);
//    astro::SkyDir zAxisCel(glastToGalactic(Hep3Vector(0, 0, 1)),
//                           astro::SkyDir::GALACTIC);

// // Return the desired rotation matrix.
//    return HepRotation(xAxisCel(), yAxisCel(), zAxisCel());

// Use astro::PointingTransform class.  Let's hope this continues to
// work as the flux package changes.

//   astro::GPS *gps = astro::GPS::instance();
   GPS *gps = GPS::instance();
   gps->getPointingCharacteristics(time);
   astro::SkyDir xAxis(gps->RAX(), gps->DECX());
   astro::SkyDir zAxis(gps->RAZ(), gps->DECZ());

   astro::PointingTransform transform(zAxis, xAxis);
   return transform.localToCelestial();
}

int LatSc::inSaa(double time) {
   astro::EarthCoordinate earthCoord( EarthLat(time), EarthLon(time) );
   return earthCoord.insideSAA();
}

} // namespace observationSim
