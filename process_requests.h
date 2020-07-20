#ifndef PROCESS_REQUESTS_H
#define PROCESS_REQUESTS_H

#include "server.h"

/* Procedure chargee de toutes les initialisations specifiques au code de
   l'etudiant */
void init();

/* Procedure chargee de la gestion d'une connexion entrante (liee a une requete 
   http */
void  process_page(struct page_request *page_request);

#endif	/* PROCESS_REQUESTS_H */
