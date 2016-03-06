#!/usr/bin/python

import CHIP_IO.GPIO as GPIO
import time, os

print "SETUP CSID1"
GPIO.setup("CSID1", GPIO.OUT)

#print os.path.exists('/sys/class/gpio/gpio133')

print "SETUP XIO-P1"
GPIO.setup("XIO-P1", GPIO.IN)
#GPIO.setup("U14_13", GPIO.IN)

print "READING XIO-P1"
GPIO.output("CSID1", GPIO.HIGH)
print "HIGH", GPIO.input("XIO-P1")

GPIO.output("CSID1", GPIO.LOW)
time.sleep(1)
print "LOW", GPIO.input("XIO-P1")

GPIO.output("CSID1", GPIO.HIGH)
print "HIGH", GPIO.input("XIO-P1")
time.sleep(1)

GPIO.output("CSID1", GPIO.LOW)
print "LOW", GPIO.input("XIO-P1")

print "CLEANUP"
GPIO.cleanup()

