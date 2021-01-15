
// 2015510087 Dogukan Berk Ozer

//required libraries
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> 

#define ROOMS 8 // the hospital have 8 rooms to apply people COVID-19 tests
#define BODY_COUNT 24 // total number of people coming to the hospital and this parameter can be changed to try the program with different numbers of total people.

void *person(void *num), *healthcare_staff(void *num), demurrage(int secs); 

int computing(), room=0, biding=0, completed = 0, last_person=0, numofpersons=0, count=0, temp=0; // required parameters and empty room function

sem_t empty_rooms[8], healthcarestaff_sth, persons, mutex, wait4room, idle_hcs, idle_person, idle_room; // to perform semaphore operations and by the way hcs = healthcare staff

int main(int argc, char *argv[])
{
    printf("\n------------------------------------------------------------------------\n");
    pthread_t hcs_id[ROOMS], person_id[BODY_COUNT]; // to uniquely identify threads
    int i, Pno[BODY_COUNT], HCSno[ROOMS];
   
    for (i = 0; i < ROOMS; i++) { // room array
        HCSno[i] = i;
    }
    for (i = 0; i < BODY_COUNT ; i++) { // people array
        Pno[i] = i;
    }  
    for (i = 0; i < 8; i++) { // for all 8 rooms
	sem_init(&empty_rooms[i], 0, 3); // because of each room capacity is 3 person
    }

    sem_init(&persons, 0, 3); // a room capacity is 3 person, initialising person semaphore referred to by sem
    sem_init(&healthcarestaff_sth, 0, 0); // initialising healthcare staff semaphore referred to by sem
    sem_init(&mutex, 0, 1); // one for each unit of the time
    sem_init(&wait4room, 0, 0);
    sem_init(&idle_hcs, 0, 0);  // initialising idle healthcare staff semaphore referred to by sem
    sem_init(&idle_person, 0, 0); // initialising idle person semaphore referred to by sem
    sem_init(&idle_room, 0, 8); // number of total rooms is 8
    
	// creating necessary threads
    for (i = 0; i < ROOMS; i++) {
        pthread_create(&hcs_id[i], NULL, healthcare_staff, (void *)&HCSno[i]);
    }    
    for (i = 0; i < BODY_COUNT; i++) {
        pthread_create(&person_id[i], NULL, person, (void *)&Pno[i]);
    }
    for (i = 0; i < BODY_COUNT; i++) {
        pthread_join(person_id[i],NULL);
    }
    for(i = 0; i < ROOMS; i++) {
        temp=1;
        sem_post(&healthcarestaff_sth);
    }
    for (i = 0; i < ROOMS; i++) {
        pthread_join(hcs_id[i],NULL);
    }
  
    return 0;
}

void *person(void *number) {    

    int num = *(int *)number;
    sem_wait(&persons); // locking people semaphore
    sem_wait(&mutex); // locking mutex semaphore
    count++;	
	
    if(room==0){
        biding=1;
        sem_wait(&wait4room);
        biding=0;
    }

    if(numofpersons%3==0){
	sleep(1);
	printf("The room is being emptied...\n	[O O O]\nThe staff is VENTILATING the room...\n");
	sleep(1);
	printf("The first person ALERTS the staff about ventilating!\n");
        sleep(1);
        sem_post(&healthcarestaff_sth); // unlocking healthcare staff semaphore
    }
    
    printf("%d. person is filling out the form. \n", num+1);
    sleep(1); 

    if(count==BODY_COUNT){
	last_person=1;
    }

    sem_post(&idle_hcs); // unlocking idle healthcare staff semaphore
    sem_wait(&idle_person); // locking idle people semaphore
    numofpersons++;
    sem_post(&mutex); // unlocking mutex semaphore
    sem_post(&persons); // unlocking people semaphore
}

void *healthcare_staff(void *number) {
 
    int num = *(int *)number;
    sem_wait(&idle_room); // locking idle room semaphore

    while(!completed){

    	int i;

    	if(room==0 && biding==1){
        	room++;
        	sem_post(&wait4room);
    	}
    	else{
        	room++;
    	}

    	sem_wait(&healthcarestaff_sth); // locking healthcare staff semaphore

	if(temp==1){
		return 0;
	}
	
   	for( i = 0; i < 3; i = i + 1 ){

		int r;
        	sem_wait(&idle_hcs); // locking idle healthcare staff semaphore
        	sem_wait(&empty_rooms[num]); // locking empty rooms semaphore
		sem_getvalue(&empty_rooms[num],&r); // placing empty_rooms[num] of pointed  semaphore to r

		// simualting of the rooms
		if(r!=0){
			if(r==2){
				int simulate = rand() % 3;
				if(simulate == 0) printf("	[X O O]\n"); // X = full seat
				else if(simulate == 1) printf("	[O X O]\n"); // O = empty seat
				else printf("	[ O O X ]\n");
			}else if(r==1){
				int simulate = rand() % 3;
				if(simulate == 0) printf("	[O X X]\n");
				else if(simulate == 1) printf("	[X O X]\n");
				else printf("	[ X X O ]\n");
			}
			// warning of the healthcare staff to idle people
			printf("The last %d people, let's start! Please, pay attention to your social distance and hygiene; use a mask.\n", r);
			
		}			
		else{
			printf("	[X X X]\nThe %d. room is in a busy state!\n", num+1); // the room is full right now
		}
	
		if(r==0){
		   room--;
		   sem_init(&empty_rooms[num], 0, 4); // // initialising empty room semaphore referred to by sem
		   printf("The COVID-19 test phase begins...\n");
		   sleep(1);
		   printf("------------------------------------------------------------------------\n");
	 	   int room_no= computing(num);
		}
		else if(last_person==1){
	 	   printf("NO PEOPLE LEFT and the COVID-19 test phase begins....\n\n"); // there is no person who is waiting for Covid-19 test(idle person)  
		   sem_post(&idle_person); // unlocking the idle person semaphore referenced by sem 
		   break;
		}
	
		sem_post(&idle_room); // unlocking the idle room semaphore referenced by sem 
		sem_post(&idle_person); // unlocking the idle person semaphore referenced by sem 
	}
	
	sleep(5);    
    }
}

int computing(int i) {
	int value;
        sem_getvalue(&empty_rooms[i],&value); // placing current value(empty_rooms[i]) of pointed semaphore to value
	return value;
}
