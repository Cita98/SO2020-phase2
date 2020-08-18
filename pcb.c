#include "pcb.h"

LIST_HEAD(pcbFree_h); /*Dichiarazione e inizializzazione nuova lista*/
HIDDEN pcb_t pcbFree_table[MAXPROC];


/* -------------------- FUNZIONE 1 -------------------- */

void initPcbs(void)
{
     int i; /* Counter */
    /* Inizializzazione della sentinella */
    INIT_LIST_HEAD(&(pcbFree_h));

    /* Inizializzazione della lista pcbFree */
    for (i = 0; i < MAXPROC; i++)
    {
	/* Creo un nuovo puntatore all'elemento i del vettore dei Pcb */
        pcb_t* newPcbEl = &pcbFree_table[i];
        /* Aggiungo l'elemento in testa alla lista pcbFree concatenando il puntatore al list_head dell'elemento e la sentinella della lista (pcbFree_h) */
	list_add(&(newPcbEl->p_next), &(pcbFree_h));
	}
}


/* -------------------- FUNZIONE 2 -------------------- */

void freePcb(pcb_t *p)
{
    /* Aggiungo l'elemento puntato da p alla lista pcbFree concatenando il puntatore al list_head dell'elemento e la sentinella (pcbFree_h) */
    list_add(&(p->p_next), &(pcbFree_h)); 
    /* Non c'è bisogno di reinizializzare il pcb poiché verrà inizializzato in fase di allocazione (allocPcb)*/
}


/* -------------------- FUNZIONE 3 -------------------- */

pcb_t* allocPcb()
{
   /* Se la lista pcbFree è vuota ritorna NULL */
   if(list_empty(&(pcbFree_h))){return NULL;}
   /* Altrimenti: */
   else{
   /* Salvo il puntatore del pcb da rimuovere*/
   pcb_t* P_pcb = container_of(pcbFree_h.next, pcb_t, p_next);
   /* Stacco, elimino l'elemento dalla lista */
   list_del(pcbFree_h.next);
   /* Inizializzo i campi, del pcb_t puntato da P_pcb, a NULL/0 */
   INIT_LIST_HEAD(&(P_pcb->p_next));
   INIT_LIST_HEAD(&(P_pcb->p_child));
   INIT_LIST_HEAD(&(P_pcb->p_sib));
   P_pcb->p_parent = NULL;
   P_pcb->p_semkey = NULL;
   P_pcb->priority = 0;

   #ifdef TARGET_UMPS
   state_t* state = &(P_pcb->p_s);
   int i; /* Counter */
    /* Inizializzo tutti i campi a 0 */
    state->entry_hi = 0;
    state->cause = 0;
    state->status = 0;
    state->pc_epc = 0;
    /* scorro e inizializzo a 0 tutti i registri */
    for (i = 0; i<29; i++)
      state->gpr[i] = 0;
    state->hi = 0;
    state->lo = 0;
   //P_pcb->priority = 0;
   #endif

   #ifdef TARGET_UARM
   P_pcb->p_s.a1 = 0;  
   P_pcb->p_s.a2 = 0;   
   P_pcb->p_s.a3 = 0;  
   P_pcb->p_s.a4 = 0;   
   P_pcb->p_s.v1 = 0;   
   P_pcb->p_s.v2 = 0;    
   P_pcb->p_s.v3 = 0;  
   P_pcb->p_s.v4 = 0;   
   P_pcb->p_s.v5 = 0;   
   P_pcb->p_s.v6 = 0; 
   P_pcb->p_s.sl = 0;  
   P_pcb->p_s.fp = 0;    
   P_pcb->p_s.ip = 0;   
   P_pcb->p_s.sp = 0;    
   P_pcb->p_s.lr = 0;    
   P_pcb->p_s.pc = 0;    
   P_pcb->p_s.cpsr = 0;
   P_pcb->p_s.CP15_Control = 0;
   P_pcb->p_s.CP15_EntryHi = 0;
   P_pcb->p_s.CP15_Cause = 0;
   P_pcb->p_s.TOD_Hi = 0;
   P_pcb->p_s.TOD_Low = 0;
   #endif


   /* Restituisco il puntatore all'elemento rimosso */
   return P_pcb;
   }
}


/* -------------------- FUNZIONE 4 -------------------- */

void mkEmptyProcQ(struct list_head *head)
{
    /* inizializzo i puntatori della sentinella */
    INIT_LIST_HEAD(head);
}


/* -------------------- FUNZIONE 5 -------------------- */

int emptyProcQ(struct list_head *head)
{ 
    return list_empty(head); /* restituisco TRUE se la lista puntata da head è vuota, FALSE altrimenti */
}


