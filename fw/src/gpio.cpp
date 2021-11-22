#include "gpio.h"
#include "pico/time.h"
#include "pico/stdlib.h"

static struct repeating_timer _timer;
static bool _timer_cb(struct repeating_timer *t);

bool _timer_cb(struct repeating_timer *t)
{
    Gpio::Instance()->Tick();
    return true;
}

void _gpio_callback(uint gpio, uint32_t events)
{
    Gpio::Instance()->RotaryEncoderTick();
}

void Gpio::Init()
{
    _key_events = 0;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    for (uint i = 0; i < sizeof(_gpios); i++)
    {
        gpio_init(_gpios[i]);
        gpio_set_dir(_gpios[i], GPIO_IN);
        gpio_set_pulls(_gpios[i], true, false);
    }

    gpio_set_irq_enabled_with_callback(GPIO_ROT_CLK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &_gpio_callback);
    gpio_set_irq_enabled_with_callback(GPIO_ROT_DATA, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &_gpio_callback);

    _encoder = new RotaryEncoder(GPIO_ROT_CLK, GPIO_ROT_DATA, RotaryEncoder::LatchMode::FOUR3);
    add_repeating_timer_ms(-CHECK_MSEC, _timer_cb, NULL, &_timer);
}


Keymap::Keys Gpio::GpioToKey(uint gpioNum)
{
    switch (gpioNum)
    {
    case GPIO_SW_1:
        return Keymap::Keys::Key1;
    case GPIO_SW_2:
        return Keymap::Keys::Key2;
    case GPIO_SW_3:
        return Keymap::Keys::Key3;
    case GPIO_SW_4:
        return Keymap::Keys::Key4;
    case GPIO_SW_5:
        return Keymap::Keys::Key5;
    case GPIO_SW_6:
        return Keymap::Keys::Key6;
    case GPIO_SW_7:
        return Keymap::Keys::Key7;
    case GPIO_SW_8:
        return Keymap::Keys::Key8;
    case GPIO_ROT_SW:
        return Keymap::Keys::RotClick;
    default:
        return Keymap::Keys::None;
    }
}

void Gpio::Tick()
{
    static uint8_t debounceCounts[9] = {0};
    uint32_t oldState = _key_events;
    uint32_t curState = ~gpio_get_all();

    // Button press events
    for (size_t i = 0; i < 9; i++)
    {
        bool isSet = curState & (1 << _gpios[i]);
        bool oldValue = oldState & (1 << _gpios[i]);
        if (isSet == oldValue)
        {
            // Set the timer which will allow a change from the current state.
            debounceCounts[i] = isSet
                ? (RELEASE_MSEC / CHECK_MSEC)
                : (PRESS_MSEC / CHECK_MSEC);
        }
        else
        {
            // Key has changed – wait for new state to become stable.
            debounceCounts[i] -= 1;
            if (debounceCounts[i] == 0)
            {
                // Timer expired – accept the change.
                _key_events |= isSet << GpioToKey(_gpios[i]);

                // And reset the timer.
                debounceCounts[i] = isSet
                    ? (RELEASE_MSEC / CHECK_MSEC)
                    : (PRESS_MSEC / CHECK_MSEC);
            }
        }
    }
}

void Gpio::RotaryEncoderTick()
{
    uint32_t curState = ~gpio_get_all();
    int clk = !(curState & (1 << GPIO_ROT_CLK));
    int data = !(curState & (1 << GPIO_ROT_DATA));
    _encoder->tick(clk, data);
    RotaryEncoder::Direction dir = _encoder->getDirection();
    if (dir == RotaryEncoder::Direction::CLOCKWISE)
    {
        _key_events |= 1 << Keymap::Keys::RotCW;
    }
    else if (dir == RotaryEncoder::Direction::COUNTERCLOCKWISE)
    {
        _key_events |= 1 << Keymap::Keys::RotCCW;
    }
}


Keymap::Keys Gpio::GetKeyEvent()
{
    for (int i = 0; i <= GpioNum; i++)
    {
        // We have an event
        if ((_key_events >> i) & 1)
        {
            // Lets clear it up
            _key_events &= ~(1UL << i);
            return (Keymap::Keys)i; // todo: what if we have multiple keys pressed?
        }
    }
    return Keymap::Keys::None;
}