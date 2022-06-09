##### Library ######
add_library(
  observationSim 
  src/ContainerBase.cxx
  src/EgretSc.cxx
  src/EventContainer.cxx
  src/LatSc.cxx
  src/ScDataContainer.cxx
  src/Simulator.cxx
)

target_link_libraries(
  observationSim
  PUBLIC astro CLHEP::CLHEP flux st_stream st_app tip irfInterface dataSubselector
  PRIVATE fitsGen
)
target_include_directories(
  observationSim PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:>
)

###### Executables ######
add_executable(gtobssim src/obsSim/obsSim.cxx)
target_link_libraries(gtobssim PRIVATE observationSim irfUtil irfLoader celestialSources)
target_include_directories(
  gtobssim PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

###### Tests ######
add_executable(test_observationSim src/test/main.cxx)
target_link_libraries(test_observationSim PRIVATE observationSim irfLoader celestialSources)
target_include_directories(
  test_observationSim PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

if(NOT APPLE)
  target_compile_definitions(test_observationSim PRIVATE TRAP_FPE)
endif()

###############################################################
# Installation
###############################################################
install(DIRECTORY observationSim DESTINATION ${FERMI_INSTALL_INCLUDEDIR})
install(DIRECTORY data/ DESTINATION ${FERMI_INSTALL_REFDATADIR}/observationSim)
install(DIRECTORY pfiles/ DESTINATION ${FERMI_INSTALL_PFILESDIR})
install(DIRECTORY xml/ DESTINATION ${FERMI_INSTALL_XMLDIR}/observationSim)

install(
  TARGETS observationSim gtobssim test_observationSim
  EXPORT fermiTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
