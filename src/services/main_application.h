#if !defined(APPLICATION_H)
#define APPLICATION_H

#include "../components/led_component.h"

class MainApplication
{
public:
    static void blinky()
    {
        static const struct gpio_dt_spec _spec = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
        LedComponent _led(&_spec);

        _led.begin();
        while (1)
        {
            static bool _state = false;
            _led.set(_state);
            _state = !_state;

            k_msleep(500);
        }
    }
};

#endif // APPLICATION_H
