# HSM SafeNet 5110 - Guia de Token

**Autor:** Caio Abreu Ferreira <cferreira at segura.security>
**Descrição:** Uma série de informações de como trabalhar com eToken.

> **Laboratório e segurança:** execute somente em um token de teste. Comandos de
> inicialização, alteração de PIN e exclusão são destrutivos e irreversíveis. Use
> PINs informados interativamente; os valores `<...>` são placeholders, não segredos.
> Para este computador, use sempre `/usr/lib/libeToken.so` como biblioteca PKCS#11.

## SafeNet GUI

SACTools

## TOKEN

### Instalação

#### Arch Linux

```bash
yay -S opensc openct sac-core ccid pcsclite pcsc-tools pcsc-perl
sudo systemctl enable --now pcscd.service
sudo systemctl enable --now pcscd.socket
```

#### Debian

```bash
sudo apt-get install opensc libpcsclite1 pcsc-tools pcscd
sudo apt-get install libengine-pkcs11-openssl
sudo apt-get install gnupg-pkcs11-scd
sudo apt-get install gnupg2
```

```bash
sudo cat /etc/apt/sources.list.d/mjblenner-ubuntu-ppa-hal-hirsute.list
# deb http://ppa.launchpad.net/mjblenner/ppa-hal/ubuntu/ xenial main
sudo apt-get update
sudo apt-get install libhal1 libhal-storage1
```

### Inicializar Token

#### Criando o Security Office (SO) — destrutivo

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --slot <SLOT_DE_TESTE> --init-token --label lab_hsm
```

#### Criando o User PIN

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --init-pin --login --slot <SLOT_DE_TESTE>
```

#### Alterando o User PIN

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --change-pin
```

#### Desbloquear o HSM

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --login --login-type so --init-pin
```

#### Visualizar o conteúdo do token

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --login -O
```

#### Login

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --test --login
```

## CHAVE - Simétrica e Assimétrica

### Chave RSA 2048

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --login --keypairgen --key-type RSA:2048 --id 1 --label segura_rsa
```

### Chave EC SECP r1

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --login --keypairgen --key-type EC:secp384r1 --id 2 --label segura_ec1
```

### Chave EC Koblitz k1

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --login --keypairgen --key-type EC:prime256v1 --id 3 --label segura_k1
```

### Chave AES 128

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --login --keygen --id 5 --key-type aes:16 --label segura_aes128 --sensitive
```

### Chave AES 256

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --login --keygen --id 4 --key-type aes:32 --label segura_aes256 --sensitive
```

### Visualizar as chaves que estão armazenadas no token

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --login -O
```

### Visualizar as chaves públicas

```bash
ssh-keygen -D /usr/lib/libeToken.so
```

### Apagar objetos armazenados no token

Tipos: cert, privkey, pubkey, secrkey, data

```bash
pkcs11-tool -vv --module /usr/lib/libeToken.so --login --delete-object --id <ID_DE_TESTE> --type pubkey
```

## Certificado Digital

### Arquivo de configuração do OpenSSL

```ini
# engine.conf
openssl_conf = openssl_init

[openssl_init]
engines = engine_section

[engine_section]
pkcs11 = pkcs11_section

[pkcs11_section]
engine_id = pkcs11
MODULE_PATH = /usr/lib/libeToken.so
init = 0

[req]
distinguished_name = req_distinguished_name
[req_distinguished_name]
```

### Gerando o CSR utilizando a chave privada RSA que está armazenada no HSM

Utilizando o arquivo "engine.conf", veja o SLOT e o ID da chave privada a utilizar, no caso, Slot 0 e ID 01:

```bash
openssl req -new -subj '/CN=segura/' -sha256 -config engine.conf -engine pkcs11 -keyform engine -key 0:01 -out arquivo_certificado.csr
```

### Gerando o certificado auto-assinado

