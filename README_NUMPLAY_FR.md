# NumPlay — lecteur de cours (version sans jeux)

Cette variante retire les 25 jeux du dépôt, leurs ressources et les entrées de build/menu. La touche secrète, les paramètres et l’écran Matrices/tableur d’origine sont conservés sans modification. Après ouverture par la touche secrète configurée, NumPlay affiche une liste de trois documents : **Chimie — chapitre 1**, **Chimie — chapitre 2** et **Maths-Physique**.

## Commandes du lecteur

- Haut/Bas : choisir un document ou faire défiler ;
- OK/EXE : ouvrir ;
- Back : revenir à la liste ;
- Home/On-Off : quitter.

Le texte des PDF fournis est extrait par `pdftotext -layout`, normalisé en ASCII pour la police intégrée, puis embarqué dans `launcher/src/reader_docs.h`.

## Compilation

Pré-requis : `make`, `arm-none-eabi-gcc`, `arm-none-eabi-ld`, `arm-none-eabi-objcopy`, `arm-none-eabi-strip`, Python 3 et Node.js/npm avec `nwlink`.

```sh
make nwa       # build/NumPlay.nwa et les variantes
make check     # affiche les tailles
make clean
```

Dans l’environnement de génération utilisé ici, `node` est présent mais `make` et la toolchain ARM ne le sont pas ; aucun `.nwa` n’a donc pu être compilé.
