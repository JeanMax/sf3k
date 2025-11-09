# Persistent flash storage

This linker script is here to reserve a memory space in flash; we'll be able to use this space in our pico code to persist data.

This approach has the advantage of preventing from overwriting that space when flashing our program, and it also give us access to the address reserved in our code without further hardcoding.

See usage in `./src/utils/persist.h`

## Credits:
https://community.element14.com/products/raspberry-pi/b/blog/posts/raspberry-pico-c-sdk-reserve-a-flash-memory-block-for-persistent-storage

https://kevinboone.me/picoflash.html
