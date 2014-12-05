int stb=11;
int spin=12;
int clk=13;
//IMPORTANT i found that there is a builtin arduino library function called shiftOut() that can do the shifting part and i think that should be faster than digitalWrite() method
//There is also a counter part called shiftIn()

//later i am planning to integrate it into the stock lcd library library and make an new child class or overloading function something like that
#define en 2 //q3 of sipo
#define rs 0 //q1 of sipo

/* This program demonstrates True 3 pin operation of LCD using the strobe pin, serial in pin, and clk pin 
and all pins of lcd are directly connected to SIPO only. The data for enable and RS are also coupled
into the SIPO and the lCD is driven in 4 bit mode. The strobe pin is utilised to send the enable clk to 
the lcd as described in the working below.

Author : N.Srinivas a.k.a NeutroN StrikeR

Date : 01-08-2014
											*/


void sipo_send(uint8_t var) // you may try to make these functions inline so that this might give a performance 
{									//boost by decreasing use of pointers and ram. but this will increase however increase
									//size of the program, you have to keep that in mind.
	uint8_t i;
	
	for(i=0;i<8;i++)
	{
		if(var & (1<<7))
		digitalWrite(spin,HIGH);

		else 
		digitalWrite(spin,LOW);

		digitalWrite(clk,HIGH);
		delayMicroseconds(5); //can be reduced to 1us i have checked it
					//so our sipo clock is 1/10us that is 100khz speed
					//80 us delay for 1 byte
		digitalWrite(clk,LOW);
		delayMicroseconds(5);

		var = var<<1;
	}	

}

void send_nibble(uint8_t x)  //try to make this also inline if necessary
{
	x |= (1<<en); // set en pin as 1
	
	sipo_send(x); //send the data through sipo
	
	digitalWrite(stb,HIGH); //after sending enable the STROBE so that data is now available on the data pins

	delay(1); //give delay of 10ms for enable change clk
	
	digitalWrite(stb,LOW);//clear the strobe now

	x &= ~(1<<en); // now make the enable pin of lcd as 0
		
	sipo_send(x); // again send new data in which only enable pins state of lcd is changed
					
	digitalWrite(stb,HIGH); //again set the strobe
	delay(1); //delay 10ms
	digitalWrite(stb,LOW); // clear the strobe


		//to send a nibble 2 * 80us + 2 * 1ms = 2.16ms total delay
}

void cmd(uint8_t var)
{
	uint8_t x;

//send higher nibble
	x = 0xf0 & var; //get higher nibble into x  
	
	x &= ~(1<<rs); 	//set the rs pin as 0
	send_nibble(x); //enable pin clocking mechanism is contained in nibble fucntion

//now send lower nibble
	x = 0xf0 & (var<<4);   // now repeat same process for lower nibble
	
	x &= ~(1<<rs); 	
	send_nibble(x);
	
//to send a cmd or data byte delay is 2 * 2.16ms = 4.32ms

//so data rate is 1/4.32ms = 231 char's /second ; good enough for our display
}




void data(uint8_t var)
{
	uint8_t x;

//send higher nibble
	x = 0xf0 & var;  
	
	x |= (1<<rs); 	
	send_nibble(x);

//now send lower nibble
	x = 0xf0 & (var<<4);  
	
	x |= (1<<rs); 	
	send_nibble(x);

	
}

void lcd_init()
{//proper initialization sequence
	delay(200); //wait until lcd's internal startup finished
	cmd(0x03);//first command
	delay(5);
	cmd(0x03);//second command
	delayMicroseconds(50);
	cmd(0x03);//third command
	delayMicroseconds(50);
	cmd(0x02);//set 4 bit mode interface
	delayMicroseconds(50);
	cmd(0x28);//the real function set command
	delayMicroseconds(50);
	cmd(0x08);//turn off display curcor, blink
	delayMicroseconds(100);
	cmd(0x01);//clear display
	delay(5);//allow enough time to write zeros to the whole ddram and cgram
	cmd(0x06);//entry mode set
	delayMicroseconds(50);
	cmd(0x0e);//turn on display with 1DCB
	delayMicroseconds(50);
	cmd(0x80);//set ddram address
	delayMicroseconds(50);
  
}

void print(char* sym,unsigned char startadd)
{
	unsigned char i=0;

	if(startadd<0x80 || startadd>0xcf)
		return;
	else 
		cmd(startadd);

	while(sym[i]!='\0')
	{

		if(startadd==0x90)
		{
				cmd(0xc0);
				startadd=0xc0;
		}
		else if(startadd==0xD0)
		{
			delay(2000);// delay so that before clearing lcd data can be read
			cmd(0x01);
			startadd=0x80;
		}
	
		data(sym[i]);
		i++;
		startadd++;
	}

}

void lcd_print(char* msg,...)
{
	int x;
	va_list arg_list;//declare "arg_list" a variable of type va_list
	va_start(arg_list,msg);//initialize the variable with "va_start" which takes 
							//two arguments the list and first argument in this function		
	x=va_arg(arg_list,int);//when va_arg is called with the argument of list and a datatype 
						// it returns next value of that type in the list if it exists
						//if it doesn't exist if returns NULL or 0 (tested in Turbo C)
	if(x>=0x80 && x<=0xcf)//so i don't want to check because i only want one argument to be
	print(msg,x);	//optional so i directly get the address and specify which should be
						//the next argument in the calling function(caller)
	
	else
	print(msg,0x80);

	va_end(arg_list); //clear the memory used by arg_list variable
}


void setup()
{
  pinMode(spin,OUTPUT);
  pinMode(clk,OUTPUT);
  pinMode(stb,OUTPUT);
  
  lcd_init();
  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);
  lcd_print("mu lekhiche prograM GA TU KENE JANUBU BAIYRI T");
}

void loop()
{
// data('a');
 //data('b');
 //data('c');
 // lcd_print("hello, how do you do");
  
  
}
