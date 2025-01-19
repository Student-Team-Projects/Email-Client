#!/bin/sh

sudo apt-get install libssl-dev
sudo apt-get install libicu-dev
sudo apt install libgnutls28-dev libgsasl7-dev
sudo apt install doxygen
sudo apt install sendmail
sudo apt install graphviz
cmake -G "Unix Makefiles" -DVMIME_SENDMAIL_PATH="/usr/sbin/sendmail" ..

pushd /etc/ssl/   
sudo openssl req -newkey rsa:2048 -new -nodes -x509 -days 3650 -keyout key.pem -out cert.pem
popd

sudo apt-get install sqlite3 libsqlite3-dev
sudo apt install libxml2
sudo apt install libxml2-dev

sudo apt-get install nlohmann-json3-dev

# installing vmime
mkdir temp_vmime_installation
cd temp_vmime_installation

git clone git@github.com:kisli/vmime.git
cd vmime
mkdir build
cd build
cmake -G "Unix Makefiles" ../
cmake --build .
sudo make install
cd ../../
rm -rf vmime

cd ..
rmdir temp_vmime_installation