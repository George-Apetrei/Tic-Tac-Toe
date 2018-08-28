#include <xc.h>
__PROG_CONFIG(1,0x3FE4); 	// config. uC (WDT=dis|OSC=int)
__PROG_CONFIG(2,0x1EFF); 	// config. uC (PLL (OSC*4)=off) pag. 130 pdf. PIC16LF1937

unsigned char tabla_de_joc[9]={8,9,5,5,8,9,6,1,7};		//Starea tablei de joc, numerele reprezinta afisarea descrisa in vectorii de caractere
unsigned char digiti[9]={0x00,0x01,0x2,0x04,0x08,0x10,0x20,0x40,0x80};		//Vector in care se salveaza pozitiile digitilor, !Atentie: primul digit, stanga sus, este pe portul D, aceste vector se ocupa doar de portul C!, 
//[0] - Toti digitii stinsi, 
//[1] primul digit, dreapta jos, se aprinde, 
//[2] - al doilea, mijloc jos, se aprinde, ...
unsigned char vector_clipiri[9]={1,1,1,0,0,0,0,0,0};	
/*
Vector ce determina daca un anumit digit clipeste, se foloseste in urmatoarele situatii:
 1 digit clipeste - Marcheaza pozitia jucatorului pe tabla pentru orientare
 3 digiti clipesc - Marcheaza victoria unuia dintre jucatori sau, daca in meniu, indica selectia de meniu
 Toti digitii clipesc - Marcheaza final de joc, cu rezultat egalitate
*/
unsigned char selector=0,schimb=0,numarator_clipire=0,Jucator1,Jucator2,apasat=0,submeniu=0,victorie=0,cine_muta=1,acceseaza_butoane=1,tura=0;		
int alegere=0;
/*
Variabile cu urmatoarele functii:
selector		- Parcurge tabla de joc pentru afisarea digitilor, se acceseaza doar in functia de intrerupere
schimb			- Functia de clipire, 1 - Digiti de clipit raman aprinsi, 0 - Digitii de clipit se sting
numarator_clipire	- Temporizator pentru clipire, fiecare incrementare marcheaza parcurgerea tuturor digitilor o data
alegere			- Determina alegerile facute in meniuri si sub-meniuri, de asemenea, in timpul jocului efectiv, este folosit pentru a parcurge tabla de joc
Jucator1, Jucator2	- Cu ce muta fiecare jucator, 1 - X, 2 - 0 
apasat			- Indica daca butonul a fost apasat, o sa fie folosit ca si buton bistabil, pentru o apasare, indiferent cat de mult timp este tinuta, se considera a find apasat doar o data
submeniu		- Indica in ce submeniu se afla programul, 0 - Meniu principal, 1 sau 2- meniul al doilea, care este identic la ambele variante, insa se aduna una dintre cele doua valori pentru a se folosi la identificarea optiunii alese, 11 - Se iese din meniu si se porneste jocul contra calculatorului, 12 - Se iese din meniu si se porneste jocul cu jucator contra jucator, 100 - Se iese din program  
victorie		- Indica daca exista un castigator, 0 - Jocul poate avea loc, 1 - Jocul se incheie
cine_muta		- Indica care dintre cei doi jucatori muta la un oarecare moment, 1 - Jucatorul1, 2 - Jucatorul2
acceseaza_butoane	- Indica iesirea din bucla de citire a butoanelor, 1 - Se citesc butoanele, 0 - Se iese din bucla de citire a butoanelor
*/
unsigned char vector_caractere1[10]={0x00,0x00,0x3F,0x33,0x10,0x39,0x39,0x01,0x0F,0x30};		//PORT A
unsigned char vector_caractere2[10]={0x00,0x55,0x00,0x88,0x40,0x00,0x80,0x22,0x2A,0x94};		//PORT B
/*
 vector 1 [x] - PORT A , vector 2 [x] - PORT B
Legenda:
Set 0: digit gol, nu afiseaza nimic 	{0x00,0x00}
Set 1: X				{0x00,0x55}
Set 2: 0				{0x3F,0x00}
Set 3: P				{0x33,0x88}
Set 4: V				{0x06,0x11}
Set 5: C				{0x39,0x00}
Set 6: E				{0x39,0x80}
Set 7: T				{0x01,0x22}
Set 8: B				{0x
Set 9: K
*/
unsigned long y;

