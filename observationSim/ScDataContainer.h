/**
 * @file ScDataContainer.h
 * @brief Declaration for ScDataContainer class.
 * @author J. Chiang
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/ScDataContainer.h,v 1.13 2004/08/27 04:37:38 jchiang Exp $
 */

#ifndef observationSim_ScDataContainer_h
#define observationSim_ScDataContainer_h

#include <fstream>
#include <string>
#include <vector>

#include "astro/SkyDir.h"

#include "observationSim/ContainerBase.h"
#include "observationSim/ScData.h"
#include "observationSim/Spacecraft.h"

class EventSource;

namespace observationSim {

/**
 * @class ScDataContainer
 * @brief Stores and writes ScData to a FITS file.
 *
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/ScDataContainer.h,v 1.13 2004/08/27 04:37:38 jchiang Exp $
 */

class ScDataContainer : public ContainerBase {

public:

   /// @param filename The root name of the output FITS file.
   /// @param maxNumEntries The maximum number of entries in the ScData
   ///        buffer before a FITS file is written.
   ScDataContainer(const std::string &filename, 
                   int maxNumEntries=20000) : 
      ContainerBase(filename, maxNumEntries) {
      init();
   }

   ~ScDataContainer();

   /// @param event A pointer to the current EventSource object
   ///        that was provided by the FluxMgr object.
   /// @param spacecraft A pointer to the object that provides methods
   ///        for accessing spacecraft orbit and attitude info.
   /// @param flush A flag to indicate whether to write the accumulated
   ///        ScData and then flush the buffers.
   void addScData(EventSource *event, Spacecraft *spacecraft, 
                  bool flush=false);

   void addScData(double time, Spacecraft *spacecraft, bool flush=false);

   /// The simulation time of the most recently added entry.
   double simTime() {return m_scData[m_scData.size()-1].time();}

private:

   /// The ScData buffer.
   std::vector<ScData> m_scData;

   /// This routine contains the constructor implementation.
   void init();

   /// This routine unpacks and writes the ScData to a FT2 file.
   void writeScData();

};

} // namespace observationSim

#endif // observationSim_ScDataContainer_h
