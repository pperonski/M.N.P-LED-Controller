#pragma once

#ifdef DEBUG

// print debug message
#define LOGD(msg, ...)                 \
    Serial.print("D: ");               \
    Serial.printf(msg, ##__VA_ARGS__); \
    Serial.print(" : ");               \
    Serial.print(__LINE__);            \
    Serial.print(" in ");              \
    Serial.println(__FILE__);

#else

#define LOGD(msg, ...)

#endif

// print error message
#define LOGE(msg, ...)                 \
    Serial.print("E: ");               \
    Serial.printf(msg, ##__VA_ARGS__); \
    Serial.print(" : ");               \
    Serial.print(__LINE__);            \
    Serial.print(" in ");              \
    Serial.println(__FILE__);

// print warrning message
#define LOGW(msg, ...)                 \
    Serial.print("W: ");               \
    Serial.printf(msg, ##__VA_ARGS__); \
    Serial.print(" : ");               \
    Serial.print(__LINE__);            \
    Serial.print(" in ");              \
    Serial.println(__FILE__);

// print info message
#define LOGI(msg, ...)                 \
    Serial.print("I: ");               \
    Serial.printf(msg, ##__VA_ARGS__); \
    Serial.println();