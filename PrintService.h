/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _PRINTSERVICE_H_RPCGEN
#define _PRINTSERVICE_H_RPCGEN

#include <rpc/rpc.h>
#include "rpc_funciones.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


#define PRINTSERVICE 99
#define PRINTSERVICEVER 1

#if defined(__STDC__) || defined(__cplusplus)
#define rpc_displayvalues 1
extern  enum clnt_stat rpc_displayvalues_1(char *, int *, CLIENT *);
extern  bool_t rpc_displayvalues_1_svc(char *, int *, struct svc_req *);
extern int printservice_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define rpc_displayvalues 1
extern  enum clnt_stat rpc_displayvalues_1();
extern  bool_t rpc_displayvalues_1_svc();
extern int printservice_1_freeresult ();
#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_PRINTSERVICE_H_RPCGEN */
