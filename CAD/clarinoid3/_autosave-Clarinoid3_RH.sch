EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 8 10
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
L Switch:SW_Push SW?
U 1 1 614AB68A
P 4550 3350
F 0 "SW?" H 4550 3635 50  0000 C CNN
F 1 "LHx1" H 4550 3544 50  0000 C CNN
F 2 "clarinoid3:MB2511S2G45" H 4550 3550 50  0001 C CNN
F 3 "~" H 4550 3550 50  0001 C CNN
	1    4550 3350
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW?
U 1 1 614AB690
P 4800 3800
F 0 "SW?" H 4800 4085 50  0000 C CNN
F 1 "LHx3" H 4800 3994 50  0000 C CNN
F 2 "clarinoid3:MB2511S2G45" H 4800 4000 50  0001 C CNN
F 3 "~" H 4800 4000 50  0001 C CNN
	1    4800 3800
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW?
U 1 1 614AB696
P 5250 3350
F 0 "SW?" H 5250 3635 50  0000 C CNN
F 1 "LHx4" H 5250 3544 50  0000 C CNN
F 2 "clarinoid3:MB2511S2G45" H 5250 3550 50  0001 C CNN
F 3 "~" H 5250 3550 50  0001 C CNN
	1    5250 3350
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW?
U 1 1 614AB69C
P 4250 3800
F 0 "SW?" H 4250 4085 50  0000 C CNN
F 1 "LHx2" H 4250 3994 50  0000 C CNN
F 2 "clarinoid3:MB2511S2G45" H 4250 4000 50  0001 C CNN
F 3 "~" H 4250 4000 50  0001 C CNN
	1    4250 3800
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW?
U 1 1 614AB6A2
P 5550 3800
F 0 "SW?" H 5550 4085 50  0000 C CNN
F 1 "LHx5" H 5550 3994 50  0000 C CNN
F 2 "clarinoid3:MB2511S2G45" H 5550 4000 50  0001 C CNN
F 3 "~" H 5550 4000 50  0001 C CNN
	1    5550 3800
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x05 J?
U 1 1 614AB6A8
P 6350 4200
F 0 "J?" H 6430 4242 50  0000 L CNN
F 1 "T4Switches" H 6430 4151 50  0000 L CNN
F 2 "" H 6350 4200 50  0001 C CNN
F 3 "~" H 6350 4200 50  0001 C CNN
	1    6350 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 3350 4750 3450
Wire Wire Line
	4750 3450 5000 3450
Wire Wire Line
	5450 3450 5450 3350
Wire Wire Line
	5450 3450 5750 3450
Wire Wire Line
	5750 3450 5750 3800
Connection ~ 5450 3450
Wire Wire Line
	5000 3800 5000 3450
Connection ~ 5000 3450
Wire Wire Line
	5000 3450 5450 3450
Wire Wire Line
	4450 3800 4450 3450
Wire Wire Line
	4450 3450 4750 3450
Connection ~ 4750 3450
Wire Wire Line
	5350 3800 5350 4000
Wire Wire Line
	5350 4000 6150 4000
Wire Wire Line
	5050 3350 5050 4100
Wire Wire Line
	5050 4100 6150 4100
Wire Wire Line
	4600 3800 4600 4200
Wire Wire Line
	4600 4200 6150 4200
Wire Wire Line
	4350 3350 3800 3350
Wire Wire Line
	3800 3350 3800 4300
Wire Wire Line
	3800 4300 6150 4300
Wire Wire Line
	4050 3800 4050 4400
Wire Wire Line
	4050 4400 6150 4400
$EndSCHEMATC