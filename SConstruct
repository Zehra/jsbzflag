#!/usr/bin/env python
import sys

files = [
	'jsbzflag.cpp',
	]

env = Environment ()
env.Append (CCFLAGS = '-g')

# include paths
env.Append (CPPPATH = ['/home/mmarshall/bzflag-2.0.12/include'])
env.Append (CPPPATH = ['/home/mmarshall/v8/include'])
env.Append (LIBPATH = ['/home/mmarshall/v8'])

# libraries
env.Append (LIBS = ['v8'])
env.Append (LIBS = 'util')

env['SHLIBPREFIX'] = ''
env.SharedLibrary (target = 'jsbzflag.so', source = files)
