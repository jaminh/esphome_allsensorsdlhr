import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.components import spi
from esphome.const import (
    CONF_ID,
    CONF_PRESSURE,
    CONF_TEMPERATURE,
    DEVICE_CLASS_PRESSURE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

DEPENDENCIES = ["spi"]
CODEOWNERS = ["@jaminh"]

CONF_PRESSURE_RANGE = "pressure_range"
CONF_PRESSURE_TYPE = "pressure_type"

allsensorsdlhr_ns = cg.esphome_ns.namespace("allsensorsdlhr")
ALLSENSORSDLHRSensor = allsensorsdlhr_ns.class_(
    "ALLSENSORSDLHRSensor", sensor.Sensor, cg.PollingComponent, spi.SPIDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ALLSENSORSDLHRSensor),
            cv.Optional(CONF_PRESSURE): sensor.sensor_schema(
                unit_of_measurement="inH2O",
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_PRESSURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ).extend(
                {
                    cv.Required(CONF_PRESSURE_RANGE): cv.float_,
                    cv.Required(CONF_PRESSURE_TYPE): cv.float_,
                }
            ),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("20s"))
    .extend(spi.spi_device_schema(cs_pin_required=True))
)


async def to_code(config):

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await spi.register_spi_device(var, config)

    if CONF_PRESSURE in config:
        conf = config[CONF_PRESSURE]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_pressure_sensor(sens))
        cg.add(var.set_allsensorsdlhr_pressure_range(conf[CONF_PRESSURE_RANGE]))
        cg.add(var.set_allsensorsdlhr_pressure_type(conf[CONF_PRESSURE_TYPE]))

    if CONF_TEMPERATURE in config:
        conf = config[CONF_TEMPERATURE]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_temperature_sensor(sens))
