# Gestion de comptes bancaires en C

Application en ligne de commande pour gérer des comptes bancaires, avec sauvegarde dans un fichier CSV.

## Fonctionnalités

- Ouverture de comptes, dépôts, retraits, virements
- Montants stockés en **centimes** (`long long`) : aucune erreur d'arrondi des `float`
- Validation des entrées (montants, identifiants, noms)
- Sauvegarde **atomique** : écriture dans un fichier temporaire puis `rename`, le fichier n'est jamais à moitié écrit
- Codes d'erreur explicites (`BankStatus`)

## Utilisation

```bash
make
./banque comptes.csv ouvrir Alice
./banque comptes.csv ouvrir Bob
./banque comptes.csv depot 1 250.00
./banque comptes.csv virement 1 2 75,50
./banque comptes.csv liste
```

```
#1    Alice                    174.50 EUR
#2    Bob                       75.50 EUR
```

## Format du fichier

Une ligne par compte : `id,titulaire,solde_centimes`

```
1,Alice,17450
```

## Tests

```bash
make test
```
