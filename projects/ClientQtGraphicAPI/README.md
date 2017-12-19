# RDP Qt Client

Le ClientQtGraphicAPI est une couche graphique Qt pour client RDP qui assure l'affichage 
d'ordre de tracé RDP et capture les entrées souris et clavier.
Cette API est compatible avec un module de redemption (mod_rdp, mod_vnc...).

Un client Qt RDP utilisant cette API existe. Il permet de tester des connexions en utilisant
mod_rdp de façon simple et rapide en affichant. Ce client est également utile pour tester les
sous protocole (copier/coller, partage de disque...).



.) Interface graphique des fenêtre de l'API client
---------------------------------------------------


La fenêtre de connexion
-----------------------

Un formulaire composé de 4 champs:
	IP server: renseigne l'IP de la cible.
	User name: renseigne le nom du user du compte de la cible.
	Password: renseigne le mot de passe du compte de la cible.
	Port: renseigne le port de connexion à la cible.
Un bouton "Connection" qui permet de lancer une connexion à une cible et d'ouvrir l'écran
de session.
Un bouton "Replay" qui permet de selectionner une film RDP en format .mwrm puis de le lire.
L'écran de session va alors s'ouvrir et afficher le contenu du film.
Un bouton "Options" permet d'ouvrir une fenêtre d'interface des options du client. Cette fenêtre
n'est pas contenu dans l'API et necessite d'être implémenter.


L'écran de session
------------------

L'écran de session est une fenêtre qui permet d'afficher les ordres de tracé RDP et de capturer
les entrées claviers et souris de façon reproduire une session RDP.dont la taille s'adapte à la
résolution du client.
Il y a 3 boutons en bas de la fenêtre:
	"CTRL + ALT + DELETE" qui envoie le signale de la pression de ces 3 touches à la session windows
	distante.
	"Refresh" envoie à la cible une requête de mise à jour de la totalité de l'écran.
	"Disconnection" deconnecte la session, ferme l'écran de session et fait un retour sur le formulaire
	de connexion.
Lors de la lecture de film .mwrm, une barre de lecture horizontale permet de suivre la lecture et de sauter
à différent moment de la vidéo. Les boutons précedemment cités sont remplacé par des boutons "Play", "Stop" et "Pause".



.) Compilation du client Qt RDP
--------------------------------

Prerequies
----------
	
Installer les lib Qt4 ou Qt5.

To compile ReDemPtion you need the following packages:
- libboost-tools-dev (contains bjam: software build tool) (http://sourceforge.net/projects/boost/files/boost/)
- libboost-test-dev (unit-test dependency)
- libssl-dev
- libkrb5-dev
- libsnappy-dev
- libpng12-dev
- libffmpeg-dev (see below)
- g++ >= 4.9 or clang++ >= 3.5 or other C++14 compiler

Compilation
-----------

Pour le compiler il faut se placer dans le dossier redemption/project/ClientQtGraphicAPI:

Pour compiler avec la bibliothèque Qt4:
	$ bjam client_rdp_Qt -s qt=4

Pour compiler avec la bibliothèque Qt5:
	$ bjam -s qt=5 client_rdp_Qt



.) Utilisation du client Qt RDP
--------------------------------

Le client Qt RDP contient une implementation de mod_rdp ainsi qu'une boite de dialogue pour renseigner
les options de la session RDP.


Virtual channels
----------------

Le client Qt RDP implémente plusieurs sous protocols RDP passant par des cannaux virtuels:
	Le copier/coller
	Le partage de disque dure local
	Le canal son
	Le canal du partage de l'imprimante (n'est pas fonctionnel mais les logs sont implémentés)


Lancement en ligne de commande de l'exe client_rdp_Qt4
------------------------------------------------------

Commandes de connexion:
	-n [user_name]   renseigne le nom de l'user du compte de la cible.
	-w [user_password]   renseigne le mot de passe de l'user du compte de la cible.
	-i [target_IP]   resenigne l'adresse IP de la cible.
	-p [port]renseigne le port de connexion à la cible.

Commandes verbose:
	--cliprdr 	 affiche le contenu des PDU reçu et envoyé par le client sur le channel du copier/coller.
	--cliprdr_dump	 idem que --cliprdr en ajoutant le dump des données brutes du PDU.
	--rdpdr  affiche le contenu des PDU reçu et envoyé par le client sur le channel du partage de disque. 
	--rdpdr_dump idem que --rdpdr en ajoutant le dump des données brutes du PDU.
	--rdpsnd affiche le contenu des PDU reçu et envoyé par le client sur le channel son.
	--graphics	 affiche des informations sur les ordres de tracés reçus par le client.	
	--printeraffiche le contenu des PDU reçu et envoyé par le client sur le channel de 
			 partage de l'imprimante(non implémenté). 
	--basic_traceaffiche des logs du mod_rdp.
	--connection	 affiche des logs du mod_rdp concernant la connexion à la cible.
	--rail_order	 affiche les logs du rail order du mod_rdp.
	--asynchronous_task  affiche les logs des fonctions des tâches asynchrones du mod_rdp.
	--capabilities   affiche les logs des capabilities lors de la négociation entre mod_rdp et cible.
	--keyboard	 affiche les logs des entrées claviers.
	--rail		 affiche les logs du cannal rail.
	--rail_dump	 affiche les logs du cannal rail ainsi que le contenu brute des rail PDU.


