# UART-reader

```cpp
#include "mbed.h"
#include "UARTReader.h"

EventQueue eQueue(32 * EVENTS_EVENT_SIZE);
UARTSerial serial(NC, PA_10);
UARTReader reader(&serial, eQueue);

void serial_data() {
    uint8_t data[100] = {0};
    ssize_t len = reader.read_bytes(data, sizeof(data));
    printf("Got data[%u]: %s\n", len, data);
}

int main() {
    Thread eQueueThread;
    eQueueThread.start(callback(&eQueue, &EventQueue::dispatch_forever));

    serial.set_baud(115200);                    // optional
    serial.set_format(8, SerialBase::None, 1);  // optional

    reader.attach(serial_data);
    reader.init();

    while (1) {
        ThisThread::sleep_for(osWaitForever);
    }
}

```