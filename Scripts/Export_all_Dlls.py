from os import listdir, system
from os.path import isfile, join
import sys

Dllpath = sys.argv[1]
Exportspath = sys.argv[2]

onlyfiles = [f for f in listdir(Dllpath) if isfile(join(Dllpath, f))]

for file in onlyfiles:
	system("ListExports.exe " + join(Dllpath, file) + " > " + join(Exportspath, file) + ".exports")