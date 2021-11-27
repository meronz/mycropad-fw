#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

#include "keymap.h"

enum MessageTypes
{
  Heartbeat = 0,
  NewKeymap = 1,
  ReadKeymap = 2,
  DefaultKeymap = 3,
};

#define CDC_BUFFER_SIZE 4096
static uint8_t _buffer[CDC_BUFFER_SIZE];
uint16_t _offset = 0;
bool stx_received = false;

uint8_t *prepare_buffer_for_response(uint8_t cmd, bool ok, size_t responseSize)
{
  _buffer[0] = 0x2; // STX
  _buffer[1] = cmd;
  _buffer[2] = ok ? 0 : 0xFF;
  _buffer[3] = (responseSize)&0xFF;
  _buffer[4] = (responseSize >> 8) & 0xFF;
  _buffer[5] = (responseSize >> 16) & 0xFF;
  _buffer[6] = (responseSize >> 24) & 0xFF;

  _buffer[7 + responseSize] = 0x3; //ETX

  return _buffer + 7;
}

void parse_data()
{
  bool ok = false;
  uint8_t cmd = _buffer[1];
  size_t responseSize = 0;

  printf("parse_data: %x\n", cmd);
  switch (cmd)
  {
  case Heartbeat:
  {
    prepare_buffer_for_response(cmd, true, 0);
    break;
  }
  case ReadKeymap:
  {
    responseSize = CDC_BUFFER_SIZE;
    memset(_buffer, 0xCF, CDC_BUFFER_SIZE);
    uint8_t *responseBuf = prepare_buffer_for_response(cmd, false, 0);
    ok = Keymap::Instance()->ReadKeymap(responseBuf, &responseSize);
    prepare_buffer_for_response(cmd, ok, responseSize);
    break;
  }
  case NewKeymap:
  {
    ok = Keymap::Instance()->SetKeymap(_buffer + 2);
    prepare_buffer_for_response(cmd, ok, 0);
    break;
  }
  case DefaultKeymap:
  {
    ok = true;
    Keymap::Instance()->LoadDefault();
    prepare_buffer_for_response(cmd, ok, 0);
    break;
  }
  default:
    break;
  }

  responseSize += 8;
  
  // send data
  printf("parse_data: responding with %u bytes\n", responseSize);
  size_t sendOffset = 0;
  size_t toSend;
  do
  {
    size_t remaining = responseSize - sendOffset;
    toSend = 64 < remaining ? 64 : remaining;
    if (toSend == 0)
    {
      break;
    }
    tud_cdc_write(_buffer + sendOffset, toSend);
    tud_cdc_write_flush();
    board_delay(5);
    sendOffset += toSend;
  } while (true);
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
          parse_data();
          _offset = 0;
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