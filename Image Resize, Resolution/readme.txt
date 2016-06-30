BOICU IONUT 335CA ---- TEMA1 - APD

Part1.Homework(resize):
	Pentru prima parte a temei am creat o strctura image, in care am definit
cum arata imaginea. Am folosti 3 variabile de tip int pentru a salva dimensiunile
si culoarea maxima. 2 octeti de char pentru tipul imaginii si un vector de tipul
unsigned char pentru a salva pixelii. Apoi in citirea aflu tipul si marimea imaginii si 
initializez vectorul din strctura (marime * tip(RGB sau Greyscale)). Apoi printr-o
singura comanda fread citesc toti pixelii direct in vector.
	Pentru resize intai vad daca imaginea este RGB sau Greyscale apoi verific
daca factorul este par sau egal cu 3 si apoi incep prelucrarea imaginii ca in enunt.
	Pentru factorul par fac media pixelilor ce formeaza patratele de marimea factorului
si rezultatul il scriu in vectorul imaginii de output. In cazul in care factorul este 3,
atunci intai inmultesc fiecare element cu corespondentul sau din matricea gausiana si apoi
rezultatul il impart la 16 (suma elementelor din matricea gausiana).
	Acelasi procedeu si pentru Greyscale si pentru RGB, cu exceptia ca la RGB procedeul
se aplica pe 3 pixeli consecutivi.
	Pentru scriere se folosesc datele din imaginea de output, tipul, marimea si vectorul
de pixeli.
	Pentru paralelizare folosesc paralelizarea primului 'for' din functia de resize.
	Scalabilitatea se poate observa din urmatoarele date aflate de pe cluster:
---------------------------------------------------------
| Resize_factor | Num_Threads |   Time   | Picture size |
---------------------------------------------------------
|       2       |       1     | 0.582581 |  8000 x 4000 |
---------------------------------------------------------
|       2       |       4     | 0.298208 |  8000 x 4000 |
---------------------------------------------------------
|       2       |       6     | 0.150121 |  8000 x 4000 |
---------------------------------------------------------
|       2       |       8     | 0.077843 |  8000 x 4000 |
---------------------------------------------------------
---------------------------------------------------------
|       8       |       1     | 0.417358 |  8000 x 4000 |
---------------------------------------------------------
|       8       |       2     | 0.217072 |  8000 x 4000 |
---------------------------------------------------------
|       8       |       4     | 0.109933 |  8000 x 4000 |
---------------------------------------------------------
|       8       |       8     | 0.053126 |  8000 x 4000 |
---------------------------------------------------------

Part2.Homework1(Render):
	Pentru partea a 2a de tema am folosit o structura in care am salvat imaginea cu
tipul, dimensiunile, valoare maxima a culorilor si o matrice pentru pixeli.
	Initializarea matricei am facut-o cu valorile lui resolution pentru width si height,
cu P5 pentru tip ca fiind Greyscale si 255 culoarea maxima(alb).
	In render am calculat intai intr-o variabila division scalarea care se va realiza
de la resolution la dimensiunea logica de 100x100 pixeli. Am scris si cazul in care
rezolution e mai mic ca dimensiunea logica si ar insemna ca coordonatele unui pixel
sa fie inmultite cu acest division. Apoi pentru fiecare pixel din matrice aplicam
functia equation care intoarce valoarea 255 sau 0 in functie de rezultatul intors prin
masurarea distantei de la acel punct la dreapta suport. daca distanta era in intervalul
[-3,3] atunci era negru pixelul, altfel alb.
	Pentru scriere am copiat toti pixelii din matrice intr-un vector unsigned char
de marimea resolution^2  pentru a produce o singura scriere si un singur acces la memorie.
	Pentru paralelizarea codului am paralelizat primul for din intilaizarea matricii de pixeli,
primul din randarea matricei si primul din copierea pixelilor pe bufferul auxiliar pentru scriere.
	Scalabilitatea se poate observa prin testele facute pe cluster:

---------------------------------------
| Resolution | Num_Threads |   Time   |
---------------------------------------
|   10000    |       1     | 1.547229 |
---------------------------------------
|   10000    |       2     | 0.888884 |
---------------------------------------
|   10000    |       4     | 0.450042 |
---------------------------------------
|   10000    |       6     | 0.424852 |
---------------------------------------
|   10000    |       8     | 0.393524 |
---------------------------------------
---------------------------------------
|   1000     |       1     | 0.017563 |
---------------------------------------
|   1000     |       2     | 0.008986 |
---------------------------------------
|   1000     |       4     | 0.008327 |
---------------------------------------
|   1000     |       6     | 0.005647 |
---------------------------------------
|   1000     |       8     | 0.004284 |
---------------------------------------
