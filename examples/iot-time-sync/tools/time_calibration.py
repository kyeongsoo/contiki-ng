from gpiozero import LED
from time import sleep


# set up GPIO input and output
led = LED(17, initial_value=False)

# input("Press enter to generate triggering events: ")
print("Generating events ... ")
for i in range(100):  # Generate 10 triggering events
    led.on()
    sleep(0.5)
    led.off()
    sleep(0.5)
