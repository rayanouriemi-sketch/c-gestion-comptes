CFLAGS ?= -std=c11 -Wall -Wextra -Werror -O2

banque: src/main.c src/bank.c src/bank.h
	$(CC) $(CFLAGS) -o $@ src/main.c src/bank.c

test_bank: tests/test_bank.c src/bank.c src/bank.h
	$(CC) $(CFLAGS) -fsanitize=address,undefined -g -o $@ tests/test_bank.c src/bank.c

test: test_bank banque
	./test_bank
	rm -f demo.csv
	./banque demo.csv ouvrir Alice
	./banque demo.csv depot 1 100.50
	./banque demo.csv retrait 1 0.50
	./banque demo.csv solde 1 | grep -q "100.00 EUR"
	! ./banque demo.csv retrait 1 1000
	rm -f demo.csv

clean:
	rm -f banque test_bank *.csv

.PHONY: test clean
