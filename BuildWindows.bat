@echo off

ctime -begin Timing.ctime

if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

REM enable this for release build
REM /nodefaultlib

set cl_flags=/DWINDOWS_BUILD /DDEBUG_BUILD /GS- /arch:AVX2 /MP /W1 /MTd /Z7 /Od /EHsc /nologo /utf-8
set link_flags=/opt:icf /opt:ref /fixed /subsystem:windows /out:LibUtils.exe /incremental:no /debug:full /nologo
set src_files=../AllFiles.cpp
set lib_files=user32.lib ../Lib/TracyClient.lib gdi32.lib kernel32.lib opengl32.lib libcmtd.lib libucrtd.lib libvcruntimed.lib

python GenerateOpenGLLoadingCode.py

pushd build
cl %cl_flags% %src_files% /link %link_flags% %lib_files%
popd
echo build completed.

ctime -end Timing.ctime