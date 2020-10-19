#include "tm4c123gh6pm.h"
#define CR   0x0D

unsigned char keys[4][4]=
{
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

struct EachRoom
{
	unsigned long Number;
	unsigned long status;
	unsigned char* Password;
	unsigned long IsOpen;

};
typedef struct EachRoom Room;

Room p[50];											// Max Number Of Rooms =50
unsigned long coun=0;	
volatile unsigned char in[5];
unsigned long RoomsCounter=0;
unsigned long RoomFiveId;

void KEy_Read(unsigned char k);
unsigned char Key_In(void);
unsigned long CharToLong(unsigned char c[]);
void delay(unsigned long msec);
void SystemInit(){}

//********************************   UART Section  ********************************
void UART_Init(void){
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;
  UART0_CTL_R &= ~UART_CTL_UARTEN;
  UART0_IBRD_R = 8;
  UART0_FBRD_R = 44;
  UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0_CTL_R |= UART_CTL_UARTEN;
  GPIO_PORTA_AFSEL_R |= 0x03;
  GPIO_PORTA_DEN_R |= 0x03;
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;
  GPIO_PORTA_AMSEL_R &= ~0x03;
}
// Get Input char from UART pins by busy-wait synchronization
unsigned char UART_InChar(void)
{
  while((UART0_FR_R&UART_FR_RXFE) != 0);
  return((unsigned char)(UART0_DR_R&0xFF));
}
// Get Input char from UART pins by NonBlocking synch.
unsigned char UART_InCharNonBlocking(void)
{
  if((UART0_FR_R&UART_FR_RXFE) == 0){
    return((unsigned char)(UART0_DR_R&0xFF));
  } else{
    return 0;
  }
}
//Output a Char to the UART
void UART_OutChar(unsigned char data)
{
  while((UART0_FR_R&UART_FR_TXFF) != 0);
  UART0_DR_R = data;
}
//Input Unsigned Decimal number from UART
unsigned long UART_InUDec(void)
{
	unsigned long number=0, length=0;
	char character;
  character = UART_InCharNonBlocking();
  while(character != CR)
	{ 
    if((character>='0') && (character<='9')) 
		{
			number = 10*number+(character-'0');   // this line overflows if above 4294967295
      length++;
      UART_OutChar(character);
    }
    character = UART_InCharNonBlocking();
  }
  return number;
}
//OutPut String to the UART
void UART_OutString(unsigned char buffer[])
{

	unsigned char c;
	int i=0;
	do
	{
		c=buffer[i];
		UART_OutChar(c);
		i++;
	}
	while(c!='\0');
}
//Output unsigned Long number to UART "Long Used here to visualize only the first two digits"
void UART_OutLong(unsigned long L)
{
	if(L<10)
	{
		UART_OutChar(L+'0');
	}
	else if(L<100)
	{
		UART_OutChar(L/10+'0');
		UART_OutChar(L%10+'0');
	}
}
// Input Number of rooms which will be controlled by the receptionist
void UART_InRooms(void)
{
	unsigned long  temp;
	unsigned char c1;
	unsigned char c2[3];
	int i=0;

	do
	{
		c1=UART_InChar();
		if(c1==' ')
		{
			c2[i%3]='\0';
			i=0;
			temp=CharToLong(c2);
			p[RoomsCounter].Number=temp;
			p[RoomsCounter].IsOpen=0;
			p[RoomsCounter].status=0;
			RoomsCounter++;
		}
		else if (c1 >='0' && c1 <='9')
		{
			c2[i%3]=c1;
			i++;
		}
		else if(c1==CR)
		{
			c2[i%3]='\0';
			i=0;
			temp=CharToLong(c2);
			p[RoomsCounter].Number=temp;
			p[RoomsCounter].IsOpen=0;
			p[RoomsCounter].status=0;
			RoomsCounter++;
		
		}
		 UART_OutChar(c1);
	}
		while(c1!=CR);

}
// This Function used to get the password by the receptionist 
unsigned long UART_In4Digits(	unsigned char c[])
{
	int i;
	for(i=0;i<4;i++)
	{
		c[i]=UART_InChar();
		if(c[i]<'0' || c[i] > '9')
		{
			return 0;	
		}
		UART_OutChar(c[i]);
	}
	c[4]='\0';
	return 1;
}

// ******************************  Keypad Section ****************
void PortB_Init(void)
{//here i used port B pins to connect the keypad
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000002;
	delay=SYSCTL_RCGC2_R;
	GPIO_PORTB_CR_R |= 0xFF;
	GPIO_PORTB_LOCK_R = 0x4C4F434B;
	GPIO_PORTB_DIR_R|=0x0F;
	GPIO_PORTB_DIR_R &= ~(0xF0);
	GPIO_PORTB_DEN_R |=0xFF;
	GPIO_PORTB_AFSEL_R &= ~(0xFF);
	GPIO_PORTB_AMSEL_R &= ~(0xFF);
	GPIO_PORTB_PCTL_R &= ~(0xFFFFFFFF);
	GPIO_PORTB_PDR_R |= 0xF0;
}
// This Function used to Read the input Character from the keypad
unsigned char Key_In(void)
{
	int i,j;
	volatile unsigned char temp;
	temp=GPIO_PORTB_DATA_R & 0xF0;
	for(i=0 ; i<4;i++)
	{
		GPIO_PORTB_DATA_R = GPIO_PORTB_DATA_R&0xF0;
		GPIO_PORTB_DATA_R |= (1<<i);
		delay(1);
		for(j=0;j<4;j++)
		{
			if((GPIO_PORTB_DATA_R &0xF0)>>(j+4) == 1 )
					return keys[i][j];
		
		}
	}
	return 'x';
}

// *************************** Solenoid ******************
void PortF_Init(void)
{ // Solenoid here simulated by the built-in LED connected to PF3
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;
  delay = SYSCTL_RCGC2_R; 
  GPIO_PORTF_LOCK_R = 0x4C4F434B; 
  GPIO_PORTF_CR_R |= 0x08;   
  GPIO_PORTF_AMSEL_R &= ~(0x08); 
  GPIO_PORTF_PCTL_R &= ~(0x0000F000);
  GPIO_PORTF_DIR_R |= 0x08;
  GPIO_PORTF_AFSEL_R &= ~(0x08);           
  GPIO_PORTF_DEN_R |= 0x08; 
}
// Delay function used to make a delay in msecs 
void delay(unsigned long msec)
{ 
  unsigned long count;
  while(msec > 0 ) {  
    count = 16000;    // about 1ms
    while (count > 0) { 
      count--;
    } 
    msec--;
  }
}
// This function used to Convert the input Char to long number
unsigned long CharToLong(unsigned char c[])
{
	unsigned char temp;
	unsigned long result=0; 
	int i=0;
	
	temp=c[0];
	while(temp!='\0')
	{
		i++;
		result=result*10+(temp-'0');
		temp=c[i];
	}
	return result;
}
// This Function check if the receptionist would control the Room number 5 or not and return it's
// id in the rooms array
int CheckFun(void)
{
	int i;
	for(i=0;i<RoomsCounter;i++)
	{
		if(p[i].Number==5)
		{
			RoomFiveId=i;
			break;
		
		}
		if(i==(RoomsCounter-1))
			return 0;
	}
	for(i=0;i<5;i++)
	{
		if( (in[i]!=p[RoomFiveId].Password[i]) || p[RoomFiveId].status!=2 )
			return 0;
	}
	return 1;
}
// Used to Store The 4 digits input by the user by keypad
void KEy_Read(unsigned char k)
{
	if(k =='A') // A button reprent the Enter key in the keypad 
	{
		coun=0;
			if((coun%4)==0)
			{
				
				if(CheckFun())
					p[RoomFiveId].IsOpen=1;
			}
	}
	else
	{
		if((coun%5)<4)
			in[coun]=k;
		else
			in[4]='\0';
		coun++;
	}

}

void SysTick_Init(unsigned long period)
{
	NVIC_ST_CTRL_R =0;
	NVIC_ST_RELOAD_R =period-1;
	NVIC_ST_CURRENT_R =0;
	NVIC_SYS_PRI3_R |=((NVIC_SYS_PRI3_R &0x00FFFFFF)+0x2000000);
	NVIC_ST_CTRL_R =0x07;
}
// Systick Handler Role is every time it checks if the status of The Room 5 is Open or not to Control the LED 
// and Read the input From Keypad 
void SysTick_Handler(void)
{
	volatile unsigned char c;
	if(p[RoomFiveId].IsOpen==1)// if room 5 must be oped whether by the user or by receptionist in cleaning mood
		GPIO_PORTF_DATA_R |=0x08;
	else
		GPIO_PORTF_DATA_R &= ~(0x08);
	c=Key_In();
	if(c!='x') // x means there is n't input from the keypad
	{
		KEy_Read(c);

	}
}

int main(void)
{  
	unsigned char x1[]="Please enter the number of Rooms :";
	unsigned long i;
	volatile int j;
	PortF_Init();
	UART_Init();
	PortB_Init();
	UART_OutString(x1);
	UART_InRooms();// here the recepionist enter the number of rooms to be controlled

	j=CheckFun();
	SysTick_Init(3200000);// Interrupt every 200ms 
	while(1)
	{
		
		unsigned char x[]="\nPlease Enter the Room Number :";
		unsigned char x2[]="\nPlease Enter the Status of Room ";
		unsigned char x22[]= " From 0 ,1 Or 2 :";
		unsigned char x3[]="\nPlease Enter the 4-digits Room ";
		unsigned char x33[]=" Password : ";
		unsigned char x4[]="\nWrong Number !";
		unsigned char x5[]="\nSuccesuful Operation !";

		volatile unsigned long RoomNumber;
		unsigned long Status;
		unsigned char Password[5];
		unsigned long Password_Check;
		unsigned long checker=0;
		unsigned long InputNumber;
		
		UART_OutString(x);
		InputNumber=UART_InUDec();// Then she enter the Number of Room she want to change it's status
		for(i=0;i<RoomsCounter;i++)
		{
			if(p[i].Number==InputNumber) // Check if the the room number if one of the controlled rooms she entered at first or not
			{
				checker=1;
				RoomNumber=i;
				break;
			}
		}		
		if(checker==1)
		{
			UART_OutString(x2);
			UART_OutLong(InputNumber);
			UART_OutString(x22);
			Status=UART_InUDec();// Then she enter the new Status of the room
			p[RoomNumber].status=Status;
			if(Status ==0) // Status 0 represent Room Cleaning Mood 
			{
				p[RoomNumber].IsOpen=0;
				UART_OutString(x5);
			}
			else if(Status==1) // Status 1 represent Check Out mode "FREE"
			{
				p[RoomNumber].IsOpen=1;
				UART_OutString(x5);
			}
			else if(Status==2) // status 2 represent Chceck in mode "Occupied"
			{
				UART_OutString(x3);
				UART_OutLong(InputNumber);
				UART_OutString(x33);
				Password_Check=UART_In4Digits(Password);// Here the recepionist enter the 4-digit new password
				if(Password_Check==0)// if the Paasword contain alphapetic char not number Then it Visualize error messege
					UART_OutString(x4);
				else
				{
					p[RoomNumber].Password=Password;// if the 4- digts were only numbers then here i set the new password to the room
					p[RoomNumber].IsOpen=0;
					UART_OutString(x5);
				}
					
			}
			else// Wrong Status input 
			{
				UART_OutString(x4);
			}
		}
		
		else // Wrong Room Number input
		{
			UART_OutString(x4);
		}
		checker=0;
	}
}
