
#include "PrintService.h"
#include "rpc_funciones.h"
#include "stdlib.h"
int printService_1(char *stringToPrint)
{
    //Cliente RPC para imprimir las operaciones. Recibe la string del servidor y la envía al servidor RPC. IP viene de una
    //variable de entorno "IP_TUPLAS"
    char *host = getenv("IP_TUPLAS");
	CLIENT *clnt;
	enum clnt_stat retval_1;
	int result_1;

	clnt = clnt_create (host, PRINTSERVICE, PRINTSERVICEVER, "tcp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}

	retval_1 = rpc_displayvalues_1(stringToPrint, &result_1, clnt);
	if (retval_1 != RPC_SUCCESS) {
		clnt_perror (clnt, "call failed");
	}
    clnt_destroy(clnt);
    return result_1;

}

