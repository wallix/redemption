layout: true
---

class: center, middle

# Événement clavier

--
- Fonctionnement de RDP
- De JS vers scancode RDP

---

## Fonctionnement des claviers en RDP

--

Une disposition de clavier (ou layout) est constituée:

- d'un nom local
- d'un identifiant de clavier unique (KLID)
- d'un identifiant de layout (qui n'est pas unique)
- d'un ensemble de **keymap**

--

`US Layout` et `Chinese (Simplified) - US Layout`

http://kbdlayout.info/00000804/

<table>
<tr><td>KLID                <td>00000409 (en-US)
<tr><td>Layout Display Name <td>US
<tr><td>Layout Id           <td>4
</table>

<br/>

<table>
<tr><td>KLID                <td>00000804 (zh-Hans-CN)
<tr><td>Layout Display Name <td>Chinese (Simplified) - US
<tr><td>Layout Id           <td>4
</table>

---

### Keymap

Correspondance de touche physique (scancode) à une touche virtuelle (VK) par rapport aux modificateurs activés.

--

- Scancode: index dans une keymap

--
- VK: une touche d'action (pgup, etc), un caractère ou une touche morte

--
- Modificateur: Shift, NumLock, Ctrl, AltGr, etc

--

```cpp
keymaps = layouts[klid]
keymap = keymaps[mods]
vk = keymap[scancode]
```

---

### Scancode

Une valeur entre 0 et 127 inclut (`0x7f` en héxa) associé à des flags:

--
- Release (`0x8000`): indique que la touche est relâchée. Son absence indique que la touche est pressée.

--
- Down (`0x4000`): une priorité d'événement (???). N'existe pas en FastPath.

--
- Extended (`0x0100`): on peut le considérer comme le 8ème bit d'un scancode.

--
- Extended1 (`0x0200`): utilisé par la touche `Pause`. (`LCtrl+Extended1`, `NumLock`, `LCtrl+Extended1+Release`, `NumLock+Release`)

---

```
+----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+
| 01 |  | 3B | 3C | 3D | 3E |  | 3F | 40 | 41 | 42 |  | 43 | 44 | 57 | 58 |
+----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+
              ***  keycodes suffixed by 'x' are extended ***
+----+----+----+----+----+----+----+----+----+----+----+----+----+--------+
| 29 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0A | 0B | 0C | 0D |   0E   |
+-------------------------------------------------------------------------+
|  0F  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1A | 1B |      |
+------------------------------------------------------------------+  1C  |
|  3A   | 1E | 1F | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 2B |     |
+-------------------------------------------------------------------------+
|  2A | 56 | 2C | 2D | 2E | 2F | 30 | 31 | 32 | 33 | 34 | 35 |     36     |
+-------------------------------------------------------------------------+
|  1D  |  5Bx | 38 |           39           |  38x  |  5Cx |  5Dx |  1Dx  |
+------+------+----+------------------------+-------+------+------+-------+
```
--
```
+----+----+-------+
| 37 | 46 | 1D+45 |
+----+----+-------+
***  keycodes suffixed by 'x' are extended ***
+----+----+----+  +--------------------+
| 52x| 47x| 49x|  | 45 | 35x| 37 | 4A  |
+----+----+----+  +----+----+----+-----+
| 53x| 4Fx| 51x|  | 47 | 48 | 49 |     |
+----+----+----+  +----+----+----| 4E  |
                  | 4B | 4C | 4D |     |
     +----+       +----+----+----+-----+
     | 48x|       | 4F | 50 | 51 |     |
+----+----+----+  +---------+----| 1Cx |
| 4Bx| 50x| 4Dx|  |    52   | 53 |     |
+----+----+----+  +---------+----+-----+
```

---

### Virtual Key (VK)

Usuellement un entier 32 bits qui contient un caractère Unicode + 1 ou 2 bits pour distinguer:

- touche d'action
- touche morte
- caractère Unicode

```
0b110000_000xxxxx_xxxxxxxx_xxxxxxxx
  ^ touche d'action
   ^ dead key
            \______ Unicode ______/
```

---

