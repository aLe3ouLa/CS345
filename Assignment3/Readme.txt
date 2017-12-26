---------------------------------------------------
Name: Alexandra Barka
login: barka@csd.uoc.gr
am: 2867
hy345 - Operating 
Exercise_3 - Add two new system calls to linux kernel.
---------------------------------------------------

Perigrafh bhmatwn dhmiourgias system call
-----------------------------------------

1) Sto sched.h ebala ta 3 nea pedia (pid_t root_pid, int time_limit, int time_interval)
2) Sto init_task.h arxikopoihsa ta parapanw me -1
3) Bazoume sto syscalls.h ta function prototypes. 
4) sto /kernel/ eftia3a ta setproclimit kai getproclimit pou ekteloun ta sys call sys_setproclimit kai sys_getproclimit. Kai ta 2 akolou9oun thn ekfwnhsh
Sth setproclimit, blepoume ti einai to pid, an einai -1 pairnoume thn trexousa diergasia kai allazoume ta pedia tou 1). An exei 9ugatrikes diergasies me thn anadromikh synarthsh setproclimit_dump allazoume ta pedia kai se autes. Etsi dhmiourgoume to process family. An to pid > -1 psaxnoume na broume an uparxei prwta kai meta kanoume antistoixes epe3ergasies methn current. An einai arnhtiko to pid h an den uparxei diergasia epistrefei EINVAL. 
Sthn getproclimit pairnoume ena pid, briskoume se poia diergasia anaferetai opws parapanw(current, exist, not exist) kai gemizoume to struct proclimit me stoixeia gia ayth th diergasia. 
5) Allazoume to Makefile etsi wste na sumperilabei ta sys calls me obj += setproclimit.o getproclimit.o 

Epishs prepei na epe3ergastoume to unistd_32.h gia na baloume system call numbers gia ta system calls mas. Kai to syscall_table_32.S sto opoio prosthetoume ta onomata synarthsewn pou ylopoioun ta system calls mas. Telos gia to proclimit.h to dhmiurgoume sto /include/. 

Alles allages pou eginan: -Sto proclimit.h sto uparxei (kai prepei na einai mazi me ta tests) exoun prostethei kai ta macros pou kanoun ta syscalls na moiazoun me function calls.
- h getproclimit kalei thn cputime_to_usecs kai epistrefei ton xrono se millesecs 

Parathrhseis test files:
------------------------
 (xronoi se milleseconds)

Test1:  Sto prwto test file o total_cpu_time einai polu mikros (print1:5000 print2: 6000 print3: 7000) ayto ofeiletai oti den apasxoloume polu ton epe3ergasth me tis diergasies mas. Akoma parathroume oti para to oti kanoume sleep(5) dhladh kanoume idle thn diergasia gia 5 deuterolepta h diergasia exei diafora xronou 1000 millesecs. Ayto sumbainei giati h sleep den katalwnei cpu ektos tou invocation overhead. O pollaplasiasmos exei kapoio user time kai se ayton ofeilontai oi xronoi pou blepoume.

Test2: Sto deutero test file parathroume polu megaluterh xrhsh ths cpu sugkritika me thn 1h. Ayto symbainei logw twn pollwn fork pou kanoume .H sleep opws eipame kai prin den katanalwnei cpu alla efoson h fork einai system call 9a einai perissotero xrono se kernel mode ara 9a katanalwnei perissotero system time. 

Test3: Sto trito test file  o xronos ths diergasias einai analogo twn poswn paidiwn exei h diergasia. 

