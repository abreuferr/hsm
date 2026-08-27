#!/usr/bin/env bash
# Laboratório Dinamo. Cria objetos: execute somente em um HSM de teste.
set -euo pipefail

: "${DINAMO_HSM_HOST:?Defina DINAMO_HSM_HOST}"
: "${DINAMO_HSM_USER:?Defina DINAMO_HSM_USER}"
: "${DINAMO_HSM_PASSWORD:?Defina DINAMO_HSM_PASSWORD}"
export DFENCE_PKCS11_IP="$DINAMO_HSM_HOST"
export DFENCE_PKCS11_USER="$DINAMO_HSM_USER"
export DFENCE_PKCS11_AUTO_RECONNECT=1
MODULE_DINAMO="${MODULE_DINAMO:-/usr/lib/libtacndp11.so}"
export HSM_PKCS11_NON_PRIVATE_OBJECTS_SUPPORT=1

read -r -p "O script criará chaves de laboratório em ${DINAMO_HSM_HOST}. Continuar? [y/N] " answer
[[ "$answer" == "y" || "$answer" == "Y" ]] || exit 0

# Listar o conteudo do HSM
pkcs11-tool -vv --module "$MODULE_DINAMO" --login --pin "$DINAMO_HSM_PASSWORD" -O

# Chave AES:256
pkcs11-tool -vv --module "$MODULE_DINAMO" --login --pin "$DINAMO_HSM_PASSWORD" --keygen --key-type aes:32 --label "lab_aes256"

# Chave AES:192
pkcs11-tool -vv --module "$MODULE_DINAMO" --login --pin "$DINAMO_HSM_PASSWORD" --keygen --key-type aes:24 --label "lab_aes192"

# Chave AES:128
pkcs11-tool -vv --module "$MODULE_DINAMO" --login --pin "$DINAMO_HSM_PASSWORD" --keygen --key-type aes:16 --label "lab_aes128"

# Chave RSA:2048 (RSA:512 é criptograficamente obsoleto)
pkcs11-tool -vv --module "$MODULE_DINAMO" --login --pin "$DINAMO_HSM_PASSWORD" --keypairgen --key-type rsa:2048 --label "lab_rsa2048"
