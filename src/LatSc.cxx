/**
 * @file LatSc.cxx
 * @brief Implementation of LatSc class.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/src/LatSc.cxx,v 1.11 2004/09/27 18:00:24 jchiang Exp $
 */

#include "astro/EarthCoordinate.h"
#include "astro/PointingTransform.h"
#include "astro/GPS.h"

#include "LatSc.h"

namespace observationSim {

astro::SkyDir LatSc::zAxis(double time) {
   HepRotation rotationMatrix = InstrumentToCelestial(time);
   return astro::SkyDir(rotationMatrix(Hep3Vector(0, 0, 1)),
                        astro::SkyDir::EQUATORIAL);
}

astro::SkyDir LatSc::xAxis(double time) {
   HepRotation rotationMatrix = InstrumentToCelestial(time);
   return astro::SkyDir(rotationMatrix(Hep3Vector(1, 0, 0)),
                        astro::SkyDir::EQUATORIAL);
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

void LatSc::getScPosition(double time, std::vector<double> & position) {
   Hep3Vector pos = GPS::instance()->position(time);
   position.clear();
   position.push_back(pos.x());
   position.push_back(pos.y());
   position.push_back(pos.z());
}

void LatSc::getZenith(double time, double & ra, double & dec) {
   ra = GPS::instance()->RAZenith();
   dec = GPS::instance()->DECZenith();
}

} // namespace observationSim