void interrupt aprindere_digiti();
void afisare(unsigned char pozitie);

void init();
void init_meniu();
void init_joc();

void butoane(unsigned char este_in_meniu);

void muta_dreapta(unsigned char meniu);
void muta_stanga(unsigned char meniu);
void salveaza(unsigned char meniu);

void muta_jucator();
void muta_calculator();
int minimax(unsigned char adancime);
unsigned char verifica(unsigned char minimax);
void egalitate();

void meniu();

void PvP();
void PvC();
void joc();

void main(void)
 {
   init();		//Setarile programului
    while(1){
       
           meniu();	//Se intra in meniu
	   joc();	//Se intra in joc
	 }
      
 }
 
 
void afisare(unsigned char pozitie){			//Functia de afisare a digitilor, parametri: valoarea din vectorii de caractere, se acceseaza doar din functia de intrerupere
   //Partea dedicata clipirii
   //Functionarea clipirii: Daca in vectorul de clipiri la pozitia curenta este 1 si daca variabila schimbare este 0, la digitul curent nu se afiseaza nimic insa se incrementeaza variabila numarator
   if(vector_clipiri[selector]==1&&schimb==0){
      PORTA=vector_caractere1[0];		
      PORTB=vector_caractere2[0];
      
   //----------------------
   }else{
      //Afisarea digitilor, trimite pe porturi valoarea setului respectiv
       PORTA=vector_caractere1[pozitie];
       PORTB=vector_caractere2[pozitie];
     //------------
   }

      if(numarator_clipire==7&&schimb==1){	//Daca numarator a ajuns la 7 de incrementari si digitii de clipit sunt aprinsi		
	 numarator_clipire=0;		//Se reseteaza numaratorul
	    schimb=0;			//Se seteaza digitii sa fie stinsi
      }else if(numarator_clipire==3&&schimb==0){	//3 incrementari si digitii stinsi
	 numarator_clipire=0;				//Se reseteaza numaratorul
	 schimb=1;					//Se  seteaza ca digitii sa fie aprinsi
      }
      //Clipirea este neuniforma, digitii raman aprinsi mai mult timp decat raman stinsi
}

void interrupt aprindere_digiti(){
   if(TMR0IF){
      //Se reseteaza timerul
      TMR0IF=0;	
      TMR0=0;
      
      //Se sting toti digitii
      PORTC=digiti[0];
      PORTD=digiti[0];
      
      //Multiplexor
      //In fiecare se acceseaza functia de afisare cu parametrul valorii de la pozitia respectiva si se aprinde digitul corespunzator
      switch(selector){
	 case 0:
	 afisare(tabla_de_joc[0]);
	 PORTD=digiti[1];
	 break;
	 case 1:
	 afisare(tabla_de_joc[1]);
	 PORTC=digiti[8];
	 break;
	 case 2:
	 afisare(tabla_de_joc[2]);
	 PORTC=digiti[7];
	 break;
	 case 3:
	 afisare(tabla_de_joc[3]);
	 PORTC=digiti[6];
	 break;
	 case 4:
	 afisare(tabla_de_joc[4]);
	 PORTC=digiti[5];
	 break;
	 case 5:
	 afisare(tabla_de_joc[5]);
	 PORTC=digiti[4];
	 break;
	 case 6:
	 afisare(tabla_de_joc[6]);
	 PORTC=digiti[3];
	 break;
	 case 7:
	 afisare(tabla_de_joc[7]);
	 PORTC=digiti[2];
	 break;
	 case 8:
	 afisare(tabla_de_joc[8]);
	 PORTC=digiti[1];
	 break;
      }
      //Incrementare multiplexor pana au fost parcursi toti cei 9 digiti
      if(selector<9){
	 selector++;
      }else{
	 selector=0;			//Dupa parcurgerea tuturor digitilor, se reseteaza multiplexorul
	 numarator_clipire++;		//Se incrementeaza variabila de temporizare a clipirii o data la o rulare completa a tuturor digitilor
      }
      
      
   }
}

