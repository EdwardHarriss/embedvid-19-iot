# embedvid-19-iot

## embedvid-19.py ##

Can be used on device.

### Hardware ###

Buzzer to 24 and Button to 10. Sensors as usual

### How to Use ###

System will print 'System ready'. Press button once to start, will feel buz. 
To turn off system, hold button, will feel one buz followed after 2 seconds by 2 short buzzes.
To turn system back on press button, will feel buz.

### Tracking ###

System will track average angle difference and distance difference over 30s. If too much, will buz for 3s on 3s off over and over.
To reset press button, will feel buz.

Around 30s will send data packet to test.mosquitto.org port 1883, no encryption yet!!!!!!!!!!!!!

If system in 'off state' no data sent!

When user has moved too much system will send data packets but no average distance or temperatur


### Data Packets ###

String of Dict. 

Time:<br/>
Movement: (True if moved too much, false otherwise)<br/>
Average Distance: (0 if movement is True)<br/>
Temperature: (0 if movement is True)<br/>

---

### To do ###

- Create magneometer function
- Might have to create TOF function
- Sort out correct data sending and encryption (Tyler permitting)

