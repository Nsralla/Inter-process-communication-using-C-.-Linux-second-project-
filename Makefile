CC = gcc
CFLAGS = -w

all: supermarket team customer forkcustomers build_script

supermarket: supermarket.c read_supermarket_config.c
	$(CC) $(CFLAGS) -o supermarket supermarket.c read_supermarket_config.c

team: team.c read_supermarket_config.c
	$(CC) $(CFLAGS) -o team team.c read_supermarket_config.c -lpthread

customer: customer.c read_supermarket_config.c
	$(CC) $(CFLAGS) -o customer customer.c read_supermarket_config.c

forkcustomers: forkcustomers.c read_supermarket_config.c
	$(CC) $(CFLAGS) -o forkcustomers forkcustomers.c read_supermarket_config.c

build_script: supermarket team customer forkcustomers
	bash build.bash

	
.PHONY: clean

clean:
	rm -f supermarket team customer forkcustomers a.out