/**
 * @file Event.h
 * @brief Simple data structure to hold Event data.
 * @author J. Chiang
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/Event.h,v 1.4 2003/10/18 02:42:44 jchiang Exp $
 */

#ifndef observationSim_Event_h
#define observationSim_Event_h

#include "astro/SkyDir.h"

namespace observationSim {

/**
 * @class Event
 * @brief Simple data structure to hold Event data.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/Event.h,v 1.4 2003/10/18 02:42:44 jchiang Exp $
 */

class Event {

public:

   Event(double time, double energy, const astro::SkyDir &appDir, 
         const astro::SkyDir &srcDir, const astro::SkyDir &zAxis, 
         const astro::SkyDir &xAxis, const astro::SkyDir &zenith, 
         int eventType=4, double trueEnergy = 0., double flux_theta=0.,
         double flux_phi=0.) :
      m_time(time), m_energy(energy), m_appDir(appDir), m_srcDir(srcDir),
      m_zAxis(zAxis), m_xAxis(xAxis), m_zenith(zenith), m_eventType(eventType),
      m_trueEnergy(trueEnergy), m_flux_theta(flux_theta), 
      m_flux_phi(flux_phi) {}

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

   /// Event type (for ascertaining which set of IRFs to use).
   int eventType() const {return m_eventType;}

   /// True photon energy in MeV.
   double trueEnergy() const {return m_trueEnergy;}

   double fluxTheta() const {return m_flux_theta;}
   double fluxPhi() const {return m_flux_phi;}

private:

   double m_time;
   double m_energy;
   astro::SkyDir m_appDir;
   astro::SkyDir m_srcDir;

   astro::SkyDir m_zAxis;
   astro::SkyDir m_xAxis;
   astro::SkyDir m_zenith;

   int m_eventType;

   double m_trueEnergy;
   double m_flux_theta;
   double m_flux_phi;

};

} // namespace observationSim

#endif // observationSim_Event_h
