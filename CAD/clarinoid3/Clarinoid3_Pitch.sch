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
L Device:R_POT RV?
U 1 1 6153186E
P 4050 3000
F 0 "RV?" V 3935 3000 50  0000 C CNN
F 1 "PITCH" V 3844 3000 50  0000 C CNN
F 2 "clarinoid3:MAT060-B100K-231-A01" H 4050 3000 50  0001 C CNN
F 3 "~" H 4050 3000 50  0001 C CNN
	1    4050 3000
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J?
U 1 1 61531874
P 4500 2650
F 0 "J?" H 4580 2692 50  0000 L CNN
F 1 "PITCHHeader" H 4580 2601 50  0000 L CNN
F 2 "" H 4500 2650 50  0001 C CNN
F 3 "~" H 4500 2650 50  0001 C CNN
	1    4500 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	4300 2750 4300 3000
Wire Wire Line
	4300 3000 4200 3000
Wire Wire Line
	4300 2650 4050 2650
Wire Wire Line
	4050 2650 4050 2850
Wire Wire Line
	4300 2550 3900 2550
Wire Wire Line
	3900 2550 3900 3000
$EndSCHEMATC
