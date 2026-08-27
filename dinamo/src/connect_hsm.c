#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/usr/include/dinamo.h" /* header do Dinamo */

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
    const char *host = required_env("DINAMO_HSM_HOST");
    const char *user = required_env("DINAMO_HSM_USER");
    const char *password = required_env("DINAMO_HSM_PASSWORD");

    //Inicializa as bibliotecas do Dinamo
    nRet = DInitialize(0);

    if (nRet) {
        printf("Falha na funcao: DInitialize \nCodigo de erro: %d\n",nRet);
        return EXIT_FAILURE;
    }

    printf("Bibliotecas inicializadas.\n");

    //Inicializa a estrutura para conexão com o HSM
    snprintf(authPwd.szAddr, sizeof(authPwd.szAddr), "%s", host);
    authPwd.nPort = DEFAULT_PORT;
    snprintf(authPwd.szUserId, sizeof(authPwd.szUserId), "%s", user);
    snprintf(authPwd.szPassword, sizeof(authPwd.szPassword), "%s", password);

    nRet = DOpenSession(&hSession, SS_USER_PWD, (BYTE *)&authPwd, sizeof(authPwd),ENCRYPTED_CONN);

    if (nRet) {
        printf("Falha na funcao: DOpenSession \nCodigo de erro: %d\n",nRet);
        goto clean;
    }

    printf("Sessao com o Dinamo estabelecida.\n");

    nRet = DAdmOperation(hSession, AO_KEEPALIVE, NULL, 0, 0);

    if (nRet) {
        printf("Falha na funcao: DAdmOperation \nCodigo de erro: %d\n",nRet);
        goto clean;
    }

    printf("O Dinamo esta operacional.\n");

    clean:

    if (hSession) {
        DCloseSession(&hSession, 0);
        printf("Sessao encerrada.\n");
    }

    DFinalize();
    printf("Bibliotecas finalizada.\n");
    return nRet ? EXIT_FAILURE : EXIT_SUCCESS;
}
