@echo off

set CommonCompilerFlags=-MT -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4244 -wd4996 -wd4456 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib opengl32.lib kernel32.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

REM delete pdb
del *.pdb > NUL 2> NUL

REM Build shared static library
cl %CommonCompilerFlags% -c ..\s3mail\code\s3mail_shared.cpp
lib -nologo s3mail_shared.obj /OUT:s3mail_shared.lib

REM Optimzation switches /02
echo WAITING FOR PDB > lock.tmp
echo Building s3mail.dll...
cl %CommonCompilerFlags% ..\s3mail\code\s3mail.cpp s3mail_shared.lib /link /DLL -EXPORT:GameUpdateAndRender -EXPORT:GameHandleKeyPress -EXPORT:GameInitializeUI opengl32.lib -Out:s3mail.dll
del lock.tmp

REM echo Building s3mail.exe
cl %CommonCompilerFlags% ..\s3mail\code\win32_s3mail.cpp s3mail_shared.lib -Fmwin32_s3mail.map /link  %CommonLinkerFlags%

popd
