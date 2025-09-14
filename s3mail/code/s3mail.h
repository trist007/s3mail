#ifndef S3MAIL_H
#define S3MAIL_H

#include "s3mail_platform.h"

// STB Truetype functions
int InitFont(game_state *GameState, const char *font_path);


// OpenGL functions
void SetColor(float r, float g, float b);
void DrawRect(float x, float y, float width, float height);
void DrawRectRatio(float x, float y, float width, float height);
void DrawRectOutline(float x, float y, float width, float height);
void DrawRectOutlineRatio(float x, float y, float width, float height);
void DrawTextGame(game_state *GameState, const char *text, float x, float y);
void DrawTextGameEmail(game_state *GameState, const char *text, float x, float y);
int PointInRect(int px, int py, float x, float y, float width, float height);
void HandleResizey(int width, int height);

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
void ExtractHeader(thread_context *Thread, char *date, EmailMetadata *email_array, int32 email_count,
                   debug_platform_read_entire_file *ReadEntireFile, char *path, HeaderType header_type);


#endif //S3MAIL_H