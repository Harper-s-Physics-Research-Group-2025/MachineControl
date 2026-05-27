# Josh 2.0-- automated lipid-laser experimentation

**Problem** 

Lipids are commonly studied using lasers and X-rays at facilities that provide access to high-intensity light sources. Researchers are allocated fixed windows of exposure time for their experiments, but a significant portion of that time is lost to manual sample handling — physically placing and removing lipid samples from the beam position. This bottleneck prevents researchers from fully utilizing their allotted time and limits the volume of data they can collect.

**Our Research**

Professor Harper's research project at Calvin University aims to address this inefficiency by automating the lipid-laser experimentation process. The project comprises two complementary components: first, a physical machine capable of autonomously positioning and cycling lipid samples through the beam to generate electron density distribution data (Josh 2.0). Second, a computational pipeline built in Mathematica that applies Fourier transforms to the resulting electron density data, producing a mathematical model for any given lipid structure.

*Josh 2.0* comprises the following:
- Queen Bee PRO 500x500mm CNC machine
- Teknic sc servo motors CPM-SCSK-2310P-EQNA and associated electronics
- Neslab RTE7 temperature-controlled bath
- Oven Industries 5R6-900 temperature controller
- Newport 1815-C Optical power meter
- LabJack UV-L3 analog to digital converter
- Thor Labs PL-202 USB-powered laser
- Dreamquest Windows mini-pc

# Code Documentation v1:

TODO: write out the dependencies for the executables

Josh 2.0 integrates several components that require/can be computer-controlled: 
- a Queen Bee PRO CNC machine
- Teknic ClearPath SC servo motors
- a temperature-controlled water bath
- a thermoelectric module, a laser, and
- a light sensor connected through a LabJack U3-LV analog-to-digital converter.
  
	Our current setup uses a DreamQuest mini PC as the central hub, communicating with the servo motors, water bath, thermoelectric controller, and LabJack. And the system is operated via standalone command-line executables that accept input parameters and return operational data directly through the Command Line Interface (CLI).
	However, our primary objective is to enable Mathematica to send commands to and receive data from each of these devices. 

## Code v1

### bath_dump.cpp

Its primary purpose is to query the device and output the current water bath temperature and its target setpoint.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port as a single argument:
<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./bath_dump {COM Port}</code>
</pre>

**Example**</br>
If your water bath is connected to COM3 (Windows) or /dev/ttyUSB0 (Linux), run:
<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./bath_dump COM3</code>
</pre>

**Output Format**:
<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Temp: [Current Temperature]</code>
<code>Setpoint: [Target Temperature]</code>
</pre>


**Example**: 
<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Temp: 23.5</code>
<code>Setpoint: 25</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

----
### bath_get_temp.cpp

Its primary purpose is to query the device and output only the current water bath temperature.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port as a single argument:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./bath_get_temp {COM port}</code>
</pre>

**Example**</br>
If your water bath is connected to COM3 (Windows) or /dev/ttyUSB0 (Linux), run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./bath_get_temp COM3</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Temp: [Current Temperature]</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Temp: 23.5</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (Temperature successfully read)
* **`1`**: Failure (Hardware communication error)
* **`2`**: Invalid Arguments (Incorrect number of parameters supplied)
----

### bath_off.cpp

Its primary purpose is to send a shutdown command to the water bath device to turn it off.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port as a single argument:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./bath_off {COM port}</code>
</pre>

**Example**</br>
If your water bath is connected to COM3 (Windows) or /dev/ttyUSB0 (Linux), run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./bath_off COM3</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Success!</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Success!</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (The water bath turned off successfully)
* **`1`**: Failure (Shutdown failed / hardware communication error)
* **`2`**: Invalid Arguments (Incorrect number of parameters supplied)
----
### bath_on.cpp

Its primary purpose is to send an initialization command to the water bath device to turn it on.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port as a single argument:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./bath_on {COM port}</code>
</pre>

**Example**</br>
If your water bath is connected to COM3 (Windows) or /dev/ttyUSB0 (Linux), run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./bath_on COM3</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Success!</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Success!</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (The water bath turned on and initialized successfully)
* **`1`**: Failure (Initialization failed / hardware communication error)
* **`2`**: Invalid Arguments (Incorrect number of parameters supplied)
----
### bath_set_temp.cpp

Its primary purpose is to update the target temperature setpoint of the water bath device.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port and the desired temperature value as arguments:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./bath_set_temp {COM port} {temp}</code>
</pre>

