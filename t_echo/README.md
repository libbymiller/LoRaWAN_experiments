This is code for these little fellas:

https://www.lilygo.cc/products/t-echo

They are not ESP32s, but Nordic NRF52840 DK somethings.

Richard did a lot of the hard work here, he says:

    add https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
    to board manager list

    install 'Adafruit nRF52 by Adafruit' (not 'Arduino nRF52 boards')
    select Nordic nRF52840 DK something

(https://github.com/jarkman/WhereOTechno/blob/main/WhereOTechno.ino)

Note that: using delay means the GPS doesn't work!
