layout: true
---

class: center, middle

# Fonctions utilitaires

---

## utils/sugar/numeric/safe_conversions.hpp

--

- `safe_cast<To>(From)`
- `checked_cast<To>(From)`
- `saturated_cast<To>(From)`

--

```cpp
enum class E : uint32_t;
uint64_t x;
E e;

safe_cast<uint32_t>(x); // erreur
safe_cast<E>(x); // erreur
safe_cast<uint32_t>(e); // ok
```

--

```cpp
char * first;
char * last;

checked_cast<uint16_t>(first - last); // négative: assertion
```

--

- `safe_int<T>`
- `checked_int<T>`
- `saturated_int<T>`

--

```cpp
out_stream.out_uint32_le(safe_int{e})
```

---

### utils/sugar/cast.hpp

--

- Remplace `reinterpret_cast` pour les conversions `char*` <-> `uint8_t*`.

```cpp
auto byte_ptr_cast(char       * data) -> uint8_t *;
auto byte_ptr_cast(char const * data) -> uint8_t const *;

auto char_ptr_cast(uint8_t       * data) -> char *;
auto char_ptr_cast(uint8_t const * data) -> char const *;
```

--

```cpp
std::underlying_type_t<E> underlying_cast(E e);
```

---

class: center, middle

## Les « vues »

Donnée non-propriétaire

--

.red.bold[/!\ Attention à la durée de vie]

---

### std::string_view

--

- intervalle immuable
- constructeur implicite depuis `char*` / `std::string`
- interface presque identique à `const std::string`

--

```cpp
using namespace std::string_view_literals;

CHECK(raw_cstring == "bla bla"sv);
CHECK("bla bla"sv.find('a') == 2);
```

--

