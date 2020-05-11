#!/usr/bin/env python

from bluetooth import *
target_name = "MyPhone"
target_address = None
nearby_devices = discover_devices ()
for address in nearby_devices :
    device_name = lookup_name(address)
    print device_name + ": " + address
    if target_name == device_name:
        target_address = address
    
if target_address is not None :
    print "found target bluetooth device with address" , target_address
else :
    print "could not find target bluetooth device nearby"