**Example**</br>
If your water bath is connected to COM3 (Windows) or /dev/ttyUSB0 (Linux) and you want to set the temperature to 25.0 degrees, run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./bath_set_temp COM3 25.0</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Set temp: [Target Temperature]</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Set temp: 25</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (Temperature setpoint updated successfully)
* **`1`**: Invalid Arguments (Incorrect number of parameters supplied)
* **`2`**: Failure (Invalid temperature input format or hardware communication error)
----
### read_labjack_ain0.cpp

Its primary purpose is to open a connected LabJack U3 device, configure channel 0 as an analog input, and read its voltage level.

**Usage**</br>
To execute the program, run it from your command line. It does not require any additional arguments:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./read_labjack_ain0</code>
</pre>


**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>[Analog Input Voltage Reading]</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>1.42351</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (LabJack opened, configured, read, and closed cleanly)
* **`-1`**: Failure (An error occurred while attempting to open the LabJack U3 device)
* **`-2`**: Failure (An error occurred while reading the analog input AIN0)
----
### record.cpp

Its primary purpose is to continuously poll data from the system's devices (the Teknic motors, the RTE7 water bath, an Oven5R6900 temperature controller, and a LabJack U3) at a user-defined interval and log the results into a CSV file.

**Usage**</br>
To execute the program, run it from your command line. You can optionally provide a custom filename for the CSV output as an argument (defaults to `device_log.csv` if omitted). The program will interactively prompt you for the polling interval and device COM ports upon startup:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./record [optional_output_filename.csv]</code>
</pre>

**Example**</br>
To run the logger and save the data to a custom file named `experiment_run1.csv`:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./record experiment_run1.csv</code>
</pre>

**Output Format**:
The program streams the live polled data directly to the command line while simultaneously appending it to the CSV file using the following structure:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Enter polling interval in seconds: [user_input]</code>
<code>Enter bath port: [user_input]</code>
<code>Enter Temperature Controller port: [user_input]</code>
<code>Polling started. Press Ctrl+C to stop.</code>
<code>[[YYYY-MM-DD HH:MM:SS]] Polled data: [bath_temp],[sample_temp],[laser_intensity]</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Enter polling interval in seconds: 5</code>
<code>Enter bath port: 3</code>
<code>Enter Temperature Controller port: 4</code>
<code>Polling started. Press Ctrl+C to stop.</code>
<code>[2026-05-26 11:30:12] Polled data: 23.500000,24.100000,1.423510</code>
<code>[2026-05-26 11:30:17] Polled data: 23.500000,24.200000,1.421100</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation initialized successfully or failed.

* **`0`**: Success / Closed cleanly (Note: The program runs in an infinite loop until interrupted via `Ctrl+C`)
* **`1`**: Failure (Could not open or write to the specified CSV log file)
* **`-1`**: Failure (Hardware initialization failed for either the Teknic motors or the LabJack U3 device)
----
### sm_get_position_mm.cpp

Its primary purpose is to read the current position of the lipid holder interface from the Teknic servo motors along both the horizontal (X) and vertical (Z) axes, converting the raw motor counts into millimeters.

**Usage**</br>
To execute the program, run it from your command line. It does not require any additional arguments:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./sm_get_position_mm</code>
</pre>


**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>mm: ([X Position], [Z Position])</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>mm: (12.5, 4.25)</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (Motor positions successfully read and displayed)
* **`1`**: Failure (No communication hub found, or a critical ClearPath motor exception occurred)
* **`2`**: Invalid Arguments (Incorrect number of parameters supplied)
----
### sm_home.cpp

Its primary purpose is to home the Teknic servo motors along both the X and Z axes to their physical switch limits (top left position) to initialize and calibrate their positioning system.

**Usage**</br>
To execute the program, run it from your command line. You can optionally provide a custom timeout limit in milliseconds as an argument (defaults to 20000 ms if omitted):

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./sm_home [optional_timeout_ms]</code>
</pre>

**Example**</br>
To execute the homing routine with a custom timeout of 30000 milliseconds (30 seconds):

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./sm_home 30000</code>
</pre>

