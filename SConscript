# -*- python -*-
#
# $Id: SConscript,v 1.2 2007/12/11 19:03:27 jchiang Exp $
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
             pfiles = listFiles(['pfiles/*.par']))
