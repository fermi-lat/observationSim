/**
 * @file Event.h
 * @brief Simple data structure to hold Event data.
 * @author J. Chiang
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/Event.h,v 1.2 2003/06/19 00:14:04 jchiang Exp $
 */

#ifndef observationSim_Event_h
#define observationSim_Event_h

//#include "CLHEP/Geometry/Vector3D.h"
#include "astro/SkyDir.h"

namespace observationSim {

/**
 * @class Event
 * @brief Simple data structure to hold Event data.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/Event.h,v 1.2 2003/06/19 00:14:04 jchiang Exp $
 */

class Event {

public:

   Event(double time, double energy, const astro::SkyDir &appDir, 
         const astro::SkyDir &srcDir, const astro::SkyDir &zAxis, 
         const astro::SkyDir &xAxis, const astro::SkyDir &zenith) :
      m_time(time), m_energy(energy), m_appDir(appDir), m_srcDir(srcDir),
      m_zAxis(zAxis), m_xAxis(xAxis), m_zenith(zenith) {}

   /// Time in seconds (referenced to the zero time of the orbit
   /// calculation in astro::EarthOrbit).
   double time() const {return m_time;}

   /// Photon energy in MeV.
   double energy() const {return m_energy;}

   /// Apparent photon direction in "Celestial" coordinates.
   astro::SkyDir appDir() const {return m_appDir;}

   /// True photon direction in "Celestial" coordinates.
   astro::SkyDir srcDir() const {return m_srcDir;}

   /// Instrument z-axis in "Celestial" coordinates.
   astro::SkyDir zAxis() const {return m_zAxis;}

   /// Instrument x-axis in "Celestial" coordinates.
   astro::SkyDir xAxis() const {return m_xAxis;}

   /// Zenith direction at spacecraft location in "Celestial" coordinates.
   astro::SkyDir zenith() const {return m_zenith;}

private:

   double m_time;
   double m_energy;
   astro::SkyDir m_appDir;
   astro::SkyDir m_srcDir;

   astro::SkyDir m_zAxis;
   astro::SkyDir m_xAxis;
   astro::SkyDir m_zenith;

};

} // namespace observationSim

#endif // observationSim_Event_h
