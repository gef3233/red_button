#include <stdio.h>
//#include <libusb.h> //Seems deprecated (?)
#include <errno.h>
#include <unistd.h> //usleep
#include <string.h>
#include <signal.h>


//VID and PID of the button
#define VID 0x1d34
#define PID 0x000d

static struct libusb_device_handle *devh = NULL;


void close_button(){

	libusb_close(devh);
	libusb_exit(NULL);
}

//A function to handle CTRL + C
//When pressed it first closses the device and the exits
void sigintHandler(int sig_num)
{
	signal(SIGINT, sigintHandler);
	printf("\n Exiting \n");
	fflush(stdout);

	close_button();
	exit(0);
}


//Function to set status to device
static int set_status(unsigned char *status)
{
	int r;

	r = libusb_control_transfer(devh, 0x21, 0x09, 0x00, 0x00, status, 8, 0);
	if (r < 0) {
		fprintf(stderr, "read hwstat error %d\n", r);
		return r;
	}
	if ((unsigned int) r < 1) {
		fprintf(stderr, "short read (%d)\n", r);
		return -1;
	}

	//printf("hwstat reads %02x\n", *status);
	return 0;
}



int access_button(){

	int ret;
	libusb_init(NULL);

	//I access the  device
	devh = libusb_open_device_with_vid_pid(NULL, 0x1d34, 0x000d);

	 if (devh == NULL ){
		printf("Device not found or you don't have permissions to get to it. (Try root)\n");
		return -1;
	 }


	 if ( libusb_kernel_driver_active(devh,0)){
	
		printf("Detach from kernel\n");
	        ret = libusb_detach_kernel_driver(devh,0);

	        if (ret < 0 ){

			printf("Can't detach\n");
	                return -1;
	        }
	 }

}


//Returns the device status as int

//0x15 = Closed                (21 in decimal)
//0x17 = Open but not pressed  (23 in decimal)
//0x16 = Open and pressed      (22 in decimal)

int get_status(){
	

	int ret;

	//Send the control interrupt
	unsigned char status[8];
	status[0]=0x0;
	status[1]=0x0;
	status[2]=0x0;
	status[3]=0x0;
	status[4]=0x0;
	status[5]=0x0;
	status[6]=0x0;
	status[7]=0x2;
	ret = set_status(status);


	//I send 0x81 to the EP to retrieve the status
	char data[8];
	int dev[8],i=0;

	data[0] = 0x0;
	data[1] = 0x0;
	data[2] = 0x0;
	data[3] = 0x0;
	data[4] = 0x0;
	data[5] = 0x0;
	data[6] = 0x0;
	data[7] = 0x0;


	ret =  libusb_interrupt_transfer(devh,0x81,data,8,dev,200);




return data[0];
}

//Get the program to exec from the command line
int main(int argc, char* argv[]){

int status;
char ini[3]="./", vector[50]="",rubish[20]="";

strcpy(rubish,argv[1]);
strcpy(vector,ini);
strcat(vector,rubish);

//Check for CTRL + C
signal(SIGINT, sigintHandler);

//Access the device
access_button();

while(1){

	status = get_status();

	usleep(20000);

	//Check if the button is pressed
	if(status == 22) //Value in decimal

		system(vector);
	
		//While pressed do nothing
		while(status == 22){

			status = get_status();		

		}
	}

return 0;
}
