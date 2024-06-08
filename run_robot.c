//
//  run_robot.c
//  Robot_mekatronik_1997
//  Created by Marie Jönsson on 2024-06-08.

/*
 Ett projekt i Mekatronik Lp2 1997, Designingenjör, Högskolan i Skövde
 Hur Skalman blev till!
 I kursen mekatronik har klassen tilldelats ett projekt och delats in i grupper. 
 Syftet med projektet var att konstruera en liten robot.
 Den ska ha förmågan att inom en begränsad cirkulär yta (spelplan) lokalisera, uppsöka och transportera halvfyllda konservburkar hem till hemmabasen inom 5 minuter.
 Konservburkarna är slumpvis  utplacerade i en cirkel som är koaxial med spelplanen.
 Gruppens målsättning var att framställa en stabil farkost inom den angivna budgeten.
 Roboten skulle ha förmågan att genom en temperatursensro känna av - den av handledaren bestämda - "rätt" burk och ta den med sig in i mål.
 Resultatet blev "Skalman 1" - en sködpaddsrobot på två hjul och en kulrulle samt fångstarm i gapet.
 Skalmans program är skrivet i Dynamic C (OBS 1997) 
 */


/* 
 PA0-4 används som utgångar (PA0 fungerar ej som ingång men som utgång)
 PA0 till fångstmotor, (0x01)
 PA1 till fram/back, genom IN på L6203, vänster motor, (0x02)
 PA2 till stopp, genom ENABLE på L6203, (0x04)
 PA3 till fram/back, genom IN på L6203, höger motor, (0x08)
 PA4 til stopp, genomENABLE på L6203, höger motor, (0x10)
 
 PA5-7 används som ingångar
 
 PA5 till mikrogapströmställaren i "fällan", (0x02)
 PA6 till tempsensorn varm/kall, ger utslag: normalt 1, "rätt" 0, (0x40)
 PA7 till PÅ-varm/AV/PÅ-kall, 1 = stopp, 0 = söker efter varm, 0 = söker efter kall
 
 IN     ENABLE  De olika kombinationerna ger:
        0       = stopp
 1      1       = fram
 0      1       = back
 
 PB4-7 används som ingångar
 
 PB4 till refledetektor höger bak, (0x10)
 PB5 till reflexdetektor vänster bak, (0x20)
 PB6 till relexdetektor höger fram, (0x40)
 PB7 till relexdetektor vänster fram, (0x80)
 */


#use vdriver.lib  // behövs för att kunna använda costate

