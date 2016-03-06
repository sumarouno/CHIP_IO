#!/usr/bin/python

import CHIP_IO.ADC as ADC
import time, os

print "SETTING UP ADC"
tmp = ADC.setup()
print "SETUP RETURN VALUE", tmp

print "ENABLING ADC"
ADC.enable(1)

print "READING ADC DATA"
tmp = ADC.read_raw(0)
print "DATA0: %d" % tmp
tmp = ADC.read_raw(1)
print "DATA1: %d" % tmp

print "ADC DISABLE"
ADC.enable(0)

print "ADC CLEANUP"
ADC.cleanup()
