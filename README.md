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
passed to the executables through command line arguments and values received from the devices are 
fed back to Mathematica via stdout. 


Code:
Our code uses several packages from outside sources as well as some we wrote ourselves. The 
Teknic Clearpath SC servo motors use the Teknic sdk which is developed by them and can be 
downloaded from their website. The LabJack also has an sdk which we used for python and c++ code 
although some of the python files's names had to be changed to get the right imports. For the bath 
and temperature controller we ended up writing c++ classes to streamline serial communication to 
them although this can and has been done in the past using python (with the serial package).

For the bath and temperature controller, we wrote two c++ classes. For the bath, we only need the 
most basic functionality of turning it on and off, reading the current temperature and setting the 
setpoint. For the temperature controller we need much more functionality (ramp and soak) which is 
reflected in the length of the code :). Both classes have a similar constructor/destructor, read, 
write, checksum, and parsing methods for raw data. The bath class (RTE7) has more components 
'hardcoded' because we didn't need as much functionality and because the message format was more 
complex than the temp controller (Oven5R6900). The temp controller class has a more advanced 
'message dispatcher' that builds messages from the message id code and value to pass.


Assumptions:
The executables are contained in the binaries folder and the code to produce them is in the c++/src 
folder. We assume that the user of this code has the Teknic and LabJack sdks already installed on their 
system