//Functiile de mutare a "cursorului", parametrul pe care il primesc ambele este meniu, atunci cand acesta este 1, inseamna ca functia de mutare este accesata din meniu, atunci cand este 0, este accesata din joc
//Daca functiile de mutare sunt accesate din meniu, se misca rand cu rand, daca din joc, digit cu digit, acestea au exclusiv rolul de a arata unde se afla cursorul de selectie, pentru salvare a optiunii se foloseste functia "salveaza"
void muta_dreapta(unsigned char meniu){
   if(meniu){
     //Intai se incrementeaza variabila cursor
     alegere++;
     if(alegere>=3){
     alegere=0;
	}
	
      //Apoi se muta locatia clipirii, pe linia urmatoare
     if(vector_clipiri[0]){			//Daca clipeste primul digit inseamna ca o sa clipeasca tot randul, deoarece acesta in meniu selectia se face rand cu rand
	for(y=0;y<6;++y){			//Se parcurg digiti 0-5, se muta toti digitii cu 3 pozitii mai la dreapta
	   if(vector_clipiri[y]){
	      vector_clipiri[y]=0;
	   }else{
	      vector_clipiri[y]=1;
	   }
	}
     }else if(vector_clipiri[3]){		//Idem pentru digitii 3-8
     for(y=3;y<9;++y){
	   if(vector_clipiri[y]){
	      vector_clipiri[y]=0;
	   }else{
	      vector_clipiri[y]=1;
	   }
	}
     }else{					//Daca ultimii 3 sunt cei ce clipesc, se efectueaza o rotatie
	for(y=0;y<3;++y){
	      vector_clipiri[y]=1;
	}
	for(y=6;y<9;++y){
	   vector_clipiri[y]=0;
	}
     }
   }
   else			//Daca se acceseaza functia din joc
   {
      //Se seteaza la pozitia actuala digitul ca fiind gol si se inchide clipirea pe digit
      vector_clipiri[alegere]=0;
      tabla_de_joc[alegere]=0;
      //Bucla pentru parcurgerea digitilor
      while(1){
	 alegere++;		//Se trece la digitul din dreapta
	 if(alegere>8){		//Daca se iese din lista digitilor posibili
	 alegere=0;		//Se seteaza pozitia ca fiind prima
	 }
	 if(tabla_de_joc[alegere]==0){	//Daca digitul actual este gol
	    break;			//Se iese din bucla
	 }
      }

      if(cine_muta==1){			//Daca X muta
	      tabla_de_joc[alegere]= 1;	//Se inscrie X la pozitia actuala
      }else{				//Daca 0 muta
	       tabla_de_joc[alegere]=2;	//Se inscrie 0 la pozitia actuala
      }
      vector_clipiri[alegere]=1;	//Se seteaza digitul actual sa clipeasca
   }
}

void muta_stanga(unsigned char meniu){
      if(meniu){
     //Intai se incrementeaza variabila cursor
     alegere--;
     if(alegere<0){
     alegere=2;
	}
	
      //Apoi se muta locatia clipirii, pe linia precedenta
     if(vector_clipiri[0]){			//Daca primii 3 sunt cei ce clipesc, se efectueaza o rotatie
	   for(y=0;y<3;++y){			
	      vector_clipiri[y]=0;
	}
	for(y=6;y<9;++y){
	   vector_clipiri[y]=1;
	}
     }else if(vector_clipiri[3]){		//Se inverseaza digitii 9-3
     for(y=0;y<6;++y){
	   if(vector_clipiri[y]){
	      vector_clipiri[y]=0;
	   }else{
	      vector_clipiri[y]=1;
	   }
	}
     }else{					//Digitii 6-0
	for(y=3;y<9;++y){			
	   if(vector_clipiri[y]){
	      vector_clipiri[y]=0;
	   }else{
	      vector_clipiri[y]=1;
	   }
	}
     }
  }
   else			//Daca se acceseaza functia din joc
   {
      vector_clipiri[alegere]=0;	//Se inchide clipirea la pozitia actuala
      tabla_de_joc[alegere]=0;		//Se seteaza digitul de la pozitia actuala pe 0, adica inchis
      while(1){				//Bucla pentru parcurgerea tuturor digitilor 
	alegere--;			//Se alege digitul precedent
	 if(alegere<0){			//Daca variabila iese din lista digitilor
	 alegere=8;			//Se seteaza ca fiind pe ultimul digit
	 }
	 if(tabla_de_joc[alegere]==0){	//Daca digitul ales este 0
	    break;
	 }
      }
      if(cine_muta==1){			//Daca muta X
	      tabla_de_joc[alegere]= 1;	//Se pune X la pozitia actuala
      }else{				//Daca muta 0
	       tabla_de_joc[alegere]=2;	//Se pune 0 la pozitia actuala
      }
      vector_clipiri[alegere]=1;	//Se seteaza pozitia actuala sa clipeasca
   }
}
//------


