#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/syscall.h>
#include <linux/kernel.h>
#include <time.h>

#define poc 3.00 //price of chocolate
#define pov 1.00 //price of vanilla
#define pot 0.5 //price of toppings


// resources
int counter=0,total_icecream=0, total_cones=0,succesful_icecream=0, wasted_icecream=0;
float icecream_delay=0,belt_delay=0,belt_time=0,icecream_time=0;
double surplus = 0.0;

// semaphores declaration
sem_t cus_counter, flavor, chocolate, vanilla, toppings, t1, payment,packaging;
sem_t icecheckc;
// function prototypes
void *iceCream(void *ID);
void icecreamdrop();
void belt();
void icecreamcheck();
// main
int main()
{
sem_init(&packaging, 0, 1);
	int no_of_customers;
   // icecreamcheck();
	printf("\n\nEnter Number Of Customers : ");
	scanf("%d", &no_of_customers);
	printf("\n\n");
	int ID[no_of_customers];
	
	for(int i=0; i<no_of_customers; i++)
	{
		ID[i] = i+100;
	}
	
	//initialized semaphore
	sem_init(&cus_counter, 0, 1);
	
    sem_init(&icecheckc, 0, 1);
	sem_init(&flavor, 0, 3);
	sem_init(&chocolate, 0, 1);
	sem_init(&vanilla, 0, 1);
	
	sem_init(&toppings, 0, 2);
	sem_init(&t1, 0, 1);
	
	sem_init(&payment, 0, 1);
	
	
	//thread initialization
	pthread_t _customer[no_of_customers];
	
	for(int i=0; i<no_of_customers; i++)
	{
		pthread_create(&_customer[i], 0, &iceCream, (void*) &ID[i]);
	}
	
	for(int i=0; i<no_of_customers; i++)
	{
		pthread_join(_customer[i], NULL);
	}
	
	printf("\n\nBusiness report\n\n");
	printf("\nNumber Of Customers: %d", no_of_customers);
	//printf("\nTotal Number Of Cones: %d",total_cones);
	printf("\nTotal Ice-Cream Used: %d", total_icecream);
	printf("\nTotal Ice-Cream Wasted: %d", wasted_icecream);
	printf("\nsurplus: $ %f\n", surplus);
	//printf("\ncus_counters Remaining: %d\n\n", cus_counter);
	
	
	// destroying semaphores
	
	sem_destroy(&cus_counter);
	sem_destroy(&packaging);
	sem_destroy(&flavor);
	sem_destroy(&chocolate);
	sem_destroy(&vanilla);
	
	sem_destroy(&toppings);
	sem_destroy(&t1);
	
	sem_destroy(&payment);
	sem_destroy(&icecheckc);
	
	return 0;
}
void icecreamdrop(){

    srand(time(0));
    int temp=(rand() % 100);
    total_icecream++;
    temp=temp/10;
    icecream_delay=(float)temp;
    icecream_delay=1+(icecream_delay/10);
    //icecream_delay=1;
    if(icecream_time!=0 && ((belt_time+0.1)<icecream_time)){
        icecream_delay=(icecream_delay/2);//+(icecream_delay/4);
    }
    icecream_time=icecream_time+icecream_delay;
    printf("%f\n",icecream_time);

}
void belt(){
    srand(time(0));
    belt_delay=(rand() % 10);
    belt_delay=1+(belt_delay/10);
    //belt_delay=1;
    if(belt_time!=0 && ((icecream_time+0.1)<belt_time)){
        belt_delay=(belt_delay/2);//+(belt_delay/4);
    }
    belt_time=belt_time+belt_delay;
    printf("%f",belt_time);
}
void icecreamcheck(){
    sem_wait(&icecheckc);
    icecreamdrop();
    belt();
    //total_icecream++;
    if((icecream_time<=(belt_time+0.7) && icecream_time>=(belt_time-0.7) )|| (belt_time<=(icecream_time+0.7) && belt_time>=(icecream_time-0.7))){
        succesful_icecream++;
       // return 1;
    }
    else{
        sleep(2);
        wasted_icecream++;
        sem_post(&icecheckc);
        icecreamcheck();
    }
    sem_post(&icecheckc);
}
void *iceCream(void *_ID)
{
	int ID = *(int*)_ID, checkRaceCond_1 = 0;
	double bill = 0.0;
	
	sem_wait(&cus_counter);
	
	printf("Customer[%d] arrived at the counter.\n", ID);
	//syscall(333,"Got cus_counter.\n",ID);
	
	sem_post(&cus_counter);
	
	sleep(2);
	    //chocolate
		sem_wait(&chocolate);
		
			icecreamcheck(1);
			checkRaceCond_1++;
			bill = bill + poc;
			printf("\nCustomer[%d]: Got chocolate ice cream.\n", ID);
			sleep(1);
		
		sem_post(&chocolate);
		
		// vanilla
		sem_wait(&vanilla);
			
			icecreamcheck(1);
			
			checkRaceCond_1++;
			bill = bill + pov;
			printf("Customer[%d]: Got vanilla ice cream.\n", ID);
			sleep(1);
		
		sem_post(&vanilla);
		
		if(checkRaceCond_1 == 0)
		{
			printf("\nCustomer[%d]: exiting factory", ID);
			sleep(1);
			pthread_exit(NULL);
		}
	
	printf("\nCustomer[%d]: Got Flavour.\n", ID);
	sem_wait(&t1);
		
		bill = bill + pot;
		
	sem_post(&t1);
	
	printf("\nCustomer[%d]: Leaving Topping Counter.\n", ID);
	
	sleep(2);
	
	//packaging
	sem_wait(&packaging);

    printf("\nIce cream packed\n");

	sem_post(&packaging);
	
	sem_wait(&payment);
	
	surplus = surplus + bill;
	
	char a[100];
	sprintf(a,"Final bill: $ %f.\n",bill);
	printf("\n%s    %d",a,ID);
	//syscall(333,a,ID);	

	sem_post(&payment);
	
	sleep(2);
	
	printf("\nCustomer[%d]: exiting Ice-Cream factory.\n\n", ID);
	
	return 0;
}