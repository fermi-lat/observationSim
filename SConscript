# -*- python -*-
#
# $Id: SConscript,v 1.9 2008/10/21 20:30:32 glastrm Exp $
# Authors: James Chiang <jchiang@slac.stanford.edu>
# Version: observationSim-08-04-00
Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('observationSimLib', depsOnly = 1)
observationSimLib = libEnv.StaticLibrary('observationSim', 
                                         listFiles(['src/*.cxx']))

progEnv.Tool('observationSimLib')

test_observationSimBin = progEnv.Program('test_observationSim', 
                                         listFiles(['src/test/*.cxx']))

gtobssimBin = progEnv.Program('gtobssim', listFiles(['src/obsSim/*.cxx']))

progEnv.Tool('registerObjects', package = 'observationSim', 
             libraries = [observationSimLib], 
             binaries = [gtobssimBin], 
             testApps = [test_observationSimBin],
             includes = listFiles(['observationSim/*.h']), 
             pfiles = listFiles(['pfiles/*.par']),
             data = listFiles(['data/*'], recursive = True),
             xml = listFiles(['xml/*'], recursive = True))
