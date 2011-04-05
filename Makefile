HACK=keycode-hack.so
LOCAL_BIN:=$(shell echo $$PATH | grep -o "$$HOME/[^:]*bin" | head -1)

build: $(HACK)
	@echo Built. Now run "make install".

install: $(HACK) desktop-menu-install
	install -D $(HACK) $$HOME/local/lib/$(HACK)
ifneq ($(LOCAL_BIN),)
	install -m 0755 javaws-idrac $(LOCAL_BIN)/javaws-idrac
else
	@echo "Unable to determine user local path. Please install javaws-idrac to a directory within \$$PATH"
endif

uninstall:
	-rm -f $$HOME/local/lib/$(HACK)
ifneq ($(LOCAL_BIN),)
	-rm -f $(LOCAL_BIN)/javaws-idrac
else
	@echo "You need to delete the javaws-idrac script manually"
endif
	-xdg-desktop-menu uninstall javaws-idrac.desktop

keycode-hack.so: keycode-hack.c
	gcc -o $@ $^ -shared -s -ldl -fPIC

desktop-menu-install: javaws-idrac.desktop
	xdg-desktop-menu install $^

clean:
	rm -f $(HACK)

.PHONY: clean desktop-menu-install install build uninstall
