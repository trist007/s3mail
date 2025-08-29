#include "s3mail.h"

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