**Output Format**:
The program outputs axis calibration updates during execution, followed by the soft limits and the final verified coordinates:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>[Axis Name] completed homing with soft limits now active, current position (mm): [Position] soft limits: [ [Min], [Max] ]</code>
<code>Final position (mm): ([X Position], [Z Position])</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>X-axis completed homing with soft limits now active, current position (mm): 0 soft limits: [ 0, 150 ]</code>
<code>Z-axis completed homing with soft limits now active, current position (mm): 0 soft limits: [ 0, 100 ]</code>
<code>Final position (mm): (0, 0)</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (Both motor axes were safely and successfully homed)
* **`1`**: Failure (A motor alert occurred, the process timed out, or a communication hub could not be found)
----
### sm_manual_control.cpp

Its primary purpose is to establish a low-level keyboard hook to capture arrow key presses, allowing real-time, manual velocity control over the Teknic servo motors along both the X and Z axes. It also incorporates a global Escape key listener for an instant hardware emergency shutdown.

**Usage**</br>
To execute the program, run it from your command line. The program will automatically search for connected SC Hub ports and initialize the motor handles:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./sm_manual_control</code>
</pre>


**Output Format**:
The program outputs the initialization status and serial numbers of the detected hardware, then waits for manual arrow key inputs:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Found [Number of Hubs] SC Hubs</code>
<code>Motor X  Serial #: [Serial Number]</code>
<code>Motor Z  Serial #: [Serial Number]</code>
<code>Use arrow keys to move motors. Press 'q' to quit.</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Found 1 SC Hubs</code>
<code>Motor X  Serial #: 12435422</code>
<code>Motor Z  Serial #: 12435425</code>
<code>Use arrow keys to move motors. Press 'q' to quit.</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation initialized successfully or encountered a runtime environment issue.

* **`0`**: Success (Program exited cleanly using the 'q' key)
* **`1`**: Failure (Failed to install the low-level Windows keyboard hook)
* **`-1`**: Failure (Invalid system environment configuration—found either 0 or multiple ClearPath SC Hub controllers connected)
----
### sm_set_position_mm.cpp

Its primary purpose is to set the physical position of the lipid holder interface using the Teknic servo motors along both the X (horizontal) and Z (vertical) axes by translating millimeter inputs into motor counts.

**Usage**</br>
To execute the program, run it from your command line by providing the target X position, target Z position, desired velocity in RPM, and an optional movement timeout value in seconds:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./sm_set_position_mm {x_pos} {z_pos} {vel (rpm)} [optional_timeout_s]</code>
</pre>

**Example**</br>
To move the lipid holder to an X position of 20 mm and a Z position of 15 mm at a speed of 500 RPM, run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./sm_set_position_mm 20 15 500</code>
</pre>

**Output Format**:
Upon completing the move within the timeout window, the program echoes back the final verified coordinates in millimeters:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>([Final X Position], [Final Z Position])</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>(20, 15)</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the positioning sequence succeeded or failed.

* **`0`**: Success (Motors moved to target coordinates successfully)
* **`1`**: Failure (Incorrect number of arguments, parameters exceed the 1000 RPM velocity limit, movement timed out, motors are unhomed, or an internal SC Hub exception occurred)
* **`2`**: Invalid Arguments (Failed to convert input arguments to valid floating-point numbers)
----
### tc_dump.cpp

Its primary purpose is to query the Oven5R6900 thermoelectric temperature controller device, outputting its current operational parameters, PID configurations, and its full ramp/soak profile sequence table.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port as a single argument:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_dump {COM port}</code>
</pre>

**Example**</br>
If your temperature controller is connected to COM4 (Windows) or /dev/ttyUSB1 (Linux), run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_dump COM4</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Off (0), On (1)</code>
<code>H-bridge Output: [0 or 1]</code>
<code>Mode (0-3): [Mode Number]</code>

<code>Current Temperature: [Temperature]</code>
<code>Current Voltage: [Voltage]</code>

<code>Set Temperature: [Target Temperature]</code>
<code>Set Voltage: [Max Voltage Limit]</code>
<code>Proportional Bandwidth: [P Value]</code>
<code>Integral Gain: [I Value]</code>
<code>Derivative Gain: [D Value]</code>

<code>Sequence Pointer: [Active Step ID]</code>
<code>Ramp/Soak: [0 or 1]</code>
<code>Ramp: [0 or 1]</code>
<code>Soak: [0 or 1]</code>
<code>Ramp/soak method: [Method ID]</code>
<code>Ramp/soak max deviation (C): [Deviation]</code>
<code>Ramp/soak counter interval (s): [Interval]</code>

