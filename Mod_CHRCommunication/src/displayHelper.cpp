/**
 * @file
 * @brief   Sourcecode zum Ausgeben farbiger Texte auf dem Terminal
 *
 * @author  EA
 * @date    25.10.2017
 * @version 1.0
 */

#include <stdio.h>
#include <time.h>

#define COLORS_ACTIVE 1

void SetNormal(void)
{
#if COLORS_ACTIVE
    printf("%c[0m", 0x1B);
#endif
}

void SetBold(void)
{
#if COLORS_ACTIVE
    printf("%c[1m", 0x1B);
#endif
}

void SetColorBlack(void)
{
#if COLORS_ACTIVE
    printf("%c[30m", 0x1B);
#endif
}
void SetColorRed(void)
{
#if COLORS_ACTIVE
    printf("%c[31m", 0x1B);
#endif
}
void SetColorGreen(void)
{
#if COLORS_ACTIVE
    printf("%c[32m", 0x1B);
#endif
}
void SetColorBrown(void)
{
#if COLORS_ACTIVE
    printf("%c[33m", 0x1B);
#endif
}

void SetColorBlue(void)
{
#if COLORS_ACTIVE
    printf("%c[34m", 0x1B);
#endif
}

void SetColorViolet(void)
{
#if COLORS_ACTIVE
    printf("%c[35m", 0x1B);
#endif
}

void SetColorCyan(void)
{
#if COLORS_ACTIVE
    printf("%c[36m", 0x1B);
#endif
}

void SetColorWhite(void)
{
#if COLORS_ACTIVE
    printf("%c[37m", 0x1B);
#endif
}

char* InsertTimeStamp(char* p_pTimeStampStrg)
{
    struct timespec timeStamp;
    clock_gettime(CLOCK_REALTIME, &timeStamp);
    struct tm* ptmVar;
    ptmVar = localtime(&timeStamp.tv_sec);
    sprintf(p_pTimeStampStrg, "%02d:%02d:%02d:%03ld", ptmVar->tm_hour, ptmVar->tm_min, ptmVar->tm_sec, (timeStamp.tv_nsec / 1000000));
    return p_pTimeStampStrg;
}
