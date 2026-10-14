NumPlay — variante Tableur

Projet prêt à déposer sur GitHub (aucun envoi réseau effectué).

Fichiers modifiés
- Makefile : variante unique Tableur, nom de sortie build/Tableur.nwa, nom d'application Tableur et icône launcher/assets/icon.png.
- launcher/app_name.c : libellé d'application conservé via NP_APP_NAME, commentaire mis à jour.
- launcher/src/home.c : accueil sans « Pi : Matières » ; sommaire structuré Chimie (2 chapitres), Maths (6 chapitres) et Physique (4 chapitres) ; affichage des cours plus compact ; tampon de saisie de la séquence 2011 ouvrant directement le menu des matières.
- launcher/src/np.h : constantes internes pour les chiffres utilisés par le tampon 2011.
- launcher/src/platform.c : remontée des touches 0, 1 et 2 vers le lecteur.
- README-MODIFICATIONS.txt : ce relevé.

Cours
Le sommaire et le contenu de launcher/src/reader_docs.h s'appuient sur les cours déjà intégrés au projet : chimie corps purs/mélanges et solutions, ainsi que les chapitres de mathématiques et physique présents dans le document maths-physique.

Contraintes respectées
- Les paramètres et la touche secrète Matrices existante n'ont pas été modifiés.
- launcher/assets/icon.png est laissé en place. L'utilisateur doit le remplacer par son icône définitive.
- Aucun fichier n'a été envoyé sur Internet ou GitHub.
