#include "s3mail.h"
#include "s3mail_platform.h"

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

void DecodeSubjectIfNeeded(char *subject)
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
            
            StringCchCopy(subject, 256, result);
        }
    }
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

int
CompareByTimestamp(const void *a, const void *b)
{
    EmailMetadata *email1 = (EmailMetadata*)a;
    EmailMetadata *email2 = (EmailMetadata*)b;
    
    if(email1->parsed_time > email2->parsed_time) return -1;
    if(email1->parsed_time < email2->parsed_time) return 1;
    
    return(0);
}