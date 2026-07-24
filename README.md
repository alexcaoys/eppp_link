# ESP PPP Link component (eppp_link)

This is a modified version of eppp_link from esp-protocols. It is primarily designed for USB Serial-JTAG–only use cases (for example, the Seeed Studio XIAO ESP32-C3), enabling communication between an ESP32 and a device without Wi-Fi where high bandwidth is not required.

The component provides a general purpose connectivity engine between two microcontrollers, one acting as PPP server, the other one as PPP client.
This component could be used for extending network using physical serial connection. Applications could vary from providing RPC engine for multiprocessor solutions to serial connection to POSIX machine. This uses a standard PPP protocol (if enabled) to negotiate IP addresses and networking, so standard PPP toolset could be used, e.g. a `pppd` service on linux. Typical application is a WiFi connectivity provider for chips that do not have WiFi.
Uses simplified TUN network interface by default to enable faster data transfer on non-UART transports.

## Typical application

Using this component we can construct a WiFi connectivity gateway on PPP channel. The below diagram depicts an application where
PPP server is running on a WiFi capable chip with NAPT module translating packets between WiFi and PPPoS interface.
We usually call this node a communication coprocessor, or a "SLAVE" microcontroller.
The main device (sometimes also called the "HOST") runs PPP client and connects only to the serial line,
brings in the WiFi connectivity from the communication coprocessor.

```
        Communication coprocessor                              Main device
    \|/  +----------------+                                +----------------+
     |   |                |       (serial) line            |                |
     +---+ WiFi NAT PPPoS |=== JTAG / UART / SPI / SDIO ===| PPPoS client   |
         |        (server)|                                |                |
         +----------------+                                +----------------+
```

## Features

### Network Interface Modes

Standard PPP Mode.

### Transport layer

JTAG, UART, SPI, SDIO

## (Other) usecases

Besides the communication coprocessor example mentioned above, this component could be used to:
* Bring Wi-Fi connectivity to a computer using ESP32 chip.

## Configuration

### Choose the transport layer

Use `idf.py menuconfig` to select the transport layer:

* `CONFIG_EPPP_LINK_JTAG` -- Use JTAG transport layer
* `CONFIG_EPPP_LINK_UART` -- Use UART transport layer
* `CONFIG_EPPP_LINK_SPI` -- Use SPI transport layer
* `CONFIG_EPPP_LINK_SDIO` -- Use SDIO transport layer

## API

### Client

* `eppp_connect()` -- Simplified API. Provides the initialization, starts the task and blocks until we're connected

### Server

* `eppp_listen()` -- Simplified API. Provides the initialization, starts the task and blocks until the client connects

### Manual actions

* `eppp_init()` -- Initializes one endpoint (client/server).
* `eppp_deinit()` -- Destroys the endpoint
* `eppp_netif_start()` -- Starts the network, could be called after startup or whenever a connection is lost
* `eppp_netif_stop()` --  Stops the network
* `eppp_perform()` -- Perform one iteration of the PPP task (need to be called regularly in task-less configuration)
#ifdef CONFIG_EPPP_LINK_CHANNELS_SUPPORT
* `eppp_add_channels()` -- Register channel transmit/receive callbacks (only available if channel support is enabled)
#endif

## Throughput

Tested with WiFi-NAPT example

### JTAG

* TCP - 2Mbits/s
* UDP - 1Mbits/s

### UART @ 3Mbauds

* TCP - 2Mbits/s
* UDP - 2Mbits/s

### SPI @ 16MHz

* TCP - 5Mbits/s
* UDP - 8Mbits/s

### SDIO

* TCP - 9Mbits/s
* UDP - 11Mbits/s
