#pragma once

#define DEVICE_VERSION      1
#define DEVICE_NAME         "Gateway"
#define DEVICE_ID_AMBIENTE  1

#define ETH_SUBRED          200
#define ETH_ID              100

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)


#define AP_NAME             DEVICE_NAME "_" STR(DEVICE_ID_AMBIENTE)
#define AP_PASS             ""


#define USER_PASS_DEFAULT_SHA256    "03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4" // 1234
#define USER_PASS_ROOT_SHA256       "9a9af8f5cadbeb70fea34dc8e5953c7b7bda43128682277522f176c320034acd" // Friopacking