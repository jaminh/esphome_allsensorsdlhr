# Allsensors DLHR Pressure Sensor

This component allows reading pressure and temperature measurements using the
Amphenol Allsensors DLHR series of sensors. While these sensors support both
SPI and I2C, this component only supports reading values over SPI.

## Configuration

The first step to configuring the sensor is configuring the
https://esphome.io/components/spi.html#spi (SPI) bus.

Next configure the sensor.

### Example Configuration
```yaml
  - platform: allsensorsdlhr
    pressure:
      name: "Hvac Monitor Pressure"
      unit_of_measurement: "inH2O"
      pressure_range: 2.0
      pressure_type: 2.0
    temperature:
      name: "Hvac Monitor Temp P"
    cs_pin: GPIO15
    update_interval: "10s"
```

### Configuration Variables
* **pressure**: The information for the pressure sensor.
  * **name**: The name for the pressure sensor.
  * **unit_of_measurement**: The unit of measurement for the pressure readings.
    Defaults to "inH20" or inches of water column.
  * **pressure_range**: The pressure range the sensor is capable of measuring.
    If the sensor is a gauge type the range is from 0 to the pressure range
value. If the type is "differential" the range will be from the negative of the
range value to a positive range value (e.g. if pressure_range is set to "2.0",
the range is from -2.0 to +2.0).
  * **pressure_type**: The type of pressure sensor. Use "1.0" for gauge type
    sensors, "2.0" for differential sensors.
* **temperature**: The information for the temperature sensor.
  * **name**: The name for the temperature sensor.
* **cs_pin**: SPI Chip select pin.
* **update_interval**: The interval to check the sensor.

## Additional information
The component is set to use a CLK frequency of 2MHz. The acceptable frequency
range for the sensor is 50KHz to 5MHz.

The sensors support multiple ways of taking measurements, either by taking a
single measurement and returning the values read, or returning an average of 2,
4, 8, or 16 samples. Currently this component only supports reading an average
of 8 samples.