<code>            Sequence           0           1           2 ...</code>
<code>           Soak temp    [Value]     [Value]     [Value] ...</code>
<code>       Ramp duration    [Value]     [Value]     [Value] ...</code>
<code>       Soak duration    [Value]     [Value]     [Value] ...</code>
<code>   Remaining repeats    [Value]     [Value]     [Value] ...</code>
<code>       Next sequence    [Value]     [Value]     [Value] ...</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Off (0), On (1)</code>
<code>H-bridge Output: 1</code>
<code>Mode (0-3): 0</code>

<code>Current Temperature: 24.1</code>
<code>Current Voltage: 1.15</code>

<code>Set Temperature: 25.0</code>
<code>Set Voltage: 5.0</code>
<code>Proportional Bandwidth: 10.0</code>
<code>Integral Gain: 0.5</code>
<code>Derivative Gain: 2.1</code>

<code>Sequence Pointer: 0</code>
<code>Ramp/Soak: 0</code>
<code>Ramp: 0</code>
<code>Soak: 0</code>
<code>Ramp/soak method: 1</code>
<code>Ramp/soak max deviation (C): 5.0</code>
<code>Ramp/soak counter interval (s): 1.0</code>

<code>            Sequence           0           1           2</code>
<code>           Soak temp        20.0        22.5        25.0</code>
<code>       Ramp duration         300         120         180</code>
<code>       Soak duration         300         600         600</code>
<code>   Remaining repeats           0           1           0</code>
<code>       Next sequence           1           2           3</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (State parameters and sequence matrix successfully read and displayed)
* **`1`**: Failure (Initial parameters read failed or sequence step array retrieval timed out/failed)
* **`2`**: Invalid Arguments (Incorrect number of parameters supplied)
----
### tc_get_mode.cpp

Its primary purpose is to query the Oven5R6900 thermoelectric temperature controller device and output its current operational mode.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port as a single argument:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_get_mode {COM port}</code>
</pre>

**Example**</br>
If your temperature controller is connected to COM4 (Windows) or /dev/ttyUSB1 (Linux), run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_get_mode COM4</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Mode: [Mode Number]</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Mode: 0</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (Mode parameter successfully read and displayed)
* **`1`**: Failure (Mode read failed / hardware communication error)
* **`2`**: Invalid Arguments (Incorrect number of parameters supplied)
----
### tc_get_temp.cpp

Its primary purpose is to query the Oven5R6900 thermoelectric temperature controller device and output its current temperature reading in Celsius.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port as a single argument:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_get_temp {COM port}</code>
</pre>

**Example**</br>
If your temperature controller is connected to COM4 (Windows) or /dev/ttyUSB1 (Linux), run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_get_temp COM4</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Temp: [Current Temperature]</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Temp: 24.1</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (Temperature successfully read)
* **`1`**: Failure (Temperature read failed / hardware communication error)
* **`2`**: Invalid Arguments (Incorrect number of parameters supplied)
----
### tc_off.cpp

Its primary purpose is to send a shutdown command to the Oven5R6900 thermoelectric temperature controller device to disable its power output.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port as a single argument:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_off {COM port}</code>
</pre>

**Example**</br>
If your temperature controller is connected to COM4 (Windows) or /dev/ttyUSB1 (Linux), run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_off COM4</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Success!</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Success!</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (The temperature controller was disabled successfully)
* **`1`**: Failure (Shutdown failed / hardware communication error)
* **`2`**: Invalid Arguments (Incorrect number of parameters supplied)
----
### tc_on.cpp

Its primary purpose is to send an activation command to the Oven5R6900 thermoelectric temperature controller device to enable its power output.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port as a single argument:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_on {COM port}</code>
</pre>

**Example**</br>
If your temperature controller is connected to COM4 (Windows) or /dev/ttyUSB1 (Linux), run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_on COM4</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Success!</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Success!</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (The temperature controller output was enabled successfully)
* **`1`**: Failure (Activation failed / hardware communication error)
* **`2`**: Invalid Arguments (Incorrect number of parameters supplied)
----
### tc_ramp_soak.cpp

Its primary purpose is to program and initiate a ramp/soak temperature profile sequence on the Oven5R6900 thermoelectric temperature controller device.

**Usage**</br>
To execute the program, run it from your command line by providing the COM port and the sequence parameters (sequence number, target soak temperature, ramp duration, soak duration, max allowed temperature deviation, execution method, number of repeats, and the next sequence ID):

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_ramp_soak {COM port} {seq_num} {soak_temp} {ramp_dur} {soak_dur} {deviation} <method> <repeats> <next_seq></code>
</pre>

