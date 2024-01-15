#ifndef _CFIFODYN_H_
#define _CFIFODYN_H_
#include <GlobalDefs.h>
#include <cstdlib>
#include <iostream>
#include <string.h>


/**
 * FiFo Klasse (aus LWM_Shared uebernommen)
 **/
class CFifoDyn{

 public:
  /**
   * Kill FiFo
   **/
  void Kill();

  /**
   * Ctor
   * @return void
   **/
  CFifoDyn();
  virtual ~CFifoDyn();

  /**
   * Initiiert FiFo-Speicher
   * @param Size Groesse
   * @param Item_Size Groesse eines Eintrages
   * @return Erfolgreich?
   */
  bool Create(int Size,int Item_Size);

  /**
   * Reset FiFo
   * @return
   **/
  bool Reset();
//  bool Add_Data(void* data); // Sar 06.03.03 08:59:06
//  bool Get_Data(void* data); // Sar 06.03.03 08:59:08



  /* Erweiterungen fuer die Message-Fifos 				  */
  /* Nur verwenden wenn sichergestellt ist, dass nur eine */
  /* Funktion lesend oder schreibend auf das Fifo zugreift*/
  /**
   * Ueberprueft ob \e Length Bytes noch in den reservierten Speicherblock passen
   * @param Length Laenge
   * @return Ergebnis
   **/
  int Ready_To_Receive(unsigned short Length);

  /**
   * Berechnet Zeiger auf die naechste Einfuegestelle
   * @param Length Groesse des neuen \e Items
   * @return Zeiger auf die naechste Einfuegestelle
   **/
  void* Get_Next_Input_Pointer(unsigned short Length);

  /**
   * Zeiger auf die naechste Einfuegestelle weitersetzten
   * @param Length Groesse in Bytes
   * @return != 0 wenn erfolgreich
   **/
  int Inc_Input_Pointer(unsigned short Length);

  /**
   * Berechnet Zeiger auf die naechste Auslesestelle
   * @return Zeiger auf die naechste Auslesestelle
   **/
  void* Get_Next_Output_Pointer(void);

  /**
   * Zeiger auf die naechste Auslesestelle weitersetzen
   * @return != 0 wenn erfolgreich
   **/
  int Inc_Output_Pointer(void);
//  BOOL Inc_Output_Pointer_n(__int32 n);

  /**
   * Liefert freien Speicher in Byte zurueck
   * @return
   **/
  int GetFreeMemory();

  /**
   * Liefert freien Speicher in Prozent zurueck
   * @return Freier Speicher in Prozent
   **/
  float GetFreeMemoryInPercent();




 protected:
  bool m_Is_Initialized;  ///< Speicher ist noch nicht allokiert
  int m_Item_Size;	   ///< Groesse eines Eintrags
  int m_Size;          ///< Gesamtgroesse des Speichers

  char* m_pWurzel;		   ///< Pointer auf den Beginn des Speichers
  long  m_In;			   ///< Zaehler auf die naechste Einfuegestelle
  long  m_Out;			   ///< Zaehler auf die naechste Auslesestelle
  int  m_Last_Element;	   ///< Zaehler auf das letzte Element

  int m_HeaderSize;		///<Fixe Groesse  aus sizeof(ID) + sizeof(Length)// Sar 31.01.05 09:33:43

  int m_FreeMemory;			///< Freier Speicher in Byte
  void CalculateFreeMemory();	///< Berechnet den freien Speicher

};
#endif

