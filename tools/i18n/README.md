## Generate or update redemption.pot file

```sh
./gen_msg_catalog.py -p . -e
```


## Generate redemption.po file for a target locale

```sh
./gen_msg_catalog.py -p . -i -l en
```


## Update redemption.po for a target locale

```sh
./gen_msg_catalog.py -p . -m -l en
```


## Update all redemption.po files for all existing locales

```sh
./gen_msg_catalog.py -p . -M
```


## Compile all redemption.po files and setup redemption gettext environnement


```sh
sudo bjam install-gettext
```

To do on redemption root directory