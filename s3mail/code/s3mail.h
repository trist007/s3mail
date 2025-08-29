/* date = August 29th 2025 10:42 am */

#ifndef S3MAIL_H
#define S3MAIL_H

#include <stdio.h>
#include <string.h>
#include <strsafe.h>

void DecodeQPString(char *input, char* output, size_t output_size);
void DecodeSubjectIfNeeded(char *subject);

#endif //S3MAIL_H
