EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 5 10
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
L Connector:AudioJack3 J?
U 1 1 61549A0C
P 3200 2450
AR Path="/61549A0C" Ref="J?"  Part="1" 
AR Path="/61487F53/61549A0C" Ref="J?"  Part="1" 
AR Path="/61487F53/6154613D/61549A0C" Ref="J?"  Part="1" 
AR Path="/61487F53/61545F96/61549A0C" Ref="J?"  Part="1" 
F 0 "J?" H 3182 2775 50  0000 C CNN
F 1 "MIDIIn" H 3182 2684 50  0000 C CNN
F 2 "clarinoid3:Jack_3.5mm_CUI_SJ-3523-SMT_Horizontal" H 3200 2450 50  0001 C CNN
F 3 "~" H 3200 2450 50  0001 C CNN
	1    3200 2450
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 6154D622
P 3850 2450
F 0 "R?" V 3643 2450 50  0000 C CNN
F 1 "220" V 3734 2450 50  0000 C CNN
F 2 "" V 3780 2450 50  0001 C CNN
F 3 "~" H 3850 2450 50  0001 C CNN
	1    3850 2450
	0    1    1    0   
$EndComp
$Comp
L Device:R R?
U 1 1 6154DBB4
P 5900 2500
F 0 "R?" H 5970 2546 50  0000 L CNN
F 1 "470" H 5970 2455 50  0000 L CNN
F 2 "" V 5830 2500 50  0001 C CNN
F 3 "~" H 5900 2500 50  0001 C CNN
	1    5900 2500
	1    0    0    -1  
$EndComp
$Comp
L Diode:1N4148 D?
U 1 1 6154E038
P 4150 2600
F 0 "D?" V 4104 2680 50  0000 L CNN
F 1 "1N4148" V 4195 2680 50  0000 L CNN
F 2 "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal" H 4150 2425 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/1N4148_1N4448.pdf" H 4150 2600 50  0001 C CNN
	1    4150 2600
	0    1    1    0   
$EndComp
Text HLabel 6050 2650 2    50   Input ~ 0
MidiRx
Text HLabel 5900 2350 1    50   Input ~ 0
+3.3v
Text HLabel 5500 2750 2    50   Input ~ 0
GND
Text HLabel 5500 2450 2    50   Input ~ 0
GND
Connection ~ 5900 2650
Wire Wire Line
	5900 2650 6050 2650
Wire Wire Line
	5500 2650 5900 2650
Text HLabel 5850 1800 2    50   Input ~ 0
+5v
Wire Wire Line
	5500 2350 5500 1800
Wire Wire Line
	5500 1800 5850 1800
Text HLabel 3400 2350 2    50   Input ~ 0
GND
$Comp
L Device:CP1 C?
U 1 1 6154FC6E
P 5100 1650
F 0 "C?" H 5215 1696 50  0000 L CNN
F 1 "0.1uF" H 5215 1605 50  0000 L CNN
F 2 "" H 5100 1650 50  0001 C CNN
F 3 "~" H 5100 1650 50  0001 C CNN
	1    5100 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 1500 5500 1500
Wire Wire Line
	5500 1500 5500 1800
Connection ~ 5500 1800
Text HLabel 5100 1800 3    50   Input ~ 0
GND
Wire Wire Line
	3400 2450 3700 2450
Wire Wire Line
	4000 2450 4150 2450
Connection ~ 4150 2450
Wire Wire Line
	4150 2450 4900 2450
$Comp
L Isolator:6N138 U?
U 1 1 6154C316
P 5200 2550
F 0 "U?" H 5200 3017 50  0000 C CNN
F 1 "6N138" H 5200 2926 50  0000 C CNN
F 2 "" H 5490 2250 50  0001 C CNN
F 3 "http://www.onsemi.com/pub/Collateral/HCPL2731-D.pdf" H 5490 2250 50  0001 C CNN
	1    5200 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 2550 3650 2550
Wire Wire Line
	3650 2550 3650 2750
Wire Wire Line
	3650 2750 4150 2750
Wire Wire Line
	4150 2750 4900 2750
Wire Wire Line
	4900 2750 4900 2650
Connection ~ 4150 2750
$EndSCHEMATC
