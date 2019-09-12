/*
MIT License

Copyright (c) 2019 Pavel Slama

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef UARTREADER_H
#define UARTREADER_H

#include "mbed.h"

class UARTReader {
  public:
    UARTReader(FileHandle *fh, EventQueue &queue);
    virtual ~UARTReader();
    void attach(Callback<void()> function);
    void init();
    ssize_t read_bytes(uint8_t *buf, size_t len);
    void set_file_handle(FileHandle *fh);
    void set_timeout(uint32_t timeout);

  protected:
    FileHandle *_fileHandle;

  private:
    EventQueue &_queue;
    Callback<void()> _callback;
    char     _recv_buff[MBED_CONF_UARTREADER_BUFF_SIZE];
    int      _event_id;
    uint32_t _timeout;
    uint64_t _start_time;
    size_t   _recv_len;

    bool fill_buffer();
    int  poll_timeout();
    void process();
    void reset_buffer();
    void rx_irq();
};

#endif  // UARTREADER_H