#include "s3mail_shared.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// utility functions
int
StringLength(char *String)
{
    int Count = 0;
    while(*String++)
    {
        ++Count;
    }
    return(Count);
}

void
CatStrings(size_t SourceACount, char *SourceA,
           size_t SourceBCount, char *SourceB,
           size_t DestCount, char *Dest)
{
    // TODO(trist007): Dest bounds checking!
    
    for(int Index = 0;
        Index < SourceACount;
        ++Index)
    {
        *Dest++ = *SourceA++;
    }
    
    for(int Index = 0;
        Index < SourceBCount;
        ++Index)
    {
        *Dest++ = *SourceB++;
    }
    
    *Dest++ = 0;
}

void
DecodeQPString(char *input, char* output, size_t output_size)
{
    char *src = input;
    char *dst = output;
    
    while(*src && (size_t)(dst - output) < (output_size - 1))
    {
        if(*src == '=' && src[1] && src[2])
        {
            // Convert hex to char
            int hex_val = 0;
            sscanf(src + 1, "%2x", &hex_val);
            *dst++ = (char)hex_val;
            src += 3;
        }
        else if(*src == '_')
        {
            *dst++ = ' ';  // Underscore represents space
            src++;
        }
        else if(*src != '\0')
        {
            *dst++ = *src++;
        }
    }
    
    *dst = '\0';
}

void
DecodeSubjectIfNeeded(char *subject)
{
    char *start = strstr(subject, "=?utf-8?Q?");
    if(start)
    {
        char *encoded_start = start + 10;  // Skip "=?utf-8?Q?"
        char *encoded_end = strstr(encoded_start, "?=");
        
        if(encoded_end)
        {
            // Extract just the encoded portion
            size_t encoded_len = encoded_end - encoded_start;
            char encoded_part[256];
            snprintf(encoded_part, sizeof(encoded_part), "%.*s", (int)encoded_len, encoded_start);
            
            // Decode it
            char decoded[256];
            DecodeQPString(encoded_part, decoded, sizeof(decoded));
            
            // Replace the entire encoded section with decoded text
            size_t prefix_len = start - subject;
            char result[256];
            snprintf(result, sizeof(result), "%.*s%s%s", 
                     (int)prefix_len, subject,  // Everything before encoding
                     decoded,                   // Decoded text
                     encoded_end + 2);          // Everything after "?="
            
            strncpy(subject, result, 256);
        }
    }
}

time_t
ParseEmailDate(char *date_header)
{
    struct tm tm = {0};
    char day_name[10], month_name[10];
    int day, year, hour, min, sec;
    
    if(sscanf(date_header, "%s %d %s %d %d:%d:%d",
              day_name, &day, month_name, &year, &hour, &min, &sec) == 7)
    {
        tm.tm_mday = day;
        tm.tm_year = year - 1900; // tm_year is years since 1900
        tm.tm_hour = hour;
        tm.tm_min = min;
        tm.tm_sec = sec;
        
        // Convert month name to number
        tm.tm_mon = MonthNameToNumber(month_name) - 1;  // tm_mon is 0-11
        
        return(mktime(&tm));
    }
    
    return(-1);  // parse failed
}

void
GetDate(char *date, size_t buffer_size)
{
    
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);
    
    strftime(date, buffer_size, "%a, %d, %Y", local_time);
}

int
CheckIfEmailReceivedToday(char *date, char *date_header)
{
    // Date header: Wed, 27 Aug 2025 08:14:03
    // date = Fri, 29, 2025
    int Result = 0;
    
    int day1 = 0;
    int day2 = 0;
    int year1 = 0;
    int year2 = 0;
    
    // days match?
    if(strncmp(date, date_header, 3) == 0)
    {
        day1 = atoi(date+5);
        day2 = atoi(date_header+5);
        if(day1 == day2)
        {
            year1 = atoi(strrchr(date, ' ') + 1);
            year2 = atoi(date_header + 12);
            if(year1 == year2)
            {
                Result = 1;
                return(Result);
            }
        }
    }
    
    return(Result);
}

void
ChangeDateHeaderIfToday(char *date_header)
{
    // Date header: Wed, 27 Aug 2025 08:14:03
    char *SemiColon = strchr(date_header, ':');
    if(SemiColon)
    {
        char *time_start = SemiColon - 2;
        
        // copy the time to email_array[count].date
        memmove(date_header, time_start, strlen(time_start) + 1);
    }
}

