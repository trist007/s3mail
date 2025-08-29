/* date = August 29th 2025 10:42 am */

#ifndef S3MAIL_H
#define S3MAIL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strsafe.h>
#include <time.h>

void DecodeQPString(char *input, char* output, size_t output_size);
void DecodeSubjectIfNeeded(char *subject);
void GetDate(char *date, size_t buffer_size);
int CheckIfEmailReceivedToday(char *date, char *date_header);
void ChangeDateHeaderIfToday(char *date_header);

#endif //S3MAIL_H
