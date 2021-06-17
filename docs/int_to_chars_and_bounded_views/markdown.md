layout: true
---

class: center, middle

# Nouveaux types de séquence

--

# Et quelques fonctions utilitaires

---

## Les 4 groupes de classe ajoutée

--

Lifetime restrictive sur les chaînes:

- `SocketTransport::Name`

--

Transformation de nombre en chaîne:

- `int_to_decimal_chars()`
- `int_to_hexadecimal_chars()`

--

Vue statiquement dimensionnée

- `bounded_array_view<T, AtLeast, AtMost>`

--

Une chaîne sans allocation dynamique

- `static_string<N>`


---

## SocketTransport::Name

Le nom de la socket...

--

Et c'est tout :D

--

```cpp
SocketTransport sck("Sender"_sck_name, ...);
```

--

Le principe sera probablement ajouté au gestionnaire d'événement.

---

## Entier vers chaîne

--

2 catégories:

- sans zéro terminal (`int_to_*_chars()`)
- avec un zéro terminal (`int_to_*_zchars()`)

--

La version avec `z` retourne un type possèdant la fonction membre `c_str()`.

--

- `int_to_decimal_[z]chars(Integer)`
- `int_to_hexadecimal_{upper,lower}_[z]chars(Integer)`
- `int_to_fixed_hexadecimal_{upper,lower}_[z]chars(Integer)`

--

Chacune retourne un `int_to_[z]chars_result`.

--

```cpp
auto id = str_concat(prefix, int_to_decimal_chars(42));
```

---

### Bonus: tableau de nombre vers chaîne héxadécimal

- `static_array_to_hexadecimal_{upper,lower}_[z]chars(SizedSequence)`

retourne un `static_array_to_hexadecimal_[z]chars_result<N>`.

--

#### Avant

```cpp
auto& digest = file_contents_range.sig.digest();
char digest_s[128];
size_t digest_s_len = snprintf(digest_s, sizeof(digest_s),
    "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
    "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
    digest[ 0], digest[ 1], digest[ 2], digest[ 3], digest[ 4], digest[ 5], digest[ 6], digest[ 7],
    digest[ 8], digest[ 9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15],
    digest[16], digest[17], digest[18], digest[19], digest[20], digest[21], digest[22], digest[23],
    digest[24], digest[25], digest[26], digest[27], digest[28], digest[29], digest[30], digest[31]);
```

--

#### Après

```cpp
auto const digest_str = static_array_to_hexadecimal_lower_zchars(
    file_contents_range.sig.digest());
```

---

## Vue statiquement dimensionnée

Le principal ajout :)

---

### Problématiques de départ

Les fonctions utilisent un ensemble pointeur/taille sans indications des limites supportées

--

- L'appelant doit lire l'implémentation de la fonction pour en prendre connaissance

--

- La fonction doit vérifier que les tailles sont dans les limites acceptables

--
  - pour lancer une exception

--
  - pour tronquer

--
  - ou on fait confiance à l'appelant

--

Conséquences:

- Les tailles sont incohérentes au niveau de l'appelant
- Des vérifications inutiles sont effectuées

---

### Le système de type à la rescousse

--

`bounded_array_view<T, AtLeast, AtMost>`

--

- La limite de taille est explicitement indiquée dans le type

--

Avant

```cpp
RdpLogonInfo::RdpLogonInfo(char const* hostname, /*...*/)
{
    if (::strlen(hostname) >= sizeof(this->_hostname)) {
        LOG(LOG_WARNING, "mod_rdp: hostname too long! %zu >= %zu",
            ::strlen(hostname), sizeof(this->_hostname));
    }
    ... // traitement incohérent sur une valeur qui va être tronquée
}
```

--

Après

```cpp
RdpLogonInfo::RdpLogonInfo(bounded_array_view<char, 0, 64> hostname, /*...*/)
{
    // plus besoin de vérifier
    ...
}
```

---

Les limites sont **vérifiées à la compilation**:

--

- Une conversion peut se faire vers vue dont la taille min est plus petite.
- Une conversion peut se faire vers vue dont la taille max est plus grande.

