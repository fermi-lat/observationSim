/**
 * @file EventContainer.h
 * @brief Declaration for EventContainer class.
 * @author J. Chiang
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/EventContainer.h,v 1.1.1.1 2003/06/18 19:46:33 jchiang Exp $
 */

#ifndef observationSim_EventContainer_h
#define observationSim_EventContainer_h

#include <fstream>
#include <string>
#include <vector>

#include "FluxSvc/../src/EventSource.h"
#include "FluxSvc/../src/FluxMgr.h"
#include "observationSim/Event.h"
#include "observationSim/FitsTable.h"

namespace observationSim {

/**
 * @class EventContainer
 * @brief Stores and writes Events to a FITS file.
 *
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/EventContainer.h,v 1.1.1.1 2003/06/18 19:46:33 jchiang Exp $
 */

class EventContainer {

public:

   /// @param filename The name of the output FITS file.
   /// @param useA1fmt A flag to use the format that the A1 tool expects.
   EventContainer(const std::string &filename, bool useA1fmt=false);

   ~EventContainer() {writeEvents(); delete m_eventTable;}

   /// @param event A pointer to the current EventSource object
   ///        that was provided by the FluxMgr object fm.
   /// @param fm The FluxMgr object.
   /// @param flush A flag to indicate whether to write the accumulated
   ///        Event data and then flush the buffers.
   int addEvent(EventSource *event, FluxMgr &fm, bool flush=false);

private:

   bool m_useA1fmt;

   long m_fileNum;
   
   FitsTable *m_eventTable;

   std::vector<Event> m_events;

   /// Return the apparent direction of a detected event by drawing
   /// from the distribution representing the PSF.
   Hep3Vector apparentDir(double energy, const Hep3Vector &srcDir, 
                          const Hep3Vector &zAxis);

   /// Return the zenith for the current spacecraft location.
   Hep3Vector ScZenith(double time);

   void writeEvents();

};

} // namespace observationSim

#endif // observationSim_EventContainer_h
