#include "bank.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void bank_init(Bank *bank) {
    bank->count = 0;
    bank->next_id = 1;
}

const char *bank_status_message(BankStatus status) {
    switch (status) {
    case BANK_OK: return "OK";
    case BANK_ERR_NOT_FOUND: return "compte introuvable";
    case BANK_ERR_INSUFFICIENT_FUNDS: return "solde insuffisant";
    case BANK_ERR_INVALID_AMOUNT: return "montant invalide";
    case BANK_ERR_FULL: return "nombre maximal de comptes atteint";
    case BANK_ERR_INVALID_NAME: return "nom de titulaire invalide";
    case BANK_ERR_IO: return "erreur de lecture/écriture du fichier";
    }
    return "erreur inconnue";
}

static int valid_name(const char *name) {
    size_t len = strlen(name);
    if (len == 0 || len >= MAX_NAME) return 0;
    for (const char *p = name; *p; p++)
        if (*p == ',' || *p == '\n' || *p == '\r') return 0; /* casserait le CSV */
    return 1;
}

BankStatus bank_open_account(Bank *bank, const char *owner, int *out_id) {
    if (!valid_name(owner)) return BANK_ERR_INVALID_NAME;
    if (bank->count >= MAX_ACCOUNTS) return BANK_ERR_FULL;
    Account *a = &bank->accounts[bank->count++];
    a->id = bank->next_id++;
    strcpy(a->owner, owner);
    a->balance_cents = 0;
    if (out_id) *out_id = a->id;
    return BANK_OK;
}

Account *bank_find(Bank *bank, int id) {
    for (size_t i = 0; i < bank->count; i++)
        if (bank->accounts[i].id == id) return &bank->accounts[i];
    return NULL;
}

BankStatus bank_deposit(Bank *bank, int id, long long amount_cents) {
    if (amount_cents <= 0) return BANK_ERR_INVALID_AMOUNT;
    Account *a = bank_find(bank, id);
    if (!a) return BANK_ERR_NOT_FOUND;
    a->balance_cents += amount_cents;
    return BANK_OK;
}

BankStatus bank_withdraw(Bank *bank, int id, long long amount_cents) {
    if (amount_cents <= 0) return BANK_ERR_INVALID_AMOUNT;
    Account *a = bank_find(bank, id);
    if (!a) return BANK_ERR_NOT_FOUND;
    if (a->balance_cents < amount_cents) return BANK_ERR_INSUFFICIENT_FUNDS;
    a->balance_cents -= amount_cents;
    return BANK_OK;
}

BankStatus bank_transfer(Bank *bank, int from_id, int to_id, long long amount_cents) {
    if (amount_cents <= 0 || from_id == to_id) return BANK_ERR_INVALID_AMOUNT;
    Account *from = bank_find(bank, from_id), *to = bank_find(bank, to_id);
    if (!from || !to) return BANK_ERR_NOT_FOUND;
    if (from->balance_cents < amount_cents) return BANK_ERR_INSUFFICIENT_FUNDS;
    from->balance_cents -= amount_cents;
    to->balance_cents += amount_cents;
    return BANK_OK;
}

BankStatus bank_load(Bank *bank, const char *path) {
    bank_init(bank);
    FILE *f = fopen(path, "r");
    if (!f) return BANK_OK; /* fichier absent = banque vide */

    char line[256];
    while (fgets(line, sizeof line, f)) {
        Account a;
        char owner[MAX_NAME];
        if (sscanf(line, "%d,%63[^,],%lld", &a.id, owner, &a.balance_cents) != 3
            || bank->count >= MAX_ACCOUNTS || a.balance_cents < 0) {
            fclose(f);
            return BANK_ERR_IO;
        }
        strcpy(a.owner, owner);
        bank->accounts[bank->count++] = a;
        if (a.id >= bank->next_id) bank->next_id = a.id + 1;
    }
    fclose(f);
    return BANK_OK;
}

BankStatus bank_save(const Bank *bank, const char *path) {
    /* Écriture dans un fichier temporaire puis renommage : on ne corrompt jamais l'ancien fichier. */
    char tmp[512];
    if (snprintf(tmp, sizeof tmp, "%s.tmp", path) >= (int)sizeof tmp) return BANK_ERR_IO;
    FILE *f = fopen(tmp, "w");
    if (!f) return BANK_ERR_IO;
    for (size_t i = 0; i < bank->count; i++) {
        const Account *a = &bank->accounts[i];
        fprintf(f, "%d,%s,%lld\n", a->id, a->owner, a->balance_cents);
    }
    if (fclose(f) != 0 || rename(tmp, path) != 0) { remove(tmp); return BANK_ERR_IO; }
    return BANK_OK;
}

int parse_amount(const char *text, long long *out_cents) {
    long long euros = 0, cents = 0;
    const char *p = text;
    if (!isdigit((unsigned char)*p)) return 0;
    for (; isdigit((unsigned char)*p); p++) {
        euros = euros * 10 + (*p - '0');
        if (euros > 1000000000LL) return 0; /* plafond : 1 milliard d'euros */
    }
    if (*p == '.' || *p == ',') {
        p++;
        int digits = 0;
        for (; isdigit((unsigned char)*p) && digits < 2; p++, digits++) cents = cents * 10 + (*p - '0');
        if (digits == 0) return 0;
        if (digits == 1) cents *= 10;
    }
    if (*p != '\0') return 0;
    *out_cents = euros * 100 + cents;
    return *out_cents > 0;
}
