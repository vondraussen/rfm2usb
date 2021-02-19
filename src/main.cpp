#include <Arduino.h>
#include <ArduinoJson.h>
#include <RFM69.h>
#include <SPI.h>
#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <stdio.h>
#include "messages.pb.h"

bool decodeStateMsgSensorData(pb_istream_t* stream,
                              const pb_field_t* field,
                              void** arg);

bool encodeStateMsgSensorData(pb_ostream_t* stream,
                              const pb_field_t* field,
                              void* const* arg);

#define NODEID 1
#define NETWORKID 100
#define GATEWAYID 1
#define SENSORS_COUNT 1

#define FREQUENCY RF69_868MHZ
#define ENCRYPTKEY \
  "sampleEncryptKey"  // has to be same 16 characters/bytes on all nodes, not
                      // more not less!

#define RFM69_IRQ PC_7
#define RFM69_NSS PB_6
#define RFM69_IS_RFMHW false

// SPIClass spi();
RFM69 radio(RFM69_NSS, RFM69_IRQ, RFM69_IS_RFMHW);

void setup() {
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  SerialUSB.begin();
}

#if (NODEID == GATEWAYID)
void loop() {
  State state_msg = State_init_zero;
  state_msg.has_node = true;
  state_msg.node = Node_init_zero;
  Sensor sensorData[1] = {0};
  state_msg.sensor.funcs.decode = &decodeStateMsgSensorData;
  state_msg.sensor.arg = &sensorData;

  // receive messages
  if (radio.receiveDone()) {
    if (radio.ACKRequested())
      radio.sendACK();
    // deserialize protobuf
    pb_istream_t stream = pb_istream_from_buffer(radio.DATA, radio.DATALEN);
    pb_decode_ex(&stream, &State_msg, &state_msg, PB_ENCODE_DELIMITED);

    // Json Stuff
    StaticJsonDocument<200> doc;
    doc["id"] = state_msg.node.id;
    doc["battery"] = state_msg.node.battery_mv;
    doc["humidity"] = sensorData[0].humidity;
    doc["temperature"] = sensorData[0].temperature;
    doc["rssi"] = radio.readRSSI();
    doc["radio_temp"] = radio.readTemperature();
    serializeJson(doc, SerialUSB);
    SerialUSB.write("\n", 1);
  }
}
#else
void loop() {
  uint8_t buffer[64];
  size_t message_length;
  bool status;

  State state_msg = State_init_zero;
  state_msg.has_node = true;
  state_msg.node = Node_init_zero;

  Sensor sensorData[SENSORS_COUNT] = {0};

  state_msg.sensor.funcs.encode = &encodeStateMsgSensorData;
  state_msg.sensor.arg = &sensorData;

  // Create a stream that will write to our buffer.
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  state_msg.node.battery_mv = 0;  // adc_read_vcc();
  sensorData[0].humidity = 0;
  sensorData[0].temperature = 0;
  // sensorData[0].humidity = si7020_measure_humi();
  // sensorData[0].temperature = si7020_get_prev_temp();
  state_msg.node.id = NODEID;

  // Now we are ready to encode the message!
  status = pb_encode_ex(&stream, &State_msg, &state_msg, PB_ENCODE_DELIMITED);
  message_length = stream.bytes_written;

  // Then just check for any errors..
  if (!status) {
    // printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
  }

  radio.send(GATEWAYID, (const void*)(buffer), message_length, false);
  radio.sleep();

  // hemnode_sleep_s(POWER_DOWN_S);
}
#endif

bool decodeStateMsgSensorData(pb_istream_t* stream,
                              const pb_field_t* field,
                              void** arg) {
  Sensor* sensors = (Sensor*)*arg;

  for (uint8_t i = 0; i < 1; i++) {
    pb_decode(stream, Sensor_fields, sensors);
    return false;
  }
  return true;
}

bool encodeStateMsgSensorData(pb_ostream_t* stream,
                              const pb_field_t* field,
                              void* const* arg) {
  Sensor* sensors = (Sensor*)*arg;

  for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
    if (!pb_encode_tag_for_field(stream, field))
      return false;

    Sensor sensorData = Sensor_init_zero;
    sensorData.id = (sensors + i)->id;
    sensorData.temperature = (sensors + i)->temperature;
    sensorData.humidity = (sensors + i)->humidity;
    sensorData.light = (sensors + i)->light;

    if (!pb_encode_submessage(stream, &Sensor_msg, &sensorData))
      return false;
  }
  return true;
}