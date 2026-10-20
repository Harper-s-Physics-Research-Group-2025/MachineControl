## Analysis

*temp_fall_70_to_20" takes ~ 2*300s for the temperature to fall from 70 to 20 degrees celcius. 
=> temperature fall per second = 50/600 (C/s) ~ 0.08 C/s

*temp_rise_30_to_70" takes ~ 2*175s for the temperature to rise from 36 to 70 degrees celcius.
=> temperature fall per second = 34/350 (C/s) ~ 0.097 C/s



**This implies that the temperature controller changes temperature at ~ 0.1 C/s**


### Enviromental Coniderations
Rate of ctemperature change is computed as follows

\(\frac{\Delta T}{\text{second}}=\frac{P}{5\times c}\)\(\Delta T\): Change in temperature (in \({}^{\circ }\text{C}\))\(P\): Power applied, or heat transferred per second (in Joules per second or Watts)\(m\): Mass, which is \(5\text{ g}\)\(c\): Specific heat capacity of the specific plastic (typically between \(1.0\text{ and }2.5\text{ J/g}\cdot^\circ\text{C}\))

Assuming 5 grams of polypropelene and a 1 Watt power supply, polypropelene's rate of temperature change ~ 0.12 C/s