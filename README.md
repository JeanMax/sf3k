# SuperFreezer3000

SF3K is temperature controller (STC-1000 like) built on a Raspbeery Pico, aimed at beer fermentation chambers. Essentially, it controls a freezer and a heating mat combo to maintain the desired target temperature.

As a bonus, it can be remotely controlled through a udp socket, and it can post http status to your favorite server.

<img src="https://rmrf.fr/static/img/sf3k/box.jpg" alt="Box" width="800">
<img src="https://rmrf.fr/static/img/sf3k/overview.jpg" alt="Overview" width="800">

(you'll excuse the black tape :|)


## Build

You could basically get away with a proper `cmake` call, but there's:
```
./build.sh
```
Look inside for options, it's pretty straight forward.


## Flash

The standard pico way of flashing will work once you have a `uf2` file ready from the previous step, but there's a script here that will also handle rebooting the raspberry as needed:
```
./flash.sh
```

## Console

You need to connect with minicom or something similar.
Alternatively, you can just:
```
./console.sh
```


## Credits

Special thanks to [DO](https://github.com/do-ox), who helped with electronics shenanigans, brainstorming and ideas, and of course beer tasting!


## Beer

<img src="https://rmrf.fr/static/img/sf3k/missouk.jpg" alt="Missouk" width="800">
