# Arcsight LOG SIEM

## Reference

https://www.secef.net/wp-content/uploads/sites/10/2017/04/CommonEventFormatv23.pdf

https://kc.mcafee.com/resources/sites/MCAFEE/content/live/CORP_KNOWLEDGEBASE/78000/KB78712/en_US/CEF_White_Paper_20100722.pdf


## Introduction

Le Common Event Format d'arcsight est un format de log standard visant à être interopérable.
Ce format est celui intégré par ArcSight ESM, un outil de collecte de donnée de log de sécurité.
Il est structuré par une suite de champs prédéfinis (ou custom) contenant des integers ou des strings.


### Common Event Format (CEF)

Les logs au format Arcsight commencent tous par un entête de même structure:

    [Date] host message CEF:[Version] | [Device_Vendor] | [Device_Product] | [Device_Version] | [SignatureID] | [Name] | [Severity] | [Extension]


- `Date`: Différent format de date sont autorisées.
- `Version`: Version du format arcsight utilisée.
- `SignatureID`: n'a pas d'équivalent dans les logs actuels du WAB, il suffira d'ajouter un ID en parallèle du EventName.
- `Name`: En format arcsight le nom de l'événement est simplement le nom de l'action, ceux du WAB actuel contienne trop d'information. Par exemple au lieu de `Connection success`, au format arcsight on ne gardera que `Connection`, on utilisera un autre champs pour `success`.
- `Severity`: n'existe pas dans les logs actuels, désigne la sévérité de l'événement sur une échelle de 1 à 10.
- `Extension`: contient les autres champs. Ils peuvent être définis dans le dictionnaire Arcsight ou non. On les écrit en commençant `key name` du champ (ex: `duser=adm!in`), séparés par un espace.


Exemple:

    Sep 19 2018 08:26:10 host CEF:1|Wallix|WAB|6.0.1|0x0000254f|connection|5|suser=admin


### Custom field

Les champs custom doivent s'écrire selon la forme suivante:

    [Device_Vendor][Device_Product][CustomeName]

Exemple:

    Sep 19 2018 08:26:10 host CEF:1|Wallix|WAB|6.0.1|0x0000254f|connection|5|suser=admin WallixWABstatus=success


### Encodage des strings

L'encodage sera toujours UTF-8 avec quelques spécificités:

