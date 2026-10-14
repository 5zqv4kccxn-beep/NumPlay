# NumPlay — lecteur de cours embarqués

Cette variante contient un seul artefact `NumPlay.nwa` et aucun jeu. Au lancement, elle affiche un tableau de travail. La touche **Pi** ouvre les matières Chimie, Maths et Physique. Les chapitres sont intégrés dans `launcher/src/reader_docs.h`, généré à partir des documents de `docs/cours/`.

## Compilation

Avec la toolchain et nwlink utilisés par la CI :

```sh
make clean
make -j4 nwa check NWLINK=nwlink
```

Le résultat est `build/NumPlay.nwa`. Le Makefile conserve le préfixe de recette `.RECIPEPREFIX := >` et ne produit qu'une variante.

## Navigation

- Écran tableur : Pi ou OK ouvre les matières.
- Matières / sommaire : haut, bas, OK.
- Sommaire : droite ouvre le cours complet, gauche revient.
- Lecture : haut/bas défile, gauche revient.

Le fichier de configuration et le mécanisme de touche secrète existants n'ont pas été réécrits.
