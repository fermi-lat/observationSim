/**
 * @file ScData.h
 * @brief Simple data structure to hold ScData data.
 * @author J. Chiang
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/ScData.h,v 1.2 2003/06/19 00:14:04 jchiang Exp $
 */

#ifndef observationSim_ScData_h
#define observationSim_ScData_h

//#include "CLHEP/Geometry/Vector3D.h"
#include "astro/SkyDir.h"

namespace observationSim {

/**
 * @class ScData
 * @brief Simple data structure to hold ScData data.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/ScData.h,v 1.2 2003/06/19 00:14:04 jchiang Exp $
 */

class ScData {

public:

   ScData(double time, double RAz, double Decz, double lon, 
          double lat, const astro::SkyDir &zAxis, const astro::SkyDir &xAxis,
          int inSAA) :
      m_time(time), m_RAz(RAz), m_Decz(Decz), m_lon(lon), 
      m_lat(lat), m_zAxis(zAxis), m_xAxis(xAxis), m_inSaa(inSAA) {}

   /// Time in seconds (referenced to the zero time of the orbit
   /// calculation in astro::EarthOrbit).
   double time() const {return m_time;}

   /// The RA of the instrument z-axis in degrees.
   double raz() const {return m_RAz;}

   /// The Dec of the instrument z-axis in degrees.
   double decz() const {return m_Decz;}

   /// The Earth longitude of the spacecraft in degrees.
   double lon() const {return m_lon;}

   /// The Earth latitude of the spacecraft in degrees.
   double lat() const {return m_lat;}

   /// The spacecraft z-axis in "Celestial" (J2000?) coordinates.
   astro::SkyDir zAxis() const {return m_zAxis;}

   /// The spacecraft x-axis in "Celestial" (J2000?) coordinates.
   astro::SkyDir xAxis() const {return m_xAxis;}

   /// Flag to indicate if the spacecraft is in the SAA.
   int inSaa() const {return m_inSaa;}

private:

   double m_time;
   double m_RAz;
   double m_Decz;
   double m_lon;
   double m_lat;
   astro::SkyDir m_zAxis;
   astro::SkyDir m_xAxis;
   int m_inSaa;
};

} // namespace observationSim

#endif // observationSim_ScData_h