```cpp
static constexpr KeyLayout2::unicode_t keymap_64[256] { // Fr with NumLock
/*00-07*/       0,  0x001b,     '&',  0x00e9,     '"',    '\'',     '(',     '-',
       //              ESC                 é
/*08-0F*/  0x00e8,     '_',  0x00e7,  0x00e0,     ')',     '=',    '\b',    '\t',
       //       è                 ç        à
/*10-17*/     'a',     'z',     'e',     'r',     't',     'y',     'u',     'i',
/*18-1F*/     'o',     'p',  DK|'^',     '$',    '\r',       0,     'q',     's',
/*20-27*/     'd',     'f',     'g',     'h',     'j',     'k',     'l',     'm',
/*28-2F*/  0x00f9,  0x00b2,       0,     '*',     'w',     'x',     'c',     'v',
       //       ù        ²
/*30-37*/     'b',     'n',     ',',     ';',     ':',     '!',       0,     '*',
/*38-3F*/       0,     ' ',       0,       0,       0,       0,       0,       0,
/*40-47*/       0,       0,       0,       0,       0,       0,       0,     '7',
/*48-4F*/     '8',     '9',     '-',     '4',     '5',     '6',     '+',     '1',
/*50-57*/     '2',     '3',     '0',     '.',       0,       0,     '<',       0,
/*58-5F*/       0,       0,       0,       0,       0,       0,       0,       0,
// ....
// ....
/*A0-A7*/       0,       0,       0,       0,       0,       0,       0,       0,
/*A8-AF*/       0,       0,       0,       0,       0,       0,       0,       0,
/*B0-B7*/       0,       0,       0,       0,       0,     '/',       0,       0,
/*B8-BF*/       0,       0,       0,       0,       0,       0,       0,       0,
/*C0-C7*/       0,       0,       0,       0,       0,       0,  0x0003,       0,
/*C8-CF*/       0,       0,       0,       0,       0,       0,       0,       0,
// ....
// ....
/*F8-FF*/       0,       0,       0,       0,       0,       0,       0,       0,
};
```

---

### Touche morte

Un accent qui, associé à une VK donne:

- Soit un nouveau caractère (^ + e = ê)
- Soit une suite de caractère (^ + g = ^g)

--

```cpp
static constexpr KeyLayout2::DKeyTable::Data dkeydata_5[] {
    {.meta={.size=11, .accent=0x005E /* ^ */}},
    {.dkey={.second=0x0020 /*   */, .result=0x005E /* ^ */}},
    {.dkey={.second=0x0041 /* A */, .result=0x00C2 /* Â */}},
    {.dkey={.second=0x0045 /* E */, .result=0x00CA /* Ê */}},
    {.dkey={.second=0x0049 /* I */, .result=0x00CE /* Î */}},
    {.dkey={.second=0x004F /* O */, .result=0x00D4 /* Ô */}},
    {.dkey={.second=0x0055 /* U */, .result=0x00DB /* Û */}},
    {.dkey={.second=0x0061 /* a */, .result=0x00E2 /* â */}},
    {.dkey={.second=0x0065 /* e */, .result=0x00EA /* ê */}},
    {.dkey={.second=0x0069 /* i */, .result=0x00EE /* î */}},
    {.dkey={.second=0x006F /* o */, .result=0x00F4 /* ô */}},
    {.dkey={.second=0x0075 /* u */, .result=0x00FB /* û */}},
};
```

---

### Modificateur

- Lock: NumLock, CapLock

--
- Sur Pression: Ctrl, Os/Meta, Alt/Menu, AltGr, OEM8, Kana

--
#### Cas particulier

--
- AltGr n'existe pas
    - Ctrl + Alt = AltGr.  Sauf si Kana est sur AltGr

--
- OEM8 est sur Ctrl droit pour un clavier canadien

--
```cpp
// Shift Control Menu NumLock CapsLock OEM8
//   1      2     4      8       16      32    = 0-63
sized_array_view<KeyLayout2::unicode_t, 256> keymap_mod_7[64] {
    keymap57, keymap58, keymap41, keymap60, keymap0, keymap0, keymap63, keymap0,
    keymap64, keymap58, keymap0, keymap0, keymap0, keymap0, keymap63, keymap0,
    keymap61, keymap59, keymap0, keymap0, keymap0, keymap0, keymap0, keymap0,
    keymap62, keymap59, keymap0, keymap0, keymap0, keymap0, keymap0, keymap0,
    keymap0, keymap0, keymap0, keymap0, keymap0, keymap0, keymap0, keymap0,
    keymap0, keymap0, keymap0, keymap0, keymap0, keymap0, keymap0, keymap0,
    keymap0, keymap0, keymap0, keymap0, keymap0, keymap0, keymap0, keymap0,
    keymap0, keymap0, keymap0, keymap0, keymap0, keymap0, keymap0, keymap0,
};
```

