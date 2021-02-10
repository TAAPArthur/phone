
all: sms ttyio

install:
	install -D -m 0755 "save-sms.sh" "$(DESTDIR)/usr/bin/save-sms"
	install -D -m 0755 "send-sms.sh" "$(DESTDIR)/usr/bin/send-sms"

sms: sms.o
	$(CC) -o $@ $^

ttyio: ttyio.o
	$(CC) -o $@ $^
