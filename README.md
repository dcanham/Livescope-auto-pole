# Livescope-auto-pole

<p>so this is a thing</p>

<p>Bluetooth_and_AP_Mode folder will boot the ESP and check for a recently conencted bluetooth device. If it is not found in the first ~10 seconds, it will turn on the AP and host a simple webserver at 192.168.4.1. connect to the ESP with "password" and access the webpage. You can then press 'scan' and it will find bluetooth devices in range. You then have the ability to press connect on the device and it will attempt to connect and register for HID messages to move the servo.</p>



<p> Bluetooth_message_send_servo_test will simply allow you to send bluetooth messages via bluetooth serial to control the servo</p>


<p> Bluetooth_remote_connected_controlling_servo will control the servo from the remote, but the devices address and things are hard coded </p>