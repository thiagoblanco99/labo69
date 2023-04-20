LDLIBS = -lrt -lpthread
CFLAGS = -g -Wall -Werror

TARGETS = shm \
	posix_shm posix_shm2 posix_shm_inc \
	posix_shm_dec posix_shm_monitor

all:	$(TARGETS)

clean:
	rm -f $(TARGETS) core* *.o out.txt
