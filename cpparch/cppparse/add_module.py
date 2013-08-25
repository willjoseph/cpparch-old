import sys
import os
from subprocess import call

path = os.path.normpath(sys.argv[1])

thisPath = os.path.realpath(__file__)

root = os.path.dirname(thisPath)
sourceRoot = os.path.join(root, 'Source')

cppPath = os.path.join(sourceRoot, path + '.cpp')
hppPath = os.path.join(sourceRoot, path + '.h')
if os.path.exists(cppPath) or os.path.exists(hppPath):
	raise Exception('module already exists')

cppFile = open(cppPath, 'w')
cppFile.write('\n')
cppFile.write('#include "' + os.path.basename(path) + '.h"\n')
cppFile.write('\n')

pathUpper = path.replace('\\', '_').upper()
hppFile = open(hppPath, 'w')
hppFile.write('\n')
hppFile.write('#ifndef INCLUDED_CPPPARSE_' + pathUpper + '_H\n')
hppFile.write('#define INCLUDED_CPPPARSE_' + pathUpper + '_H\n')
hppFile.write('\n')
hppFile.write('#endif\n')

call(['svn', 'add', cppPath])
call(['svn', 'add', hppPath])



