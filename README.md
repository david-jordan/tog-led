# tog-led
## Design principles
1) If something is worth doing, it's worth over-engineering
2) Elon Musk's principle of simlifying and deleting processes is anathema

## How it works
There are 2 arduinos
A nano which interfaces to most of the switches to drive the LCD display and decide which mode should be used, and a mega which drives LEDs according to the current mode

We need to because the switches have interrupts and the LEDs don't like being interrupted: the timing is critical