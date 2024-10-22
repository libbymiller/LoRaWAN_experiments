We use MQTT to get data out of The Things Network (TTN)

You need to

 * create an application (mine is "foo-bar")
 * create an api key https://eu1.cloud.thethings.network/console/applications/foo-bar/api-keys
 * add some devices - radiolib is good on this: https://github.com/jgromes/RadioLib/tree/master/examples/LoRaWAN
  - https://github.com/jgromes/RadioLib/blob/master/examples/LoRaWAN/LoRaWAN_Starter/notes.md
  - note the issue with persistence and conformity. I only have persistence things that work with ESP32 not these Nordic ones.
  - when you add the device you therefore need to go into advanced settings -> Join settings and check "resets join nonces" (which is insecure but 🤷)
  - you can set payoad formatters to CayenneLPP which makes encoding and decoding much simpler

Then to actually get the data out you can install mosquitto on your server and read the data coming out, like this:

    mosquitto_sub -v -t "/ttn/v3/foo-bar@ttn/devices/#"

This is all insecure unless you secure it! https://www.thethingsindustries.com/docs/integrations/pubsub/mqtt-client/

If you want data in say real-time javascript using websockets you can use a config like the one in this directory to 
make a bridge. Insecure again! (WSS is possible, not sure how yet). You need to copy it into /etc/mosquitto/conf.d

There's an example in this directory of using leaflet (https://leafletjs.com) to display the results in real time on a map (you need to add GPS coordinates to 
centre the map).

There's also a simple python example of logging to a file (which mosquitto doesn't do), assuming you have a broker running.





