/**
 * @file Container.h
 * @brief Declaration for Container class.
 * @author J. Chiang
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/Container.h,v 1.2 2003/06/19 00:14:04 jchiang Exp $
 */

#ifndef observationSim_Container_h
#define observationSim_Container_h

#include "CLHEP/Geometry/Vector3D.h"

namespace observationSim {

/**
 * @class Container
 * @brief Base class for observationSim Containers.  
 *
 * This class is abstract only in that its constructor is protected
 * (not private, so that the sub-classes have access) and presently
 * simply provides a method for obtaining the rotation matrix from
 * instrument to "Celestial" (J2000?) coordinates.
 *
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/observationSim/observationSim/Container.h,v 1.2 2003/06/19 00:14:04 jchiang Exp $
 */

class Container {

public:

   virtual ~Container() {}

   HepRotation glastToCelestial(double time);

protected:

   Container() {}

};

} // namespace observationSim

#endif // observationSim_Container_h
