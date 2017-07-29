from os import listdir, system
from os.path import isfile, join
import sys

Exportspath = sys.argv[1]
Idtpath = sys.argv[2]
Symbolspath = sys.argv[3]

onlyfiles = [f for f in listdir(Exportspath) if isfile(join(Exportspath, f))]
for f in onlyfiles:
	exp = join(Exportspath, f)
	idt = join(Idtpath, f.replace("dll.exports", "idt")) if isfile(join(Idtpath, f.replace("dll.exports", "idt"))) else join(Idtpath, f.replace("exe.exports", "idt"))

	if isfile(idt):
		print("python.exe Export_Converter.py " + exp + " " + idt + " > " + join(Symbolspath, f.replace(".exports", ".symbols")))
		system("python.exe Export_Converter.py " + exp + " " + idt + " > " + join(Symbolspath, f.replace(".exports", ".symbols")))