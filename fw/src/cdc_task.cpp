#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

#include "keymap.h"

enum MessageTypes
{
  NewKeymap = 1
};

#define CDC_BUFFER_SIZE 4096
static uint8_t _buffer[CDC_BUFFER_SIZE];
uint16_t _offset = 0;
bool stx_received = false;

bool parse_data(void)
{
  uint8_t cmd = _buffer[1];
  switch (cmd)
  {
  case NewKeymap:
  {
    return Keymap::Instance()->SetKeymap(_buffer+2);
  }
  default:
    break;
  }

  return false;
}

void cdc_task(void)
{
  // connected() check for DTR bit
  // Most but not all terminal client set this when making connection
  // if ( tud_cdc_connected() )
  {
    // connected and there are data available
    if (tud_cdc_available())
    {
      // read datas
      uint8_t *p = _buffer + _offset;
      size_t bytesFree = CDC_BUFFER_SIZE - _offset;
      uint32_t count = tud_cdc_read(p, sizeof(bytesFree));
      _offset += count;

      if (!stx_received)
      {
        if (_buffer[0] == 0x02)
        {
          stx_received = true;
        }
        else
        {
          _offset = 0; // receiving garbage
          return;
        }
      }

      if (stx_received)
      {
        // etx received
        if (_buffer[_offset - 1] == 0x03)
        {
          bool ok = parse_data();
          _offset = 0;

          char response[1];
          response[0] = ok;
          tud_cdc_write(response, sizeof(response));
          tud_cdc_write_flush();
        }
      }
    }
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void)itf;
  (void)rts;

  // TODO set some indicator
  if (dtr)
  {
    // Terminal connected
  }
  else
  {
    // Terminal disconnected
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void)itf;
}