void salveaza(unsigned char meniu){	//Functie pentru butonul din mijloc, are ca scop salvarea selectiei facute, parametrul pe care-l primeste este indicativ daca functia este apelata din meniu sau din joc
   if(meniu==1){				//Daca functia este accesata din menu
      if(submeniu==0){			//Daca programul se afla in meniul principal
      switch(alegere){			//Determina din care sub-meniu este accesata
	 case 0:			//PvC
	   submeniu+=1;
	   init_meniu();
	   break;
	 case 1:			//PvP
	   submeniu+=12;
	    Jucator1=1;
	    Jucator2=2;	
	 break;
	 case 2:			//Iesire din program
	    exit(1);
	 break;
      }
   }else{				//Daca programul se afla din meniul secundar
      switch(alegere){			//Determina care dintre alegeri este facuta
	 case 0:			//X
	    Jucator1=1;
	    Jucator2=2;
	    submeniu+=10;
	 break;
	 case 1:			//0
	    Jucator1=2;
	    Jucator2=1;
	    submeniu+=10;
	 break;
	 case 2:			//Bck, se intoarce in meniul precedent
	    submeniu=0;
	    init_meniu();
	 break;
      }
   }
   }		
   else					//Daca functia este accesata din joc
   {
      if(cine_muta==1&&Jucator1==1){				//Daca muta X si jucatorul1 este X
	 tabla_de_joc[alegere]=1;	//Se salveaza alegerea
      }else{					//Daca nu, se salveaza 0
	 tabla_de_joc[alegere]=2;	//Se salveaza alegerea
      }
      vector_clipiri[alegere]=0;		//Se inchide clipirea
   }
   acceseaza_butoane=0;					//Se pune pe 0 variabila de accesare a butoanelor pentru a iesi din bucla din functia butoane 
}


void init(){				//Setarile programului
    //Setari oscilator si prescalar
    OSCCON=0x6B;
    OPTION_REG=0x03;
   
   //Setari porturi
    TRISA=0x00;
    ANSELA=0x00;
   
    TRISB=0x00;
    ANSELB=0x00;
   
    TRISC=0x00;
    ANSELB=0x00;
   
    TRISD=0xE0;
    ANSELD=0x00;
   
   //TMR0 si intreruperi
    TMR0IF=0;
    TMR0=0;
   
    TMR0IE=1;
    GIE=1;
}

void init_joc(){	//Functia de setare a starii initiale a jocului
   
   for(y=0;y<9;++y){
      tabla_de_joc[y]=0;		//Tabla de joc se seteaza pe 0, toti digitii sunt stinsi
      vector_clipiri[y]=0;		//Vectorul de clipiri se seteaza pe 0, nu clipeste nici un digit
   }
   tura=0;
   victorie=0;				//Se seteaza variabila victorie pe 0, astfel incat sa fie posibila reinceperea jocului intr-o bucla
}


