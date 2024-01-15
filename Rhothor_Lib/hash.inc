long HashVal(const char** s) { for (long i=0; **s && **s!='('; (*s)++) i=i*2+**s;  return i; }

#define _rtGetStatus 220757
#define _rtAbort 13620
#define _rtGetIO 13641
#define _rtGetFieldSize 1761557
#define _rtGetMaxSpeed 881458
#define _rtGetSerial 220562
#define _rtGetScannerDelay 14114003
#define _rtGetSetpointFilter 56508604
#define _rtArcTo 13635
#define _rtBurst 13802

#define _rtJumpTo 27855
#define _rtLineTo 27691
#define _rtListClose 222807
#define _rtListOpen 111440
#define _rtPulse 13987
#define _rtSetMatrix 223458
#define _rtSetOscillator 3579688
#define _rtSetSpeed 111794
#define _rtSleep 13918
#define _rtSetIO 13833

#define _rtWaitIO 27881
#define _rtSetLaser 111632
#define _rtMoveTo 27883
#define _rtSetOffsXY 223429
#define _rtSetOffsZ 111672
#define _rtSetLaserTimes 3575161
#define _rtSetResolver 894980
#define _rtWaitResolver 1794052
#define _rtSelectDevice 1789135
#define _rtAddCalibrationData 111289069

#define _rtStoreCalibrationFile 462725905
#define _rtLoadCalibrationFile 227222289
#define _rtGetResolvers 1765499
#define _rtGetVersion 441544
#define _rtRunServer 225108
#define _rtResetCalibration 28652000
#define _rtMoveTo3D 111702
#define _rtLineTo3D 110934
#define _rtJumpTo3D 111590
#define _rtPulse3D 56118

#define _rtSetJumpSpeed 1789234
#define _bcSamplePoint 794124
#define _rtFormatFlash 888786
#define _rtFileOpen 110208
#define _rtFileClose 220343
#define _rtEraseFromFlash 7082226
#define _rtGetFlashFirstFileEntry 1804255501
#define _rtGetFlashNextFileEntry 902151437
#define _rtFileFetch 220326
#define _rtGetFlashMemorySizes 225536445

#define _rtAddCalibrationDataZ 222578228
#define _rtStoreCalibrationFileZ 925451900
#define _rtLoadCalibrationFileZ 454444668
#define _rtResetCalibrationZ 57304090
#define _rtSetMinGatePeriod 28600582
#define _rtSetPulseBulge 3581707
#define _rtSetLoop 55818
#define _rtDoLoop 27354
#define _rtArcMoveTo 219563
#define _rtElse 6819

#define _rtEndIf 13640
#define _rtIfIO 6721
#define _rtWaitPosition 1794368
#define _rtSetImageOffsXY 7148037
#define _rtSetImageOffsRelXY 57184993
#define _rtParse 13851
#define _rtSuspend 56548
#define _rtIncrementCounter 28441196
#define _rtResetCounter 1791340
#define _rtSetLaserLink 1787467

#define _rtGetLaserLink 1762891
#define _rtFontDef 55328
#define _rtFontDefEnd 443220
#define _rtCharDef 54688
#define _rtVarBlockFetch 3573798
#define _rtSetOTF 27770
#define _rtGetCounter 440684
#define _rtCircle 27465
#define _rtCircleMove 440837
#define _rtSetMaxSpeed 893746

#define _rtSetVarBlock 894445
#define _rtSetImageMatrix 7148066
#define _rtResetResolver 3584004
#define _rtSetImageOffsZ 3573976
#define _rtSetCanLink 445963
#define _rtOpenCanLink 893195
#define _rtWaitCanLink 895499
#define _rtGetCanLink 439819
#define _rtSetFieldSize 1786133
#define _rtSetResolverRange 28642195

#define _rtSetResolverTrigger 114569696
#define _rtSetOffsIndex 1788082
#define _rtStoreCalibration 28920288
#define _rtSetWobble 223781
#define _rtScanCanLink 889611
#define _rtLoadCalibration 14201312
#define _rtGetFileIndex 1761522
#define _rtIndexFetch 443590
#define _rtFileUpload 441378
#define _rtFileUploadAtIndex 56507026

#define _rtFileDownload 1764034
#define _rtFileCloseAtIndex 28214546
#define _rtFileCloseAtHost 14107254
#define _rtSendUartLink 1788299
#define _rtSetLead 55738
#define _rtSetHover 111692
#define _rtSetTableSnap 1788498
#define _rtSetTableOffsXY 7154053
#define _rtSetImageMatrix3D 28592434
#define _rtSystemSetIO 908521

#define _rtSetCounter 446828
#define _rtReset 13906
#define _rtSetRotation 895456
#define _rtSetImageRotation 28593888
#define _rtSetWobbleEx 895382
#define _rtElseIfIO 110321
#define _rtWhileIO 56101
#define _rtDoWhile 54897
#define _rtGetID 13630
#define _bcSelectDevice 1588431

#define _rtResetEventCounter 57327212
#define _rtWaitEventCounter 28687980
#define _rtSystemSuspend 3635300
#define _rtSystemResume 1817339
#define _rtSetTarget 223610
#define _rtGetTarget 220538
#define _rtTableMove 222789
#define _rtGetTablePositions 56449267
#define _rtSetTableStepSize 28617909
#define _rtSetTableDelay 3576819

#define _rtSetTable 111693
#define _rtTableMoveTo 891435
#define _rtSetAnalog 223101
#define _rtGetAnalog 220029
#define _rtGetDeflReplies 7042145
#define _rtTableJog 111317
#define _rtTableLineTo 891243
#define _rtTableJumpTo 891407
#define _rtTableArcTo 445411
#define _rtSetCfgIO 111389

#define _rtGetCfgIO 109853
#define _rtSystemUartOpen 7269552
#define _rtSystemUartWrite 14539377
#define _rtUartRead 112010
#define _rtSetTableWhileIO 14308773
#define _rtSetQueryTarget 7164218
#define _rtGetQueryTarget 7065914
#define _rtSystemUDPsend 3633324
#define _rtGetIP 13642
#define _rtUDPsend 54572

#define _rtSetTableSnapSize 28617397
#define _rtSetResolverPosition 229139264
#define _rtTableJumpTo3D 3565798
#define _rtWaitIdle 111925
#define _rtSystemSelectFile 29077233
#define _rtBurstEx 55466
#define _rtWaitStall 224216
#define _rtSynchronise 905999
#define _rtSetWhileIO 447525
#define _rtSetLaserFirstPulse 114402531

#define _rtSystemCheck 908309
#define _rtPrint 13956
#define _rtSetFont 55772
#define _rtLoadProfile 888897
#define _rtStoreProfile 1808833
#define _rtSystemSetResolverSpeed 1860963314








