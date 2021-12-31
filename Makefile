C_BIN := sms ttyio vibrator
SCRIPTS := $(wildcard *.sh)
CFLAGS := -Wall -Werror

all: $(C_BIN)

install: $(C_BIN)
	for bin in $(SCRIPTS:.sh=); do \
		install -D -m 0755 "$$bin.sh" "$(DESTDIR)/usr/bin/$$bin" ;\
	done
	install -m 0755 -Dt "$(DESTDIR)/usr/bin/" $(C_BIN)

sms: sms.c
	$(CC) $(CFLAGS) -o $@ $^

vibrator: vibrator.c
	$(CC) $(CFLAGS) -o $@ $^

ttyio: ttyio.c
	$(CC) $(CFLAGS) -o $@ $^

sms-test: sms.c tests/sms_unit.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

ttyio-test: ttyio.c tests/ttyio_unit.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test: CFLAGS += -O0 -g
test: sms-test ttyio-test *.sh all
	./sms-test
	./ttyio-test
	tests/test.sh

clean:
	rm -f *.o $(C_BIN) *-test
