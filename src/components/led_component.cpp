#include "led_component.h"

LedComponent::LedComponent(const gpio_dt_spec *spec) : _led(spec) {}

const int8_t LedComponent::begin()
{
    if (!gpio_is_ready_dt(_led))
        return -1;

    return gpio_pin_configure_dt(_led, GPIO_OUTPUT_ACTIVE);
}

const int8_t LedComponent::set(bool v)
{
    return gpio_pin_set_dt(_led, v);
}