- Les champs sont séparés par des `|`
- Sauf pour le champ `Extension`, `|` devient `\|`
- `=` devient `\=` uniquement pour le champ `Extension`
- `\`  devient `\\`
- un saut de ligne devient `\n` ou `\r`. Seulement autorisé dans `Extension`



## Correspondance des champs wab log siem et arcsight

Dans le tableau ci-dessous, les `Key Name` marquées custom sont des champs du WAB qui n'ont pas de correspondance sémantique avec arcsight (liste non exhaustive). Cette liste pourra être modifiée et complétée à l'avenir.

  WAB Key Name  | ArcSight Key Name |       Full Name       | Définition des champs sur le WAB
----------------|-------------------|-----------------------|----------------------------------
objet           | custom            |                       | Nom de l'objet de config modifié
type (WABAUDIT) |  custom           |                       | Type de l'objet de config modifié
user            |  suser**          | sourceUserName**      | Nom du compte primaire
info            |  msg*             | message*              | Info diverses...
type (SSH, RDP) |  Name             | Name                  | Nom de l'événement ou de l'action (contenu dans le CEF)
session_id      |  custom           |                       | ID de la session du compte primaire
client_ip       |  src**            | sourceAdress**        | Adresse du client
target_ip       |  dst              | destinationAdress     | Adresse de la cible
device          |  dhost            | destinationHostName   | Nom de la cible
service         |  app              | applicationProtocol   | Nom du protocole utilisé
account         |  duser            | destinationuser       | Nom du compte secondaire
pattern         |  custom           |                       | Regex
data            |  msg*             | message*              | Données diverses...
desciption      |  msg*             | message*              | Description diverse...
command_line    |  custom           |                       | Ligne de commande détecté par la session probe
command         |  custom           |                       | Ligne de commande
status          |  custom           |                       | Résultat d'une action ou état d'un objet
source          |  dproc            | destinationProcessName| Processus à l'origine de l'information
file_name       |  fname            | fileName              | Nom d'un fichier
window          |  custom           |                       | Titre d'une fenêtre window

`*` `sourceUserName` désigne le nom du compte primaire sur le WAB tandis que `sourceAdress` désigne l'IP du client. Ceci est plutôt ambigu sur ce que désigne la source. Par opposition `stinationHostName`, `destinationAdress` et `destinationuser` où le mot `destination` implique toujours la cible.

`**` `desciption`, `data` et `info` sont des strings contenant des données peu non structurées, ce qui correspond bien au concept du champ `message` avec Arcsight. Néanmoins, il sera peut-être nécessaire de répartir le contenu de ces 3 champs autrement.

Pour les LOG RDP et LOG SSH, les entêtes `[RDP Session]` et `[SSH Session]` sont redondantes avec le champ service (`applicationProtol` pour Arcsight).


Exemple:

LOG WAB AUDIT actuel:

    [wabaudit] action="view" type="session"
    object="150fc97a1a07e596000c29812e63" user="admin"
    client_ip="192.168.140.1" infos="Username ['admin'], Secondary
    ['administrator@acme.net@win2k16.acme.net'], Protocol ['RDP'],
    ClientIP ['192.168.140.1']"

Devient en format Arcsight:

    Sep 19 2018 08:26:10 host CEF:1|Wallix|WAB|6.0.1|0x0000232f|view|5|suser=admin src=192.168.140.1 WallixWABobject=150fc97a1a07e596000c29812e63
    WallixWABtype=session msg=Username ['admin'], Secondary\n
    ['administrator@acme.net@win2k16.acme.net'], Protocol ['RDP'],\n
    ClientIP ['192.168.140.1']

LOG SSH actuel:

    [SSH Session] type='CHANNEL_EVENT' session_id='002ac1d68450742e1928b88df3ca15385d710b33'
    client_ip='192.168.1.10' target_ip='192.168.1.200' user='maint' device='debian' service='ssh'
    account='admin' data='AuthAgent Open Success'

Devient en format Arcsight:

    Sep 19 2018 08:26:10 host CEF:1|Wallix|WAB|6.0.1|0x0000782f|CHANNEL_EVENT|5|suser=maint src=192.168.1.10 duser=admin dst=192.168.1.200
    dhost=debian app=ssh WallixWABsession_id=SESSIONID­0000  msg=AuthAgent Open Success

LOG RDP actuel:

    [RDP Session] type="DRIVE_REDIRECTION_READ" session_id="SESSIONID­0000" client_ip="192.168.1.10"
    target_ip="192.168.1.200" user="maint" device="win2k8" service="rdp" account="Maintenance"
    file_name="home/out.txt"

Devient en format Arcsight:

    Sep 19 2018 08:26:10 host CEF:1|Wallix|WAB|6.0.1|0x0000782f|DRIVE_REDIRECTION_READ|5|suser=maint duser=Maintenance WallixWABsession_id=SESSIONID­0000
    src=192.168.1.10 dst=192.168.1.200 dhost=win2k8 app=rdp fname=home/out.txt


## CONCLUSION

Beaucoup de champs des logs de SIEM du WAB ne sont pas ou difficilement interprétable par le dictionnaire de champs Arcsight. Ce problème requiert l'utilisation de nombreux champs customisés et donc des possibilités d'exploitation par l'outil ArcSight ESM limitées. C'est particulièrement bloquant pour les logs WAB AUDIT, où `objet` et `type` sont des champs primordiaux pour en comprendre le sens. Les utilisateurs d'outils acceptant ce format devront donc s'adapter aux champs
spécifiques au WAB, sinon l'interopérabilité de logs restera toute relative.
