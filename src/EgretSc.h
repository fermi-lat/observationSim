/**
 * @file EgretSc.h
 * @brief Declaration for the EGRET spacecraft object.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/src/EgretSc.h,v 1.2 2004/02/06 00:25:19 jchiang Exp $
 */

#ifndef observationSim_EgretSc_h
#define observationSim_EgretSc_h

#include "observationSim/Spacecraft.h"

namespace observationSim {

/**
 * @class EgretSc
 *
 * @brief Provide spacecraft attitude and orbital position information
 * for the EGRET instrument.
 *
 * This class mostly just encapsulates spacecraft data obtained from
 * the exposure history file.
 *
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/src/EgretSc.h,v 1.2 2004/02/06 00:25:19 jchiang Exp $
 */

class EgretSc : public Spacecraft {

public:

   EgretSc(astro::SkyDir &zAxis, astro::SkyDir &xAxis, 
           double earthLon, double earthLat, int inSaa) :
      m_zAxis(zAxis), m_xAxis(xAxis), m_earthLon(earthLon),
      m_earthLat(earthLat), m_inSaa(inSaa) {}

   EgretSc(double raz, double decz, double rax, double decx,
           double earthLon, double earthLat, int inSaa) :
      m_zAxis(astro::SkyDir(raz, decz, astro::SkyDir::EQUATORIAL)), 
      m_xAxis(astro::SkyDir(rax, decx, astro::SkyDir::EQUATORIAL)), 
      m_earthLon(earthLon), m_earthLat(earthLat), m_inSaa(inSaa) {}

   virtual ~EgretSc() {}

   virtual astro::SkyDir zAxis(double) {return m_zAxis;}
   virtual astro::SkyDir xAxis(double) {return m_xAxis;}

   virtual double EarthLon(double) {return m_earthLon;}
   virtual double EarthLat(double) {return m_earthLat;}

   virtual HepRotation InstrumentToCelestial(double);

   virtual int inSaa(double) {return m_inSaa;}

   virtual void getScPosition(double time, std::vector<double> & scPosition);

private:

   astro::SkyDir m_zAxis;
   astro::SkyDir m_xAxis;
   double m_earthLon;
   double m_earthLat;
   int m_inSaa;

};

} // namespace observationSim

#endif // observationSim_EgretSc_h
