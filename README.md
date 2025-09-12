A C++ program for receiving, reading, and sending email when email is stored in S3.  This assumes that you are using SES to send email.  This also uses awscli and will require ~/.aws directory with credentials.

3 layer approach

s3mail - Platform Independent layer with rendering(opengl) and utility functions (Static lib)
s3mail_game - UI Logic hot-reloadable (DLL)
win32_s3mail - Platform layer
