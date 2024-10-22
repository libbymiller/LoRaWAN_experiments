# LoRaWAN_experiments

As part of Bristol Wireless, we're placed a LoRAWAN the Things Network gateway on a towerblock in Bristol

The idea of this code is to map out how far it reaches. So we have some LoRa devices with GPS which we wander around 
with and capture their data and the gateway data.

It's not quite there yet, but close. We need

* provisioned GPS devices
* gateway up and running
* data coming out of TTN to our servers
* ability to map that data


# TODO

* Finish provisioning the devices
 - I'd like to make their IDs in TTN their mac addresses
 - they need labelling up if so
* Tweak the device code
 - in particular it needs to ping the server pretty often. Need to check the rules for this.
* Tweak the mapping code
 - different devices and gateways should have different colours
 - need a static mapper too



