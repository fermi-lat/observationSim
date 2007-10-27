/**
 * @file LatSc.h
 * @brief Declaration for the LAT spacecraft object.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/src/LatSc.h,v 1.9 2006/11/06 23:59:59 jchiang Exp $
 */

#ifndef observationSim_LatSc_h
#define observationSim_LatSc_h

#include "observationSim/Spacecraft.h"

namespace observationSim {

/**
 * @class LatSc
 *
 * @brief Provide spacecraft attitude and orbital position information
 * for the LAT.
 *
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/src/LatSc.h,v 1.9 2006/11/06 23:59:59 jchiang Exp $
 */

class LatSc : public Spacecraft {

public:

   LatSc() : Spacecraft() {}
   LatSc(const std::string & ft2file);

   virtual ~LatSc() {}

   virtual astro::SkyDir zAxis(double time);
   virtual astro::SkyDir xAxis(double time);

   virtual double EarthLon(double time);
   virtual double EarthLat(double time);

   virtual HepRotation InstrumentToCelestial(double time);

   virtual bool inSaa(double time);

   virtual void getScPosition(double time, std::vector<double> & scPosition);

   virtual void getZenith(double time, double & ra, double & dec);

   virtual double livetimeFrac(double time) const;

private:

   double m_dt;
   std::vector<double> m_start;
   std::vector<double> m_livetimefrac;

};

} // namespace observationSim

#endif // observationSim_LatSc_h
