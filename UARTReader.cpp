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

#include "mbed.h"
#include "UARTReader.h"

UARTReader::UARTReader(FileHandle *fh, EventQueue *queue):
    _fileHandle(NULL),
    _queue(queue),
    _event_id(0),
    _timeout(MBED_CONF_UARTREADER_DEFAULT_TIMEOUT),
    _start_time(0),
    _recv_len(0) {
    set_file_handle(fh);
    reset_buffer();
}

UARTReader::~UARTReader() {
    _fileHandle->sigio(NULL);

    if (_callback) {
        _callback = NULL;
    }

    set_file_handle(NULL);

    if (_event_id != 0) {
        _queue->cancel(_event_id);
        _event_id = 0;
    }
}

void UARTReader::attach(Callback<void()> function) {
    _callback = function;
}

bool UARTReader::fill_buffer() {
    pollfh fhs;
    fhs.fh = _fileHandle;
    fhs.events = POLLIN;
    int count = poll(&fhs, 1, poll_timeout());

    if (count > 0 && (fhs.revents & POLLIN)) {
        ssize_t len = _fileHandle->read(_recv_buff + _recv_len, sizeof(_recv_buff) - _recv_len);

        if (len > 0) {
            _recv_len += len;
            return true;
        }
    }

    return false;
}

void UARTReader::init() {
    reset_buffer();

    _fileHandle->set_blocking(false);
    _fileHandle->sigio(callback(this, &UARTReader::rx_irq));
}

int UARTReader::poll_timeout() {
    int timeout;
    uint64_t now = Kernel::get_ms_count();

    if (now >= _start_time + _timeout) {
        timeout = 0;

    } else if (_start_time + _timeout - now > INT_MAX) {
        timeout = INT_MAX;

    } else {
        timeout = _start_time + _timeout - now;
    }

    return timeout;
}

void UARTReader::process() {
    if (_fileHandle->readable()) {
        while (true) {
            if (!fill_buffer()) {
                if (_callback) {
                    _callback.call();
                }

                reset_buffer();

                break;
            }

            _start_time = Kernel::get_ms_count();
        }
    }

    _event_id = 0;
}

ssize_t UARTReader::read_bytes(char *buf, size_t len) {
    if (_recv_len > 0) {
        memcpy(buf, _recv_buff, (len > _recv_len ? _recv_len : len));
        return (len > _recv_len ? _recv_len : len);
    }

    return 0;
}

void UARTReader::reset_buffer() {
    _recv_len = 0;
    memset(_recv_buff, 0, sizeof(_recv_buff));
}

void UARTReader::rx_irq() {  // ISR
    if (_event_id == 0) {
        _event_id = _queue->call(callback(this, &UARTReader::process));
    }
}

void UARTReader::set_file_handle(FileHandle *fh) {
    _fileHandle = fh;
}

void UARTReader::set_timeout(uint32_t timeout) {
    _timeout = timeout;
}
