import sys
from collections import deque

class Buffer(deque):
    def put(self, iterable):
        for i in iterable:
            self.append(i)
    def peek(self, how_many):
        return ''.join([self[i] for i in range(how_many)])
    def get(self, how_many):
        return ''.join([self.popleft() for _ in range(how_many)])

def get_num(first, buf):
	num = first
	while len(buf) > 0 and buf.peek(1).isnumeric():
		num+= buf.get(1)
	return int(num)

def demangle_arg(buf):
	#print("debug", buf)
	isPointer = False
	isReference = False
	isUnsigned = False
	isConst = False
	hasTemplate = False
	arg = ""

	while arg == "":
		letter = buf.get(1)
		if letter == 'P':
			isPointer = True
		elif letter == 'R':
			isReference = True
		elif letter == 'U':
			isUnsigned = True
		elif letter == 'C':
			isConst = True
		elif letter == 't':
			hasTemplate = True
		elif letter == 'i':
			arg = "int"
		elif letter == 'd':
			arg = "double"
		elif letter == 'v':
			arg = "void"
		elif letter == 'c':
			arg = "char"
		elif letter == 'f': #todo: check
			arg = "float" 
		elif letter.isnumeric():
			num = get_num(letter, buf)
			arg = buf.get(num)

	if hasTemplate:
		#Todo: understand what they represent
		if buf.get(2) != "1Z":
			print("unknow template format for demangling")
			sys.exit(-1)

	return ("unsigned " if isUnsigned else "") + arg + (("<" + demangle_arg(buf) + ">") if hasTemplate else "" ) + (" *" if isPointer else "") + (" &" if isReference else "") + (" const" if isConst else "")

def demangle_args(mangled_args):
	buf = Buffer(mangled_args)
	args = []
	while len(buf) > 0:
		args.append(demangle_arg(buf))
		if len(buf) > 1 and buf.peek(1) == 'T':
			buf.get(1)
			repetition_num = get_num(buf.get(1), buf)
			repetition_num -= 1
			while repetition_num > 0:
				args.append(args[-1])
				repetition_num -= 1

		elif len(buf) == 1 and buf.peek(1) == 'e':
			buf.get(1)
			args.append("...")

	return '(' + ', '.join(args) + ')'

def number_extract(txt):
	num = ""
	num_count = 0
	for i in range(0, len(txt)):
		if txt[i].isnumeric():
			num += txt[i]
			num_count+=1
		else:
			break
	return (txt[num_count:num_count+int(num)], txt[num_count+int(num):len(txt)])

def demangle_name(mangled):
	if len(mangled) == 0:
		return mangled
	elif mangled[0] == '.':
		return mangled #todo undertand this case
	elif mangled[0] == '_':
		return mangled #todo constructor
	else:
		if "__" not in mangled:
			print("unknow mangled symbol format:", mangled)
			sys.exit(-1)
		else:
			const = ""
			fname, rest = mangled.split("__")
			if(rest[0] == "C"):
				rest = rest[1:len(rest)]
				const = "const"
			namespace, args = number_extract(rest)
			return namespace + "::" +  fname + demangle_args(args) + const

#key : ordinal
#value : address
exports = dict()

#key : ordinal
#value : name
idt = dict()

#Load exports
with open("Euser.exports") as f:
	for line in f:
		line = line.rstrip()
		ordinal, address = line.split(": ")
		if ordinal in exports:
			print("an ordinal can not appears 2 time in the export list:", ordinal)
			sys.exit(-1)
		exports[ordinal] = address


#Load idt
with open("euser.idt") as f:
	for line in f:
		line = line.rstrip()
		if line != "" and line != " ":
			ordinal, name = line.split(" Name=")
			if ordinal in idt:
				print("an ordinal can not appears 2 time in the id list:", ordinal)
				sys.exit(-1)
			idt[ordinal] = name


#produce address: name
for ordinal in exports:
	if ordinal not in idt:
		print(ordinal, "is not in both table")
		sys.exit(-1)
	address = exports[ordinal]
	name = idt[ordinal]
	#print(address + ": " + name)


print(demangle_name("PageSizeInBytes__7UserHalRi"))
print(demangle_name("newL__5CBaseUi"))
print(demangle_name("BinarySearchUnsigned__C17RPointerArrayBasePUiRi"))
print(demangle_name("CalibrationPoints__7UserHalR21TDigitizerCalibration"))
print(demangle_name("BuildVarArrayL__13CArrayPakBaseRPt13CArrayVarFlat1Zv"))
print(demangle_name("Print__6RDebugGt11TRefByValue1ZC7TDesC16e"))
print(demangle_name("AppendFormat__6TDes16Gt11TRefByValue1ZC7TDesC16P14TDes16Overflowe"))
print(demangle_name("Pow10__4MathRdi"))
print(demangle_name("Pow__4MathRdRCdT2"))