--

```cpp
void foo(bounded_array_view<int, 10, 30>);

foo(bounded_array_view<int, 15, 30>(x)); // ok
foo(bounded_array_view<int,  5, 30>(x)); // no

foo(bounded_array_view<int, 10, 20>(x)); // ok
foo(bounded_array_view<int, 10, 40>(x)); // no
```

---

Les limites de tailles sont automatiquement déduites:

--

- Depuis un tableau C.
- Depuis `std::array`.
- Depuis n'importe quel type qui spécialise `sequence_to_av_bounds_impl`.

--

```cpp
std::array<int, 10> array;
bounded_array_view av1{array}; // bounded_array_view<int, 10, 10>
```

--

```cpp
auto str = int_to_decimal_chars(n);
bounded_array_view av2{str}; // bounded_array_view<int, 0, N>
```

---

Une sous-vue peut-être extraite tout en propageant les tailles statiques

--

```cpp
void foo(bounded_array_view<int, 10, 30> av)
{
    bar(av.first<3>()); // bounded_array_view<int, 3, 3>
    bar(av.first<20>()); // bounded_array_view<int, 10, 20>

    bar(av.first(20)); // array_view<int>
}

```

---

Un premier pas vers la suppression des conversions implicites (`-Wconversion`)

--

- `bounded_array_view::size()`: retourne `std::size_t`.

--

```cpp
void foo(bounded_array_view<0, 255> av)
{
  out_stream.out_uint8_t(av.size()); // std::size_t -> uint8_t
                                     // perte de précision, possible incohérence
}
```
--

- `bounded_array_view::msize()`: retourne la taille dans le plus petit type numérique possible.

--

```cpp
void foo(bounded_array_view<0, 255> av)
{
  out_stream.out_uint8_t(av.msize()); // msize() retourne un uint8_t
}
```

---

### Construire une `bounded_array_view`

---

#### Depuis des séquences à taille statique:

--

- `bounded_array_view<T, N, M>{seq}`: Ne fonctionne pas avec les tableaux C de `char` ou `uint8_t`.

--
- `bounded_array_view{seq}`: déduction automatique. Ne fonctionne pas avec les tableaux C de `char` ou `uint8_t`.

--
- `make_bounded_array_view(seq)`

--
- `"bla bla"_sized_av`

---

#### Depuis une séquence "dynamique"

--
- `bounded_array_view<T, AtLeast, AtMost>::assumed(ptr, n)`

--
- `bounded_array_view<T, AtLeast, AtMost>::assumed(ptr)` (quand AtLeast == AtMost)

--
- `bounded_array_view<T, AtLeast, AtMost>::assumed(seq)`

--
- `make_bounded_array_view<AtLeast, AtMost>(seq)`

---

#### Via coopération

--

En assumant la possibilité de tronquer

- `foo(truncated_array_view(array))`

--

En délégant un comportement hors-borne

- `foo(checked_bounded_array_view(array, stategy_func))`


---

### Les autres vues

--

- `writable_bounded_array_view<T, AtLeast, AtMost>`

--
- `sized_array_view<T, N>` (alias de bounded_array_view)

--
- `writable_sized_array_view<T, N>` (alias de writable_bounded_array_view)

---

## `static_string<n>`

Un tableau de taille n+1 toujours terminé par `'\0'`.

--

Avant

```cpp
struct A {
    A(bounded_chars_views<0, 127> name) {
        memcpy(m_name, name.data(), name.size());
        m_name[name.size()] = 0;
    }

    char m_name[128];
}
```

--

Après

```cpp
struct A {
    A(bounded_chars_views<0, 127> name) : m_name(name) {
    }

    static_string<127> m_name;
}
```

---

Taille optimisée

- `sizeof(static_string<12>)` == `14` (12 char + '\0' + taille sous forme de uint8_t)

--

Utilise `memcpy`, mais est plus rapide que `memcpy` :D

--

2 fonctions utilitaires `static_str_concat()` et `static_str_assign()`
