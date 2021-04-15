C_BIN = sms ttyio vibrator

all: $(C_BIN)

SCRIPTS := $(wildcard *.sh)

install: $(C_BIN)
	for bin in $(SCRIPTS:.sh=); do \
		install -D -m 0755 "$$bin.sh" "$(DESTDIR)/usr/bin/$$bin" ;\
	done
	install -m 0755 -Dt "$(DESTDIR)/usr/bin/" $(C_BIN)

sms: sms.c
	$(CC) -o $@ $^

vibrator: vibrator.c
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