```bash
openssl req -x509 -engine pkcs11 -config engine.conf -keyform engine -new -key 0:01 -sha256 -days 3650 -out certificado_auto_assinado.pem -subj "/CN=segura"
```

### Importar certificado para o HSM

#### Converter o arquivo PEM no formato DER

```bash
openssl x509 -outform der -in certificado_auto_assinado.pem -out certificado_auto_assinado.der
```

#### Gravando o certificado no token

**Importante:** O ID deve ser o MESMO da chave pública

```bash
pkcs11-tool --module /usr/lib/libeToken.so --login --write-object certificado_auto_assinado.der --type cert --id 01 --label "segura"
```

#### Exibir o certificado gravado no eToken

```bash
pkcs11-tool --module /usr/lib/libeToken.so --login --list-objects
```

Saída esperada:
```
Certificate Object; type = X.509 cert
  label:      segura
  subject:    DN: CN=segura
  ID:         01
```

#### Apagar o certificado do eToken — destrutivo

```bash
pkcs11-tool --module /usr/lib/libeToken.so --login --delete-object --type cert --id 01
```

## OPENVPN

### Gerando o CSR utilizando a chave privada RSA que está armazenada no HSM

Utilizando o arquivo "engine.conf", veja o SLOT e o ID da chave privada a utilizar, no caso, Slot 0 e ID 01. O CSR deve ser assinado pela infraestrutura:

```bash
openssl req -new -subj '/CN=segura/' -sha256 -config engine.conf -engine pkcs11 -keyform engine -key 0:01 -out arquivo_certificado.csr
```

O arquivo `arquivo_certificado.csr` deve ser enviado para o responsável o qual irá assinar o certificado. O responsável irá enviar um e-mail com o certificado assinado. O certificado deve ser salvo em um arquivo com a extensão .PEM: `certificado_assinado.pem`.

### Converter de PEM para DER

```bash
openssl x509 -outform der -in certificado_assinado.pem -out certificado_assinado.der
```

### Importar o certificado assinado (DER) no HSM

**IMPORTANTE:** O certificado deve ter o mesmo ID da chave pública/privada RSA que gerou o certificado.

```bash
pkcs11-tool --module /usr/lib/libeToken.so -l --write-object certificado_assinado.der --type cert --id 01 --label "segura"
```

### Obtendo o "Serialized id"

```bash
openvpn --show-pkcs11-ids /usr/lib/libeToken.so
```

Saída esperada:
```
The following objects are available for use.
Each object shown below may be used as parameter to
--pkcs11-id option please remember to use single quote mark.

Certificate
       DN:             CN=cferreira
       Serial:         SERIAL
       Serialized id:  SERIALIZED_ID
```

### Arquivo de configuração do OpenVPN

```ini
# client.ovpn
client
dev tun
remote <IP_SERVER> 444 udp4
keepalive 10 120
nobind
persist-tun
persist-key
auth-nocache
verify-x509-name "<IP_SERVER>" name
remote-cert-tls server
tls-client
key-direction 1
route <REDE_PRIVADA> <MASCARA_DE_REDE> <GATEWAY_VPN>
<ca>
-----BEGIN CERTIFICATE-----
(...))
-----END CERTIFICATE-----
</ca>
pkcs11-providers /usr/lib/libeToken.so
pkcs11-id 'SERIALIZED_ID'
tls-version-min 1.1
```

## OPENSSH

### Cliente

#### Configuração do cliente SSH

```bash
# /etc/ssh/ssh_config
(...)
PKCS11Provider /usr/lib/libeToken.so
```

#### Obter a chave pública armazenada no eToken

> **Dica:** Utilize chave de criptografia do tipo EC pois é um algoritmo de criptografia mais rápido no processo de criptografia/descriptografia.

```bash
pkcs11-tool --module /usr/lib/libeTPkcs11.so --slot 0 --login --list-objects
```

#### Copiar a chave pública para o arquivo authorized_keys

