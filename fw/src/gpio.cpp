#include "gpio.h"

Gpio::Gpio()
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

    _encoder = new RotaryEncoder(GPIO_ROT_CLK, GPIO_ROT_DATA, RotaryEncoder::LatchMode::TWO03);
}

void Gpio::Tick()
{
  //TODO: Debounce
  uint32_t gpio_state = gpio_get_all();

  _key_events |= !(gpio_state & (1 << GPIO_SW_1)) << Keys::Key1;
  _key_events |= !(gpio_state & (1 << GPIO_SW_2)) << Keys::Key2;
  _key_events |= !(gpio_state & (1 << GPIO_SW_3)) << Keys::Key3;
  _key_events |= !(gpio_state & (1 << GPIO_SW_4)) << Keys::Key4;
  _key_events |= !(gpio_state & (1 << GPIO_SW_5)) << Keys::Key5;
  _key_events |= !(gpio_state & (1 << GPIO_SW_6)) << Keys::Key6;
  _key_events |= !(gpio_state & (1 << GPIO_SW_7)) << Keys::Key7;
  _key_events |= !(gpio_state & (1 << GPIO_SW_8)) << Keys::Key8;
  _key_events |= !(gpio_state & (1 << GPIO_ROT_SW)) << Keys::RotClick;

  int clk = !(gpio_state & (1 << GPIO_ROT_CLK));
  int data = !(gpio_state & (1 << GPIO_ROT_DATA));
  _encoder->tick(clk, data);
  RotaryEncoder::Direction dir = _encoder->getDirection();
  if (dir == RotaryEncoder::Direction::CLOCKWISE)
  {
      _key_events |= 1 << Keys::RotCW;
  }
  else if (dir == RotaryEncoder::Direction::COUNTERCLOCKWISE)
  {
      _key_events |= 1 << Keys::RotCCW;
  }
}

uint32_t Gpio::GetKeyEvent()
{
    const uint8_t eventsNum = 8 * sizeof(_key_events);
    for (int i = 0; i < eventsNum; i++)
    {
        // We have an event
        if ((_key_events >> i) & 1)
        {
            // Lets clear it up
            _key_events &= ~(1UL << i);
            return i;
        }
    }
    return 0;
}