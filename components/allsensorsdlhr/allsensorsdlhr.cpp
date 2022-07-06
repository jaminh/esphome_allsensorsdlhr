#include "allsensorsdlhr.h"
#include "esphome/core/log.h"

namespace esphome {
namespace allsensorsdlhr {

static const char *const TAG = "allsensorsdlhr";

const uint8_t SUCCESS_STATUS = 0x40;

const uint8_t CMD_STATUS = 0xF0;
const uint8_t CMD_READ = 0x00;

const uint8_t CMD_SINGLE_SAMPLE = 0xAA;
const uint8_t CMD_AVG2_SAMPLES = 0xAC;
const uint8_t CMD_AVG4_SAMPLES = 0xAD;
const uint8_t CMD_AVG8_SAMPLES = 0xAE;
const uint8_t CMD_AVG16_SAMPLES = 0xAF;

void ALLSENSORSDLHRSensor::setup() {
  ESP_LOGD(TAG, "Setting up All Sensors DLHR Sensor ");
  this->spi_setup();
}

uint8_t ALLSENSORSDLHRSensor::readsensor_() {
  // Send command to measure data.
  this->enable();
  cmd_buf_[0] = this->transfer_byte(CMD_AVG8_SAMPLES);
  cmd_buf_[1] = this->transfer_byte(CMD_READ);
  cmd_buf_[2] = this->transfer_byte(CMD_READ);
  this->disable();
  ESP_LOGV(TAG, "Command status %d", cmd_buf_[0]);
  if (SUCCESS_STATUS != cmd_buf_[0]) {
    ESP_LOGD(TAG, "Error while getting measurement %d %d %d", cmd_buf_[0], cmd_buf_[1], cmd_buf_[2]);
	return cmd_buf_[0];
  }

  retry_count_ = 0;
  return cmd_buf_[0];
}

// returns status
uint8_t ALLSENSORSDLHRSensor::readstatus_() { return status_; }

uint8_t ALLSENSORSDLHRSensor::readmeasurement_() {
  // Read new data.
  // transfer 7 bytes (the last two are temperature only used by some sensors)
  this->enable();
  data_buf_[0] = this->transfer_byte(CMD_STATUS);
  data_buf_[1] = this->transfer_byte(CMD_READ);
  data_buf_[2] = this->transfer_byte(CMD_READ);
  data_buf_[3] = this->transfer_byte(CMD_READ);
  data_buf_[4] = this->transfer_byte(CMD_READ);
  data_buf_[5] = this->transfer_byte(CMD_READ);
  data_buf_[6] = this->transfer_byte(CMD_READ);
  this->disable();
  
  // Check the status codes:
  // status = 0 : normal operation
  // status = 1 : device in command mode
  // status = 2 : stale data
  // status = 3 : diagnostic condition
  status_ = data_buf_[0];
  ESP_LOGV(TAG, "Read sensor status %d", status_);

  // if device is normal and there is new data, bitmask and save the raw data
  if (status_ == SUCCESS_STATUS) {
    // 24 - bit pressure
    pressure_count_ = ((uint32_t)(data_buf_[1]) << 16) | ((uint32_t)(data_buf_[2]) << 8) | ((uint32_t)(data_buf_[3]));
    // 16 - bit temperature (24 - bit output, but only upper 16 bits are significant)
    temperature_count_ = ((uint16_t)(data_buf_[4]) << 8) | ((uint16_t)(data_buf_[5]));
    ESP_LOGD(TAG, "Sensor pressure_count_ %d", pressure_count_);
    ESP_LOGD(TAG, "Sensor temperature_count_ %d", temperature_count_);
  }
  return status_;
}

void ALLSENSORSDLHRSensor::publishmeasurement_() {
  read_status_ = readmeasurement_();
  if (read_status_ == SUCCESS_STATUS) {
    if (this->pressure_sensor_ != nullptr)
      this->pressure_sensor_->publish_state(read_pressure_() * 1.0);
    if (this->temperature_sensor_ != nullptr)
      this->temperature_sensor_->publish_state(read_temperature_() * 1.0);
  }
  else if (retry_count_ < 5) {
	ESP_LOGV(TAG, "Retry publish measurements, got status %d", read_status_);
	retry_count_++;
	this->set_timeout("publish_measurements", 10, [this]() { this->publishmeasurement_(); });
  }
  else {
	ESP_LOGD(TAG, " Failed to read measurements after %d retries with status %d", retry_count_, read_status_);
  }
}

// The pressure value from the most recent reading in raw counts
int ALLSENSORSDLHRSensor::rawpressure_() { return pressure_count_; }

// The temperature value from the most recent reading in raw counts
int ALLSENSORSDLHRSensor::rawtemperature_() { return temperature_count_; }

// Converts a digital pressure measurement in counts to pressure measured
float ALLSENSORSDLHRSensor::countstopressure_(const int counts, const float pressure_range, const float pressure_type) {
  return 1.25 * (((float) counts - (0.5 * (float) 0x1000000)) / (float) 0x1000000) * (pressure_type * pressure_range);
}

// Converts a digital temperature measurement in counts to temperature in C
// This will be invalid if sensore daoes not have temperature measurement capability
float ALLSENSORSDLHRSensor::countstotemperatures_(const int counts) { return (((float) counts / 65535.0) * 125.0) - 40.0; }

// Pressure value from the most recent reading in units
float ALLSENSORSDLHRSensor::read_pressure_() {
  return countstopressure_(pressure_count_, allsensorsdlhr_pressure_range_, allsensorsdlhr_pressure_type_);
}

// Temperature value from the most recent reading in degrees C
float ALLSENSORSDLHRSensor::read_temperature_() { return countstotemperatures_(temperature_count_); }

void ALLSENSORSDLHRSensor::update() {
  ESP_LOGV(TAG, "Update All Sensors DLHR Sensor");
  if (readsensor_() == SUCCESS_STATUS) {
	ESP_LOGV(TAG, "Start Publish measurements");
	this->set_timeout("publish_measurements", 10, [this]() { this->publishmeasurement_(); });
  }
}

float ALLSENSORSDLHRSensor::get_setup_priority() const { return setup_priority::LATE; }

void ALLSENSORSDLHRSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "ALLSENSORSDLHR:");
  LOG_PIN("  CS Pin: ", this->cs_);
  ESP_LOGCONFIG(TAG, "  Pressure Range: %0.1f", allsensorsdlhr_pressure_range_);
  ESP_LOGCONFIG(TAG, "  Pressure Type: %0.1f", allsensorsdlhr_pressure_type_);
  LOG_UPDATE_INTERVAL(this);
}

void ALLSENSORSDLHRSensor::set_allsensorsdlhr_pressure_range(float pressure_range) {
  this->allsensorsdlhr_pressure_range_ = pressure_range;
}

void ALLSENSORSDLHRSensor::set_allsensorsdlhr_pressure_type(float pressure_type) {
  this->allsensorsdlhr_pressure_type_ = pressure_type;
}

}  // namespace allsensorsdlhr
}  // namespace esphome
