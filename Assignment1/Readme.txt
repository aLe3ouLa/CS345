README FILE

-------------------

Alexandra Barka

barka@csd.uoc.gr

am: 2867

hy345

Exercise_1 - Ylopoihsh tou dikou mas shell

--------------------


To shell kanei thn e3hs douleia. 
Oles mas oi entoles perikleiontai apo ena while (1). 
Apo ayto to while mporoume na bgoume mono an o xrhsths kanei type "exit"

Arxika loipon typwnei to prompt poy zhteitai.
 Epeita to programma diabazei thn entolh pou dinetai apo ton xrhsth kai thn xwrizei 
se arguments.Exoume 2 kathgories entolwn: aytes pou e3uphretountai kanontas fork 
to shell kai exec'ed kai aytes pou ektelountai apo to idio to shell shell.


Sth deyterh periptwsh anhkoun oi entoles exit, cd, mytime :

-no args: Se ayth thn prwth periptwsh o xrhsths den exei dwsei katholou entolh h exei 
kanei "time ". Opote to shell mas paei sto epomeno loop h an uparxei to time tupwnei 
to xrono kai proxwra sto epomeno loop.

-exit: To programma 

-cd: To programma allazei directory. An dothei "cd " proxwra sthn epomenh loop. An dwthei 
time cd ../ kanei allagei directory kai tupwnei ton xrono.


Me fork: To fork tha mas kanei ena antigrafo thw mhtrikhs diergasias. 


ama to fork mas dwsei pid == 0 tote tha kanoume ta e3hs: 

-an exoume background processes tupou: ls -al & tote typwnoume to pid kai to name ths 
diergasias. 

-an exoume redirection allazoume thn kateuthunsh tou programmatos apo h pros enan descriptor.

-an exoume pipeline xwrizoume thn entolh kai kanoume katallhla redirections apo thn mia 
sthn allh diergasia.

-an kanoume history tupwnei to istoriko twn mehri twra entolwn. 




Paradeigmata gia th xrhsh tou mysh pou ylopoihthikan:


ls

ls -al

cat sometxt.txt

cat < sometxt.txt > someothertxt.txt

ps axl | grep usr

ls -al | sort -r -k 5 | head -n 2

history

history | grep ls

mytime cat /usr/share/dict/american-english

mytime ps axl | grep usr

cd ../

cd ./SomeDir

exit

mkdir new

cd ./new

cp test1.c test2.c

cat mysh.c | grep fork

gcc mysh.c
./a.out


