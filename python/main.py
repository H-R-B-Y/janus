
import sys, os
from waveshare_epd import epd2in13_V4
import time
from PIL import Image,ImageDraw,ImageFont
import traceback

import socket

import socket
def getNetworkIp():
	while (1):
		try:
			s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
			s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
			s.connect(('<broadcast>', 12345))
			s.close()
			return s.getsockname()[0]
		except OSError as e:
			time.sleep(1)
			continue
		except Exception as e:
			return "Error"

print (getNetworkIp())

def main():
	epd = epd2in13_V4.EPD()
	epd.init()
	epd.Clear(0xFF)

	image = Image.new(
		'1',
		(epd.height, epd.width),
		255
	)
	draw=ImageDraw.Draw(image)

	draw.text((0,0), getNetworkIp())

	epd.display(epd.getbuffer(image))

	epd.sleep()

if __name__ == "__main__":
	main()