.red.bold[/!\ Ne pas l'utiliser si on attend un `char const*` terminé par `'\0'`]

--

```cpp
bool file_exists(std::string_view path)
{
  struct stat st;
  return stat(path.data(), &st) != 0; // wrong !
}
```

---

### zstring_view

--

Une vue sur une chaîne terminée par `'\0'`.

--

```cpp
bool file_exists(zstring_view path)
{
  struct stat st;
  return stat(path.c_str(), &st) != 0;
}
```

--

```cpp
file_exists("test.txt"_zv);
file_exists(/*std::string=*/str);
file_exists(zstring_view(raw_cstring, zstring_view::is_zero_terminated());
```

--

(`std::cstring_view(std::null_terminated, "bla bla")`)

---

### array_view&lt;T> / writable_array_view&lt;T>

--

- Vue généraliste
- Constructeur avec tous types qui supportent `data()` et `size()`

--
- Sauf les tableaux `char[N]` et `uint8_t[N]`

--
- Mais peut être construit par l'intermédiaire de `make_array_view(arr)`/`make_writable_array_view(arr)`

--
- Fonctions membres `first()`, `drop_front()`, etc

--

```cpp
using u8_array_view = array_view<std::uint8_t>;
using writable_u8_array_view = writable_array_view<std::uint8_t>;
// (writable_)[us]{8,16,32,64}_array_view

using chars_view = array_view<char>;
using writable_chars_view = writable_array_view<char>;
```

---

### bytes_view / writable_array_view

--

- `array_view<uint8_t>` qui prend indifférement `char` et `uint8_t`
- Membres supplémentaires: `as_charp()`, `as_u8p()`, `as_chars()`

---

### buffer_view

`bytes_view` qui accepte les tableaux

---

### std::span&lt;T>

--

- même principe que `array_view`
- accepte tous types qui respectent `std::contiguous_range` and `std::sized_range`

--

```cpp
std::span<T> == writable_array_view<T>
std::span<T const> == array_view<T>
```

--

- la taille de la vue peut-être en template

```cpp
void foo(std::span<T, 32>);
```

---

class: center, middle

## Les fichiers

---

### unique_fd

--

Le principe de `std::unique_ptr` pour des fds: non-copiable, mais déplaçable.

```cpp
if (unique_fd file{"filename.txt"}) {
    unique_fd clone = file; // erreur
    MyTransport trans{std::move(file)}; // ok
    //...
}
```

---

### File

Un wrapper de `FILE`.

---

class: center, middle

## Manipulation de chaîne

---

### std::to_chars / std::from_chars

--

- Ne dépends pas de la locale

--
- N'utilise pas de variable globale comme `errno`

--

```cpp
std::to_chars_result to_chars(char* first, char* last,
                              Integer value, int base = 10);

struct std::to_chars_result
{
    char* ptr;
    std::errc ec;
};
```

--

```cpp
static_string<n> to_chars_10(Integer);
```

--

```cpp
std::from_chars_result from_chars(const char* first, const char* last,
                                  Integer& value, int base = 10);

struct std::from_chars_result {

    const char* ptr;
    std::errc ec;
};
```

--

```cpp
Integer from_chars_10<Integer>(chars_view);
```

---

### `utils/sugar/algostring.hpp`

--

```cpp
std::string str_concat(Strings const&... strs);
void str_append(std::string& str, Strings const&... strs);
void str_assign(std::string& str, Strings const&... strs);
```

--

- Fonctionne avec `char`, `char*` et tout ce qui est convertible en `chars_view`.
- Fonctions optimisées pour réduire les allocations dynamiques.

--

```cpp
std::string_view sv;
std::string s;

sv + s; // erreur
s + s + s; // jusqu'à 2 allocations

str_concat(sv, s); // ok
str_concat(s, s, s); // au plus 1 allocation
```

---

### `utils/strutils.hpp`

--

```cpp
bool ends_with(chars_view str, chars_view suffix);
bool ends_case_with(chars_view str, chars_view suffix);
```

--

- Il manque `starts_with`.
- Clang-tidy possède des avertissements pour remplacer `s.find(str) == 0` par `Absell::StartsWith(s, str)`.

--
- Les fonctions `starts_with` et `ends_with` existent dans `std::sting_view`/`std::string` en C++20

--

```cpp
std::size_t strlcpy(char* dest, chars_view src, std::size_t n);
std::size_t strlcpy(char* dest, char const* src, std::size_t n);
std::size_t strlcpy(char (&dest)[N], chars_view src); /* n'existe pas */
```

- `strlcpy` est l'équivalent de la fonction dans BSD, c'est `strncpy` sans remplissage de `'\0'`.

--
- `strlcpy` retourne la taille de `src` !

--
- Ne permet pas d'avoir la taille finale de `dest`.

--

```cpp
bool strbcpy(char* dest, chars_view src, std::size_t n);
bool strbcpy(char* dest, chars_view src, std::size_t n);
bool strbcpy(char (&dest)[N], chars_view src); /* n'existe pas */
```

--
- `strbcpy` retourne `true` si la chaîne à pu être entièrement copiée.

---

### `utils/literals/utf16.hpp`

```cpp
"abc"_utf16_le.value == char[/*8*/]{'a', '\0', 'b', '\0', 'c', '\0', '\0', '\0'};
"abc"_utf16_le.av() == chars_view(/*...*/, 6);
```

--
- Erreur de compilation si la chaîne de départ n'est pas ascii

---

class: center, middle

# Convention d'écriture

---

## Accolades

--
- En début de ligne pour les fonctions

--
- Accolade ouvrante sur la même ligne pour les conditions (if, for, etc)

--
- Saut de ligne après une accolade fermante suivit de else

--
- À mettre tout le temps

--

```cpp
void foo()
{
    if (cond) {
        // ...
    }
    else {
        // ...
    }
}
```

--
- On évite les conditions sur 1 ligne sauf s'il y a une forte volonté de symétrie.

--

```cpp
if (x > 4) foo(bar);
if (x > 3) foo(bar);
if (x > 2) foo(bar);
if (x > 1) foo(bar);
```

---

## Condition

--
- Saut de ligne entre plusieurs blocs de `if`

--
- Pas de parenthèse inutile, cela inhibe des avertissements

--

```cpp
if ((tmp[0] == '.') && (tmp[1] = '/') /* no */) {

}
```

--

```cpp
// Pas bien
if (cond1) {
}
if (cond2) {
}
```

--

```cpp
// Bien
if (cond1) {
}

if (cond2) {
}
```

---

### Multi-ligne

--
- Les opérateurs logiques en début et alignement des conditions

--
- Idem pour la parenthèse fermante

---
Difficile de voir où finie la condition:

```cpp
if (this->wait_answer &&
    !ini.is_asked<cfg::context::keepalive>() &&
    ini.get<cfg::context::keepalive>()) {
    LOG_IF(bool(this->verbose & Verbose::state),
        LOG_INFO, "auth::keep_alive ACL incoming event");
}
```

--
Peu élégant et faible séparation visuelle entre condition et contenu:

```cpp
if (this->wait_answer
    && !ini.is_asked<cfg::context::keepalive>()
    && ini.get<cfg::context::keepalive>()) {
    LOG_IF(bool(this->verbose & Verbose::state),
        LOG_INFO, "auth::keep_alive ACL incoming event");
}
```

--
Ce qu'il faut:

```cpp
if (this->wait_answer
 && !ini.is_asked<cfg::context::keepalive>()
 && ini.get<cfg::context::keepalive>()
) {
    LOG_IF(bool(this->verbose & Verbose::state),
        LOG_INFO, "auth::keep_alive ACL incoming event");
}
```

---

- Si on utilise une clause d'init, mettre le `;` en début de ligne.

--

```cpp
if (auto x = foobar()
  ; x != y
) {
    // ...
}

for (auto x = foobar(i)
   ; x != y
   ; ++i
) {
    // ...
}
```

---

## switch

--
- Pas de `default`, cela inhibe des avertissements

--

```cpp
enum class E { A, B, C };

switch (x) {
    case E::A: break;
    case E::B: break;
    case E::C: break;
    default:; /* pas d'avertissement si on ajoute un membre dans E */
}
```

--
- On peut utiliser `REDEMPTION_UNREACHABLE()` si **on est certain** que la valeur est forcément dans l'enum.

--

```cpp
int foo()
{
    switch (x) {
        case E::A: return 1;
        case E::B: return 55;
        case E::C: return 21;
    }
    REDEMPTION_UNREACHABLE();
    // pas de return
}
```

---

## Ternaire

--
- Une espace autour de `?` et `:`

--
- Si sur plusieurs lignes, mettre `?` et `:` en début de ligne avec une indentation.

--

```cpp
// Pas bien
PngParams png_params = {
    0, 0,
    ini.get<cfg::video::png_interval>(),
    100u,
    ini.get<cfg::video::allow_rt_without_recording>() ?
    ini.get<cfg::video::png_limit>() : 0,
    true,
    this->client_info.remote_program,
    ini.get<cfg::video::rt_display>()
};
```

--

```cpp
// Bien
PngParams png_params = {
    0, 0,
    ini.get<cfg::video::png_interval>(),
    100u,
    ini.get<cfg::video::allow_rt_without_recording>()
        ? ini.get<cfg::video::png_limit>()
        : 0,
    true,
    this->client_info.remote_program,
    ini.get<cfg::video::rt_display>()
};
```

---

## reinterpret_cast

- Ne pas l'utiliser :D

--
- Les conversions `uint8_t*` <-> `char*` se font avec `bytes_ptr_cast` et `chars_ptr_cast` ou `byte_ptr(p).as_u8p()`/`byte_ptr(p).as_charp()`

---

## Indentation

--

- 4 espaces

--
- Pour les fonctions, ne pas aligner sur la parenthèse

--

```cpp
foofoofooooooooooooooo(
    param1, param2,
    param3, param4
);
```

---

## Déclaration de variable

--
- 1 par ligne

--
- Déclarer au plus tard et les initialiser à ce moment

--
- Les initialiser directement dans le corps des classes si possible

--

```cpp
class A
{
    int a = 42;

public:
    ...
};
```

--
Ne pas utiliser `memset` ou `memcpy` pour initialiser une variable

--
- Risque si le type change

--
- `type value {};` fait la même chose en une seule ligne.

--
- Si la copie n'est pas possible sans memcpy, alors le type n'est probablement pas adapté.

---

## Déduction de type

--

- Utiliser `auto` dès que l'écriture du type est une évidence ou fait doublon.

--

```cpp
auto foo = std::make_unique<Foo>();
auto it = vec.begin();
```

--
- CTAD / guide de déduction

--

```cpp
array_view av = str; /* av est de type array_view<char> */
```

---

## Initialisation

--

- Ne pas initialiser une variable si celle-ci doit être "initialisée" plus tard.
--
 Car cela inhibe les avertissements du compilateur et de valgrind.

---

## Règle du 0/3/5

--
- 0: Ne pas implémenter de dtor, copy-ctor, copy-assignment

--
- 3: Si une des 3 précédentes est implémenté, le faire pour les 3

--
- 5: Si on veut la sémantique de déplacement, règle du 3 + move-ctor et move-assignment

--
- Une implémentation par défaut doit être faite avec `= default`

--
- Les fonctions indésirables avec `= delete`

--
- Les classes avec un membre `virtual` doivent hériter de `noncopyable` (en privé)

--

```cpp
struct A
{
    A() = default;
    A(...) { ... }

    ~A() = default;

    A(A const&) = delete;
    A& operator=(A const& other) = delete;
};
```
--

```cpp
struct A : private noncopyable
{
    A() = default;
    A(...) { ... }
};
```

---

## Les pointeurs

--
- Les allocations devraient être faites avec `std::make_unique`/`std::make_unique_for_overwrite`.

--
    - Pas besoin de s'occuper de la libération
    - Pas de risque de copie impromptue

--
- Les `char*` ou `T*` + taille devraient être des vues ou des conteneurs concrets.

--
- Pour des valeurs optionnelles, `std::option` est probablement un meilleur choix.

--
- Un paramètre de sortie devrait être un objet avec un constructeur explicite (`out_param<T>`).

--
- Si le pointeur ne doit pas être nul: `non_null_ptr<T>` ou référence.

---

## std::string

--

- Il n'y a jamais besoin de faire `std::string(x)`

--
- Avoir `std::string` en paramètre est probablement une erreur
    - Sauf si l'objet est stocké quelque part (utilisation de `std::move`).
    - Ou que la chaîne est modifiée dans la fonction.

---

## Les boucles sur containers

--

```cpp
for (auto&& x : cont) {
    ...
}
```

---

class: center, middle

# Cpp Core Guidelines

`https://github.com/isocpp/CppCoreGuidelines/blob/master`

---

class: center, middle

# Passage de paramètre

---

## Bool

--

- Devrait être un type nommé `enum class IsPotato : bool`

--

```cpp
dump_png24(trans, img, true); // à quoi correspond true ?
```

--

```cpp
dump_png24(trans, img, IsBGRFormat(true)); // explicite
```

--

```cpp
RdpNego nego(
    true, "test", true, false, "127.0.0.1", false,
    rand, timeobj, extra_message, lang, tls_client_params);
```

--

```cpp
RdpNego nego(
    UseTls(true), "test", UseNla(true), UseAdminMode(false), "127.0.0.1",
    UseKerberos(false), rand, timeobj, extra_message, lang, tls_client_params);
```

---

.red.bold[/!\ Ne pas construire une `enum class : bool` avec un entier]

--

L'implémentation peut

- soit tronquer l'entier.
- soit faire une comparaison par rapport à 0.

--

```
IsBGRFormat(0); // false
IsBGRFormat(1); // true
IsBGRFormat(2); // false (gcc, clang) / true (msvc) -> 0b10
IsBGRFormat(bool(2)); // true
IsBGRFormat(3); // true
IsBGRFormat(4); // false
```

---

## Entier

--

- Devrait être un type nommé.
--
.. Encore :D

--

```cpp
set_delay(23); // quelle unité de temps ?
```

--

```cpp
set_delay(std::chrono::milliseconds(23)); // explicite
set_delay(23ms); // plus simple
```

--

Nouveaux suffixes pour les multiples de 2

```cpp
unsigned x = 12_Ki;
unsigned x = 12_Mi;
...
```

--

Type pour représenter des tailles: `ByteSize`, `U32ByteSize`, `KibiByte`, ...

```cpp
ByteSize x = 12_KiB;
ByteSize x = 12 * 1_KiB;
...
```

---

L'idéal serait que toute unité possède un type dédié avec la surcharge des opérateurs acceptés entre type.

--

```cpp
X x;
Y y;
Width w;
Height h;

Distance dist = w - x;
Distance dist = w - y; // compilation error
Area = w * h;
Area = w * y;
Area = w * x; // compilation error
```

---

## Enum

--

`enum` est à proscrire au profit de `enum class`:

--
- Pas de conversion implicite.

--
- Les valeurs de l'enum ne se propagent pas au scope parent.

--

Les opérations entre énumération devraient être explicitement définies.

--
- Obligatoire avec `enum class`.

--
- Explicite les opérations et types compatibles.

---

### flag_t&lt;E>

--

- Définis les opérateurs classiquement utilisés: `&`, `|`, `~`

--
- Définis les opérateurs `+` et `-`, alias de `f1 & f2` et `f1 & ~f2`

--
- Contiens des fonctions utilitaires telle que `test(E)`, `as_uint()`

--
- Pourrait avoir `contains(m)` (`(f & m) == m`)

--

```cpp
enum class E { A, B, C, /*_max_element*/ };

template<>
struct utils::enum_as_flag<E>
{
    static const std::size_t max = 3 /* std::size_t(E::_max_element) */;
};

E::A | E::B; // = flag_t<E>
```

--

```cpp
ENUM_FLAG(E, A, B, C);
```

---

# Minimisation des includes

--

- Compile plus vite
- Recompile moins

--

Déclaration anticipée et séparation .hpp/.cpp

--

```cpp
// .hpp
class Foo;

void bar(Foo&);

// .cpp
#include "bar.hpp"
#include "foo.hpp"

void bar(Foo& foo) { /* ... */ }
```

--

Récupérer une interface plutôt qu'instancier une implémentation.

--

- mod/rdp/new_mod_rdp.hpp (`new_mod_rdp()`)

--
- mod/vnc/new_mod_vnc.hpp (`new_mod_vnc()`)

--
- test_only/acl/sesman_wrapper.hpp (`InifileWrapper`, `SesmanWrapper`)

--
- test_only/core/font.hpp (`global_font()`, ...)

--
- test_only/front/front_wrapper.hpp (`FrontWrapper`)

--
- test_only/gdi/test_graphic.cpp (`TestGraphic`)

---

class: center, middle

# Outils autour du proxy

---

## Analyseur de code C++

--

- `tools/c++-analyzer/*` (valgrind, gdb, clang-tidy, etc)

--
- `tools/c++-analyzer/lua-checker/*`

---

## Bjam

--

- `tools/bjam/bjam_completion.*` (bash et zsh)

--
- `tools/bjam/bjam_filter.awk` (pretty_bjam) affichage élégant de la sortie

--
- `bjam --help` affiche des infos spécifique au projet

--
- `bjam env_help` affiche les variables de configuration du projet

---

## Tests

--

- `tools/bjam/unit_test_color.sh`: mise en couleur des tests

--
- `export REDEMPTION_LOG_PRINT=...`:
    - `0`: pas de log
    - `1`: affiche les logs
    - `e`: uniquement `LOG_ERR` et `LOG_DEBUG`
    - `d`: uniquement `LOG_DEBUG`
    - `w`: autre que `LOG_INFO`

--
- `export BOOST_STACKTRACE=...`: pour les modes `debug` et `san`
    - `1`: active libboost_stacktrace_backtrace
    - `static_backtrace`: active libbacktrace

--
- `export REDEMPTION_FILTER_ERROR=...`: listes des erreurs à ne pas afficher dans la pile d'appel

---

## Rdproxy

--

--
- `rdpproxy --help`

--
- `rdpproxy --config-file=<(cat path/rdpproxy.ini; echo -e "[websocket]\nenable_websocket=yes")`

--
- `tools/rdpproxy_color.awk` affiche les logs en couleur
