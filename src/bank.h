#ifndef BANK_H
#define BANK_H

#include <stddef.h>

#define MAX_ACCOUNTS 1000
#define MAX_NAME 64

/* Les montants sont stockés en centimes pour éviter les erreurs d'arrondi des flottants. */
typedef struct {
    int id;
    char owner[MAX_NAME];
    long long balance_cents;
} Account;

typedef struct {
    Account accounts[MAX_ACCOUNTS];
    size_t count;
    int next_id;
} Bank;

typedef enum {
    BANK_OK = 0,
    BANK_ERR_NOT_FOUND,
    BANK_ERR_INSUFFICIENT_FUNDS,
    BANK_ERR_INVALID_AMOUNT,
    BANK_ERR_FULL,
    BANK_ERR_INVALID_NAME,
    BANK_ERR_IO
} BankStatus;

void bank_init(Bank *bank);
const char *bank_status_message(BankStatus status);

BankStatus bank_open_account(Bank *bank, const char *owner, int *out_id);
Account *bank_find(Bank *bank, int id);
BankStatus bank_deposit(Bank *bank, int id, long long amount_cents);
BankStatus bank_withdraw(Bank *bank, int id, long long amount_cents);
BankStatus bank_transfer(Bank *bank, int from_id, int to_id, long long amount_cents);

/* Format CSV : id,titulaire,solde_centimes */
BankStatus bank_load(Bank *bank, const char *path);
BankStatus bank_save(const Bank *bank, const char *path);

/* "12.34" -> 1234. Retourne 0 si le texte n'est pas un montant positif valide. */
int parse_amount(const char *text, long long *out_cents);

#endif