void init_meniu(){		//Seteaza meniul si submeniurile
   if(submeniu==0){		//Primul meniu, si se reseteaza un numar de variabile
      alegere=0;
   tabla_de_joc[0]=3;
   tabla_de_joc[1]=4;
   tabla_de_joc[2]=5;
   tabla_de_joc[3]=3;
   tabla_de_joc[4]=4;
   tabla_de_joc[5]=3;
   tabla_de_joc[6]=6;
   tabla_de_joc[7]=1;
   tabla_de_joc[8]=7;
      	 alegere=0;
      for(y=0;y<9;++y){
	       vector_clipiri[y]=0;		//Se inchide clipirea pe toti digitii
	    }
	 vector_clipiri[0]=1;
	 vector_clipiri[1]=1;
	 vector_clipiri[2]=1;
	 cine_muta=1;
   /*
   {3,4,5	P v C
    5,4,5	P v P
    6,1,7}	E X T
   */
   }else{
   tabla_de_joc[0]=0;
   tabla_de_joc[1]=1;
   tabla_de_joc[2]=0;
   tabla_de_joc[3]=0;
   tabla_de_joc[4]=2;
   tabla_de_joc[5]=0;
   tabla_de_joc[6]=8;
   tabla_de_joc[7]=5;
   tabla_de_joc[8]=9;
   /*
   {3,4,5	  X
    5,4,5	  0
    6,1,7}	B C K
   */
   }
}

void muta_jucator(){			//Functia de mutare a jucatorului
   if(cine_muta==1){				//Daca X muta
      for(y=0;y<9;++y){				//Bucla parcurge toti digitii
	 if(tabla_de_joc[y]==0){		//Atunci cand gaseste un digit care este gol
	    vector_clipiri[y]=1;		//Seteaza digitul respectiv sa clipeasca
	    tabla_de_joc[y]=1;
	    alegere=y;				//Seteaza pozitia goala ca si pozitie de inceput pentru mutarea urmatoare
	    break;
	 }
      }

      butoane(0);				//Se acceseaza functia de butoane
      cine_muta++;				//Se incrementeaza cine_muta astfel incat urmatoarea mutare o sa fie a lui 0
   }else{			//Daca 0 muta
      for(y=0;y<9;++y){				//Bucla parcurge toti digitii
	 if(tabla_de_joc[y]==0){		//Atunci cand gaseste un digit care este gol
	    vector_clipiri[y]=1;		//Seteaza digitul respectiv sa clipeasca
	    tabla_de_joc[y]=2;
	    alegere=y;				//Seteaza pozitia goala ca si pozitie de inceput pentru mutarea urmatoare
	    break;
	 }
      }

      butoane(0);				//Se acceseaza functia de butoane
      cine_muta--;				//Se modifica variabila astfel incat urmatoarea mutare sa fie a lui X
   }
}

