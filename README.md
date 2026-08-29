# Laboratórios de HSM

Material didático para conhecer HSMs, TPM 2.0 e a interface PKCS#11. Os exemplos
foram organizados como roteiros de laboratório; não representam configuração de
produção nem devem ser executados em tokens ou partições que contenham dados reais.

## Antes de começar

- Execute os exercícios em ambiente isolado e use identificadores, PINs e chaves de
  teste. Onde aparecer `<...>`, informe um valor local seu — nunca grave segredos no Git.
- Leia o comando antes de executá-lo. `init`, `clear`, `delete`, `remove` e
  `evictcontrol` podem apagar objetos, PINs ou toda a configuração do dispositivo.
- Faça um inventário dos objetos (`pkcs11-tool ... -O`, `softhsm2-util --show-slots`
  ou a ferramenta do fornecedor) antes e depois de cada laboratório.
- Para os tokens SafeNet conectados a esta máquina, a biblioteca PKCS#11 é sempre
  `/usr/lib/libeToken.so`.

## Trilhas de estudo

1. **Fundamentos e laboratório sem hardware:** [SoftHSM](softhsm/doc/softhsm.txt).
2. **Token USB e PKCS#11:** [SafeNet eToken](safenet/doc/safenet_5110.md).
3. **Segurança de plataforma:** [TPM 2.0](tpmv2/README.md).
4. **HSMs de rede:** [Dinamo](dinamo/README.md) e exemplos C/PKCS#11 em
   `dinamo/src/`.
5. **HSMs empresariais/cloud:** [Thales Luna DPoD](thales_luna/README.md) e
   [Entrust nShield](ethurst_nshield/doc/docker.txt).
6. **Cloud KMS (planejado):** [Google Cloud KMS](google_kms/README.md) — só
   cabeçalho por enquanto, sem conteúdo de laboratório.

## Convenções dos roteiros

- **Consulta:** apenas lê estado; é o ponto de partida recomendado.
- **Laboratório:** cria objetos de teste com rótulos próprios, por exemplo
  `lab_<nome>_<data>`.
- **Destrutivo:** altera PINs, inicializa, persiste ou remove objetos. Só use com a
  confirmação de que o alvo é o dispositivo de laboratório.

## Exemplos Dinamo

Os programas C não têm credenciais embutidas. Defina o alvo apenas no ambiente da
sessão e execute em uma rede de laboratório:

```bash
export DINAMO_HSM_HOST='<HSM_IP>'
export DINAMO_HSM_USER='<HSM_USER>'
export DINAMO_HSM_PASSWORD='<HSM_PASSWORD>'
```

Os binários, contextos TPM, chaves, certificados e assinaturas gerados localmente
são ignorados pelo Git.
