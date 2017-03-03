#define SND_DATA_LEN 8
#define RCV_DATA_LEN 8

unsigned char rcv_data[RCV_DATA_LEN] ;
unsigned char snd_data[SND_DATA_LEN] ;

void interrupt InterI2C( void ) ;
void InitI2C_Slave(int address) ;
int I2C_ReceiveCheck() ;