**Example**</br>
To configure sequence slot 0 on COM4 to ramp to 25.0°C over 300 seconds, soak for 300 seconds, with a 5.0°C deviation tolerance, using method 1, repeating 0 times, and linking to sequence step 1:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_ramp_soak COM4 0 25.0 300 300 5.0 1 0 1</code>
</pre>

**Output Format**:
The program outputs the parsed configuration parameters assigned to the execution profile slot:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Sequence: [Sequence ID]</code>
<code>Ramp to (C): [Target Temperature]</code>
<code>Ramp for (s): [Ramp Time]</code>
<code>Soak for (s): [Soak Time]</code>
<code>Tolerance (C): [Max Allowed Deviation]</code>
<code>Method: [Run Method ID]</code>
<code>Repeats: [Iteration Count]</code>
<code>Next Sequence: [Linked Sequence ID]</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Sequence: 0</code>
<code>Ramp to (C): 25</code>
<code>Ramp for (s): 300</code>
<code>Soak for (s): 300</code>
<code>Tolerance (C): 5</code>
<code>Method: 1</code>
<code>Repeats: 0</code>
<code>Next Sequence: 2</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the profile sequence was successfully pushed to the hardware.

* **`0`**: Success (Profile parameters successfully loaded and ramp/soak routine initialized)
* **`1`**: Failure (Incorrect number of arguments supplied, device is not explicitly set to ramp/soak mode [Mode 2], or a hardware transmission failure occurred)
* **`2`**: Invalid Arguments (Failed to safely parse command line parameters into their designated numerical or text data types)
----
### tc_set_temp.cpp

Its primary purpose is to update the constant target temperature setpoint of the Oven5R6900 thermoelectric temperature controller device.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port and the desired temperature value in Celsius as arguments:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_set_temp {COM port} {temp}</code>
</pre>

**Example**</br>
If your temperature controller is connected to COM4 (Windows) or /dev/ttyUSB1 (Linux) and you want to set the target temperature to 25.0°C, run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_set_temp COM4 25.0</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Set temp: [Target Temperature]</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Set temp: 25</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (Temperature setpoint updated successfully)
* **`1`**: Failure (Incorrect number of arguments supplied or hardware communication error)
* **`2`**: Invalid Arguments (Failed to convert temperature input argument to a valid floating-point number)
----
### tc_set_temp.cpp

Its primary purpose is to update the constant target temperature setpoint of the Oven5R6900 thermoelectric temperature controller device.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port and the desired temperature value in Celsius as arguments:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_set_temp {COM port} {temp}</code>
</pre>

**Example**</br>
If your temperature controller is connected to COM4 (Windows) or /dev/ttyUSB1 (Linux) and you want to set the target temperature to 25.0°C, run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_set_temp COM4 25.0</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Set temp: [Target Temperature]</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Set temp: 25</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (Temperature setpoint updated successfully)
* **`1`**: Failure (Incorrect number of arguments supplied or hardware communication error)
* **`2`**: Invalid Arguments (Failed to convert temperature input argument to a valid floating-point number)
----
### tc_set_mode.cpp

Its primary purpose is to update the operational mode of the Oven5R6900 thermoelectric temperature controller device.

**Usage**</br>
To execute the program, run it from your command line by providing the appropriate COM port and the desired mode integer as arguments:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_set_mode {COM port} <mode></code>
</pre>

**Example**</br>
If your temperature controller is connected to COM4 (Windows) or /dev/ttyUSB1 (Linux) and you want to switch it to Mode 2, run:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<span style="color: #6a9955; display: block; margin-bottom: 5px; font-family: sans-serif; font-size: 12px; font-weight: bold;">BASH</span>
<code>./tc_set_mode COM4 2</code>
</pre>

**Output Format**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Mode: [Selected Mode Number]</code>
</pre>

**Example**:

<pre style="background-color: #1e1e1e; color: #d4d4d4; padding: 15px; border-radius: 5px; font-family: 'Courier New', Courier, monospace; overflow-x: auto;">
<code>Mode: 2</code>
</pre>

Error Handling & Exit Codes</br>
The program utilizes standard exit codes to communicate whether the operation succeeded or failed.

* **`0`**: Success (Operational mode updated successfully)
* **`1`**: Failure (Mode configuration change failed / hardware communication error)
* **`2`**: Invalid Arguments (Incorrect number of parameters supplied or mode argument could not be parsed into an integer)
----




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
