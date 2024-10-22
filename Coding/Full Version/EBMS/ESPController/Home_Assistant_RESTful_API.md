# EBMS v4
## Home Assistant RESTful API integration



### Secrets Configuration YAML file for Home Assistant
```
# Use this file to store secrets like usernames and passwords.
# Learn more at https://www.home-assistant.io/docs/configuration/secrets/

Ebms_api_token: XXXXXXXXXXXXXXXXXXXXXXXX
```

### Example Configuration YAML file for Home Assistant
```
# Example configuration.yaml entry for Home Assistant integration with EBMS

rest:
  - resource: http://192.168.0.70/ha
    scan_interval: 10
    timeout: 5
    method: "GET"
    headers:
      Content-Type: application/json
      ApiKey: !secret Ebms_api_token
    sensor:
      - unique_id: "Ebms.activerules"
        value_template: "{{ value_json.activerules }}"
        name: "EBMS Active rules"
        state_class: "measurement"

      - unique_id: "Ebms.chargemode"
        value_template: "{{ value_json.chgmode }}"
        name: "EBMS Charge mode"
        state_class: "measurement"

      - unique_id: "Ebms.lowest_bank_voltage"
        value_template: "{{ value_json.lowbankv }}"
        name: "EBMS Lowest bank voltage"
        unit_of_measurement: "mV"
        device_class: "voltage"

      - unique_id: "Ebms.highest_bank_voltage"
        value_template: "{{ value_json.highbankv }}"
        name: "EBMS Highest bank voltage"
        unit_of_measurement: "mV"
        device_class: "voltage"

      - unique_id: "Ebms.lowest_cell_voltage"
        value_template: "{{ value_json.lowcellv }}"
        name: "EBMS Lowest cell voltage"
        unit_of_measurement: "mV"
        device_class: "voltage"

      - unique_id: "Ebms.highest_cell_voltage"
        value_template: "{{ value_json.highcellv }}"
        name: "EBMS Highest cell voltage"
        unit_of_measurement: "mV"
        device_class: "voltage"

      - unique_id: "Ebms.highest_external_temp"
        value_template: "{{ value_json.highextt }}"
        name: "EBMS Highest cell temperature"
        unit_of_measurement: "°C"
        device_class: "temperature"

      - unique_id: "Ebms.highest_internal_temp"
        value_template: "{{ value_json.highintt }}"
        name: "EBMS Highest passive balance temperature"
        unit_of_measurement: "°C"
        device_class: "temperature"

      - unique_id: "Ebms.current"
        value_template: "{% if 'c' in value_json %}{{ value_json.c }}{% else %}0{% endif %}"
        name: "EBMS DC Current"
        unit_of_measurement: "A"
        device_class: "current"
        icon: "mdi:current-dc"

      - unique_id: "Ebms.voltage"
        value_template: "{% if 'v' in value_json %}{{ value_json.v }}{% else %}0{% endif %}"
        name: "EBMS DC voltage"
        unit_of_measurement: "V"
        device_class: "voltage"

      - unique_id: "Ebms.power"
        value_template: "{% if 'pwr' in value_json %}{{ value_json.pwr }}{% else %}0{% endif %}"
        name: "EBMS Battery power"
        unit_of_measurement: "W"
        device_class: "power"

      - unique_id: "Ebms.stateofcharge"
        value_template: "{% if 'soc' in value_json %}{{ value_json.soc }}{% else %}0{% endif %}"
        name: "EBMS State of charge"
        unit_of_measurement: "%"
        device_class: "battery"

      - unique_id: "Ebms.dynamic_charge_voltage"
        value_template: "{% if 'dyncv' in value_json %}{{ value_json.dyncv }}{% else %}0{% endif %}"
        name: "EBMS Dynamic charge voltage"
        unit_of_measurement: "V"
        device_class: "voltage"

      - unique_id: "Ebms.dynamic_charge_current"
        value_template: "{% if 'dyncc' in value_json %}{{ value_json.dyncc }}{% else %}0{% endif %}"
        name: "EBMS Dynamic charge current"
        unit_of_measurement: "A"
        device_class: "current"

    binary_sensor:
      - unique_id: "Ebms.charge_allowed"
        value_template: "{{ value_json.chgallow }}"
        name: "EBMS Battery charging allowed"

      - unique_id: "Ebms.discharge_allowed"
        value_template: "{{ value_json.dischgallow }}"
        name: "EBMS Battery discharging allowed"
```