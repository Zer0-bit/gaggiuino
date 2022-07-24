# gaggiuino-nextgen-frontend
Next generation UI/UX for for Gaggiuino.

For successful build the file from *src\lv_conf.h*  needs to be moved in *.pio\libdeps\esp32dev\lvgl\src*

The process of making this header file external is described [here](https://docs.lvgl.io/latest/en/html/porting/project.html#configuration-file) but movingt that file breaks the build in a way where the touch won't work anymore, so needs more tinkering to get this working.

Display wiring tneeds to follow the below diagram:
![img](https://cdn.discordapp.com/attachments/999304221566439544/999364952940871830/unknown.png)