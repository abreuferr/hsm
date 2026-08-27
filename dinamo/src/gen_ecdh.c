#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/usr/include/dinamo.h" /* header do Dinamo */

#define ECDH_KEY_NAME       "lab_ecdh_key"
#define ECDH_PAIR_KEY_NAME  "lab_ecdh_pair_key"
#define ECDH_NEW_DERIVED_KEY "lab_ecdh_derived_key"
#define ECDH_KEY_ALG    (ALG_ECC_BRAINPOOL_P512T1)

static const char *required_env(const char *name)
{
    const char *value = getenv(name);
    if (value == NULL || value[0] == '\0') {
        fprintf(stderr, "Defina a variável de ambiente %s.\\n", name);
        exit(EXIT_FAILURE);
    }
    return value;
}

int main(void)
{
    int nRet;
    struct AUTH_PWD authPwd = {0};
    HSESSIONCTX hSession = NULL;
    HKEYCTX hKey = NULL;
    HKEYCTX hPairKey = NULL;
    BYTE pbPubKey[1024] = {0};
    DWORD dwPubKeyLen = sizeof(pbPubKey);
    BYTE pbSecret[1024] = {0};
    DWORD dwSecretLen = sizeof(pbSecret);
    const char *host = required_env("DINAMO_HSM_HOST");
    const char *user = required_env("DINAMO_HSM_USER");
    const char *password = required_env("DINAMO_HSM_PASSWORD");

    //Inicializa as bibliotecas do Dinamo
    nRet = DInitialize(0);
    if(nRet)
    {
        printf("Falha na funcao: DInitialize \nCodigo de erro: %d\n", nRet);
        return EXIT_FAILURE;
    }
    printf("Bibliotecas inicializadas.\n");

    //Inicializa a estrutura para conexão com o HSM
    snprintf(authPwd.szAddr, sizeof(authPwd.szAddr), "%s", host);
    authPwd.nPort = DEFAULT_PORT;
    snprintf(authPwd.szUserId, sizeof(authPwd.szUserId), "%s", user);
    snprintf(authPwd.szPassword, sizeof(authPwd.szPassword), "%s", password);
    nRet = DOpenSession(&hSession, SS_USER_PWD, (BYTE *)&authPwd, sizeof(authPwd), ENCRYPTED_CONN);
    if(nRet)
    {
        printf("Falha na funcao: DOpenSession \nCodigo de erro: %d\n", nRet);
        goto CLEAN;
    }
    printf("Sessao com o Dinamo estabelecida.\n");
    nRet = DGenerateKey(hSession, ECDH_KEY_NAME, ECDH_KEY_ALG, 0, &hKey);
    if(nRet)
    {
        printf("Falha ao gerar chave principal.\n");
        goto CLEAN;
    }
    printf("Chave principal gerada com sucesso.\n");
    nRet = DGenerateKey(hSession, ECDH_PAIR_KEY_NAME, ECDH_KEY_ALG, 0, &hPairKey);
    if(nRet)
    {
        printf("Falha ao gerar chave do par.\n");
        goto CLEAN;
    }
    printf("Chave do par gerada com sucesso.\n");

    nRet = DExportKey(hPairKey, NULL, PUBLICKEY_BLOB, 0, pbPubKey, &dwPubKeyLen);
    if(nRet)
    {
        printf("Falha ao exportar chave publica do par.\n");
        goto CLEAN;
    }
    printf("Chave publica do par exportada com sucesso.\n");

    nRet = DGenEcdhKey(hSession,
                       DN_GEN_KEY_KDF_RAW_SECRET,
                       ECDH_KEY_NAME,
                       pbPubKey,
                       dwPubKeyLen,
                       pbSecret,
                       &dwSecretLen,
                       0);
    if(nRet)
    {
        printf("Falha ao gerar ECDH RAW Secret.\n");
        goto CLEAN;
    }
    printf("Segredo ECDH RAW secret gerado com sucesso.\n");
    BYTE pbKDFData[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};
    GEN_ECDH_X9_63 stEcdhX963Info;
    memset(&stEcdhX963Info, 0, sizeof(stEcdhX963Info));
    strncpy(stEcdhX963Info.szTargetKeyName, ECDH_NEW_DERIVED_KEY, sizeof(stEcdhX963Info.szTargetKeyName));
    stEcdhX963Info.dwTargetKeyAlg = ALG_AES_256;
    /* A chave derivada permanece não exportável; este é o comportamento seguro. */
    stEcdhX963Info.dwTargetKeyAttributes = TEMPORARY_KEY;
    stEcdhX963Info.dbPubKey.pvData = pbPubKey;
    stEcdhX963Info.dbPubKey.dwDataLen = dwPubKeyLen;
    stEcdhX963Info.dbKDFData.pvData = pbKDFData;
    stEcdhX963Info.dbKDFData.dwDataLen = sizeof(pbKDFData);

    nRet = DGenEcdhKey(hSession,
                       DN_GEN_KEY_X9_63_SHA256,
                       ECDH_KEY_NAME,
                       &stEcdhX963Info,
                       sizeof(stEcdhX963Info),
                       NULL,
                       NULL,
                       0);
    if(nRet)
    {
        printf("Falha ao gerar ECDH X9.63 Secret.\n");
        goto CLEAN;
    }
    printf("Chave ECDH X9.64 derivada com sucesso.\n");

    nRet = DRemoveObj(hSession, ECDH_NEW_DERIVED_KEY);
    if(nRet)
    {
        printf("Falha ao remover chave derivada.\n");
        goto CLEAN;
    }
    printf("Chave derivada removida com sucesso.\n");
CLEAN:
    if (hKey) {
        DDestroyKey(&hKey, REMOVE_FROM_HSM);
    }
    if (hPairKey) {
        DDestroyKey(&hPairKey, REMOVE_FROM_HSM);
    }
    printf("Contexto das chaves foram liberados.\n");
    if (hSession) {
        DCloseSession(&hSession, 0);
        printf("Sessao encerrada.\n");
    }
    memset(pbSecret, 0, sizeof(pbSecret));
    DFinalize();
    printf("Bibliotecas finalizada.\n");
    return nRet ? EXIT_FAILURE : EXIT_SUCCESS;
}
