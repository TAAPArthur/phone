
all: sms ttyio

install: sms ttyio
	install -D -m 0755 "save-sms.sh" "$(DESTDIR)/usr/bin/save-sms"
	install -D -m 0755 "send-sms.sh" "$(DESTDIR)/usr/bin/send-sms"
	install -m 0755 -Dt "$(DESTDIR)/usr/bin/" "sms"
	install -m 0755 -Dt "$(DESTDIR)/usr/bin/" "ttyio"

sms: sms.o
	$(CC) -o $@ $^

ttyio: ttyio.o
	$(CC) -o $@ $^
