# Machine Control
This repository contains code that integrates our SC-Motors C++ Commands with Mathematica

Overview:
Our lipid sampling setup contains many elements which must or may be controlled by a computer.
The setup is made using a Queen Bee PRO cnc machine, Teknic sc servo motors, a temperature controlled 
water bath, thermoelectric, laser, and light sensor which is plugged into a LabJack U3-LV analog to 
digital converter. Thus we have computer communication between the DreamQuest mini pc and the sc 
motors, bath, thermoelectric controller, and LabJack. Our goal was to create the functionality of 
sending and receiving values between Mathematica and these devices. We did this by creating a series 
of executables that can be run from Mathematica using the RunProcess command. Values to send are 
input through command line arguments which are handled by the executables and values received from 
the devices are fed back to Mathematica via stdout. 