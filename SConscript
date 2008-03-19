# -*- python -*-
#
# $Id: SConscript,v 1.3 2008/02/26 05:14:32 glastrm Exp $
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
gtorbsimBin = progEnv.Program('gtorbsim', listFiles(['src/orbSim/*.cxx']))

progEnv.Tool('registerObjects', package = 'observationSim', 
             libraries = [observationSimLib], 
             binaries = [gtobssimBin, gtorbsimBin], 
             testApps = [test_observationSimBin],
             includes = listFiles(['observationSim/*.h']), 
             pfiles = listFiles(['pfiles/*.par']),
             data = listFiles(['data/*'], recursive = True))
