/**
 * @file MapSource.h
 * @brief A source class for the flux package that uses FITS images as
 * templates for the photon distribution on the sky.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/users/jchiang/fluxSources/fluxSources/MapSource.h,v 1.4 2004/04/15 15:53:03 jchiang Exp $
 */

#ifndef mySpectrum_MapSource_h
#define mySpectrum_MapSource_h

#include <valarray>

#include "flux/Spectrum.h"

//namespace fluxSources {

/**
 * @class MapSource
 *
 * @brief A source class for the flux package that uses FITS images as
 * templates for the photon distribution on the sky.
 *
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/users/jchiang/fluxSources/fluxSources/MapSource.h,v 1.4 2004/04/15 15:53:03 jchiang Exp $
 */

class MapSource : public Spectrum {

public:

   /// This constructor is required and used in FluxSource for
   /// "SpectrumClass" sources.
   MapSource(const std::string &params);

   virtual ~MapSource(){}

   /// @return Particle energy in MeV.
   /// @param xi Uniform random deviate on the unit interval.
   virtual float operator() (float xi) const;

   /// @return Particle type, "gamma".
   virtual const char * particleName() const {return "gamma";}

   /// @return Total flux (photons/m^2).
   /// @param time Simulation time in seconds.
   virtual double flux(double time) const;

   /// @return "Effective" solid angle (sr).
   virtual double solidAngle() const;

   /// @return Title describing the spectrum.
   virtual std::string title() const {return "MapSource";}

   /// @return Interval to the next event (seconds)
   virtual double interval(double time);

   /// @return Photon energy (MeV).
   virtual double energy(double time);

   /// @return Photon direction in (l, b).
   virtual std::pair<double, double> dir(double energy);

   /// @return Integral over the map.
   double mapIntegral() const {
      return m_mapIntegral;
   }

private:

   double m_flux;
   double m_gamma;
   double m_emin;
   double m_emax;

   std::vector<std::string> m_axisTypes;
   std::vector<double> m_lon;
   std::vector<double> m_lat;
   std::valarray<double> m_lonArray;
   std::valarray<double> m_latArray;
   std::valarray<double> m_solidAngles;
   std::valarray<double> m_image;

   std::vector<double> m_integralDist;

   double m_mapIntegral;

   void samplePixel(unsigned int indx, double &lon, double &lat);
   double mapValue(unsigned int i, unsigned int j);

   void readFitsFile(std::string fitsFile);

};

//} // namespace fluxSources

#endif // mySpectrum_MapSource_h