void muta_calculator(){
   //Caz special, daca este prima mutare. Exista deoarece calculatorului ii ia foarte mult timp sa gaseasca o pozitie pentru prima mutare, si deoarece algoritmul este doar partial nu o sa gaseasca cea mai buna mutare
	 if(tura==0){
	    tabla_de_joc[4]=1;				//Daca calculatorul muta X si este prima tura, se pune in mijloc
	    goto ETICHETA_IESIRE;			//Se sare la eticheta de iesire din bucla
	 }else if(tura==1){				
	    if(tabla_de_joc[4]==0){			//Daca calculatorul este 0 si este prima tura a sa, si X nu a mutat in centru
	       tabla_de_joc[4]=2;			//Muta 0 in centru
	    goto ETICHETA_IESIRE;
	       }else{					//Daca X a mutat in centru
		  tabla_de_joc[0]=2;			//Muta 0 in coltul stanga sus
		  goto ETICHETA_IESIRE;
	       }
      }
//------------------------
      
      //Cod pentru calculare cel mai bun scor
      //Se bazeaza pe un set de bucle ce parcurg toate posibilitatile
	int tabel_scoruri[9]={0,0,0,0,0,0,0,0,0},max=0;				//Variabila tabel salveaza toate scorurile obtinute de pe urma algoritmului, max este folosit pentru a compara valorile si pentru a o gasi pe cea mai mare
	unsigned char adancime=1,mutare=0;
	int scor=0,scor_temp=0;						//Scorul obtinut de pe urma verificarii
	for(unsigned char y0=0;y0<9;++y0){		
		if(tabla_de_joc[y0]==0){							//Daca la pozitia y digitul este gol, se continua
			tabla_de_joc[y0]=Jucator2;					//Se considera posibilitatea mutarii in acea pozitie
			scor_temp=minimax(adancime);			//Se citeste daca jocul este intr-o situatie in care exista castigator
			if(scor_temp==0){				//Daca se intoarce 0 atunci la pozitia actuala jocul nu s-a incheiat
			/*1
			1*/
			   adancime++;
			   for(unsigned char y1=0;y1<9;++y1){
			   if(tabla_de_joc[y1]==0){	
			      tabla_de_joc[y1]=Jucator1;
			      scor_temp=minimax(adancime);
			      if(scor_temp==0){
				 /**2
				 2**/
				 adancime++;
			   for(unsigned char y2=0;y2<9;++y2){
			   if(tabla_de_joc[y2]==0){	
			      tabla_de_joc[y2]=Jucator2;
			      scor_temp=minimax(adancime);
			      if(scor_temp==0){
				 /***3
				 3***/
				 adancime++;
			   for(unsigned char y3=0;y3<9;++y3){
			   if(tabla_de_joc[y3]==0){	
			      tabla_de_joc[y3]=Jucator1;
			      scor_temp=minimax(adancime);
			      if(scor_temp==0){
				 /****4
				 4****/
				 adancime++;
			   for(unsigned char y4=0;y4<9;++y4){
			   if(tabla_de_joc[y4]==0){	
			      tabla_de_joc[y4]=Jucator2;
			      scor_temp=minimax(adancime);
			      if(scor_temp==0){
				 //Algoritmul este pe jumatate dezactivat pentru a scadea durata de timp pentru determinarea celei mai bune mutari, pentru reactivarea sa trebuie decomentat codul
				 /*****5
				 *
				 adancime++;
			   for(unsigned char y5=0;y5<9;++y5){
			   if(tabla_de_joc[y5]==0){	
			      tabla_de_joc[y5]=Jucator1;
			      scor_temp=minimax(adancime);
			      if(scor_temp==0){
				 /******6
				 6******
				 adancime++;
			   for(unsigned char y6=0;y6<9;++y6){
			   if(tabla_de_joc[y6]==0){	
			      tabla_de_joc[y6]=Jucator2;
			      scor_temp=minimax(adancime);
			      if(scor_temp==0){
				 /*******7
				 7*******
				 adancime++;
			   for(unsigned char y7=0;y7<9;++y7){
			   if(tabla_de_joc[y7]==0){	
			      tabla_de_joc[y7]=Jucator1;
			      scor_temp=minimax(adancime);
			      if(scor_temp==0){
				 /********8
				 8********
				 adancime++;
			   for(unsigned char y8=0;y8<9;++y8){
			   if(tabla_de_joc[y8]==0){	
			      tabla_de_joc[y8]=Jucator2;
			      scor_temp=minimax(adancime);
			      if(scor_temp==0){
				 `scor+=0;
			      }else{
				 scor+=scor_temp;
				 
			      }
			      tabla_de_joc[y8]=0;
			   }
			   }
			   adancime--;
				 /********8
				 8********
			      }else{
				 scor+=scor_temp;
			      }
			      tabla_de_joc[y7]=0;
			   }
			   }
			   adancime--;
				 /*******7
				 7*******
			      }else{
				 scor+=scor_temp;
			      }
			      tabla_de_joc[y6]=0;
			   }
			   }
			   adancime--;
				 /******6
				 6******
			      }else{
				 scor+=scor_temp;
			      }
			      tabla_de_joc[y5]=0;
			   }
			   }
			   adancime--;
				 /*****5
				 5*****/
			      }else{
				 scor+=scor_temp;
			      }
			      tabla_de_joc[y4]=0;
			   }
			   }
			   adancime--;
				 /****4
				 4****/
			      }else{
				 scor+=scor_temp;
			      }
			      tabla_de_joc[y3]=0;
			   }
			   }
			   adancime--;
				 /***3
				 3***/
			      }else{
				 scor+=scor_temp;
			      }
			      tabla_de_joc[y2]=0;
			   }
			   }
			   adancime--;
				 /**2
				 2**/
			      }else{
				 scor+=scor_temp;
			      }
			      tabla_de_joc[y1]=0;
			   }
			   }
			   adancime--;
			/*1
			1*/
			}else{
			   scor+=scor_temp;
			}
			
			tabla_de_joc[y0]=0;						//Se inverseaza schimbarea facuta anterior, deoarece nu aici se decide mutarea
	}else{
		tabel_scoruri[y0]=-1000;							//Daca la pozitia i deja exista X sau 0, se inscrie -1000 pentru a fi ignorata de algoritmul de selectie a mutarii
	       continue;
	   }
	tabel_scoruri[y0]+=scor;
	scor=0;
}
for(y=0;y<9;++y){							//Bucla pentru a atribui variabilei max cea mai mica valoare din tabel
	if(max>tabel_scoruri[y]){
		max=tabel_scoruri[y];
	}
}
for(y=0;y<9;++y){							//Bucla pentru compararea tuturor valorilor din tabel, se gaseste cea mai mare valoare si se memoreaza pozitia acesteia
	if(tabel_scoruri[y]>max){
		max=tabel_scoruri[y];
		mutare=y;
	}
}	

	tabla_de_joc[mutare]=Jucator2;				//Se inscrie pe tabla de joc mutarea calculatorului
ETICHETA_IESIRE:;		//Eticheta de iesire pentru cazul special
if(cine_muta==2){
   cine_muta--;
}else{
   cine_muta++;
}

	
}