---

## Événement JS vers scancode RDP

--

D'une VK à un scancode

---

### Problèmes

- Le layout clavier du client n'est pas connu ou n'a pas d'équivalent, on ne peut pas se baser sur `KeyboardEvent.code` (équivalent scancode).

--

- Mais il faut utiliser `KeyboardEvent.code` pour distinguer certaines touches (ctrl droit/gauche, nombre du pavé numérique, etc).

--

- Les modificateurs activés par le client ne sont pas forcément ceux associés au scancode dans le layout cible (il faut simuler des frappes).

--
    - `é` = `AltGr+e` (US international) et `2` (Fr)

--

- Certaines touches que le client peut écrire n'existe pas dans le layout du serveur.

--

- Les touches mortes et touches de composition.

--
    - `^` + `e` = `ê`
    - `Compose` + `<` + `<` = `«`

---

### KeyboardEvent

Classe utlisée par les événements clavier (`keydown` and `keyup`).

--

On y trouve:

- `altKey`, `ctrlKey`, `metaKey`, `shiftKey`: indique si le modificateur est enclenché.

--
- `getModifierState(mod)`: indique si un modificateur est enclenché.

--

Ils sont inutiles :)

--

Sauf pour synchroniser au moment de la prise de focus (NumLock et CapsLock).

--
- `key`: une VK comme (`a`, `A`, `AudioVolumeDown`, `Control`, `Dead`, `4`, etc)

--
- `code`: l'équivalent scancode (`KeyQ`, `KeyQ`, `VolumeDown`, `ControlRight`, `Digit6`, `Numpad4`, etc)

---

#### Choisir entre KeyboardEvent.key et KeyboardEvent.code

--
- `code` doit être priorisé s'il commence par `Numpad`. `NumLock` étant synchronisé, il suffit normalement d'envoyer le scancode correspondant.

--
    - En prenant en compte `Shift` qui inverse `NumLock` (ou se référer à `key`).
--
    - Certains claviers ont 3 caractères associés à une touche du pavé numérique.

--
- `code` doit être utilisé si `key` contient `Control`, `Shift` ou `OS` (distinction droite/gauche)

--
    - Que faire si `key === 'Alt'` et `code === 'ControlLeft'` ?
--
    - Que faire si `code === 'ControlRight'` mais qu'elle n'existe pas (remplacée par OEM8) ?

--
- Dans le cas contraire, `key` représente la VK.

--
À noter que lorsque `key.length === 1`, cela représente une lettre.

---

### Table de correspondance key/code vers scancodes

--
```js
convert(key:String, isReleased:Boolean):Array[Scancode /* Number */]
```

--
En interne: key -> tableau de (liste des modificateurs + scancode)

--

(Ancienne version)

```js
keymap: {
    // scancode = x & 0xff
    // modMask = x & (0x10000 << mod)
    //            (N)oMod, (S)hift, (C)trl, (A)ltGr, Caps(L)ock
    //           mod: 0 | 1 | 2 |  3  | 4 |  5  |  6  |   7   | 8
    /* \b */ '\u0008': [
        0x01ff000e /* N | S | A | S+A | L | L+S | L+A | L+S+A | C */,
    ],
    /*  a */ '\u0061': [
        0x0021001e /* N |   |   |     |   | L+S |     |       |   */,
    ],
}
```

- Un même VK peut avoir plusieurs séquences de scancode.
- Il est préférable de limiter « l'émulation » des modificateurs.

---

#### Implémentation de l'algorithme

- Recherche une correspondance exacte sur les modificateurs (boucle sur un masque binaire)

--
    - Si trouvé, retourne un tableau avec le scancode correspondant

--
    - Sinon prend le premier élément puis met dans le tableau de retour
        - La désactivation des modificateurs qui ne sont pas nécessaires
        - L'activation des modificateurs qui manque
        - Le scancode
        - La désactivation des modificateurs ajoutés
        - La réactivation des modificateurs enlevés à l'étape 1

---

### Touche morte et touche de composition

`KeyboardEvent.key` contient `Dead`, mais ne permet de savoir la touche finale.

--

Il faut rediriger les frappes vers un `<input type=text/>` pour avoir le caractère final.

---

### Touche absente dans le layout cible

Il n'est pas possible d'inventer une touche :D

--

Mais on peut utiliser l'Unicode plutôt que les scancode si le serveur le supporte.

--

À noter que Ctrl ne peut pas être combinée avec un caractère Unicode.

---

## Question ?
