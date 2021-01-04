import serial
import os, sys

ser = serial.Serial(input("Enter Serial"), 9600, timeout = 1)

while True:
	