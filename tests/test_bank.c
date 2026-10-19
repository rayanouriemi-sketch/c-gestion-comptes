#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/bank.h"

static Bank b, loaded;

int main(void) {
    int alice, bob;
    long long c;
    bank_init(&b);

    assert(bank_open_account(&b, "Alice", &alice) == BANK_OK);
    assert(bank_open_account(&b, "Bob", &bob) == BANK_OK);
    assert(bank_open_account(&b, "", NULL) == BANK_ERR_INVALID_NAME);
    assert(bank_open_account(&b, "a,b", NULL) == BANK_ERR_INVALID_NAME);

    assert(bank_deposit(&b, alice, 10000) == BANK_OK);
    assert(bank_deposit(&b, alice, -5) == BANK_ERR_INVALID_AMOUNT);
    assert(bank_deposit(&b, 999, 100) == BANK_ERR_NOT_FOUND);
    assert(bank_withdraw(&b, alice, 20000) == BANK_ERR_INSUFFICIENT_FUNDS);
    assert(bank_withdraw(&b, alice, 2500) == BANK_OK);
    assert(bank_transfer(&b, alice, bob, 5000) == BANK_OK);
    assert(bank_transfer(&b, alice, bob, 5000) == BANK_ERR_INSUFFICIENT_FUNDS);
    assert(bank_transfer(&b, alice, alice, 1) == BANK_ERR_INVALID_AMOUNT);
    assert(bank_find(&b, alice)->balance_cents == 2500);
    assert(bank_find(&b, bob)->balance_cents == 5000);

    /* Sauvegarde puis rechargement : mêmes données, et les ids continuent */
    const char *path = "test_bank.csv";
    assert(bank_save(&b, path) == BANK_OK);
    assert(bank_load(&loaded, path) == BANK_OK);
    assert(loaded.count == 2 && strcmp(bank_find(&loaded, bob)->owner, "Bob") == 0);
    assert(bank_find(&loaded, bob)->balance_cents == 5000);
    int carol;
    assert(bank_open_account(&loaded, "Carol", &carol) == BANK_OK && carol == 3);
    remove(path);

    /* Fichier corrompu */
    FILE *f = fopen(path, "w");
    fputs("pas,un,csv,valide\n", f);
    fclose(f);
    assert(bank_load(&loaded, path) == BANK_ERR_IO);
    remove(path);

    assert(parse_amount("12.34", &c) && c == 1234);
    assert(parse_amount("12,5", &c) && c == 1250);
    assert(parse_amount("7", &c) && c == 700);
    assert(!parse_amount("0", &c));
    assert(!parse_amount("-3", &c));
    assert(!parse_amount("1.234", &c));
    assert(!parse_amount("abc", &c));
    assert(!parse_amount("12.", &c));

    puts("Tous les tests passent.");
    return 0;
}
