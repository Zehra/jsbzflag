#!/usr/bin/env python
import sys

files = [
	'jsbzflag.cpp',
	]

env = Environment ()
env.Append (CCFLAGS = '-g')

# include paths
env.Append (CPPPATH = '/home/mmarshall/bzflag-2.0.12/include')

# libraries
env.Append (LIBS = ['mozjs'])
env.Append (LIBS = 'util')

env['SHLIBPREFIX'] = ''
env.SharedLibrary (target = 'jsbzflag.so', source = files)
