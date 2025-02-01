EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 3 10
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L clarinoidSymbols:PCM5102 U?
U 1 1 614F5ACA
P 5600 2850
AR Path="/614F5ACA" Ref="U?"  Part="1" 
AR Path="/61487F53/614DB2F5/614F5ACA" Ref="U?"  Part="1" 
F 0 "U?" H 5600 3565 50  0000 C CNN
F 1 "PCM5102" H 5600 3474 50  0000 C CNN
F 2 "clarinoid3:HILETGOPCM5102" H 4950 3000 50  0001 C CNN
F 3 "" H 4950 3000 50  0001 C CNN
	1    5600 2850
	1    0    0    -1  
$EndComp
$Comp
L Connector:AudioJack3 J?
U 1 1 614F5AD0
P 5750 3800
AR Path="/614F5AD0" Ref="J?"  Part="1" 
AR Path="/61487F53/614DB2F5/614F5AD0" Ref="J?"  Part="1" 
F 0 "J?" H 5732 4125 50  0000 C CNN
F 1 "AudioOut" H 5732 4034 50  0000 C CNN
F 2 "clarinoid3:Jack_3.5mm_CUI_SJ-3523-SMT_Horizontal" H 5750 3800 50  0001 C CNN
F 3 "~" H 5750 3800 50  0001 C CNN
	1    5750 3800
	1    0    0    -1  
$EndComp
$Comp
L Device:R_POT_Dual RV?
U 1 1 6155EB56
P 3600 2000
AR Path="/6155EB56" Ref="RV?"  Part="1" 
AR Path="/61487F53/6152256E/6155EB56" Ref="RV?"  Part="1" 
F 0 "RV?" H 3600 1767 50  0000 C CNN
F 1 "VOL" H 3600 1676 50  0000 C CNN
F 2 "clarinoid3:Bourns_POT_PTR902-2020K-A103_PANEL" H 3850 1925 50  0001 C CNN
F 3 "~" H 3850 1925 50  0001 C CNN
	1    3600 2000
	1    0    0    -1  
$EndComp
Text HLabel 3500 2100 3    50   Input ~ 0
AGND
Text HLabel 4000 2100 3    50   Input ~ 0
AGND
Text HLabel 3200 2100 3    50   Input ~ 0
AudioL+
Text HLabel 3700 2100 3    50   Input ~ 0
AudioR+
$EndSCHEMATC
