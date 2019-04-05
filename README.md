# uwpflash
UNISOC flash tool.

It is used to update images for UNISOC devices through UART interface.

# usage
```
-h               show this message.
-t <type>        interface type.
				 default: UART
-d <dev>         device name.
				 default: /dev/ttyUSB0
-f <file>        image file name.
-a <address>     flash address.
```
# example

It is able to update one or more images with one command:

```
uwpflash -t UART -d /dev/ttyUSB0 -f kernel.bin -a 0x2000000
```

or

```
uwpflash -t UART -d /dev/ttyUSB0 -f mcuboot.bin -a 0x2000000 -f signed-kernel.bin -a 0x2010000
```
