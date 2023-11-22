libexa-wayland.a: client.c build/xdg-shell-client-protocol.h build/xdg-decoration-unstable-v1-client-protocol.h
               cp /usr/include/wayland* build/
               cp -R /usr/include/xkbcommon build/
               $(CC) $(CFLAGS) client.c -c -o build/client.o -I build/
               $(AR) rcs build/libexa-wayland.a build/client.o

build/xdg-shell-client-protocol.h: /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
               wayland-scanner client-header < $^ > $@

build/xdg-decoration-unstable-v1-client-protocol.h: /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
               wayland-scanner client-header < $^ > $@

clean:
               rm -rf build/*
