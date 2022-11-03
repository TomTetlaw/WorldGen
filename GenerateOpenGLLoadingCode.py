import re

Functions = []

with open("OpenGLFunctions.txt", 'r') as F:
	for L in F:
		Line = L.strip()
		if Line != "":
			Functions.append(Line)

Functions.sort()

with open("OpenGL.Generated.h", 'w') as F:
	F.write("#ifndef OPENGL_GENERATED_H\n")
	F.write("#define OPENGL_GENERATED_H\n")
	for P in Functions:
		F.write("extern PFN%sPROC %s;\n" % (P.upper(), P))
	F.write("void LoadOpenGLFunctions();\n")
	F.write("#endif")
	
with open("OpenGL.Generated.cpp", 'w') as F:
	F.write("#include \"Includes.h\"\n")
	for P in Functions:
		F.write("PFN%sPROC %s = 0;\n" % (P.upper(), P))
	F.write("void LoadOpenGLFunctions() {\n")
	for P in Functions:
		F.write("\tif(!(%s = (PFN%sPROC)wglGetProcAddress(\"%s\"))) PlatformFatalError(\"Failed to load OpenGL function %s!\");\n" % (P, P.upper(), P, P))
	F.write("}\n")