Este arquivo deve ser copiado para o servidor SSH e deve ser copiado para o diretório `.ssh`:

```bash
ssh-keygen -D /usr/lib/libeToken.so >> ~/.ssh/authorized_keys
```

#### Login no servidor SSH

```bash
ssh -I /usr/lib/libeToken.so <USUARIO>@<HOST_SSH>
```

## ASSINATURA

### Assinar e Verificar um arquivo utilizando chave RSA id 01

A informação a ser criptografada é inserida no HSM → Criptografada dentro do HSM → exportada para o computador.

#### Arquivo de teste

```bash
echo "19d512fc649e1668eb84741284ad95ec03f3225719a83e40389dbade4eabe5ed" > digest.txt
xxd -r -p digest.txt digest.bin
```

#### Assinar digitalmente o conteúdo de um arquivo com a chave privada RSA e mecanismo RSA-PKCS

```bash
pkcs11-tool --module /usr/lib/libeToken.so --login --sign -m RSA-PKCS --id 01 -i digest.bin -o signature.bin
```

#### Exibir o arquivo assinado

```bash
base64 digest.bin.RSA-PKCS.sign
```

#### Exportar a chave PÚBLICA

```bash
pkcs11-tool --read-object --login --id 01 --type pubkey --module /usr/lib/libeToken.so -o public-key.der
```

#### Converter o arquivo do formato DER (binário) para o formato PEM (ASCII)

```bash
openssl rsa -inform DER -outform PEM -in public-key.der -pubin -out public-key.pem
```

### RSA-PKCS

#### Verificar conteúdo da assinatura assinado em RSA-PKCS

```bash
openssl rsautl -verify -inkey public-key.pem -in signature.bin -pubin -out verified.bin
```

#### Convertendo os arquivos

```bash
xxd -p -c32 verified.bin verified.txt
```

#### Comparando os arquivos

```bash
cat digest.txt verified.txt
```

#### Assinar digitalmente o conteúdo de um arquivo com a chave privada RSA e mecanismo ECDSA-SHA384

```bash
pkcs11-tool --module /usr/lib/libeToken.so --login --sign -m ECDSA-SHA384 --id 02 -i 244letras.txt -o 244letras.txt.ECDSA-SHA384.sign
```

## CRIPTOGRAFIA

### Criptografar e Descriptografar um arquivo utilizando chave RSA id 01

O arquivo é criptografado fora do HSM pois é utilizada a chave pública para executar o processo. Já o processo de descriptografia ocorre dentro do HSM pois a chave privada não pode ser exportada do HSM.

#### Exportar a chave PÚBLICA

```bash
pkcs11-tool --read-object --login --id 01 --type pubkey --module /usr/lib/libeToken.so -o chave_publica.der
```

#### Converter o arquivo com a chave pública do formato DER (binário) para o formato PEM (ASCII)

```bash
openssl rsa -inform DER -outform PEM -in chave_publica.der -pubin -out chave_publica.pem
```

#### Criptografando um arquivo utilizando a chave pública

```bash
openssl pkeyutl -encrypt -inkey chave_publica.pem -pubin -pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:sha256 -pkeyopt rsa_mgf1_md:sha256 -in 189letras.txt -out 189letras.txt.RSA-AOEP
```

#### Descriptografando um arquivo utilizando a chave privada

O processo de descriptografia ocorre dentro do HSM:

```bash
pkcs11-tool --module /usr/lib/libeToken.so --login --decrypt --id 01 -m RSA-PKCS-OAEP -i 189letras.txt.RSA-AOEP -o 189letras.txt.RSA-AOEP.verificado
```

```bash
pkcs11-tool --module /usr/lib/libeToken.so --login --decrypt --id 01 -m RSA-PKCS-OAEP --hash-algorithm=sha256 -i 189letras.txt.RSA-AOEP -o 189letras.txt.RSA-AOEP.verificado
```
