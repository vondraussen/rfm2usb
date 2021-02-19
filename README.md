# RFM2USB Firmware [![CI](https://github.com/vondraussen/rfm2usb/actions/workflows/main.yml/badge.svg?branch=master)](https://github.com/vondraussen/rfm2usb/actions/workflows/main.yml)
> this is work in progress!

Arduino based firmware for [RFM2USB](https://github.com/vondraussen/rfm2usb_hw) hardware which allows you to send and receive messages via RFM69 (sub gigahertz transceiver).

Messages are protobuf encoded and printout via USB CDC as json string.
## Build
Build it with [VSCode & PlatformIO](https://docs.platformio.org/en/latest/integration/ide/vscode.html#installation)

## Protobuf Generation
Use the VS-Code tasks to get the protoc compiler (nano-pb) and build the protobuf files. This is only needed after you cloned or changed the `messages.proto` file.

`F1 + Tasks: Run Task -> install nano pb`

`F1 + Tasks: Run Task -> generate protobuf`
