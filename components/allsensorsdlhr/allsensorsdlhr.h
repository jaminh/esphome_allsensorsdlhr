// for All Sensors DLHR sensor
#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/spi/spi.h"
#include "esphome/core/component.h"

namespace esphome {
namespace allsensorsdlhr {

class ALLSENSORSDLHRSensor : public PollingComponent,
                           public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
                                                 spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_2MHZ> {
 public:
  void set_pressure_sensor(sensor::Sensor *pressure_sensor) { pressure_sensor_ = pressure_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }
  void setup() override;
  void update() override;
  float get_setup_priority() const override;
  void dump_config() override;
  void set_allsensorsdlhr_pressure_range(float pressure_range);
  void set_allsensorsdlhr_pressure_type(float pressure_type);

 protected:
  float allsensorsdlhr_pressure_range_ = 0.0; // Range of pressure in inches of water column
  float allsensorsdlhr_pressure_type_ = 2.0; // 1.0 for gauge, 2.0 for differential
  uint8_t cmd_buf_[3];             // buffer to hold command response
  uint8_t data_buf_[7];             // buffer to hold sensor data
  uint8_t status_ = 0;         // byte to hold status information.
  uint8_t read_status_ = 0;         // byte to hold read status information.
  uint8_t retry_count_ = 0;         // byte to hold status information.
  int pressure_count_ = 0;     // hold raw pressure data (24 - bit)
  int temperature_count_ = 0;  // hold raw temperature data (16 - bit)
  sensor::Sensor *pressure_sensor_;
  sensor::Sensor *temperature_sensor_;
  uint8_t readsensor_();
  uint8_t readstatus_();
  uint8_t readmeasurement_();
  void publishmeasurement_();
  int rawpressure_();
  int rawtemperature_();
  float countstopressure_(int counts, float pressure_range, float pressure_type);
  float countstotemperatures_(int counts);
  float read_pressure_();
  float read_temperature_();
};

}  // namespace allsensorsdlhr
}  // namespace esphome
