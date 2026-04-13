#pragma once

#define DEVICE_VERSION      1
#define DEVICE_NAME         "Gateway"
#define DEVICE_ID_AMBIENTE  1

#define ETH_SUBRED          200
#define ETH_ID              100

#define STR_HELPER2(x) #x
#define STR2(x) STR_HELPER2(x)

#define AP_NAME DEVICE_NAME "_" STR2(DEVICE_ID_AMBIENTE)
#define AP_PASS ""

#define USER_PASS_DEFAULT_SHA256    "03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4" // 1234
#define USER_PASS_ROOT_SHA256       "9a9af8f5cadbeb70fea34dc8e5953c7b7bda43128682277522f176c320034acd" // Friopacking

#define ETH_MOSI            11
#define ETH_MISO            12
#define ETH_SCK             13
#define ETH_CS              14
#define ETH_RST             9
#define ETH_INT             10
#define ETH_SPI_CLOCK_MHZ   25

#define ADV_DATA_QUEUE_LEN  64
#define MAX_SLOTS           32