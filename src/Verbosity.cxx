#include "Verbosity.h"

namespace observationSim {

Verbosity * Verbosity::s_instance(0);

bool print_output(unsigned int local_verbosity) {
   return Verbosity::instance()->value() >= local_verbosity;
}

bool clobber() {
   return Verbosity::instance()->clobber();
}

} // namespace observationSim