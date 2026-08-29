# TPM 2.0 Guide

> **Laboratório:** use um TPM de teste. `tpm2_clear` remove a configuração e os
> objetos persistentes do TPM; `tpm2_evictcontrol` altera handles persistentes.
> Confirme o fabricante, o dispositivo e os handles em uso antes de executar esses
> comandos. Arquivos `.ctx`, `.pub`, `.priv` e chaves gerados são locais e ignorados.

# 1. Teoria

## 1.1. O que é um TPM

Um **TPM (Trusted Platform Module)** é um chip de segurança criptográfico baseado em hardware, padronizado pelo Trusted Computing Group (TCG). É um processador dedicado projetado para executar operações criptográficas e armazenar chaves, certificados e outras informações sensíveis de forma segura.

## 1.2. A que se destina um TPM?

O TPM tem várias finalidades principais:

**Autenticação e integridade do sistema**: Verifica se o sistema não foi comprometido durante o boot, medindo e armazenando hashes dos componentes de software (BIOS, bootloader, kernel) em registros especiais chamados PCRs (Platform Configuration Registers).

**Geração e armazenamento seguro de chaves**: Cria chaves criptográficas dentro do chip e as mantém protegidas contra extração, mesmo com acesso físico ao dispositivo.

**Atestação remota**: Permite que sistemas remotos verifiquem a integridade e autenticidade da plataforma através de certificados e assinaturas digitais.

**Criptografia de dados**: Protege dados sensíveis como chaves de criptografia do BitLocker no Windows ou outras soluções de criptografia de disco.

**Autenticação de usuários**: Suporta autenticação baseada em certificados e pode armazenar credenciais de forma segura.

## 1.3. Um TPM pode ser considerado um HSM?

Tecnicamente, um TPM pode ser considerado um tipo de HSM (Hardware Security Module), mas com ressalvas importantes:

**Semelhanças**: Ambos são dispositivos de hardware dedicados à segurança criptográfica, oferecem armazenamento seguro de chaves, executam operações criptográficas em hardware e possuem resistência a ataques físicos.

**Diferenças significativas**: Os HSMs tradicionais são dispositivos de alta performance projetados para ambientes empresariais críticos, com capacidades muito superiores de processamento criptográfico, maior capacidade de armazenamento e certificações mais rigorosas (como FIPS 140-2 nível 3 ou 4). Já os TPMs são otimizados para integração em computadores pessoais e dispositivos embarcados, com foco em custo-benefício.

**Classificação**: O TPM é mais precisamente classificado como um "HSM de baixo custo" ou "HSM embarcado", adequado para aplicações que requerem segurança básica a moderada, mas não para ambientes que demandam o máximo nível de segurança criptográfica que os HSMs dedicados oferecem.

## 1.4. O que é um arquivo .ctx (context)

- É um arquivo temporário que armazena o contexto de um objeto TPM
- Contém o handle (identificador) do objeto carregado na memória do TPM
- **NÃO** contém a chave em si - apenas uma referência para ela

```
TPM (Hardware)                   Sistema (Software)
┌─────────────────┐              ┌──────────────────┐
│                 │              │                  │
│ Chave Primária  │<--- Handle --│  primary.ctx     │
│ (ECC)           │  0x80000000  │  (referência)    │
│                 │              │                  │
└─────────────────┘              └──────────────────┘
```

# 2. Hieraquia de Chaves

## 2.1. Não Hierarquização de Chaves

```bash
# Verificar se existe
ls /dev/tpm*

# Verificar versão
tpm2_getcap properties-fixed

# Verificar se tem ECC
tpm2_getcap ecc-curves

# DESTRUTIVO: somente em TPM de laboratório; remove objetos/configuração.
# tpm2_clear

# Criar chave ECC
tpm2_createprimary -C o -G ecc -g sha256 -c primary.ctx

# Criar chave AES "do lado de fora"
openssl rand -out aes.key 32

# Ler chave AES (somente para demonstrar o arquivo criado fora do TPM)
xxd aes.key

# Selar o segredo usando a chave do TPM
tpm2_import -C primary.ctx -G keyedhash -i aes.key -u sealed.pub -r sealed.priv

# Carregar o segredo pro TPM
tpm2_load -C primary.ctx -u sealed.pub -r sealed.priv -c sealed.ctx

# Tornar persistente no TPM. Antes, confirme que o handle está livre:
# tpm2_getcap handles-persistent
tpm2_evictcontrol -C o -c sealed.ctx 0x81010002

# Apagar a chave AES
shred -v -n 5 -z -u aes.key

# Recuperar a chave AES usando o handle persistente. Ao fim do laboratório,
# remova o handle: tpm2_evictcontrol -C o -c 0x81010002
tpm2_unseal -c 0x81010002 -o aes.key
```

## 2.2. Hierarquização de Chaves

#### 2.2.1. Chave Primária (Primary Key)
- Não é um par de chaves que você usa diretamente
- É uma chave "raiz" da hierarquia
- Serve como "pai" (parent) para criar outras chaves
- É derivada deterministicamente de uma semente (seed) interna do TPM
- Sempre produz a mesma chave quando criada com os mesmos parâmetros

