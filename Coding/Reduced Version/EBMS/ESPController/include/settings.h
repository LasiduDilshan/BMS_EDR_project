#ifndef EBMS_SETTINGS_H_
#define EBMS_SETTINGS_H_

#include "defines.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "Rules.h"

#include "ArduinoJson.h"
bool ValidateGetSetting(esp_err_t err, const char *key);

bool getString(nvs_handle_t handle, const char *key, char *out_value, size_t size);
bool getSetting(nvs_handle_t handle, const char *key, float *out_value);
bool getSetting(nvs_handle_t handle, const char *key, uint8_t *out_value);
bool getSetting(nvs_handle_t handle, const char *key, int8_t *out_value);
bool getSetting(nvs_handle_t handle, const char *key, uint16_t *out_value);
// bool getSetting(nvs_handle_t handle, const char *key, uint32_t *out_value);
bool getSetting(nvs_handle_t handle, const char *key, int32_t *out_value);
bool getSetting(nvs_handle_t handle, const char *key, int16_t *out_value);
bool getSetting(nvs_handle_t handle, const char *key, bool *out_value);

bool getSettingBlob(nvs_handle_t handle, const char *key, void *out_value, size_t size);

void InitializeNVS();
void SaveConfiguration(Ebms_eeprom_settings *settings);
void LoadConfiguration(Ebms_eeprom_settings *settings);
void ValidateConfiguration(Ebms_eeprom_settings *settings);
void DefaultConfiguration(Ebms_eeprom_settings *settings);

void SaveWIFI(const wifi_eeprom_settings *wifi);
bool LoadWIFI(wifi_eeprom_settings *wifi);

void GenerateSettingsJSONDocument(DynamicJsonDocument *doc, Ebms_eeprom_settings *settings);
void JSONToSettings(DynamicJsonDocument &doc, Ebms_eeprom_settings *settings);

void writeSetting(nvs_handle_t handle, const char *key, bool value);
void writeSetting(nvs_handle_t handle, const char *key, uint8_t value);
void writeSetting(nvs_handle_t handle, const char *key, uint16_t value);
void writeSetting(nvs_handle_t handle, const char *key, int16_t value);
void writeSetting(nvs_handle_t handle, const char *key, int8_t value);
void writeSetting(nvs_handle_t handle, const char *key, const char *value);
void writeSettingBlob(nvs_handle_t handle, const char *key, const void *value, size_t length);

#endif