# ignore warnings from gpiozero about the lack of lgpio and RPi
# modules, which are not available on non-Raspberry Pi platforms.
import warnings
warnings.filterwarnings("ignore", module="gpiozero")
from gpiozero import LED
from time import sleep


# set up GPIO input and output
led = LED(27)


print("Start LED blinking...")
for i in range(10):
    led.on()
    print("- LED is ON")
    sleep(1)
    led.off()
    print("- LED is OFF")
    sleep(1)
print("LED blinking is done.")