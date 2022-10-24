To flash:


1. Create a secrets storing header file under **src/stack/secrets.h** with the below content:
```
#ifndef SECRETS_H
#define SECRETS_H

#define wifi_ssid "home-router-ssid"
#define wifi_password "home-router-ssid-password"
#define BASIC_AUTH_USERNAME "gaggiuino"
#define BASIC_AUTH_PASSWORD "oniuiggag"

#endif
```
Make sure to change the values for **wifi_ssid** and **wifi_password** to reflect your settings.

2. Upload Filesystem Image
3. Upload 