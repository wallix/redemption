#/usr/bin/bash

#openssl genrsa -out rdpserver.key -des3 -passout "pass:inquisition" 2048
#openssl req -new -key rdpserver.key -passin "pass:inquisition" -out rdpserver.csr -subj "/CN=cgr.ifr.lan"

#cat >tlssrv_x509v3ext <<END_EXT
#keyUsage=keyEncipherment,dataEncipherment
#extendedKeyUsage=serverAuth
#END_EXT

#openssl x509 -req -days 365 -in rdpserver.csr -signkey rdpserver.key -passin "pass:inquisition" -out rdpserver.cert -extfile tlssrv_x509v3ext

# to get clear x509 certificate (no key)
#openssl rsa -in rdpserver.key -passin "pass:inquisition" -out clearPrivateKey.pem



openssl genrsa -out win2k8server.key -des3 -passout "pass:inquisition" 2048
openssl req -new -key win2k8server.key -passin "pass:inquisition" -out win2k8server.csr -subj "/CN=win2k8-64.qa.ifr.lan"

cat >tlssrv_x509v3ext <<END_EXT
keyUsage=keyEncipherment,dataEncipherment
extendedKeyUsage=serverAuth
END_EXT

openssl x509 -req -days 365 -in win2k8server.csr -signkey win2k8server.key -passin "pass:inquisition" -out win2k8server.cert -extfile tlssrv_x509v3ext

# to get clear x509 certificate (no key)
openssl rsa -in win2k8server.key -passin "pass:inquisition" -out clearwin2k8PrivateKey.pem

#to get p12 certificate
openssl pkcs12 -export -in win2k8server.cert -inkey win2k8server.key -passin "pass:inquisition" -name win2k8-64.qa.ifr.lan -out win2k8server.p12 -passout "pass:inquisition"
