# 3DS vJoy Controller
This project allows anybody with one (or more) 3DS and vJoy to use their device(s) as a singular, coherent controller.
When a 3DS joins the server (running on the system where the button inputs are wanted), it will be assigned the next available buttons on the virtual controller and update those depending on which buttons are pressed on the 3DS.

Focus of the project is the simple protocol and the extensibility for different devices.

For educational purposes only.
# Requirements
- Nintendo 3DS
  - with the spicy firmware Nintendo loves
- [vJoy](https://github.com/shauleiz/vJoy)
- Python
  - `pyvjoy`-Library
## Build-Requirements
- [devkitARM](https://devkitpro.org/wiki/Getting_Started)
# Getting the 3DS application

## From Releases
Go over to the releases and download the binary

## By building yourself
`cd 3ds && make`

## From Universal Updater
> [!NOTE]  
> The app is submited to Universal DB to be available in the Universal Updater
> Still in revision

# Usage
1. Install the `.3dsx` app to your 3DS devices
2. Start the server using `python3 server.py`
3. Start the application on your 3DS devices
4. Enter the IP of the server
5. Profit

# Protocol
![Protocol](assets/protocol.svg)

## Data
The data is sent in the following format:
```
<Packet>  ::= '<' <Data> '>'
<Data>    ::= <Integer> ';' <Data> | <Integer>
<Integer> ::= <Digit> | <Digit> <Integer>
<Digit>   ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
```

## Example
```plaintext
<4; 17408; 16998>
```
- First integer will be interpreted as pressed buttons binary formatted
  - 4 = 100
    - 0 = A
    - 0 = B
    - 0 = X
    - 1 = Y
- Second integer will be interpreted as the X-Axis value
  - The value is in the range of 0-32768
- Third integer will be interpreted as the Y-Axis value
  - The value is in the range of 0-32768