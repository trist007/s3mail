A C++ program for receiving, reading, sending email when email is stored in S3.  This assumes that you are using SES to send email.

3 layer approach

s3mail - Platform Independent layer with render and utility functions (Static lib)
s3mail_game - UI Logic hot-reloadable (DLL)
win32_s3mail - Platform layer
