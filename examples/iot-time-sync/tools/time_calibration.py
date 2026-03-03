import warnings
warnings.filterwarnings("ignore", module="gpiozero")
# ignore warnings from gpiozero about the lack of lgpio and RPi
# modules, which are not available on non-Raspberry Pi platforms.
from gpiozero import LED
from time import sleep


# set up GPIO input and output
led = LED(17, initial_value=False)

print("Generating timing signals ... ")
sleep(2) # initial delay

# generate timing signals with the 1st one triggering the calibration
# process at a target node
for i in range(101):  
    led.on()
    sleep(0.5)
    led.off()
    sleep(0.5)
print("Completed timing signal generation.")
