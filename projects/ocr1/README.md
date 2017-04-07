# OCR

- Comment faire l'apprentissage sur un (nouveau) jeu de caractères ?
Les définitions de caractères sont dans learn.ok sous la forme de fichiers images .pbm
Les fichier .txt associé correspond au texte présent dans l'image.
Chaque caractères ou ensemble de caractères est séparer par un espace.
Note: '\' est un caractère d'échapement, le doubler pour le prendre en compte.

- Quel est la procédure de génération du module de reconnaissance de caractères ?
$ `make`
cette commande génère des fichier **.hxx** et les dossiers `classifiers` et `fonts`.
Les fichiers `{,classifiers/}*.hxx` sont utilisé par l'ocr.

/!\ **Ne pas oublier de modifier** `whitespace_width` **lorsqu'une police est ajoutée**.

En cas d'erreurs avec learning, display_learning permet de voir les caractères extrait.

- Comment lancer la reconnaissance de caractères sur une image fixe ?
Utiliser le programme extract_text. Celui-ci ne supporte que que les extensions ppm.
Note: Il existe des programmes comme bmptoppm ou pngtopnm dans le gestionnaire de paquets
(`pngtopnm file.png > file.pbm`).



# BUGS

Certains caractères sont reconnu en tant que plusieurs caractères.
(par exemple `ï` en Tahoma est reconnu en tant que  `.i.`).
Ceci est un "bug" de learning et la classification d'un tel caractère ammène une erreur.

Pour corriger le problème un fichier `nom_de_la_police.repl` doit être créé dans learn.ok.
Celui-ci contient les caractères de remplacement (code c++):

`{"sequence à trouver", "séquence de remplacement"},`



# Win 2012 et smoothing

`tools/ocr/img_win2012_smoothing_to_monochrome input_image output_image`
  transforme une image de barre de titre windows2012 standard en une image noir et blanc

`tools/ocr/img_win2012_smoothing_to_monochrome_d input_images`
  prend plusieurs fichier en paramètre et appel le script precédent avec une extension
  de sortie en .monochrome + en .pbm + un fichier .txt contenant le nom du fichier (il
  faut que le fichier soit nommé par les lettres dans l'image avec un espace séparateur
  entre chaque).
  À noter qu'il faut etre dans le dossier des images car le chemin complet est utilisé
  comme texte dans le fichier .txt

(Note: les couleurs en rdp et vnc sont différentes, pour avoir la liste complète des
couleurs consitutant une image: `identify -verbose -unique mon_image`.)

Questions OCR (OBSOLETE)
-----------------------

- Qu'est-ce que Milena ?

Milena est une bibliothèque générique de traitement d'image développée par le LRDE
(laboratoire de recherche dépendant de l'Epita). Milena fait partie du projet
Olena, et c'est aussi le nom des bibliothèques core d'Olena.

- Quels sont les codes livrés par l'Epita spécifiques à notre projet ?

La livraison de l'epita pour redemption comporte:
- Un Makefile -> a merger dans le Jamroot de redemption
- des fichiers README et INSTALL intégrés à OCR.txt (ce fichier) et supprimés.

- les jeux de définition de caractères : dossier learn.ok

- les fichiers source python suivants:

cart.py -> utilisé par classifieur
gen_classifier.py -> crée l'include classifier.hxx utilisé par classfier.hh
 gen_classifier utilise le contenu de features.txt, lui même construit à
 partir du contenu du dossier learn à l'aide du programme learning.

- les sources C++ suivant

classification.hh
classifier.hh
extract_bars.hh -> extraction des barres de titre d'une image
extract_text.cc
extract_text.hh -> extraction du texte contenu dans les barres de titre
learning.cc -> exploite le contenu de learn (image pbm et codes caractères) pour
            créer le fichier features.txt utilisé par gen_classifier.py


- Quels sont les codes générique Milena utilisés ?

L'OCR utilise la version de milena contenue dans le dossier milena, c'est à peu
de choses près [différences ?] la même chose que le contenu du dossier milena
de olena-1.0. L'archive olena-1.0 est conservée dans le dossier à titre de
référence, mais pas directement utilisée lors de la phase de compilation.

- Où sont les fichiers de définition de jeux de caractères ?

- Comment faire l'apprentissage sur un (nouveau) jeu de caractères ?

Les définitions de caractères sont dans learn (lien vers learn.ok) sous la forme
de fichiers images .pbm
Les fichier .txt associé correspond au texte présent dans l'image.
Chaque caractères ou ensemble de caractères est séparer par un espace.
Note: '\' est un caractère d'échapement, le doubler pour le prendre en compte.

- Quel est la procédure de génération du module de reconnaissance de caractères ?
make apprentissage

- Comment compiler Redemption avec le support de l'OCR ?

- Comment lancer la reconnaissance de caractères sur une image fixe ?
voir à "make test".

- Comment appeler la reconnaissance de caractères depuis un buffer ?



README ORIGINEL (OBSOLETE)
--------------------------

Commandes:
+ make all: compilation du programme et apprentissage.
+ make apprentissage: apprentissage seul à partir des fichiers dans
"learn"
+ make test: lancement de la procédure de test. Tout les screens se
trouvant dans le dossier "train" sont analysés puis traduit en texte.
+ make dist: génère une archive de rendu.


* extract_bars:
Détection des lignes de titres.

** Input: un screenshot ppm
** Output: plusieurs images titres pbm.
** Principe:
   + Calcul de composantes alpha-connexes à une couleur réference
   (alpha-connexion permet de gérer le dégradé)
   + Reconstruction de l'image ou on met en blanc le texte (est
   considéré comme texte tout ce qui est proche du blanc (pour l'aliasing)


* classify:
Découpage des caractères et classification.

** Input: une barre de titre
** Output: le texte du titre
** Principe:
   + calculs des composantes connexes et attributs (bbox, aire)
   + merge des composantes qui ont des points communs lors de la
   projection sur x (permet de gérer les accents)
   + classification

* classifier.cc_:
  L'arbre de décision généré.

* gen_classifier.py, cart.py:
  Construction de l'arbre de décision.
  Il faut mettre des fichiers pbm et leurs traductions dans le dossier
  "learn". Ensuite "make apprentissage" s'occupe de créer l'arbre de
  décision et créer le code c++ correspondant dans le fichier classifier.cc_.

* test.sh:
  Mettre tout plein de fichiers dans "train" et regarder les résultats
  dans "output"


* utilitaire

 recode utf-8..latin-1 < tmp > tmp2


 INSTALL ORIGINEL (OBSOLETE)
 ---------------------------

 INSTALLATION

	Instructions

I. Milena

1. Telecharger et installer milena
2. Modifier le makefile courrant pour indiquer le chemin de milena
3. Realiser l'apprentissage:
   make apprentissage


II. Integration

1. Aller dans le repertoire de redemption et compiler.
   cd integration
   make
2. Configurer utils/authhook.py puis
   python ./utils/authhook.py
   ./rdpproxy
