/**
 * @file
 * @brief   Headerfile zum Ausgeben farbiger Texte auf dem Terminal
 *
 * @author  EA
 * @date    25.10.2017
 * @version 1.0
 */

#ifndef DISPLAY_HELPER_H_
#define DISPLAY_HELPER_H_

void SetNormal(void);
void SetBold(void);

void SetColorBlack(void);
void SetColorRed(void);
void SetColorGreen(void);
void SetColorBrown(void);
void SetColorBlue(void);
void SetColorViolet(void);
void SetColorCyan(void);
void SetColorWhite(void);

char* InsertTimeStamp(char* p_pTimeStampStrg);

#endif /* DISPLAY_HELPER_H_ */
