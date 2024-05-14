# Livescope-auto-pole

<p>Livescope Pole V1</p>

This project is for controlling a 1 1/8" inch pole that your livescope is mounted on, however it can be updated to fit other poles. Lots of code here for testing, so each folder will be explained below. Bluetooth_and_AP_Mode is the main code that is working and partially tested, it powers on, looks for the most recently connected device, if it does not find it, it will go into "AP" mode where your phone can connect to its wifi. Then go to a browser and go to 192.168.4.1 and a webpage should show. Click "scan" and it will look for bluetooth devices in the area. Select the device you wish to connect and it will shut off the AP and start working. Now each time the device boots up it will look for that device. 

Parts list:
ESP32 with 18650: https://a.co/d/enj4STK
3v to 5v step up: https://www.pololu.com/product/2562 (required as the ESP only outputs 5v if on USB power)
Servo: https://a.co/d/6YfNTrJ
1" ram mount: https://a.co/d/4ML1X51
Box: Printed in PETG, files can be found 3d Printed Parts folder. 
Misc wires and solder

<p>Bluetooth_and_AP_Mode folder will boot the ESP and check for a recently conencted bluetooth device. If it is not found in the first ~10 seconds, it will turn on the AP and host a simple webserver at 192.168.4.1. connect to the ESP with "password" and access the webpage. You can then press 'scan' and it will find bluetooth devices in range. You then have the ability to press connect on the device and it will attempt to connect and register for HID messages to move the servo.</p>



<p> Bluetooth_message_send_servo_test will simply allow you to send bluetooth messages via bluetooth serial to control the servo</p>


<p> Bluetooth_remote_connected_controlling_servo will control the servo from the remote, but the devices address and things are hard coded </p>
