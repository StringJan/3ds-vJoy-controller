# 3DS vJoy Controller
This project allows anybody with one (or more) 3DS and vJoy to use their device(s) as a singular, coherent controller
# Requirements
- Nintendo 3DS
  - with this spicy firmware Nintendo loves
- [vJoy](https://github.com/shauleiz/vJoy)
- Python
  - `pyvjoy`
## Build
- [devkitARM](https://devkitpro.org/wiki/Getting_Started)
# Getting the 3DS application
## Universal Updater
> [!NOTE]  
> The app is being submited to Universal DB to be available in the Universal Updater directly

## Release
Go over to the releases and download the binary
## Build
`cd 3ds && make`
# Usage
1. Install the `.3dsx` app to your 3DS devices
2. Start the server using `python3 server.py`
3. Start the application on your 3DS devices
4. Enter the IP of the server
