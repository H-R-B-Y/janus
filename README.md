# Janus
The gate of the world.

## Overview
Janus is a small controller for Waveshare e‑Paper displays on Raspberry Pi. It monitors network interfaces and updates the display (or terminal) to reflect connectivity and IP state. C and Python components are included; the C app provides the event loop and service.



## Build
On a Raspberry Pi with the supported display connected:

```zsh
make
```

If you are not using the same screen, or you just want terminal output without touching display libraries:

```zsh
make terminal
```

## Configuring interfaces
Janus tracks two network interfaces, defined as `INTERFACE_1` and `INTERFACE_2` (defaults are `"eth0"` and `"wlan0"`). You can override them via make CFLAGS, which pass compiler `-D` defines:

```zsh
make CFLAGS="-DINTERFACE_1=\"eth1\" -DINTERFACE_2=\"wlan1\""
```

## Install
This installs binaries and registers a systemd service so Janus starts on boot:

```zsh
sudo make install
```

Warning: install uses `sudo` and will add a `systemctl` service. Review the Makefile and service settings before installing.
