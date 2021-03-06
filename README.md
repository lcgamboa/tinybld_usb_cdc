
# USB-CDC Tiny PIC bootloader firmware for PIC18F4550


Tiny Bootloader is a serial bootloader for the Microchip PIC microcontrollers.

USB-CDC Tiny PIC bootloader is a USB-CDC implementation compatible with tiny bootloader serial protocol.

USB-CDC Tiny PIC bootloader size optimization is not the focus of this project.
The focus of project is implement a c version of PIC18F4550 tiny bootload firmware and a improved USB-CDC version.   

The code is based in Tiny Bootloader (http://www.etc.ugal.ro/cchiculita/software/picbootloader.htm) and Minimal USB CDC ACM (http://www.sparetimelabs.com/usbcdcacm/usbcdcacm.php)

The code was tested with sdcc 3.5.0. (Also it compile with XC8)

##Limitations 

The CDC-USB bootloader size is 3720 bytes (1860 Words).
The code of bootloader is located in address 0x7178 at 0x7FFF of flash memory.

Memory range of user code must be set to 0x0000 at 0x7177.

* In sdcc compiler the memory range is set in .lkr file. 
* In XC8  compiler the memory range is set in "ROM ranges" option of XC8-Linker. The option "Program the device with default config words" must be unchecked. 


##Project folders

* hex\_firmware/          - firmware of USB-CDC Tiny bootloader 
* src/firmware\_asm.X/    - Source of original serial asm firmware (100 Words -  0,61% of total flash memory)
* src/firmware\_c.X/      - Source of serial c firmware for sdcc   (388 Words -  2,37% of total flash memory)  
* src/firmware\_usb.X/    - **Source of USB-CDC c firmware for sdcc** (1860 Words - 11,35% of total flash memory)

* src/template\_usb.X/    - Sample user project configured to work with USB-CDC for sdcc and XC8.    

##How to use

1. Burn the bootloader firmware in PIC18F4550 with any pic programmer (pickit2, pickit3...);
2. Open the .hex file in PC side software;
3. Power the circuit with PIC18F4550 connected to USB;
4. Reset the PIC18F4550;
5. Immediately scan for serial ports in PC side software; (Bootloader is active for few seconds) 
6. Select the serial port (COMx in windows or /dev/ttyACMx on linux) with 115200bps in PC side software;
7. Reset the PIC18F4550;
8. Immediately press "Write Flash" in PC side software; (Bootloader is active for few seconds)
9. Now the .hex is writed in PIC18F4550, repeat the process (except item 1) to write a new .hex file.

##PC side software:

###For linux
* **Tiny Pic Bootloader for GNU/Linux** https://github.com/lcgamboa/tinybldlin 

To configure edit modules/pictype.py pictype 0x55 and change bsize for 3720. 

Like:
```
    elif pt==0x55:
        PicType="18F 2550/4550";
        max_flash=0x8000;
        bsize=3720;
```

In Linux the service ModemManager automatic probe tje ports /dev/ttyACMx. 
To use /dev/ttyACMx with Tiny bootloader it is necessary stop, disable or configure ModemManager to not probe the port.
  
To stop the service (per session): 
```
sudo systemctl stop ModemManager.service
```

To disable the server (permanently): 
```
sudo systemctl disable ModemManager.service
```

To configure ModemManager to ignore USB-CDC:
```
1. Open or create the file: sudo gedit /etc/udev/rules.d/99-ttyacms.rules
2. Add the line: ATTRS{idVendor}=="0408" ATTRS{idProduct}=="000a", ENV{ID\_MM\_DEVICE\_IGNORE}="1"
3. Save the file
4. Reload udev rules with command: sudo udevadm control --reload-rules
```


##For windows
* **Tiny Multi Bootloader+**  http://tinypicbootload.sourceforge.net/

To configure edit piccodes.ini pictype 0x55 and change bsize for 3720.

Like:
```
$55, C, 18F 2550/4550,              $8000, $100, 3720, default,
```

* **Tiny Bootloader**       http://www.etc.ugal.ro/cchiculita/software/picbootloader.htm

To configure edit tinybld.ini SizeOfBLD\_W for 1860.

Like:
```
SizeOfBLD_W=1860
```


Luis Claudio Gamboa Lopes
