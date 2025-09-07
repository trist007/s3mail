/* date = August 29th 2025 10:42 am */

#ifndef S3MAIL_H
#define S3MAIL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strsafe.h>
#include <time.h>
#include <GL/gl.h>

// OpenGL functions
void SetColor(float r, float g, float b);
void DrawRect(float x, float y, float width, float height);
void DrawRectRatio(float x, float y, float width, float height);
void DrawRectOutline(float x, float y, float width, float height);
void DrawRectOutlineRatio(float x, float y, float width, float height);
void DrawText(game_state *GameState, const char *text, float x, float y);
void DrawTextEmail(game_state *GameState, const char *text, float x, float y);
int PointInRect(int px, int py, float x, float y, float width, float height);
void HandleResizey(int width, int height);

// STB Truetype functions
int InitFont(game_state *GameState, const char *font_path);

// utility functions
int StringLength(char *String);
void CatStrings(size_t SourceACount, char *SourceA,
                size_t SourceBCount, char *SourceB,
                size_t DestCount, char *Dest);
void DecodeQPString(char *input, char* output, size_t output_size);
void DecodeSubjectIfNeeded(char *subject);
void GetDate(char *date, size_t buffer_size);
int CheckIfEmailReceivedToday(char *date, char *date_header);
void ChangeDateHeaderIfToday(char *date_header);
time_t ParseEmailDate(char *date_header);
int MonthNameToNumber(char *month);
int CompareByTimestamp(const void *a, const void *b);

#endif //S3MAIL_H