int minimax(unsigned char adancime){

	//Verifica daca s-a ajuns la o situatie in care mutarea este castigatoare pentru oricare dintre cei 2 jucatori, adica jocul s-a incheiat
	//!!!: Toate scorurile sunt impartite la adancimea relativa la care au fost gasite deoarece un rezultat pozitiv peste 3 iteratii este mai putin important decat un rezultat negativ dupa o iteratie
	//Toate scorurile negative au o pondere mai mare deoarece, spre exemplu, este mai important sa gaseasca o solutie pentru a nu pierde in tura imediat urmatoare decat sa castige peste 3 ture
	if(verifica(1)==1 &&Jucator2==1){
		return 10/(adancime);													//Daca  X castiga si calculatorul este X
	}else if(verifica(1)==1&&Jucator2==2){
		return -100/(adancime);													//Daca X castiga si calculatorul este 0
	}
	
	if(verifica(1)==2&&Jucator2==1){
		return -100/(adancime);													//Daca 0 castiga si calculatorul este X
	}else if(verifica(1)==2&&Jucator2==2){
		return 10/(adancime);													//Daca 0 castiga si calculatorul este 0
	}
	return 0;		//Se intoarce 0 daca la pozitia actuala nu exista o victorie					
}


unsigned char verifica(unsigned char minmax){		//Functia de verificare daca exista un castigator
	char pozitii_castigatoare[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};			//Pozitiile pentru care exista o pozitie castigatoare
	    for(int i = 0; i < 8; ++i) {
        if((tabla_de_joc[pozitii_castigatoare[i][0]] == tabla_de_joc[pozitii_castigatoare[i][1]]&&tabla_de_joc[pozitii_castigatoare[i][0]]!=0) && tabla_de_joc[pozitii_castigatoare[i][0]] == tabla_de_joc[pozitii_castigatoare[i][2]]){
			//If-ul verifica toate pozitiile din toate posibilitatile de mai sus si verifica daca exista una, si aceeasi, dintre valorile posibilie(-1 sau -2) in fiecare pozitie specificata pentru fiecare posbilitate din tabelul de conditii
	
	   if(minmax==1){						//Daca functia este accesata din algoritmul de calculare a mutarii calculatorului
	   if(tabla_de_joc[pozitii_castigatoare[i][0]]==1){	//Daca castigatorul este X
		     return 1;					
		  }else{					//Daca castigatorul este 0
		     return 2;
		  }
	}else{
	  
	   //Seteaza sa clipeasca randul castigator
	   vector_clipiri[pozitii_castigatoare[i][0]]=1;
	   vector_clipiri[pozitii_castigatoare[i][1]]=1;
	   vector_clipiri[pozitii_castigatoare[i][2]]=1;
	   for(y=0;y<140000;y++);				//Asteapta o perioada de timp pentru a putea evidentia castigatorul
	      
	   //Se sting digitii indicativi
	   vector_clipiri[pozitii_castigatoare[i][0]]=0;
	   vector_clipiri[pozitii_castigatoare[i][1]]=0;
	   vector_clipiri[pozitii_castigatoare[i][2]]=0;
	   //Se seteaza variabila victorie pentru a iesi din bucla de joc
	   victorie=1;
	   
	   break;	//Se iese din bucla de verificare
	
	}
    }
 }
    return 0;
}

