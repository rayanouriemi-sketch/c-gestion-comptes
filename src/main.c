#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bank.h"

static Bank bank; /* ~80 Ko : en statique plutôt que sur la pile */

static void usage(const char *prog) {
    fprintf(stderr,
            "Usage : %s <fichier.csv> <commande> [arguments]\n"
            "  ouvrir <nom>                    ouvre un compte\n"
            "  depot <id> <montant>            dépose de l'argent (ex : 12.50)\n"
            "  retrait <id> <montant>          retire de l'argent\n"
            "  virement <de> <vers> <montant>  transfère entre deux comptes\n"
            "  solde <id>                      affiche un solde\n"
            "  liste                           liste tous les comptes\n",
            prog);
}

static int parse_id(const char *text, int *out) {
    char *end;
    long v = strtol(text, &end, 10);
    if (*end != '\0' || v <= 0 || v > 1000000) return 0;
    *out = (int)v;
    return 1;
}

static void print_account(const Account *a) {
    printf("#%-4d %-20s %10lld.%02lld EUR\n", a->id, a->owner, a->balance_cents / 100, a->balance_cents % 100);
}

static BankStatus run(int argc, char **argv, int *modified) {
    const char *cmd = argv[2];
    int id, to;
    long long amount;
    *modified = 0;

    if (strcmp(cmd, "liste") == 0 && argc == 3) {
        for (size_t i = 0; i < bank.count; i++) print_account(&bank.accounts[i]);
        return BANK_OK;
    }
    if (strcmp(cmd, "ouvrir") == 0 && argc == 4) {
        BankStatus s = bank_open_account(&bank, argv[3], &id);
        if (s == BANK_OK) { printf("Compte #%d ouvert pour %s\n", id, argv[3]); *modified = 1; }
        return s;
    }
    if (strcmp(cmd, "solde") == 0 && argc == 4 && parse_id(argv[3], &id)) {
        Account *a = bank_find(&bank, id);
        if (!a) return BANK_ERR_NOT_FOUND;
        print_account(a);
        return BANK_OK;
    }
    if ((strcmp(cmd, "depot") == 0 || strcmp(cmd, "retrait") == 0) && argc == 5 && parse_id(argv[3], &id)) {
        if (!parse_amount(argv[4], &amount)) return BANK_ERR_INVALID_AMOUNT;
        BankStatus s = cmd[0] == 'd' ? bank_deposit(&bank, id, amount) : bank_withdraw(&bank, id, amount);
        if (s == BANK_OK) { print_account(bank_find(&bank, id)); *modified = 1; }
        return s;
    }
    if (strcmp(cmd, "virement") == 0 && argc == 6 && parse_id(argv[3], &id) && parse_id(argv[4], &to)) {
        if (!parse_amount(argv[5], &amount)) return BANK_ERR_INVALID_AMOUNT;
        BankStatus s = bank_transfer(&bank, id, to, amount);
        if (s == BANK_OK) { print_account(bank_find(&bank, id)); print_account(bank_find(&bank, to)); *modified = 1; }
        return s;
    }
    usage(argv[0]);
    exit(2);
}

int main(int argc, char **argv) {
    if (argc < 3) { usage(argv[0]); return 2; }

    BankStatus s = bank_load(&bank, argv[1]);
    if (s != BANK_OK) { fprintf(stderr, "Erreur : %s (%s)\n", bank_status_message(s), argv[1]); return 1; }

    int modified;
    s = run(argc, argv, &modified);
    if (s == BANK_OK && modified) s = bank_save(&bank, argv[1]);
    if (s != BANK_OK) { fprintf(stderr, "Erreur : %s\n", bank_status_message(s)); return 1; }
    return 0;
}
