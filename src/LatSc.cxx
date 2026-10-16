/**
 * @file LatSc.cxx
 * @brief Implementation of LatSc class.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/ScienceTools-scons/observationSim/src/LatSc.cxx,v 1.25 2009/06/11 16:49:10 jchiang Exp $
 */

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "tip/IFileSvc.h"
#include "tip/Table.h"

#include "astro/EarthCoordinate.h"
#include "astro/PointingTransform.h"
#include "astro/GPS.h"

#include "LatSc.h"

namespace observationSim {

LatSc::LatSc(const std::string & ft2file) : Spacecraft() {
   const tip::Table * scData = 
      tip::IFileSvc::instance().readTable(ft2file, "SC_DATA");
   tip::Table::ConstIterator it(scData->begin());
   tip::ConstTableRecord & row(*it);
   for ( ; it != scData->end(); ++it) {
      m_start.push_back(row["START"].get());
      m_stop.push_back(row["STOP"].get());
      double duration(m_stop.back() - m_start.back());
      m_livetimefrac.push_back(row["livetime"].get()/duration);
   }
   m_dt = (m_stop.back() - m_start.front())/m_start.size();
   delete scData;
}

auto LatSc::zAxis(double time) -> astro::SkyDir {
   CLHEP::HepRotation rotationMatrix = InstrumentToCelestial(time);
   return astro::SkyDir(rotationMatrix(CLHEP::Hep3Vector(0, 0, 1)),
                        astro::SkyDir::EQUATORIAL);
}

auto LatSc::xAxis(double time) -> astro::SkyDir {
   CLHEP::HepRotation rotationMatrix = InstrumentToCelestial(time);
   return astro::SkyDir(rotationMatrix(CLHEP::Hep3Vector(1, 0, 0)),
                        astro::SkyDir::EQUATORIAL);
}

auto LatSc::EarthLon(double time) -> double {
   astro::GPS * gps(astro::GPS::instance());
   gps->time(time);
   return gps->lon();
}

auto LatSc::EarthLat(double time) -> double {
   astro::GPS * gps(astro::GPS::instance());
   gps->time(time);
   return gps->lat();
}

auto LatSc::InstrumentToCelestial(double time) -> CLHEP::HepRotation {
   astro::GPS * gps(astro::GPS::instance());
   gps->time(time);
   astro::PointingTransform transform(gps->zAxisDir(), gps->xAxisDir());
   return transform.localToCelestial();
}

auto LatSc::inSaa(double time) -> bool {
   if (::getenv("DISABLE_SAA")) {
      return false;
   }
   astro::GPS * gps(astro::GPS::instance());
   return gps->earthpos(time).insideSAA();
}

void LatSc::getScPosition(double time, std::vector<double> & position) {
   CLHEP::Hep3Vector pos = astro::GPS::instance()->position(time);
   position.clear();
// GPS returns the position in units of km, but FT2 wants meters so
// we multiply by 10^3.
   double mperkm(1e3);
   position.push_back(pos.x()*mperkm);
   position.push_back(pos.y()*mperkm);
   position.push_back(pos.z()*mperkm);
}

void LatSc::getZenith(double time, double & ra, double & dec) {
   astro::GPS * gps(astro::GPS::instance());
   gps->time(time);
   astro::SkyDir zenithDir(gps->zenithDir());
   ra = zenithDir.ra();
   dec = zenithDir.dec();
}

auto LatSc::livetimeFrac(double time) const -> double {
   if (m_start.size() == 0) { // We are not using an FT2 file.
      return Spacecraft::livetimeFrac(time);
   }
   if (time < m_start.front() || time > m_start.back()) {
      return 0;
   }
   size_t indx = (std::upper_bound(m_start.begin(), m_start.end(), time)
                  - m_start.begin() - 1);
   if (m_start.at(indx) <= time && time <= m_stop.at(indx)) {
      return m_livetimefrac.at(indx);
   }
   return 0;
}

} // namespace observationSim
