/**
 * @file ContainerBase.cxx
 * @author J. Chiang
 *
 * $Header$
 */

#include <ctime>

#include <sstream>
#include <stdexcept>

#include "tip/Table.h"
#include "tip/Header.h"

#include "observationSim/ContainerBase.h"

namespace observationSim {

std::string ContainerBase::outputFileName() const {
   std::ostringstream outputfile;
   outputfile << m_filename;
   if (m_fileNum < 10) {
      outputfile << "_000";
   } else if (m_fileNum < 100) {
      outputfile << "_00";
   } else if (m_fileNum < 1000) {
      outputfile << "_0";
   } else {
      outputfile << "_";
   }
   outputfile << m_fileNum << ".fits";
   return outputfile.str();
}

void ContainerBase::writeDateKeywords(tip::Table * table, double start_time, 
                                      double stop_time) {
   static double secsPerDay(8.64e4);
   tip::Header & header = table->getHeader();
   astro::JulianDate current_time = currentTime();
   try {
      header["DATE"].set(current_time.getGregorianDate());
   } catch (...) {
   }
   astro::JulianDate mission_start(2005, 7, 18, 0);
   astro::JulianDate date_start(mission_start + start_time/secsPerDay);
   astro::JulianDate date_stop(mission_start + stop_time/secsPerDay);
   try {
      header["DATE-OBS"].set(date_start.getGregorianDate());
      header["DATE-END"].set(date_stop.getGregorianDate());
   } catch (...) {
   }
   double duration = stop_time - start_time;
   try {
      header["TSTART"].set(start_time);
      header["TSTOP"].set(stop_time);
   } catch (...) {
   }
   try {
      header["ONTIME"].set(duration);
      header["TELAPSE"].set(duration);
   } catch (...) {
   }
}

astro::JulianDate ContainerBase::currentTime() {
   std::time_t my_time = std::time(0);
   std::tm * now = std::gmtime(&my_time);
   if (now != 0) {
      double hours = now->tm_hour + now->tm_min/60. + now->tm_sec/3600.;
      astro::JulianDate current_time(now->tm_year + 1900, now->tm_mon + 1,
                                     now->tm_mday, hours);
      return current_time;
   } else {
      throw std::runtime_error("EventContainer::currentTime:\n"
                               + std::string("cannot be ascertained, ")
                               + "std::time returns a null value.");
   }
}

} // namespace observationSim
