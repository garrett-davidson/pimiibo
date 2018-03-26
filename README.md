# pimiibo

This is a tool for spoofing amiibo NFC tags using a Raspberry Pi and PN532 NFC reader/writer with [libnfc](http://nfc-tools.org/index.php/Libnfc). Enjoy and please let me know if you have any issues :)

## Hardware
This project requires 3 main components: a Pi, a PN532 NFC reader/writer, and NTAG 215 NFC tags. You *must* use NTAG 215 tags. Any other tag type (e.g NTAG 210, 216, etc.) **will not work**. Anything meeting these requirements should be fine, but here is my setup for reference:

1. Raspberry Pi Zero W

2. Elechouse PN532 NFC Module v3: [Amazon](https://smile.amazon.com/gp/product/B01NBSW0NU/).

3. NFC NTAG 215 tags. I've personally tested with [these](https://smile.amazon.com/gp/product/B078WMQPCZ/) and [these](https://smile.amazon.com/gp/product/B0759W25TL/).

## Software setup

1. Clone this repository with submodules:

    `git clone --recurse-submodules https://github.com/garrett-davidson/pimiibo.git`

2. Install `libnfc-dev`:

    `sudo apt-get update && sudo apt-get install libnfc-dev`

3. Compile sources. This will probably take a minute or two the first time on a Pi.

    `make`


## Set up I2C (Optional)
This section is dependent on your NFC board and how plan to communicate with it. If you plan to use I2C, you need to enable I2C on your Pi and tell `libnfc` how to find it. If you plan to use SPI, HSU, or other communication methods, follow the specific instructions for your board.


(**Note for SPI and HSU/UART users**: if you're using a Pi 3 or Pi Zero W with Bluetooth built-in, then you will want to disable Bluetooth when communicating with the board. This is because the serial pin got hijacked for Bluetooth and the software pin has clocking issues. You can read more [here](https://spellfoundry.com/2016/05/29/configuring-gpio-serial-port-raspbian-jessie-including-pi-3/).)

1. Enable I2C on your Pi.

    `sudo raspi-config`

    Then navigate to `Interfacing Options` -> `I2C` -> `Yes`.

2. Install i2c-tools

    `sudo apt-get install i2c-tools`

3. Configure `libnfc` to search for your board:

    `sudo nano /etc/nfc/libnfc.conf`

    Add these lines to the end of the file:

    ```
    device.name = "_PN532_I2c"
    device.connstring = "pn532_i2c:/dev/i2c-1"
    ```

4. Make sure your board is in I2C mode. Some boards, like the one linked above, support multiple communication modes, which are selected with physical dipswitches. For that Elechouse board, I2C mode is `1 0`. This means toggle the top switch on (to the right), and the bottom switch off (to the left).

5. Wire the board. For through-hole boards (including the Pi), I *highly recommend* soldering all of the headers on before continuing. A loose connection on any of these wires can damage your Pi and your NFC board, or waste an NFC tag. Once that is done, wire the board as follows:

    | NFC Board Pin | Pi Pin        |
    |:-------------:|:-------------:|
    | Ground        | Ground        |
    | VCC           | 5 V           |
    | SDA           | SDA.1 (Pin 3) |
    | SCL           | SCL.1 (Pin 5) |


6. Check to make sure it is working.

    `sudo i2cdetect –y 1`

    If all goes well, you should see a device show up as a number on that list (mine is 24). If the list is empty (all dashes), double check your setup.

## Getting the required files
After you have followed the above setup, you just need two more files to start making your own amiibo: an amiibo dump, and the key file.

### Amiibo Dump
Amiibo dumps are not hard to get. Amiibo are 540 bytes and usually stored in a .bin (binary) file. Assuming you are simply cloning your own legitimate amiibo, you can use any dumping tool to dump it to a .bin file. Otherwise, that's probably copyright infringement or something.

### Key file
This is the file containing Nintendo's key, which they use to encrypt/decrypt data on the amiibo. It is probably also copyrighted content, but it's a 160 byte .bin file which matches the MD5 `45fd53569f5765eef9c337bd5172f937`.

## Usage

Start the program:
`./pimiibo path-to-key-file path-to-amiibo-file`

Once you see `***Scan tag***`, place and hold your blank NFC tag on the reader/writer. You should then see messages scrolling past with each data page as it begins writing them. ***Do not remove your tag until the write is finished.*** When you see `Finished writing tag`, it is safe to remove your tag and enjoy your new amiibo!

## Common Problems

* Failed to initialize adapter
  ```
  Initializing NFC adapter
  error	libnfc.bus.i2c	Error: wrote only -1 bytes (10 expected).
  ```
  Your Pi could not find your NFC device. Double check your wiring and try again.

* Failed to write a page
  ```
  Writing to 4: a5 b0 d1 00...Failed
  Failed to write to tag
  Write: RF Transmission Error
  ```
  This means your tag is already locked. The NTAG 21x spec declares locking bits which permanently prevent certain parts of a tag from being written to once they are set. Therefore once the locking bits are set, you cannot rewrite this tag to another amiibo. All amiibo are required to have certain locking bits sets, so you cannot change a tag once you've used it.

    If this happened anywhere other than page 4, it probably means that your device lost connection to the tag. Try again while keeping the tag closer to your device. Hopefully in the future I'll add a feature to check which of these problems occurred.