/* -------------------- FUNZIONE 6 -------------------- */

void insertProcQ(struct list_head* head, pcb_t* p)
{
	pcb_t *iterator; /* Variabile di tipo puntatore all'elemento pcb_t, consente di scorrere la lista */
	/* Scorro la lista dei processi puntata da head */
	list_for_each_entry(iterator, head, p_next)
	{ /* Confronto il campo priorità dell'elemento puntato da p con ogniuno di quelli presenti nella lista per inserirlo in ordine decrescente */
	  /* Se l'elemento puntato da p ha priorità maggiore rispetto a quello puntato dall'iteratore */
		if (p->priority > iterator->priority)
		{
	  /* Allora aggiungo p prima di tale elemento */ 
			list_add(&(p->p_next), iterator->p_next.prev);
			return;
		}
	}
	/* Se la priorità di p non è maggiore di nessun elemento nella lista allora andrà aggiunto per ultimo, in coda alla lista */
	list_add_tail(&(p->p_next), head);
	return;
}



/* -------------------- FUNZIONE 7 -------------------- */

pcb_t *headProcQ(struct list_head *head)
{
    /* Se la lista è vuota ritorno null */
    if(list_empty(head)){return NULL;}
    /* Altrimenti estraggo l'indirizzo del primo elemento della lista */
    struct list_head *first = list_next(head);
    /* Estraggo l'indirizzo del contenitore di first. (Ovvero il primo pcb della lista, contenente il puntatore first) */
    pcb_t *firstPcb = container_of(first, pcb_t, p_next);
    /* lo restituisco */
    return firstPcb;
}


/* -------------------- FUNZIONE 8 -------------------- */

pcb_t* removeProcQ(struct list_head *head)
{
     pcb_t *firstPcb = headProcQ(head); /* Ritorna NULL se la lista è vuota o il primo elemento della lista */
    if(firstPcb != NULL) /* Se la lista non è vuota */
    { list_del(head->next);} /* elimino l'elemento dalla lista*/
    /* Ritorno NULL o il puntatore all'elemento rimosso */
    return firstPcb;
}


/* -------------------- FUNZIONE 9 -------------------- */

pcb_t *outProcQ(struct list_head *head, pcb_t *p)
{
    /*  Scorro la lista dei pcb  */
    struct list_head* pos;
    list_for_each(pos, head)
    {
        /*  Estraggo il puntatore all'elemento corrente */
        pcb_t* curr = container_of(pos, pcb_t, p_next);
        /*  Se l'elemento corrente è uguale a quello da eliminare */
        if(p == curr)
        {
            list_del((&(p->p_next))); /* Lo elimino */
            return p;
        }
    }	
	return NULL; /* Altrimenti ritorno NULL se l'elemento richiesto non è presente */
}


/* -------------------- FUNZIONE 10 -------------------- */

int emptyChild(pcb_t *p) 
{
   return list_empty(&(p->p_child)); /* Restituisce true se la lista dei processi figli associata al pcb puntato da p è vuota, false altrimenti */
}


/* -------------------- FUNZIONE 11 -------------------- */

void insertChild(pcb_t *prnt,pcb_t *p) 
{
    p->p_parent = prnt; /* p è figlio di prnt */
    /* aggiungo l'elemento in coda alla lista dei figli*/
    list_add_tail(&(p->p_sib), &(prnt->p_child));
}


/* -------------------- FUNZIONE 12 -------------------- */

pcb_t* removeChild(pcb_t *p)
{
	pcb_t* first_PcbChild = NULL; /* Variabile che conterrà il puntatore al primo figlio */
	/*Controllo se p ha figli.*/    
    if (!emptyChild(p)) 
    {
	/* estraggo l'indirizzo del primo concatenatore figlio */
        struct list_head* firstChild = list_next(&(p->p_child));
        /* estraggo l'indirizzo del contenitore del primo concatenatore */
        first_PcbChild = container_of(firstChild, pcb_t, p_sib); 
        /* elimino il puntatore al padre */
        first_PcbChild->p_parent = NULL;
        /* elimino il figlio dalla lista */
        list_del(firstChild);	
        		
    }
	return first_PcbChild; /* restituisco NULL o il puntatore al figlio rimosso */
}


/* -------------------- FUNZIONE 13 -------------------- */

pcb_t* outChild(pcb_t *p) 
{
    /*controllo se p ha un padre. Se non ha un padre restituisco NULL*/
    if ((&(p->p_parent))== NULL)
        return NULL;
        
        else /* Se ha un padre */
        {
            /*elimino l'elemento dall'albero*/
            list_del (&(p->p_sib));
            p->p_parent = NULL;
            /*restituisco l'elemento rimosso*/
            return p;
        }
}