int
MonthNameToNumber(char *month)
{
    if(strcmp(month, "Jan") == 0) return 1;
    if(strcmp(month, "Feb") == 0) return 2;
    if(strcmp(month, "Mar") == 0) return 3;
    if(strcmp(month, "Apr") == 0) return 4;
    if(strcmp(month, "May") == 0) return 5;
    if(strcmp(month, "Jun") == 0) return 6;
    if(strcmp(month, "Jul") == 0) return 7;
    if(strcmp(month, "Aug") == 0) return 8;
    if(strcmp(month, "Sep") == 0) return 9;
    if(strcmp(month, "Oct") == 0) return 10;
    if(strcmp(month, "Nov") == 0) return 11;
    if(strcmp(month, "Dev") == 0) return 12;
    
    return(1);
}

int
CompareByTimestamp(const void *a, const void *b)
{
    EmailMetadata *email1 = (EmailMetadata*)a;
    EmailMetadata *email2 = (EmailMetadata*)b;
    
    if(email1->parsed_time > email2->parsed_time) return -1;
    if(email1->parsed_time < email2->parsed_time) return 1;
    
    return(0);
}

int
FindHeaderLines(char *email_content)
{
    int lines_count = 0;
    int char_pos = 0;
    int i = 0;
    
    while(email_content[i] != '\0')
    {
        char current_char = email_content[i];
        
        if(current_char == '\n')
        {
            if(char_pos == 0)
            {
                return(lines_count);
            }
            
            lines_count++;
            char_pos = 0;
        }
        else if(current_char == '\r')
        {
            // skipping carriage returns
        }
        else
        {
            // regular char
            char_pos++;
        }
        
        i++;
    }
    
    return(lines_count);
}

char*
FindMIMESection(char *email_content)
{
    //char *double_newline = strstr(email_content, "\n\n");
    //char *crlf_double = strstr(email_content, "\r\n\r\n");
    char *MIME_SECTION = strstr(email_content, "--_000_");
    
    //if(double_newline) return double_newline + 2;
    //if(crlf_double) return crlf_double + 4;
    if(MIME_SECTION) return MIME_SECTION;
    return(0);
}

int
tristanstrncmp(char *x, char *y, size_t n)
{
    while(n > 0)
    {
        if(*x != *y)
        {
            return(unsigned char)*x - (unsigned char)*y;
        }
        
        // search has read until the end of the buffer
        if(*x == '\0') return(0);
        
        x++;
        y++;
        n--;
    }
    
    // there was a match when n == 0 the whole buffs were
    // compared and they match
    return(0);
}

char *
FindTextPlainContent(char *MIME_Section)
{
    char *ptr = MIME_Section;
    
    // go through the Mime boundary line to the next line
    while(*ptr != '\0' && *ptr != '\n') ptr++;
    if(*ptr == '\n') ptr++;
    
    // check lines for Content-Type until we hit blank line(\n\n)
    while(*ptr != '\0' && !(*ptr == '\n' && *(ptr+1) == '\n'))
    {
        if(tristanstrncmp(ptr, "Content-Type:", 13) == 0)
        {
            if(strstr(ptr, "text/plain"))
            {
                // found the match text/plain
                while(*ptr != '\0' && !(*ptr == '\n' && *(ptr+1) == '\n'))
                {
                    ptr++;
                }
                
                // skip the \n\n
                if(*ptr == '\n') ptr += 2;  
                
                // return the beginning of the text/plain email body
                return(ptr);
            }
        }
        
        // move to next line
        while(*ptr != '\0' && *ptr != '\n') ptr++;
        if(*ptr == '\n') ptr++;
    }
    
    // no text/plain section found
    return(0); 
}

char *
FindTextPlainEnd(char *content_start)
{
    char *ptr = content_start;
    
    while(*ptr != '\0')
    {
        if(*ptr == '\n' || ptr == content_start)
        {
            // skip newline
            if(*ptr == '\n') ptr++;
            
            if(*ptr == '-' && *(ptr+1) == '-')
            {
                return(ptr);
            }
        }
        
        ptr++;
    }
    
    // no boundary found
    return(0);
}

