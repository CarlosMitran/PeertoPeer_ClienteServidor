
#include "PrintService.h"
#include "rpc_funciones.h"
#include "stdio.h"
#include "string.h"

bool_t
rpc_displayvalues_1_svc(char *arg1, int *result,  struct svc_req *rqstp) {
    //Servidor RPC. Levanta el servidor y escucha. Cuando recibe la información la imprime
    bool_t retval;
    if (strlen(arg1) < 1) {
        *result = -1;
        retval = FALSE;
        return retval;
    }
    printf("%s \n", arg1);
    retval = TRUE;
    *result = 0;
	return retval;
}

int
printservice_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free (xdr_result, result);

	/*
	 * Insert additional freeing code here, if needed
	 */

	return 1;
}
