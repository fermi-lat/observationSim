# -*- python -*-
#
# $Id: SConscript,v 1.15 2010/02/08 05:47:05 jchiang Exp $
# Authors: James Chiang <jchiang@slac.stanford.edu>
# Version: observationSim-08-04-05
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

progEnv.Tool('registerTargets', package = 'observationSim', 
             staticLibraryCxts = [[observationSimLib,libEnv]], 
             binaryCxts = [[gtobssimBin,progEnv]], 
             testAppCxts = [[test_observationSimBin,progEnv]],
             includes = listFiles(['observationSim/*.h']), 
             pfiles = listFiles(['pfiles/*.par']),
             data = listFiles(['data/*'], recursive = True),
             xml = listFiles(['xml/*'], recursive = True))
