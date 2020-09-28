ESP32 Remote JTAG
=================

WIP


Everything here is licensed at Apache (see LICENSE) except for libxsvf which is done by Clifford Wolf and licensed under ISC License (See src/libxsvf/COPYING) and `data` folder which for now is LGPL. The original library is at [http://www.clifford.at/libxsvf/](http://www.clifford.at/libxsvf/) and I modified a little to work with ESP32.

The `upload.py` program can upload SVF files to ESP32 through serial port and `uploadwifi.py` to upload through wireless network.

```bash
python3 uploadwifi.py [IP] [SVF FILE]
```