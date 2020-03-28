#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>

/*
 * MODEL DETAILS:
 * All people within a certain distance of an infected person have a probability of getting infected
 * Everyone either recovers or dies after TIME_TO_REMOVE ticks
 */

#ifdef DISEASE_COVID
THIS IS SMARTASS COMMENTING
#endif

// Definition for margin of error
// Used for floating point compares
// Value also functions as DELTA_T for physical calculations
#define EPSILON 0.001
#define DELTA_T EPSILON

// Number of ticks after which a person is removed
#define TIME_TO_REMOVE 100
// People within this distance of an infected person get infected
#define SPREAD_DISTANCE 2
// If your RNGesus gives you a number less than this you are gone
#define INFECTION_PROBABILITY 0.25
// Bounds for vectors
#define P_MAX 20.00
#define V_MAX 1.00

struct vector2D {
    float x;
    float y;
};

typedef struct vector2D vector2D;

struct vector2D vec_add(struct vector2D v1, struct vector2D v2) {
    vector2D ret_v;
    ret_v.x = v1.x + v2.x;
    ret_v.y = v1.y + v2.y;
    return ret_v;
}

vector2D scal_mul(vector2D v, float scalar) {
    vector2D ret_v;
    ret_v.x = v.x * scalar;
    ret_v.y = v.y * scalar;
    return ret_v;
}

struct person {
    struct vector2D pos;
    struct vector2D vel;
    enum hstate_e {HEALTHY=0, INFECTED=1, REMOVED=2} health;
    int ticks_since_infected;
};

char * health_state(int i) {
    switch(i) {
        case 0:
            return "HEALTHY";
        case 1:
            return "INFECTED";
        case 2:
            return "REMOVED";
        default:
            return "INVALID_CODE";
    }
}

typedef struct person person;

void println_vec(struct vector2D v) {
    printf("{%0.3f, %0.3f}\n", v.x, v.y);
}

void println_per(person * p) {
    printf("Position: "); println_vec(p->pos);
    printf("Velocity: "); println_vec(p->vel);
    printf("Health: %s\n", health_state(p->health));
}

float distance(person * p1, person * p2);
void p_copy(person * dest, person * src);
void infect(person * p);

void tick(person ** system, int system_size);

struct vector2D rand_vector(float bounds);
person * rand_init_p();

void print_stats(person ** society, int p_count);

int main() {
    srand(time(NULL));
    int n_people;
    printf("Enter number of persons in the sim: ");
    scanf("%d", &n_people);
    person ** society = malloc(sizeof(person *) * n_people);
    for(int i = 0; i < n_people; i++) {
        society[i] = rand_init_p();
    }
    for(int i = 0; i < n_people; i++) {
        printf("Person No %d\n", i);
        println_per(society[i]);
    }
    int sim_time;
    printf("How long do you want to run the sim (Keep this very large): ");
    scanf("%d", &sim_time);
    printf("Infected Patient 0\n");
    society[0]->health = INFECTED;
    for(int i = 0; i < sim_time; i++) {
        tick(society, n_people);
    }
    printf("=== SIM OVER ===\n");
    for(int i = 0; i < n_people; i++) {
        printf("Person No %d\n", i);
        println_per(society[i]);
    }
    print_stats(society, n_people);
    printf("\n");
}

person * rand_init_p() {
    person * ret_p = (person *)malloc(sizeof(person));
    ret_p->health = HEALTHY;
    ret_p->ticks_since_infected = 0;
    ret_p->pos = rand_vector(P_MAX);
    ret_p->vel = rand_vector(V_MAX);
    return ret_p;
}

struct vector2D rand_vector(float bounds) {
    // Initialise and assign
    struct vector2D ret_v;
    ret_v.x = (rand() / (float)RAND_MAX) * bounds;
    ret_v.y = (rand() / (float)RAND_MAX) * bounds;
    
    // Return
    return ret_v; 
}

void p_copy(person * dest, person * src) {
    *dest = *src;
}

float distance(person * p1, person * p2) {
    float dx = abs(p1->pos.x - p2->pos.x);
    float dy = abs(p1->pos.y - p2->pos.y);
    return sqrt((dx * dx) + (dy * dy));
}

void tick(person ** system, int p_count) {
    person ** temp = malloc(sizeof(person *) * p_count);
    for(int i = 0; i < p_count; i++) {
        temp[i] = malloc(sizeof(person));
        p_copy(temp[i], system[i]);
    }
    for(int i = 0; i < p_count; i++) {
        // If a particular person is infected
        if (system[i]->health == INFECTED) {
            // Iterate over everyone else
            for(int j = 0; j < p_count; j++) {
                // If the infected person is too close to some uninfected one
                if ((distance(system[i], system[j]) < SPREAD_DISTANCE) && (i != j)) {
                    // Pray that you dont get infected
                    float prayer = (rand() / (float)RAND_MAX);
                    // But if you dont pray hard enough
                    if (prayer < INFECTION_PROBABILITY) {
                        // You get infected
                        temp[j]->health = INFECTED;
                    }
                }
            }
        }
    }
    for (int i = 0; i < p_count; i++) {
        if (temp[i]->health == INFECTED) {
            temp[i]->ticks_since_infected += 1;
        }
        if(temp[i]->ticks_since_infected > TIME_TO_REMOVE) {
            temp[i]->health = REMOVED;
        }
        temp[i]->pos = vec_add(temp[i]->pos, scal_mul(temp[i]->vel, EPSILON));
        temp[i]->vel = rand_vector(V_MAX);
        p_copy(system[i], temp[i]);
    }
}

void print_stats(person ** society, int p_count) {
    int i_count = 0;
    int r_count = 0;
    int h_count = 0;
    for(int i = 0 ; i < p_count; i++) {
        if (society[i]->health == INFECTED) {
            i_count++;
        } else if (society[i]->health == REMOVED) {
            r_count++;
        } else {
            h_count++;
        }
    }
    printf("Infected = %d\n", i_count);
    printf("Removed = %d\n", r_count);
    printf("Healthy = %d\n", h_count);
}