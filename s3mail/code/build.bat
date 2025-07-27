@echo off

set CommonCompilerFlags=-MT -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4244 -wd4996 -wd4456 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib opengl32.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

REM delete pdb
del *.pdb > NUL 2> NUL
REM Optimzation switches /02
REM echo WAITING FOR PDB > lock.tmp

echo Building s3mail_game.dll...
REM cl /LD /Zi /Od /nologo /I.. ..\s3mail_game.c /link /EXPORT:GameUpdateAndRender /EXPORT:GameHandleKeyPress /OUT:s3mail_game.dll
cl %CommonCompilerFlags% ..\s3mail\code\s3mail_game.cpp  /link /DLL -EXPORT:GameUpdateAndRender -EXPORT:GameHandleKeyPress -Out:s3mail_game.dll

REM del lock.tmp

echo Building s3mail.exe
REM cl /Zi /Od /nologo /I.. ..\s3mail.c opengl32.lib gdi32.lib user32.lib kernel32.lib /Fe:s3mail.exe
cl %CommonCompilerFlags% ..\s3mail\code\win32_s3mail.cpp -Fmwin32_s3mail.map /link  %CommonLinkerFlags%

popd
