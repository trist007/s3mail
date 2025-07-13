@echo off

set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -DBS3MAIL_INTERNAL=1 -DS3MAIL_SLOW=1 -DS3MAIL_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib

REM TODO - can we just build both with one exe?

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

REM rc ..\s3mail\code\resource.rc

REM 32-bit build
REM cl %CommonCompilerFlags% ..\handmade\code\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
REM del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% ..\s3mail\code\win32_s3mail.cpp /link %CommonLinkerFlags%
if %errorlevel% == 0 (
REM  color 02
REM echo Success: %errorlevel%
  echo [32mSUCCESS: Build completed with code %errorlevel%[0m
REM  color 07
) else (
REM  color 04
REM echo Failed: %errorlevel%
  echo [31mFAILED: Build failed with code %errorlevel%[0m
REM  color 07
)
popd
