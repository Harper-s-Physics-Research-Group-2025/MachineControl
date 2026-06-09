import serial
import ctypes
import decimal
import sys
from time import sleep
from math import ceil,floor

error_message = ctypes.WinError()

# Format the exception message
exception_message = f"could not open port {'COM9'!r}: {error_message}"

def checkSum(s):
    value = 0
    for w in s[1:]:
        value +=int(w)
    value = value % 256
    ss=str(hex(value)[2:]).rjust(2,'0')
    return bytearray(ss,'utf8')

def hexc2dec(bufp):
    newval=0
    divvy=268435456
    #print(bufp)
    try:
        for pn in range(1,9):
            vally=bufp[pn+1]
            #print(vally)
            if(vally < 97):
                subby=48
            else:
                subby=87
            newval+=((bufp[pn]-subby)*divvy)
            divvy/=16
    except:pass
    if(newval > 2147483647):
        newval=newval-pow(2,32)
    return newval

#input s is a string of bytes that matches a valid command (see our document)
#input i is a integer
#return value is a integer (see our document)
#Function responsible for reading temperature
def setTemperture(s, i):
    command=b'*00'
    command+=s
    if i==0:
        command+=b"00000000"
    else:
        command+=bytearray(str(hex (i & ((1 << 32) - 1))[2:]).rjust(8,'0'),'utf8')
    command+=checkSum(command)
    command+=b'\r'
    ser.write(command)
    #print(command)

    response= ser.read_until(b'^')
    print(hexc2dec(response))
    return hexc2dec(response)

def dropzeros(number):
    mynum = decimal.Decimal(number).normalize()
    # e.g 22000 --> Decimal('2.2E+4')
    return mynum.__trunc__() if not mynum % 1 else float(mynum)

def getCurrentTemp(port): # This might not work 
    return setTemperture(b'd2',int(100))

def startRamp(temp, rate, port, soaktime=6000): #think this will work
    #stop previous ramp

    setTemperture(b"e8", 0)

    print(setTemperture(b'14',int(0)))

    #set desired temp
    setTemperture(b"80",int(temp*100))

    #set ramp time
    '''currentTemp = getCurrentTemp(port)
    print(currentTemp)
    deltaC = abs(currentTemp-temp)
    time = int(deltaC/rate)'''
    #previously time
    setTemperture(b"90",int(1000))

    #set soak time
    setTemperture(b"a0",int(soaktime))

    #set repeats
    setTemperture(b"b0",int(0))
    setTemperture(b"c0",int(0))

    #run method - 0 changes regardless of curr temp, 1 waits for set temp to be reached
    setTemperture(b"d0", int(1))

    #ramp soak allowable delta (precision?) 100*Temperature delta
    print(setTemperture(b"d2",int(500)))


    #set increment counter (.2*base time in seconds)
    setTemperture(b"d4",int(5))

    #Start ramp
    setTemperture(b"e8",int(1))

def setTemp(tempWanted):
    setTemperture(b"10", round(tempWanted*100))

def tempRamp(initial_temp,final_temp,time):
    delta_T = final_temp - initial_temp
    time_step = time/delta_T
    if final_temp > initial_temp:
        sleep(time_step*(initial_temp - floor(initial_temp)))
        setTemp(ceil(initial_temp))
        number_of_steps = ceil(final_temp) - floor(initial_temp)
        for i in range(number_of_steps-2):
            sleep(time_step)
            setTemp(ceil(initial_temp) + 1 + i)
        sleep(time_step*(ceil(final_temp) - final_temp))
        setTemp(final_temp)
    elif final_temp == initial_temp:
        print('Final temp and initial temp are equal.')
    else:
        sleep(time_step*(ceil(initial_temp) - initial_temp))
        setTemp(floor(initial_temp))
        number_of_steps = ceil(initial_temp) - floor(final_temp)
        for i in range(number_of_steps-2):
            sleep(time_step)
            setTemp(floor(initial_temp) - 1 - i)
        sleep(time_step*(final_temp - floor(final_temp)))
        setTemp(final_temp)

# ---------------------------------------- EXECUTIVE CODE -------------------------------------------------

if __name__ == '__main__':
    ser= serial.Serial('COM9', 19200, timeout=1)
    ser.close()
    ser.open()
    if len(sys.argv) < 3:
        print("Usage: python setTemperature.py <command> <value>")
        sys.exit(1)

    choice = sys.argv[1].lower()
    values = sys.argv[2:]
    match choice:
        case 's':
            ser.reset_input_buffer()
            setTemperture(b"10", round(int(values[0])*100))
        #TODO: Indocummentation emphasise on the need to pass in correct values
        case 'r':
            init_temp = int(values[0])
            fin_temp  = int(values[1])
            time =  float(values[2])
            tempRamp(init_temp, fin_temp, time)
        case 'c':
            lowTemp=float(values[0])
            highTemp=float(values[1])
            interval=float(values[2])
            cycles=abs(int(values[3]))
            for cycle in range(cycles):
                setTemperture(b"10", round(lowTemp*100))
                sleep(interval)
                setTemperture(b"10", round(highTemp*100))
                sleep(interval)
        case _:
            print("Sorry, this option does not exist")
    ser.close()