Boite de dialogue des options du client Qt RDP
----------------------------------------------

La boite de dialogue se divise en 4 onglets:
	
	Onglet "General":
	- Permet d'enregistrer et de charger un profil d'option
	- La checkbox "Record movie", si coché, les session seront enregistrées en format .mwrm
	- La checkbox "TLS" active le TLS lors de la connexion
	- La checkbox "NLA" active le NLA lors de la connexion

	Onglet "View":
	- Des combobox permettent de selectionner la profondeur de couleur, le nombre d'écran ainsi que la résolution.
	- La checkbox "Span screen" adapte la taille de la fenêtre à celle du moniteur.
	- la checkbox "Disable wallaper" permet de désactivé le fond d'écran de la cible (pour raison de performance).

	Onglet "Services"
	- Des checkbox permettent d'activer les cannaux virtuel de partage de disque et de copier/coller.
	- Un champs permet de spécifier le fichier partagé par le protocole de partage de disque.

	Onglet "Keyboard"
	- Une combobox permet de spécifier la langue du clavier
	- Un tableau permet de configurer des touches du clavier (le Qt scan code et nécessaire ainsi que le scan code ou 
	  le code ascii de la touche.
	


.) Implementation d'un client à partir de la couche graphique Qt
-----------------------------------------------------------------

L'API consiste à créer des clients composés de deux élément principaux: 
	La partie front qui implémente le front de l'API graphique Qt en RDP.
	La partie mod qui implémente un mod de connexion à la cible de redemption.


Implémentation du Front
-----------------------

Il faut créer une classe front qui inclue dans un fichier
   redemption/project/ClientQtGraphicAPI/src/rdp_client_graphic_api/front_qt_rdp_graphic_api.hp
et hérite de la classe FrontQtRDPGraphicAPI.
La fonction 
    // virtual mod_api * init_mod()
doit être réimplémenté pour initialisé et retourner la variable 'mod' (utiliser le mod de redemption de votre choix)

en option il est possible de reimplémenter les fonction suivantes:

    //  void callback() override {
    //   FrontQtRDPGraphicAPI::callback();
    //  }
	
callback() est appelée quand il y a un event socket, il est nécessaire de continuer à appeler 
l'implémentation FrontQtRDPGraphicAPI::callback() dans l'override.

    //  void connect() override {
    //   FrontQtRDPGraphicAPI::connect();
    //  }

connect() est appelée quand le bouton "connection" est pressé, il est nécessaire de continuer à appeler 
l'implémentation FrontQtRDPGraphicAPI::connect() dans l'override.

    //  void options() override {}

options() est appelée quand le bouton "options" est pressé, l'appel à l'implémentation de la classe mère 
n'est pas nécessaire.

	Il y a un exemple d'implémentation avec mod_vnc dans 
		redemption/project/ClientQtGraphicAPiI/src/frontDemoQtClient.cpp


La fonction "main" avec Qt
-------------------------- 

La fonction "main" devra être de la forme suivante:
	- Appel de QApplication::QApplication(int argc, char** argv)
	- Appel du constructeur de votre classe front (celle qui hérite de la classe FrontQtRDPGraphicAPI)
- Appel de QApplication::exec()

Exemple:
    // int main(int argc, char** argv){

    //		QApplication app(argc, argv);

    //		VotreFront votreFront();

    //		app.exec();
    // }


Compilation avec bjam
---------------------

Dans le fichier 
	redemption/projects/ClientQtGraphicAPI/Jamroot
ajouter les lignes suivantes:
   
	$exe votre_client_exe :
	$src/rdp_client_graphic_api/Qt4/Qt.hpp
	$src/rdp_client_graphic_api/front_qt_rdp_graphic_api.hpp
	$src/rdp_client_graphic_api/keymaps/qt_scancode_keymap.hpp
	$[Optionnel: chemins vers les fichiers .hpp contenant des classes dérivés de QObject si vous en avez ajouté]
	$[nom_du_fichier_qui_contient_votre_class_front.hpp]
	$$(obj_list)
	$$(lib_list)
	$config.o
	$hexdump.o
	$$(obj_list_VNC)

	$[/qt//QtGui] pour Qt4 ou  [/qt5//QtGui
					/qt5//QtNetwork
					/qt5//QtWidgets] Pour Qt5
	$:
	$$(EXE_DEPENDENCIES)

Enfin compiler votre client à l'aide de la commande suivante:
	$ bjam votre_client_exe


