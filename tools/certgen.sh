#/usr/bin/bash

openssl genrsa -out rdpproxy.key -des3 -passout "pass:inquisition" 2048
openssl req -new -key rdpproxy.key -passin "pass:inquisition" -out rdpproxy.csr -subj "/CN=cgr.ifr.lan"

cat >tlssrv_x509v3ext <<END_EXT
keyUsage=keyEncipherment,dataEncipherment
extendedKeyUsage=serverAuth
END_EXT

openssl x509 -req -days 365 -in rdpproxy.csr -signkey rdpproxy.key -passin "pass:inquisition" -out rdpproxy.crt -extfile tlssrv_x509v3ext

# to get clear x509 certificate (no key)
openssl rsa -in rdpproxy.key -passin "pass:inquisition" -out clearRdpproxyPrivateKey.pem

#to get p12 certificate
openssl pkcs12 -export -in rdpproxy.crt -inkey rdpproxy.key -passin "pass:inquisition" -name cgr.ifr.lan -out rdpproxy.p12 -passout "pass:inquisition"
