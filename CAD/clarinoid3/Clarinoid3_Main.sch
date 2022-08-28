EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 10
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
L teensy:Teensy4.0 U?
U 1 1 614C6005
P 2350 2750
AR Path="/614C6005" Ref="U?"  Part="1" 
AR Path="/61487F53/614C6005" Ref="U?"  Part="1" 
F 0 "U?" H 2350 4365 50  0000 C CNN
F 1 "Teensy4.0" H 2350 4274 50  0000 C CNN
F 2 "teensy:Teensy40" H 1950 2950 50  0001 C CNN
F 3 "" H 1950 2950 50  0001 C CNN
	1    2350 2750
	1    0    0    -1  
$EndComp
$Sheet
S 4950 1350 1250 950 
U 614DB2F5
F0 "Clarinoid3_Main_Audio" 50
F1 "Clarinoid3_Main_Audio.sch" 50
$EndSheet
$Comp
L Connector_Generic:Conn_01x04 J?
U 1 1 614F7E10
P 6700 1650
AR Path="/614F7E10" Ref="J?"  Part="1" 
AR Path="/61487F53/614F7E10" Ref="J?"  Part="1" 
F 0 "J?" H 6780 1642 50  0000 L CNN
F 1 "Speaker" H 6780 1551 50  0000 L CNN
F 2 "Connector_Molex:Molex_SL_171971-0004_1x04_P2.54mm_Vertical" H 6700 1650 50  0001 C CNN
F 3 "~" H 6700 1650 50  0001 C CNN
	1    6700 1650
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J?
U 1 1 614F7E16
P 1800 4900
AR Path="/614F7E16" Ref="J?"  Part="1" 
AR Path="/61487F53/614F7E16" Ref="J?"  Part="1" 
F 0 "J?" H 1880 4942 50  0000 L CNN
F 1 "LEDs" H 1880 4851 50  0000 L CNN
F 2 "Connector_Molex:Molex_SL_171971-0003_1x03_P2.54mm_Vertical" H 1800 4900 50  0001 C CNN
F 3 "~" H 1800 4900 50  0001 C CNN
	1    1800 4900
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x07 J?
U 1 1 614F7E1C
P 1800 6300
AR Path="/614F7E1C" Ref="J?"  Part="1" 
AR Path="/61487F53/614F7E1C" Ref="J?"  Part="1" 
F 0 "J?" H 1880 6342 50  0000 L CNN
F 1 "OLED" H 1880 6251 50  0000 L CNN
F 2 "Connector_Molex:Molex_SL_171971-0007_1x07_P2.54mm_Vertical" H 1800 6300 50  0001 C CNN
F 3 "~" H 1800 6300 50  0001 C CNN
	1    1800 6300
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J?
U 1 1 614F7E22
P 1800 6900
AR Path="/614F7E22" Ref="J?"  Part="1" 
AR Path="/61487F53/614F7E22" Ref="J?"  Part="1" 
F 0 "J?" H 1880 6942 50  0000 L CNN
F 1 "T4PitchHeader" H 1880 6851 50  0000 L CNN
F 2 "" H 1800 6900 50  0001 C CNN
F 3 "~" H 1800 6900 50  0001 C CNN
	1    1800 6900
	1    0    0    -1  
$EndComp
$Sheet
S 5000 2850 1200 950 
U 6152256E
F0 "Clarinoid3_Main_Power" 50
F1 "Clarinoid3_Main_Power.sch" 50
$EndSheet
$Comp
L Connector_Generic:Conn_01x04 J?
U 1 1 6153B15F
P 1800 5600
AR Path="/6153B15F" Ref="J?"  Part="1" 
AR Path="/61487F53/6153B15F" Ref="J?"  Part="1" 
F 0 "J?" H 1880 5592 50  0000 L CNN
F 1 "T4I2C" H 1880 5501 50  0000 L CNN
F 2 "Connector_Molex:Molex_SL_171971-0004_1x04_P2.54mm_Vertical" H 1800 5600 50  0001 C CNN
F 3 "~" H 1800 5600 50  0001 C CNN
	1    1800 5600
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J?
U 1 1 6153B165
P 1800 5200
AR Path="/6153B165" Ref="J?"  Part="1" 
AR Path="/61487F53/6153B165" Ref="J?"  Part="1" 
F 0 "J?" H 1880 5192 50  0000 L CNN
F 1 "T4Enc" H 1880 5101 50  0000 L CNN
F 2 "" H 1800 5200 50  0001 C CNN
F 3 "~" H 1800 5200 50  0001 C CNN
	1    1800 5200
	1    0    0    -1  
$EndComp
$Comp
L Connector:USB_B_Micro J?
U 1 1 61545332
P 6250 4700
AR Path="/61545332" Ref="J?"  Part="1" 
AR Path="/61487F53/61545332" Ref="J?"  Part="1" 
F 0 "J?" H 6307 5167 50  0000 C CNN
F 1 "USB_B_Micro" H 6307 5076 50  0000 C CNN
F 2 "" H 6400 4650 50  0001 C CNN
F 3 "~" H 6400 4650 50  0001 C CNN
	1    6250 4700
	1    0    0    -1  
$EndComp
$Sheet
S 8050 2900 700  500 
U 61545F96
F0 "Clarinoid3_Main_MidiIn" 50
F1 "Clarinoid3_Main_MidiIn.sch" 50
F2 "MidiRx" I L 8050 3250 50 
F3 "+3.3v" I L 8050 3150 50 
F4 "GND" I L 8050 2950 50 
F5 "+5v" I L 8050 3050 50 
$EndSheet
$Sheet
S 8050 3650 550  350 
U 6154613D
F0 "Clarinoid3_Main_MidiOut" 50
F1 "Clarinoid3_Main_MidiOut.sch" 50
F2 "MidiTX" I L 8050 3900 50 
F3 "+3.3v" I L 8050 3800 50 
F4 "GND" I L 8050 3700 50 
$EndSheet
$EndSCHEMATC
