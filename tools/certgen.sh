#/usr/bin/bash

openssl genrsa -out rdpserver.key -des3 -passout "pass:inquisition" 2048
openssl req -new -key rdpserver.key -passin "pass:inquisition" -out rdpserver.csr -subj "/CN=ReDemPtion"

cat >tlssrv_x509v3ext <<END_EXT
keyUsage=keyEncipherment,dataEncipherment
extendedKeyUsage=serverAuth
END_EXT

openssl x509 -req -days 365 -in rdpserver.csr -signkey rdpserver.key -passin "pass:inquisition" -out rdpserver.cert -extfile tlssrv_x509v3ext