main()
{
    // deklararade variabler
    int indata, INDATA, k, p, m, HB, VB, HF, VF, VFHF, VBHB, VFmikro, HFmikro, VFHFmikro;
        
    k = 0; VF = 0; HFmikro = 0;
    p = 1; HF = 0; VFHFmikro = 0;
    m = 1; VFHF = 0;
    HB = 0; VBHB = 0;
    VB = 0; VFmikro = 0;
    
    VdInit();    // behövs för att använda tidsfunktionerna DelayMS och DelaySec
    
    resPIOCA(0x1f); // Sätter PA0-PA04 till utgångar
    resPIODA(0x01); // Sätter PA0 till 0
    rePIODA(0x1E);  // Sätter PA1-PA4 till 0 =  stopp på motorerna
    
    while(1)
    {
        hitwd;  //vakthunden väcks
        runwatch;
        
        costate
        {
            INDATA = inport(PIODB) & (0xf0); // hämtar insignalerna från PA5-7
        }
        
            while(!k)
                /* startsignalen till motorerna. Så länge som k=0, står motorerna på stopp. När startknappen trycks in ändras k till 1 och programmet fortsätter */
            {
                stoppB();
                indata = inport(PIODA) & (0xE0);
                if (!(indata & 0x80))
                {
                    k = 1;
                }
                
            }   // end while(!k) loop
        
            
            while(k)
            {
                indata = inport(PIODA) & (0xE0);
                INDATA = inport(PIODB) & (0xf0);
                
                /* k har blivit 1 och de båda motorera som driver roboten ställs på fram så roboten kör framåt tills någon av kantsensorerna triggas */
                if ((HB == 0) && (VB == 0) && (VF == 0) && (HF == 0) && (VBHB == 0) && (VFHF == 0) (INDATA & 0x10) && (INDATA & 0x20) && (INDATA & 0x80) && (!(indata & 0x20)) && (indata & 0x40))
                { 
                    framB();
                }
            
                hitwd;      // vakthunden väcks en gång till för säkerhets skull

        
        
        // ********** Sensorfunktioner  ***********
        
        costate         // costate 1 börjar
        {
            // här avläses insignaler
            indata =  inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
            
            /* Om kantsensorn som sitter bak till höger triggas, sätts HB=1, HF=0 och VF=0 */
            if ((!(INDATA & 0x10)) && (INDATA & 0x20) && (INDATA & 0x40) && (INDATA & 0x80) && (!(indata & 0x20)) && (indata & 0x40)
                {
                HB = 1;
                HF = 0;
                VF = 0;
                m = 1;
                }
                
                /* Så länge som HB = 1 kör roboten framåt en kort sträcka för att komma bort från kanten. Sen svänger den lite åt vänster för att sedan åka rakt framåt. HB=0 i slutet av snurran så den inte kan gå in en gång till i snurran */
            while(HB == 1)
            {
            framB;
            waitfor(DelayMS(1500));
            svvanster();
            waitfor(DelayMS(200));
            framB();
            HB = 0;
            }
            
            // här avläses insignalerna
            indata = inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
            
            if ( (!(INDATA & 0x20)) && (INDATA & 0x10) && (INDATA & 0x40) && (INDATA & 0x80) && (!(indata & 0x20)) && (indata & 0x40))
                {
                VB = 1;
                HF = 0;
                VF = 0;
                m = 1;
                }
                
            while(VB == 1)
                /* så länge som VB = 1 kör roboten framåt en kort sträcka för att komma bort från kanten. Sen svänger den lite till höger innan det blir full fart framåt igen. VB = 0 i slutet av snurran så den inte kan gå in i snurran igen*/
                {
                framB();
                waitfor(DelayMS(1500));
                svhoger();
                waitfor(DelayMS(200));
                framB();
                VB = 0;
                }
                
            // här avläses insignalerna
            indata = inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
                
            if ((!(INDATA & 0x40)) && (INDATA & 0x10) && (INDATA & 0x20)) && (INDATA & 0x80) && (!(indata & 0x20)) && (indata & 0x40)
                {
                HF = 1;
                }
            
            while(HF=1)
                /* Så länge som HF=1 backar roboten en kort sträcka för att komma ifrån kanten. För att sedan svänga en kort stund åt höger innan den backar tills någon eller båda kantsensorerna triggas. HF = 2 för att den inte ska gå in i while(k) snurran som sätter motorerna till fram igen, eller gå in i samma snurra en gång till */
                {
                backB;
                waitfor(DelayMS(2000));
                    svhoger();
                waitfor(DelayMS(3000));
                    backB();
                    HF = 2;
                } // end while(HF=1) loop
                
            
            // här avläses insignalerna
            indata = inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
                
            while(VF == 1)
                /* så länge som VF = 1 backar roboten en liten bit för att komma bort från kanten. För att sedan svänga en kort stund åt höger innan den backar tills någon eller båda två bakre kantsensorerna triggas. VF=2 för att den inte ska gå in i while(k) snurran som sätter motorerna till fram igen, eller gå in i samma snurra en gång till*/
                {
                backB();
                waitfor(DelayMS(2000));
                svhoger();
                waitfor(DelayMS(3000));
                backB();
                VF = 2;
                } // end while(VF == 1)
            
                
            // här avläses insignalerna
            indata = inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
                
            if ((!(INDATA & 0xC0)) && (INDATA & 0x10) && (INDATA & 0x20)) && (!(indata & 0x20)) && (indata & 0x40)
                /* om båda kantsensorerna som sitter framtill triggas, sätts VFHF = 1 */
                {
                VFHF = 1;
                }
                
            while(VFHF == 1)
                /* så länge som VFHF = 1 backar roboten en liten bit för att komma bort från kanten. För att sedan svänga en kort stund åt vänster innan den backar tills någon eller båda två bakre kantsensorerna triggas. VFHF=2 för att den inte ska gå in i while(k) snurran som sätter motorerna till fram igen, eller gå in i samma snurra en gång till*/
                {
                backB();
                waitfor(DelayMS(2000));
                svvanster();
                waitfor(DelayMS(3000));
                backB();
                VFHF = 2;
                } // end while(VFHF == 1)
                
                
            // här avläses insignalerna
            indata = inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
                
            if ((!(INDATA & 0x30)) && (INDATA & 0x40) && (INDATA & 0x80)) && (!(indata & 0x20)) && (indata & 0x40)
                /* om båda kantsensorerna som sitter framtill triggas, sätts VBHB = 1 */
                {
                VBHB = 1;
                VFHF = 0;
                VF = 0;
                HF = 0:
                m = 1;
                }
            
            while(VBHB == 1)
                /* så länge som VBHBF = 1 kör roboten framåt en kort sträcka bit för att komma bort från kanten. För att sedan svänga en kort stund åt höger innan den kör framåt tills någon eller båda två främre kantsensorerna triggas. VBHB=2 för att den inte ska gå in i while(k) snurran som sätter motorerna till fram igen, eller gå in i samma snurra en gång till*/
                {
                framB();
                waitfor(DelayMS(1500));
                svhoger();
                waitfor(DelayMS(200));
                framB();
                VBHB = 2;
                } // end while(VBHB == 1)
                
                
            // här avläses insignalerna
            indata = inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
            
            if (indata & 0x20) && (!(indata & 0x40)) && (INDATA & 0x10) && (INDATA & 0x20) && (INDATA & 0x80) && (p == 1)
                /* om temperatursensorn och mikroströmbrytaren triggas stannar roboten och fångstarmens motor (F.M) startar och burken fångas in och ställs på kant. p sätts till 0 så den inte går in i denna funktion igen */
                {
                stoppB();   // stannar båda motorerna
                start();    // F.M. startar och kroken fångar burken
                waitfor(DelayMS(15000));
                p = 0;
                }
        
            // här avläses insignalerna
            indata = inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
                
            if (indata & 0x20) && (INDATA & 0x10) && (INDATA & 0x20) && (INDATA & 0x40) && (INDATA & 0x80) && (p == 0) && (VFmikro == 0) && (HFmikro == 0) && (VFHFmikro == 0)
                /* om mikroströmbrytaren triggas och p= 0 kör roboten hädanefter framåt tills någon eller båda av de främre sensorerna triggas */
                {
                framB();
                p = 0;
                }
                
                
            // här avläses insignalerna
            indata = inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
                
            if ((!(INDATA & 0x40)) && (INDATA & 0x20) && (INDATA & 0x10)) && (INDATA & 0x20) && (INDATA & 0x80) && (HFmikro == 0)
                /* om kantsensorn som sitter framtill till höger och mikroströmställaren triggas, sätts HFmikro = 1 */
                {
                HFmikro = 1;
                }
                
            while(HFmikro == 1)
                /* så länge som HFmikro = 1 backar roboten en kort sträcka för att sedan svänga lite till vänster innan roboten forrtsätter framåt tills någon av de två främre kantsensorerna triggas igen */
                {
                backB();
                waitfor(DelayMS(1000));
                svvanster();
                waitfor(DelayMS(500));
                } // end while(HFmikro == 1)
                
            // här avläses insignalerna
            indata = inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
                
            if ((!(INDATA & 0x80)) && (indata & 0x20) && (INDATA & 0x10)) && (INDATA & 0x20) && (INDATA & 0x40) && (VFmikro == 0)
                /* om kantsensorn som sitter framtill till vänster och mikroströmställaren triggas, sätts VFmikro = 1 */
                {
                VFmikro = 1;
                }
                
            while(VFmikro == 1)
                /* så länge som VFmikro = 1 backar roboten en kort sträcka för att sedan svänga lite till vänster innan roboten forrtsätter framåt tills någon av de två främre kantsensorerna triggas igen */
                {
                backB();
                waitfor(DelayMS(1000));
                svhoger();
                waitfor(DelayMS(500));
                } // end while(VFmikro == 1)
                
                
            // här avläses insignalerna
            indata = inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
                
            if ((!(INDATA & 0xC0)) && (indata & 0x20) && (INDATA & 0x10)) && (INDATA & 0x20) && (VFHFmikro == 0)
                /* om båda kantsensorerna som sitter fram och mikroströmställaren triggas, sätts VFHFmikro = 1 */
                {
                VFHFmikro = 1;
                }
                
            while(VFHFmikro == 0)
                /* så länge som VFHFmikro = 1 backar roboten en kort sträcka för att sedan svänga lite till vänster för att sedan fortsätta framåt tills någon av de två främre kantsensorerna triggas igen */
                {
                backB();
                waitfor(DelayMS(1000));
                svvanster();
                waitfor(DelayMS(500));
                } // end while(VFHFmikro == 0)
                    
            // här avläses insignalerna
            indata = inport(PIODA) & (0xE0);
            INDATA = inport(PIODB) & (0xf0);
                
            if ((!(indata & 0x20)) && (indata & 0x40) && (INDATA & 0x10)) && (INDATA & 0x20) && (INDATA & 0x40) && (INDATA & 0x80) && (m==2)
                /* om mikroströmställaren triggas och m==2, backar roboten tills någon av de bakre sensorerna triggas */
                {
                backB();
                m = 2;
                }
                
                
        } // slut på costate 1
                
                
        
        costate     // costate 2
                {
                
                // här avläses insignalerna
                indata = inport(PIODA) & (0xE0);    // Här avläses om startkanppen trycks in
                
                if (indata & 0x80)
                    /* om startknappen trycks in stannar motorerna då k blir 0 igen och alla deklarerade variabler sätts till noll igen */
                    {
                    k = 0; HF = 0;
                    p = 1; VBHB = 0;
                    m = 1; VFHF = 0;
                    HB = 0;
                    VB = 0;
                    VF = 0;
                    }
                }   // slut på costate 2
        
        }   // slut på while(1) loop
    }   // slut på  while(k) loop
} // slut på  main


/* ******  UNDERFUNKTIONER  ********* */

/* ***** Båda motorerna ****** */

framB()
    {   // Fram på båda motorerna, PA1&3 = 1 och PA2&4 = 1
                setPIODA(0x1E);
    }

backB()
    {   // Back på båda motorerna, PA1&3=0 och PA2&4=1
                resPIODA(0x0A);
                setPIODA(0x14);
    }

stoppB()
    {   // Stopp på båda motorerna, PA2&4=0
                resPIODA(0x14);
    }
                

/* ******* vänster motor ****** */

framV()
    {      // Fram på vänster motor, PA1 pch PA2=1
                setPIODA(0x06);
    }
                
backV()
    {   // Back på vänster motor, PA1=0 och PA2 = 1
                resPIODA(0x02);
                setPIODA(0x04);
    }

stoppV()
    {   // stopp på vänster motor, PA2=0
                resPIODA(0x04);
                
    }

                
/* ***** Höger motor ****** */

framH()
    {   // Fram på höger motor, PA3=1 och PA4=1
                setPIODA(0x018);
    }


backH()
    {   // Back på höger motor, PA3=0 och PA4=1
                resPIODA(0x08);
                setPIODA(0x10);
    }
                

stoppH()
    { // Stopp på höger mmotor, PA4=0
                resPIODA(0x10);
    }


/* ****** fångstmotorn ****** */

start()
    {   // Startar fångstmotorn, PA0=1
                setPIODA(0x10);
            }

                

/* ******svängar ****** */
svhöger()
    {   // svänger höger
                framV();
                backH();
    }
                
                
svvanster()
    {      // Svänger vänster
                framH();
                backV();
    }
