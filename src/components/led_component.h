#if !defined(LED_COMPONENT_H)
#define LED_COMPONENT_H

#include <zephyr/drivers/gpio.h>

class LedComponent
{
private:
    const gpio_dt_spec *_led;

public:
    LedComponent(const gpio_dt_spec *spec);

    const int8_t begin();
    const int8_t set(bool state);
};

#endif // LED_COMPONENT_H
