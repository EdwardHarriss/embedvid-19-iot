# embedvid-19-iot

## embedvid-19.py ##

Before running, downlaod pyjwt and https://www.adafruit.com/product/3317
```
sudo pip3 install pyjwt
```
### Hardware ###

Buzzer to 24 and Button to 10. Sensors as usual

### How to Use ###

System will print 'System ready'. Press button once to start, will feel buz. 
To turn off system, hold button, will feel one buz followed after 2 seconds by 2 short buzzes.

### Tracking ###

System will track average angle difference and distance difference over 30s. If too much, will buz for 3s on 3s off over and over.
To reset press button, will feel buz.

Around 30s will send data packet to website

When system is turning off will send away from desk final packet

When user has moved too much system will send data packets but no average distance or temperature


### Data Packets ###

String of Dict. 

Time:<br/>
Movement: (True if moved too much, false otherwise)<br/>
Average Distance: (0 if movement is True)<br/>
Temperature: (0 if movement is True)<br/>

---

### To do ###

- Improve Magneometer

