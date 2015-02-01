# Seeduino Stalker

Here are my experiments with the Seeeduino Stalker.

# Power Budget

The solar-powered Stalker kit has a very small power budget, limited by the tiny 0.5W solar panel. This means in an "average" day there will be a budget of about 4Wh of power, which translates to about 800mAh per day.

## XBee Power Consumption

I'm trying to reduce the power consumption of the XBee, but I also need to know the power budget. How much does it use when turned on, joining a PAN, transmitting data, and simply being idle? There is an [unanswered question on the Electrical Engineering Stack Exchange site](http://electronics.stackexchange.com/questions/96451/actual-power-consumtion-of-xbee-series-2-in-sleep-power-down-mode) relating to XBee Series 2 power consumption in sleep/power down modes. The linked article shows up in [this search](http://www.socgadget.com/?s=XBee), but the site is quite broken.

Just measuring the consumption while an XBee Series 2 is turned on, I have found that this specific item (chiptenna, set up as a router in AT mode, sitting on my breadboard with 3.3V supply) consumes 7mA while idle, 37mA while a radio session is active. Thus the power budget should allow for about 20h of active radio time per day, if the radio was the only consumer.

## Stalker Power Consumption

 - Measured with my Digitech QM1328 multimeter
 - Meter is inline with "USB5V" power line (which is actually 3.3V from UARTSBee module)

When "shut down", 1.32mA consumption. When radio is powered up, consumption rises to 15mA for a few seconds, then rises to 45mA.
