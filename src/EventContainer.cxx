/**
 * @file EventContainer.cxx
 * @brief Implementation for class that keeps track of events and when they
 * get written to a FITS file.
 * @author J. Chiang
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/src/EventContainer.cxx,v 1.8 2003/07/03 03:31:49 jchiang Exp $
 */

#include <sstream>

#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/JamesRandom.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Geometry/Vector3D.h"

#include "astro/SkyDir.h"
#include "astro/EarthCoordinate.h"

#include "latResponse/IAeff.h"
#include "latResponse/IPsf.h"
#include "latResponse/IEdisp.h"
#include "latResponse/Irfs.h"

#include "latResponse/../src/Glast25.h"

#include "observationSim/Spacecraft.h"
#include "observationSim/../src/LatSc.h"
#include "observationSim/EventContainer.h"

namespace {
   double my_acos(double mu) {
      if (mu > 1) {
         return 0;
      } else if (mu < -1) {
         return M_PI;
      } else {
         return acos(mu);
      }
   }
} // unnamed namespace

namespace observationSim {

void EventContainer::init() {
   m_events.clear();
}

int EventContainer::addEvent(EventSource *event, 
                             latResponse::Irfs &response, 
                             Spacecraft *spacecraft,
                             bool flush) {
   
   std::string particleType = event->particleName();
   double time = event->time();
   double energy = event->energy();
   Hep3Vector launchDir = event->launchDir();

// We need the source direction in J2000 coordinates here.
// Unfortunately, because EventSource objects are so closely tied to
// the LAT geometry as embodied in the various FluxSvc and astro
// classes (e.g., GPS, EarthOrbit, EarthCoordinate), launchDir is
// accessible only as a Hep3Vector in LAT coordinates.  Therefore, we
// are forced to cheat and use a LatSc object explicitly in order to
// recover the source direction in J2000 coordinates.
   LatSc latSpacecraft;
   HepRotation rotMatrix = latSpacecraft.InstrumentToCelestial(time);
   astro::SkyDir sourceDir(rotMatrix(launchDir), astro::SkyDir::CELESTIAL);

   astro::SkyDir zAxis = spacecraft->zAxis(time);
   astro::SkyDir xAxis = spacecraft->xAxis(time);

   double inclination = sourceDir.difference(zAxis)*180./M_PI;
   double effArea = (*response.aeff())(energy, sourceDir, zAxis, xAxis);
   if ( energy > 31.623 
        && RandFlat::shoot() < m_prob
        && RandFlat::shoot() < effArea/event->totalArea()/1e4
        && !spacecraft->inSaa(time) ) {
      astro::SkyDir appDir = response.psf()->appDir(energy, sourceDir, 
                                                    zAxis, xAxis);
                                                        
      m_events.push_back( Event(time, energy, 
                                appDir, sourceDir, zAxis, xAxis,
                                ScZenith(time)) );
//      std::cout << "adding an event: " << m_events.size() << std::endl;
      if (flush || m_events.size() >= m_maxNumEvents) writeEvents();
      return 1;
   }
   if (flush) writeEvents();
   return 0;
}

astro::SkyDir EventContainer::ScZenith(double time) {
   GPS *gps = GPS::instance();
   gps->getPointingCharacteristics(time);
   double lon_zenith = gps->RAZenith()*M_PI/180.;
   double lat_zenith = gps->DECZenith()*M_PI/180.;
   return astro::SkyDir(Hep3Vector(cos(lat_zenith)*cos(lon_zenith),
                                   cos(lat_zenith)*sin(lon_zenith),
                                   sin(lat_zenith)),
                        astro::SkyDir::CELESTIAL);
}

void EventContainer::writeEvents() {

   makeFitsTable();

   if (!m_useA1fmt) {
      std::vector<std::vector<double> > data(8);
      for (std::vector<Event>::const_iterator it = m_events.begin();
           it != m_events.end(); it++) {
         data[0].push_back(it->time());
         data[1].push_back(it->energy());
         data[2].push_back(it->appDir().dir().x());
         data[3].push_back(it->appDir().dir().y());
         data[4].push_back(it->appDir().dir().z());
         data[5].push_back(it->srcDir().dir().x());
         data[6].push_back(it->srcDir().dir().y());
         data[7].push_back(it->srcDir().dir().z());
      }
      m_eventTable->writeTableData(data);
   } else {
      std::vector<std::vector<double> > data(11);
// pre-allocate the memory for each vector
      for (std::vector<std::vector<double> >::iterator vec_it = data.begin();
           vec_it != data.end(); vec_it++)
         vec_it->reserve(m_events.size());
      for (std::vector<Event>::const_iterator it = m_events.begin();
           it != m_events.end(); it++) {
         data[0].push_back(it->appDir().ra());
         data[1].push_back(it->appDir().dec());
         data[2].push_back(it->energy());
         data[3].push_back(it->time());
         data[4].push_back(it->xAxis().dir().x());
         data[5].push_back(it->xAxis().dir().y());
         data[6].push_back(it->xAxis().dir().z());
         data[7].push_back(it->zAxis().dir().x());
         data[8].push_back(it->zAxis().dir().y());
         data[9].push_back(it->zAxis().dir().z());
         data[10].push_back(
            it->zenith().dir().angle(it->appDir().dir())*180./M_PI);
      }
      m_eventTable->writeTableData(data);
   }
// Delete the old table...
   delete m_eventTable;

// flush the Event buffer...
   m_events.clear();

// and update the m_fileNum index.
   m_fileNum++;
}

void EventContainer::makeFitsTable() {

// Prepare the column name, format, and unit specifiers that are
// required by the FITS binary table format.  This implementation will
// likely need to be generalized, or at least, canonicalized.

   std::vector<std::string> colName;
   std::vector<std::string> fmt;
   std::vector<std::string> unit;

   if (!m_useA1fmt) { // the default
      colName.push_back("time");fmt.push_back("1E");unit.push_back("seconds");
      colName.push_back("energy");fmt.push_back("1E");unit.push_back("MeV");
      colName.push_back("app_x");fmt.push_back("1E");unit.push_back("dir_cos");
      colName.push_back("app_y");fmt.push_back("1E");unit.push_back("dir_cos");
      colName.push_back("app_z");fmt.push_back("1E");unit.push_back("dir_cos");
      colName.push_back("src_x");fmt.push_back("1E");unit.push_back("dir_cos");
      colName.push_back("src_y");fmt.push_back("1E");unit.push_back("dir_cos");
      colName.push_back("src_z");fmt.push_back("1E");unit.push_back("dir_cos");
   } else {
      colName.push_back("RA"); fmt.push_back("1E"); unit.push_back("deg");
      colName.push_back("DEC"); fmt.push_back("1E"); unit.push_back("deg");
      colName.push_back("energy"); fmt.push_back("1E"); unit.push_back("MeV");
      colName.push_back("time"); fmt.push_back("1D"); unit.push_back("s");
      colName.push_back("SC_x0");fmt.push_back("1E");unit.push_back("dir cos");
      colName.push_back("SC_x1");fmt.push_back("1E");unit.push_back("dir cos");
      colName.push_back("SC_x2");fmt.push_back("1E");unit.push_back("dir cos");
      colName.push_back("SC_x");fmt.push_back("1E");unit.push_back("dir cos");
      colName.push_back("SC_y");fmt.push_back("1E");unit.push_back("dir cos");
      colName.push_back("SC_z");fmt.push_back("1E");unit.push_back("dir cos");
      colName.push_back("zenith_angle"); fmt.push_back("1E");
      unit.push_back("deg");
   }

// Create the FITS filename using the root name, m_filename, and
// m_fileNum.
   std::ostringstream outputfile;
   outputfile << m_filename;
   if (m_fileNum < 10) {
      outputfile << "_000";
   } else if (m_fileNum < 100) {
      outputfile << "_00";
   } else if (m_fileNum < 1000) {
      outputfile << "_0";
   } else {
      std::cerr << "Too many Event output files." << std::endl;
      exit(-1);
   }
   outputfile << m_fileNum << ".fits";

   m_eventTable = new FitsTable(outputfile.str(), "LAT_event_summary", 
                                colName, fmt, unit);
}

} // namespace observationSim
