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