void egalitate(){
	    for(y=0;y<9;++y){
	       vector_clipiri[y]=1;		//Se seteaza toti digitii sa clipeasca
	    }
	    for(y=0;y<140000;y++);				//Asteapta o perioada de timp pentru a putea afisa 
	       for(y=0;y<9;++y){
	       vector_clipiri[y]=0;		//Se inchide clipirea pe toti digitii
	    }
}

void butoane(unsigned char este_in_meniu){
      while(acceseaza_butoane){	//Daca variabila acceseaza_butoane este 1, se intra in bucla
      //Am folosit o functie ce blocheaza butonul apasat astfel incat, atat timp cat butonul a fost si este in continuare apasat, se considera ca si cum a fost apasat doar o singura data, si toate celelalte butoane sunt blocate atat timp cat unul este apasat
      if(apasat==0){				//Daca nu a fost apasat nici un buton
      if(RD5){					//Daca se apasa pe butonul din "dreapta"
	   muta_dreapta(este_in_meniu);
	    apasat=1;
      }
      if(RD7){					//Daca se apasa pe butonul din "stanga"
	 muta_stanga(este_in_meniu);
	 apasat=1;
      }
      if(RD6){					//Daca se apasa pe butonul din "mijloc"
	 salveaza(este_in_meniu);
	 apasat=1;
      }
      //In toate situatiile se seteaza variabila apasat pe 1, deci in if-ul acesta nu se mai poate intra
         
   }else if(apasat==1&&(RD7||RD6||RD5)==0){		//Daca a fost apasat un buton si toate butoanele sunt depresate in momentul asta, se reseteaza variabila apasat, deci permite apasarea unui alt buton
      apasat=0;
   }
}

acceseaza_butoane=1;				//In afara buclei, variabila se pune inapoi pe valoarea 1 pentru a putea reintra in bucla in alta functie
}


void meniu(){				//Functie pentru a parcurge meniul
   init_meniu(0);			//Se seteaza tabla de joc in pozitia de start
   while(submeniu<10){			//Atat timp cat nu s-au facut alegeri care sa iasa din meniu
      butoane(1);  			//Se acceseaza functia de apelare a butoanelor
   }
}

void PvC(){		//Bucla de joc pentru modul jucator vs calculator
   do{
      if(cine_muta==Jucator1){			
	       muta_jucator();			//Muta jucator
	 tura++;
      }else{
	       muta_calculator();		//Muta calculator
	 tura++;
      }
      verifica(0);				//Verifica daca exista un castigator
      
   }while(tura<9&&victorie==0);				//Atat timp cat au fost efectuate mai putin de 9 ture si variabila victorie este 0, adica nu exista castigator, se repeta bucla
   if(victorie==0){
      egalitate();
   }
}

void PvP(){		//Bucla de joc pentru modul jucator vs jucator 
   do{
      muta_jucator();
      tura++;
      verifica(0);
   }while(tura<9&&victorie==0);//Atat timp cat au fost efectuate mai putin de 9 ture si variabila victorie este 0, adica nu exista castigator, se repeta bucla
   if(victorie==0){
      egalitate();
   }
}

void joc(){			//Functie pentu a selecta modul de joc
   init_joc();			//Se initializeaza tabla de joc cu parametrii de start
      if(submeniu==11){		//Daca s-a ales PvC in primul meniu
	 PvC();			//Player vs Computer
      }else{			//Daca s-a ales PvP in primul meniu
	 PvP();			//Player vs Player
      }
         submeniu=0;		//Se reseteaza programul, pentru reluarea meniului

}