#### 2.2.2. Par de Chaves ECC (Child Key)
- Este é o par de chaves que você realmente usa
- É uma chave "filha" criada sob a chave primária
- É a chave que protegerá sua chave AES

#### 2.2.3. Diagrama
```
TPM Root (Owner Hierarchy)
    └── Primary Key (Chave Primária)
            └── ECC Key Pair (Seu par de chaves ECC)
                    └── AES Key (Chave AES protegida)
```

#### 2.2.4. Vantagens desta estrutura:
- **Segurança**: As chaves filhas são protegidas criptograficamente pelas chaves pais
- **Organização**: Permite criar múltiplas chaves sob a mesma hierarquia
- **Gerenciamento**: Facilita backup, migração e políticas de acesso
- **Isolamento**: Diferentes aplicações podem ter suas próprias hierarquias

# 3. Comandos

```bash
# Update package list
sudo apt update

# Install TPM 2.0 tools and libraries
sudo apt install tpm2-tools libtss2-dev libtss2-tcti-device0 libtss2-tcti-mssim0

# Check if TPM is detected
sudo dmesg | grep -i tpm

# Check TPM device
ls -la /dev/tpm*

# Criar chave primária ECC (Primary Key)
#
#       Não é um par de chaves que você usa diretamente
#       É uma chave "raiz" da hierarquia
#       Serve como "pai" (Primary) para criar outras chaves
#       É derivada deterministicamente de uma semente (seed) interna do TPM
#       Sempre produz a mesma chave quando criada com os mesmos parâmetros
#
#       ┌─────────────────┐
#       │      TPM        │
#       │  ┌───────────┐  │
#       │  │ Primary   │  │
#       │  │   Key     │  │
#       │  └───────────┘  │
#       └─────────────────┘
tpm2_createprimary -C o -g sha256 -G ecc -c primary.ctx

# Par de Chaves ECC (Child Key)
#
#       Cria um novo par de chaves (filha) sob uma chave pai já existente no TPM.
#
tpm2_create -C primary.ctx -g sha256 -G ecc:ecdsa-sha256:nistp256 -r ecc_priv.key -u ecc_pub.key -c ecc.ctx

# -C primary.ctx (Parent Context)
#       - C = Context/Parent
#       Especifica a chave pai sob a qual a nova chave será criada
#       A nova chave será protegida criptograficamente por esta chave pai
#       Pode ser um arquivo .ctx, handle ou nome
# -g sha256 (Hash Algorithm)
#       g = hash alg
#       Define o algoritmo de hash usado para:
#       Geração de chaves
#       Assinaturas
#       Operações de integridade
#       Opções : sha1, sha256, sha384, sha512
# -G ecc (Key Algorithm)
#       G = key alg
#       Define o tipo/algoritmo da chave a ser criada
#       Opções: rsa, ecc, aes e hmac
# -r ecc_priv.key (Private key output)
#       r = private
#       Arquivo onde será salva a parte privada da chave
#       Esta parte está encriptada com a chave pai
#       Só pode ser usada no TPM que a criou
# -u ecc_pub.key (Public key output)
#       u = public
#       Arquivo onde será salva a parte pública da chave
#       Contém: Informações públicas da chave, Parâmetros do algoritmo e Políticas de uso
# -c ecc.ctx (Context output)
#       c = context
#       Salva o contexto da chave criada
#       Usado para referenciar a chave em comandos futuros
#
#       ┌─────────────────┐      Sistema de Arquivos
#       │      TPM        │      ┌─────────────────┐
#       │  ┌───────────┐  │      │ ecc_priv.key    │ (encriptada)
#       │  │ Primary   │  │      │ ecc_pub.key     │ (pública)
#       │  │   Key──── │──│────> │ ecc.ctx         │ (contexto)
#       │  │           │  │      └─────────────────┘
#       │  │    ECC    │  │
#       │  │    Key    │  │
#       │  └───────────┘  │
#       └─────────────────┘

```

# 2. PKCS#11

## REFERÊNCIA

### PKCS#11
[tpm2-pkcs11](https://github.com/tpm2-software/tpm2-pkcs11)


### Geral
- [7 cool things to do with your TPM on Linux](https://www.monperrus.net/martin/7-things-to-do-with-your-TPM-on-Linux)

### Ferramentas e Código
- [tpm2-tools - GitHub Repository](https://github.com/tpm2-software/tpm2-tools)
- [tpm2-tools Documentation](https://github.com/tpm2-software/tpm2-tools/blob/master/docs/README.md)
- [TPM.dev Tutorials](https://github.com/tpm2dev/tpm.dev.tutorials)
- [tpm2-software.github.io - Tutorials](https://tpm2-software.github.io)

### Guias e Livros
- [A Practical Guide to TPM 2.0 - Springer](https://link.springer.com/book/10.1007/978-1-4302-6584-9)
