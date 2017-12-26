README FILE
-------------------
Alexandra Barka
barka@csd.uoc.gr
am: 2867
hy345
Exercise_2 - Mantepste grhgora ena string xrhsimopoiwntas polla threads.
-------------------


server.c
--------

To programma pairnei apo thn grammh entolwn enan akeraio o opoios einai o ari9mos grammatwn pou tha exei to 
string. Emeis sthn askhsh anoigoume ena socket kai perimenoume kapoion client na sundethei. To programma termatizei me to ctrl_C me signal handler. Afou sundethei kapoios client mas stelnei ton ari9mo twn threads me ton opoio tha vroume  to string. Kai dhmiourgoume 3 memory segments. Sto 1o grafei o client tous xarakthres sto 2o grafei o server an einai swstoi oi oxi kai sto 3o einai to segment twn semaphores. Epeita dhmiourgoume & kanoume join ta threads. H douleia pou kanoun ta threads (grafei server- diavazei client + katallhlos sugxronismos me semaphores) ka9orizetai sthn sunarthsh pou pairnei ws orisma  h pthread_create. Afou brei o client to string typwnei ton xrono pou tou phre na to brei kleinoume th sundesh kai epeita kanoume detach kai apodesmeuoume ta memory segments. 

client.c
--------

O client sundeetai se enan server, stelnei ton # twn threads pou prepei na dhmiourghthoun kai lambanei to posous xarakthres prepei na mantepsei kai ta kleidia twn segments. Epeita kanei attach sta memory segments kai dhmiourgei & kanei join ta threads. H douleia pou kanoun ta threads (grafei client-diavazei server + katallhlos sugxronismos me semaphores) ka9orizetai sthn sunarthsh pou pairnei ws orisma  h pthread_create. Afou to brei, typwnei to string pou brike kai ton xrono pou tou phre, kleinei th sundesh kai kanei detach apo ta memory segments. 




