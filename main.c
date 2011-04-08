
#include "msp430fg4618.h"
#include "stdio.h"
#include "string.h"

#define TIME_OUT 5000
#define SEUIL_MUR 400

char booba[42];
unsigned char * commande = "D,3,3";

void send_string(char * str, int len)
{
  int i;
  for (i = 0; i < len; i++)
  {
     while ((IFG2 & 2) == 0);
     UCA0TXBUF = str[i];
  }
  
   while ((IFG2 & 2) == 0);
   UCA0TXBUF = '\r';
}

int write_khepera(unsigned char * outstring, unsigned char * instring)
{
  int i = 0, j = 0, ret = 0;
  
  while(outstring[i] != '\0')
  {
     while ((IFG2 & 2) == 0);
     UCA0TXBUF = outstring[i++];
     if ((IFG2 & 1) == 0)
        instring[j++] = UCA0RXBUF;  
  }
  
  while ((IFG2 & 2) == 0);
  UCA0TXBUF = '\r';
  
  
  
  ret = j;
  
  return ret;
}

int read_khepera(unsigned char * instring)
{
  int i = 0, scrut = 0, ret = 0;
  do{
    
    //Wait for input
    while( ((IFG2 & 1) == 0) || (scrut < TIME_OUT) ){
      scrut++;
    }
    instring[i++] = UCA0RXBUF;    
  }
  while(instring[i - 1] != '\n');
  
  ret = i;
  
  return ret;
}
  

int open_khepera()
{
  int ret = 0, i, scrut;
  char buff[32];
  
  UCA0CTL1 = 0x01;
  UCA0CTL1 = 0x00;
  
  P2SEL |= 0x30; //bit 4 and 5 set to 1
  
  //RS232 link setup
  UCA0CTL0 =
    (0 << 0) | //Asynchronous mode
    (0 << 1) | //UART
    (0 << 2) | //UART
    (0 << 3) | //Only one stop bit
    (0 << 4) | //8 bit data
    (0 << 5) | //lSB first
    (0 << 7);  //No parity bit
  
  UCA0CTL1 = 
    (1 << 6) |
    (0 << 7);
  
  UCA0BR0 = 0x03;
  UCA0BR1 = 0x00;
  UCA0MCTL = 
    (0 << 0) | //Oversampling disabled
    (1 << 1) | //BRS
    (1 << 2) | //BRS
    (0 << 3) | //BRS
    (0 << 4) | //BRF
    (0 << 5) | //BRF
    (0 << 6) | //BRF
    (0 << 7);  //BRF
  
  //Wait for free output
  scrut = 0;
  while ( ((IFG2 & 2) == 0) && (scrut < TIME_OUT) ){
    scrut++;
  }                    
  UCA0TXBUF = '\r';
  
  /*if ((UCA0STAT & 4) == 4)
    return -1;
  if (((UCA0STAT & 5) == 5) || ((UCA0STAT & 6) == 6))
    return -2;
  if (scrut >= TIME_OUT)
   return -3;*/
 
  scrut = 0;
  do{
    //Wait for input
    while( ((IFG2 & 1) == 0) && (scrut < TIME_OUT) ){
      scrut++;
    }
  }
  while(UCA0RXBUF != '\n');
  
  /*if ((UCA0STAT & 4) == 4)
    return -1;
  if (((UCA0STAT & 5) == 5) && ((UCA0STAT & 6) == 6))
    return -2;
  if (scrut >= TIME_OUT)
   return -3;*/
  
  send_string("L,0,1", 5);
  
  scrut = 0;
  i = 0;
  do{
    //Wait for input
    while( ((IFG2 & 1) == 0) || (scrut < TIME_OUT) ){
      scrut++;
    }
    buff[i++] = UCA0RXBUF;    
  }
  while(buff[i - 1] != '\n');
   
  /*if ((UCA0STAT & 4) == 4)
    return -1;
  if (((UCA0STAT & 5) == 5) || ((UCA0STAT & 6) == 6))
    return -2;
  if (scrut >= TIME_OUT)
   return -3;*/
  
  return ret;
}

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  open_khepera();
  int arret_urgence=0, lu;
  P1DIR &= ~(0x03); //Initialisation des deux boutons en ecoute
  for(;;)
  {
    // wait for button press to run the kepera
    while (((P1IN & 0x01)) && ((P1IN & 0x02)));
    while ((!(P1IN & 0x01)) || (!(P1IN & 0x02)));
    strcpy((char*)commande, "D,3,3");
    write_khepera(commande, (unsigned char*)booba);  
    
    // is the user pressing a button? if yes, then turn in the associated direction
    if ((P1IN & 0x01) || (P1IN & 0x02))
    {
      while(P1IN & 0x01)
      {
          write_khepera("D,1,-1" , (unsigned char*)booba);
      }
      while(P1IN & 0x02)
      {
          write_khepera("D,-2,2" , (unsigned char*)booba);
      }
      // rerun the kepera
      write_khepera(commande, (unsigned char*)booba); 
    }
    
    // are we in front of a wall?
    while (!arret_urgence)
    { 
      lu = 0;
      lu = write_khepera("N", (unsigned char*)booba);
      if (lu >= 29)
      {
        strcpy((char*)commande, "D,0,0");
        arret_urgence=1;
      }
    }

  }
  return 0;
}
