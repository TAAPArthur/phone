all: sms ttyio

install: sms ttyio
	install -D -m 0755 -Dt "$(DESTDIR)/usr/bin/" $((basename -s .sh -a *.sh))
	install -m 0755 -Dt "$(DESTDIR)/usr/bin/" "sms"
	install -m 0755 -Dt "$(DESTDIR)/usr/bin/" "ttyio"

sms: sms.c
	$(CC) -o $@ $^

ttyio: ttyio.c
	$(CC) -o $@ $^

clean:
	rm -f *.o ttyio sms
