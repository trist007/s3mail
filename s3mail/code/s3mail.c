#include "s3mail.h"

void DecodeQPString(char *input, char* output, size_t output_size)
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