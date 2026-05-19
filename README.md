# Josh 2.0 -- automated lipid-laser experimentation

## Problem 
Lipids are studied with lasers and X-rays. Certain institutions provide opportunities to use these high-intensity light sources to study these lipids in the form of exposure time; however, exposure time isn't maximized to obtain the maximum amount of data, as it requires individuals to manually place and remove their lipids from the position of exposure.  

## Our Research
Professor Harper leads a research project at Calvin University to automate the process of lipid-laser experimentation. This project has two aspects: 
**(1)** a physical machine that experiments on the lipids to generate electron density distribution data, 
**(2)** using Mathematica and Fourier transforms to analyse the electron density data and create a mathematical model for any specific lipid.

**Josh 2.0** is the embodiment of the "physical machine". It is a tool that is able to perform lipid-laser experimentation (moving lipid samples in an out of laser point of a specific intensity while the samples are kept at a controlled temprature) on multiple lipid samples automatically and sequentially. It comprises the following:
- Queen Bee PRO 500x500mm CNC machine
- Teknic sc servo motors CPM-SCSK-2310P-EQNA and associated electronics
- Neslab RTE7 temperature controlled bath
- Oven Industries 5R6-900 temperature controller
- Newport 1815-C Optical power meter
- LabJack UV-L3 analog to digital converter
- Thor labs pl-202 usb powered laser
- Dreamquest Windows mini-pc

## Overview
Our lipid sampling setup contains many elements which must or may be controlled by a computer. The setup is made using a Queen Bee PRO cnc machine, Teknic sc servo motors, a temperature controlled water bath, thermoelectric, laser, and light sensor which is plugged into a LabJack U3-LV analog to digital converter. Thus we have computer communication between the DreamQuest mini pc and the sc motors, bath, thermoelectric controller, and LabJack. Our goal was to create the functionality of sending and receiving values between Mathematica and these devices. We did this by creating a series of executables that can be run from Mathematica using the RunProcess command. Values to send are passed to the executables through command line arguments and values received from the devices are fed back to Mathematica via stdout. 

### Code
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

**sFoundation20.dll**
The sFoundation20.dll package is required...

**bath_dump.exe**
----

**bath_get_temp.exe**
----
**bath_off.exe**
----
**bath_on.exe**
----
**bath_set_temp.exe**
----
**read_labjack_ain0.exe**
----
**record.exe**
----
**sm_get_position_mm.exe**
----
**sm_home.exe**
----
**bath_on.exe**
----
**sm_manual_control.exe**
----
**sm_set_position_mm.exe**
----
**tc_dump.exe**
----
**tc_get_mode.exe**
----
**tc_get_temp.exe**
----
**tc_on.exe**
----
**tc_ramp_soak.exe**
----
**tc_set_mode.exe**
----
**tc_set_temp.exe**
----



### How to use
The binaries folder comes with precompiled binaries and the sFoundation20.dll which is essential for 
controlling the servo motors. Thus the executables should work straight off the bat without extra 
configuration. 
Run the executables in cmd like this: C:\> <executable_name> <param 1> <param 2> ...
In Mathematica use the "RunProcess" command (pass parameters as command line argument strings).
Find the COM port of the device in the Windows Device Manager.
You may, for many reasons, want to reconfigure the servo motors. To do this, 
install Teknic software from https://teknic.com/files/downloads/ClearView_Install.zip. This will 
install the sdk and 'Clearview' which is Teknic's gui for monitoring and configuring motors. With 
Clearview, one can wire the limit switches and set up the homing procedure, software position limits, 
velocity, accel, and torque limits. If you want to write more or modify code, you may want to install 
the LabJack sdk for the U3-LV from 
https://files.labjack.com/installers/LJM/Windows/x86_64/release/LabJack_2024-05-16.exe. You may also 
need to mess around with the python file names and folder structure to get python code to run 
properly. For c++, the necessary parts of the sdk either need to be in the same directory as the code 
or some environment variables need to be set. The .vscode folderhas the files for vscode to autoset 
the environment but if you want to use a different editor you will have to run the commands manually. 
The commands to set up the environment are in init_mach_ctrl_env.cmd. They let the compiler know where 
the Teknic and LabJack libraries are. The compile command is in .vscode/tasks.json under the section 
'args: '.


**CNC Setup**
Limit switches are used for homing
Motors home to A, A with all switches normally closed (open when activated).
This puts the motor in the top left corner.
Motors then back off 6400 from the limit switches and are zeroed. Axes are as such:
    ------------------------------
    | (A,A)         ^ +z		  |
    |	            |		      |
    |       -x	    |	+x	      |
    |        <------|------>	  |
    |	            |		      |
    |	            |		      |
    |	            v -z		  | (*)y
    ------------------------------
Software limits are used for soft limits for normal use (won't throw fatal errors)
Software limits are x = [-6400, 180000] = [-8, 225]mm
			        z = [-78000, 6400] = [-97.5, 8]mm


**Troubleshooting**
There are currently a few known issues with the code that cannot be resolved easily.
1. The default drivers for the usb to rs232 cables are not compatible with windows 11. I have tried to 
    install different ones but they periodically revert back. Current fix: right click on the 
    connection in Device Manager and click uninstall device, check 'Attempt to remove driver' and hit 
    uninstall. Unplug and replug the usb cable.
2. The program to home the setup sometimes needs to be run at least 2 or 3 times when the motors are 
    plugged in for the first time or after a fault is triggered.
3. I don't exactly know how to run the ramp/soak feature of the temperature controller. It likes to 
    iterate through all the ramp/soak sequences (8 or 16) even if I tell it to only run the current 
    sequence.
4. The bath cannot be concurrently be in computer and manual mode. To switch modes, turn the bath on 
    and toggle the computer control switch.
There are a few troubleshooting tips. If the program is not running well in Mathematica, try running 
it in command prompt or powershell. If there is a problem with the servo motors, the Clearview 
software is very helpful for debugging. If there is trouble with the temperature controller, run 
M5R6900.exe (in the dropbox folder) for a graphical interface. tc_dump.exe is also helpful.


## Teams 
### 2024/2025
- Josh Darrow
- Zachary Mejer
- Samuel Ntadom
  
### 2025/2026
- Josh Darrow
- Zachary Mejer
- Samuel Ntadom
