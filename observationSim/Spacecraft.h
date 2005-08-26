/**
 * @file Spacecraft.h
 * @brief Declaration of Spacecraft base class.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/Spacecraft.h,v 1.6 2005/08/26 05:28:12 jchiang Exp $
 */

#ifndef observationSim_Spacecraft_h
#define observationSim_Spacecraft_h

#include <vector>
#include "astro/SkyDir.h"

namespace observationSim {

/**
 * @class Spacecraft
 *
 * @brief An abstract base class that defines the interface for
 * objects that provide information on spacecraft position and
 * attitude.
 *
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/Spacecraft.h,v 1.6 2005/08/26 05:28:12 jchiang Exp $
 */

class Spacecraft {

public:

   virtual ~Spacecraft() {}

   /// Spacecraft z-axis in J2000 coordinates.
   virtual astro::SkyDir zAxis(double time) = 0;

   /// Spacecraft x-axis in J2000 coordinates.
   virtual astro::SkyDir xAxis(double time) = 0;

   /// Earth longitude in degrees.
   virtual double EarthLon(double time) = 0;

   /// Earth latitude in degrees.
   virtual double EarthLat(double time) = 0;

   /// Rotation matrix from instrument to J2000 coordinates
   virtual HepRotation InstrumentToCelestial(double time) = 0;

   /// true if in SAA
   virtual bool inSaa(double time) = 0;

   /// Spacecraft position in geocentric coordinates (km)
   virtual void getScPosition(double time,
                              std::vector<double> & scPosition) = 0;

   virtual void getZenith(double time, double & ra, double & dec) = 0;

   virtual double livetimeFrac(double) const {
      return 1;
   }

   virtual void setLivetimeFrac(double) {};

};

} // namespace observationSim

#endif // observationSim_Spacecraft_h