void
ParseEmail(char *email_content, char parsed_email[][256], int *line_count)
{
    int lines = 0;
    
    size_t content_length = strlen(email_content);
    char *email_copy = (char*)malloc(content_length + 1);
    strcpy(email_copy, email_content);
    
    char *line = strtok((char *)email_copy, "\n");
    
    while(line != NULL && lines < 1000)
    {
        // remove trailing \r if present
        size_t len = strlen(line);
        if(len > 0 && line[len - 1] == '\r')
        {
            line[len - 1] = '\0';
        }
        
        strncpy(parsed_email[lines], line, 255);
        parsed_email[lines][255] = '\0';
        lines++;
        line = strtok(NULL, "\n");
    }
    
    free(email_copy);
    *line_count = lines;
}

void
ExtractHeader(thread_context *Thread, char *date, EmailMetadata *email_array, int32 email_count, char *path, HeaderType header_type, game_memory *Memory)
{
    
    char *header_name;
    int32 header_len;
    
    // Set up header string and length
    switch(header_type)
    {
        case HEADER_FROM:
        {
            header_name = "From:";
            header_len = 5;
            break;
        }
        case HEADER_SUBJECT:
        {
            header_name = "Subject:";
            header_len = 8;
            break;
        }
        case HEADER_DATE:
        {
            header_name = "Date:";
            header_len = 5;
            break;
        }
        default: // add this to handle unexpected values
        {
            header_name = "Unknown";
            header_len = 8;
            
            // don't know what to search for
            return; 
        }
    }
    
    for(int count = 0;
        count < email_count;
        count++)
    {
        char *Match = "Unknown";
        char full_path_to_file[128];
        
        snprintf(full_path_to_file, sizeof(full_path_to_file), "%s/%s", path, email_array[count].filename);
        
        debug_read_file_result ReadResult = Memory->DEBUGPlatformReadEntireFile(Thread, full_path_to_file, Memory);
        if(ReadResult.ContentsSize != 0)
        {
            char *line = strtok((char *)ReadResult.Contents, "\n");
            while(line != NULL)
            {
                if(strncmp(line, header_name, header_len) == 0)
                {
                    Match = line + header_len;
                    
                    // Skip any leading whitespace
                    while(*Match == ' ' || *Match == '\t')
                    {
                        Match++;
                    }
                    
                    // break out of the while loop
                    break;
                }
                
                // move to the next line if current line wasn't a match
                line = strtok(NULL, "\n");
            }
        }
        
        switch(header_type)
        {
            case HEADER_FROM:
            {
                // Check if this is in the "Name <email>" format
                char *AngleBracketStart = strchr(Match, '<');
                if(AngleBracketStart != NULL)
                {
                    // Copy everything before the '<'
                    int name_len = AngleBracketStart - Match;
                    
                    // Remove trailing whitespace from the name
                    while((name_len > 0) && ((Match[name_len - 1] == ' ') || (Match[name_len - 1] == '\t') || (Match[name_len - 1] == '\r')))
                    {
                        name_len--;
                    }
                    
                    snprintf(email_array[count].from, sizeof(email_array[count].from), "%.*s", name_len, Match);
                }
                else
                {
                    // No angle brackets found, use the whole string
                    snprintf(email_array[count].from, sizeof(email_array[count].from), "%s", Match);
                }
                
                break;
            }
            case HEADER_SUBJECT:
            {
                char *CarriageReturn = strchr(Match, '\r');
                int len = 0;
                size_t name_len = strlen(Match);
                if(CarriageReturn != NULL)
                {
                    name_len = CarriageReturn - Match;
                    
                    // Remove trailing whitespace from the name
                    while((name_len > 0) && ((Match[name_len - 1] == ' ') || (Match[name_len - 1] == '\t') || (Match[name_len - 1] == '\r')))
                    {
                        name_len--;
                    }
                    
                    if(name_len <= INT_MAX)
                    {
                        len = (int)name_len;
                    }
                    
                    snprintf(email_array[count].subject, sizeof(email_array[count].subject), "%.*s", len, Match);
                }
                else
                {
                    snprintf(email_array[count].subject, sizeof(email_array[count].subject), "%s", Match);
                }
                
                DecodeSubjectIfNeeded(email_array[count].subject);
                break;
            }
            case HEADER_DATE:
            {
                snprintf(email_array[count].date, sizeof(email_array[count].date), "%s", Match);
                email_array[count].parsed_time = ParseEmailDate(Match);
                break;
            }
        }
    }
}