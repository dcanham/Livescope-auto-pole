# Livescope-auto-pole

# Livescope Pole V1

This project is for controlling a 1 1/8" inch pole that your livescope is mounted on, however it can be updated to fit other poles. Lots of code here for testing, so each folder will be explained below. Bluetooth_and_AP_Mode is the main code that is working and partially tested, it powers on, looks for the most recently connected device, if it does not find it, it will go into "AP" mode where your phone can connect to its wifi. Then go to a browser and go to 192.168.4.1 and a webpage should show. Click "scan" and it will look for bluetooth devices in the area. Select the device you wish to connect and it will shut off the AP and start working. Now each time the device boots up it will look for that device. 

## Parts list:
- ESP32 with 18650: https://a.co/d/enj4STK
- 3v to 5v step up: https://www.pololu.com/product/2562 (required as the ESP only outputs 5v if on USB power)
- Servo: https://a.co/d/6YfNTrJ
- 1" ram mount: https://a.co/d/4ML1X51
- Wireless BT Remote: https://a.co/d/i9t7pQf
- Box: Printed in PETG, files can be found 3d Printed Parts folder. 
- Misc wires and solder
- A 1 and 1/8 livescope pole with hole at top, I used the summit fishing pole. 



## To do:
- Clean Up Code
- Test other bluetooth remotes
- Add comments
- Ajust gears to have deeper teeth for better grip
