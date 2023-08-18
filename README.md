# ir_mouse_controller
control system (linux based) mouse pointer using TV remote controller

# requirements
```
sudo apt install avr-gcc avr-libc
```

# compile
```
make move main
```

# flash atmega328p
```
make flash
```

# example run
```
./move /dev/ttyUSB0 /dev/input/event7
```

# video
https://www.youtube.com/watch?v=MetcF2h9pq0

