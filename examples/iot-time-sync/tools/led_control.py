from gpiozero import LED
from time import sleep


# set up GPIO input and output
led = LED(27)

print("Start LED blinking...")
while True:
    led.on()
    sleep(1)
    led.off()
    sleep(1)
