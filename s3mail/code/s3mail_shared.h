/* date = September 27th 2025 3:16 pm */

#ifndef S3MAIL_SHARED_H
#define S3MAIL_SHARED_H

#include "s3mail_platform.h"

// utility functions
int StringLength(char *String);
void CatStrings(size_t SourceACount, char *SourceA,
                size_t SourceBCount, char *SourceB,
                size_t DestCount, char *Dest);
void DecodeQPString(char *input, char* output, size_t output_size);
void DecodeSubjectIfNeeded(char *subject);
time_t ParseEmailDate(char *date_header);
void GetDate(char *date, size_t buffer_size);
int CheckIfEmailReceivedToday(char *date, char *date_header);
void ChangeDateHeaderIfToday(char *date_header);
int MonthNameToNumber(char *month);
int CompareByTimestamp(const void *a, const void *b);
void ParseEmail(char *email_content, char parsed_email[][256], int *line_count);

struct game_memory;

void ExtractHeader(thread_context *Thread, char *date, EmailMetadata *email_array, int32 email_count, char *path, HeaderType header_type, game_memory *Memory);

#endif //S3MAIL_SHARED_H
