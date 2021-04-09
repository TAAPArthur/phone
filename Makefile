all: sms ttyio

SCRIPTS := $(wildcard *.sh)

install: sms ttyio
	for bin in $(SCRIPTS:.sh=); do \
		install -D -m 0755 "$$bin.sh" "$(DESTDIR)/usr/bin/$$bin" ;\
	done
	install -m 0755 -Dt "$(DESTDIR)/usr/bin/" "sms"
	install -m 0755 -Dt "$(DESTDIR)/usr/bin/" "ttyio"

sms: sms.c
	$(CC) -o $@ $^

ttyio: ttyio.c
	$(CC) -o $@ $^

sms-test: sms.c tests/sms_unit.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) -lscutest

ttyio-test: ttyio.c tests/ttyio_unit.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) -lscutest

test: sms-test ttyio-test *.sh all
	./sms-test
	./ttyio-test
	tests/test.sh

clean:
	rm -f *.o ttyio sms sms-test
