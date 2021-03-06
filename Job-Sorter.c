/**
 * * * * * * * * * * * * * * * * * * *
 * Brennan Couturier
 * * * * * * * * * * * * * * * * * * *
 * Compile with gcc -Wall -o Job-Sorter Job-Sorter.c
 * * * * * * * * * * * * * * * * * * *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * This is how much space that is initially given to the input string. If it is too much, we realloc it to a lesser value.
 * If that's not enough space for the line, there's probably a problem with the line, so we exit()
 */
#define INITIAL_BUFFER_SIZE 255

/**
 * This determines how many tokens the program looks for in the input line
 *  ex. person name, job name, arrival time, duration
 */
#define NUM_TOKENS 4

/**
 * A time slot that doesn't have a job is idle. This macro holds the string used to describe that state, i.e. "IDLE".
 *  Pretty self-explanatory I'd have thought.
 */
#define IDLE_JOB_NAME "IDLE"

//-----------------------JOB INFO-----------------------//
typedef struct job{
    char* person_name;
    char* job_name;
    size_t arrival_time;
    size_t duration;
} job;

/**
 * Allocates space for and initializes a job struct
 * Takes the job's data as parameters
 * Returns a pointer to the constructed struct
 */
job* create_job(char* person_name, char* job_name, size_t arrival_time, size_t duration);

/**
 * Checks if a given job is idle by checking its job_name value.
 * Returns 1 if the job is an idle job, 0 otherwise
 */
int is_job_idle(job* j);

/**
 * Prints out the given job's data in an organized manner.
 * Purely for testing.
 */
void print_job(job* j);

/**
 * Frees all the memory used by the job
 */
void destroy_job(job* j);

//-----------------------LINKED LIST INFO-----------------------//
typedef struct node{
    job* job;
    struct node* next;
} node;

/**
 * Allocates space for and initializes a node struct with the given job
 * Takes a job struct as a parameter
 * Returns a pointer to the allocated node
 */
node* create_node(job* j);

/**
 * Allocates space for and initializes a node struct with an IDLE job
 * Returns a pointer to the allocated node
 */
node* create_idle_node();

/**
 * Prints out the given node's data in a formatted manner. Prints its job's data, as well as the value of n->next
 * Purely for testing
 */
void print_node(node* n);

/**
 * Cycles through a singly linked list referenced by head
 * Returns the number of elements in the list
 */
size_t get_length_list(node* head);

/**
 * Cycles through a singly linked list referenced by head, printing out the nodes as it goes
 */
void print_full_list(node* head);

/**
 * Prints output as requested by Assignment 1's instructions
 */
void print_output(node* head, node* job_list_head);

/**
 * Cycles through a singly linked list referenced by head and frees the nodes as it goes
 */
void destroy_list(node* head);

/**
 * Frees a given node from memory. Frees the job first, then itself.
 */
void destroy_node(node* n);

/**
 * Adds a given node to a list, if a node with a job with the same person_name as the passed one is found,
 *  the one with the higher final_index takes the spot. The other is overwritten.
 */
void add_node_to_cultivated_list(node** cultivated_list_head, node** head_ref, node* to_insert);
/**
 * Adds a node to the end of a singly linked list referenced by head_ref
 * Returns 0. It really could be a void function...
 */
int add_node_to_list_end(node** head_ref, node* to_insert);

/**
 * This is the big chungus of functions for this program. It implements the shortest job first algorithm.
 * Given a node with a job of duration n, it first creates n new nodes with increasing arrival_times and decreasing durations
 * It then attempts to add each node individually into the list, based on its arrival_time and duration.
 * The process of adding each node is extensive and complicated, so much that I don't want to describe it here.
 * There are comments throughout the function, though, so it shouldn't be too hard to figure it out.
 * Returns 0, exits if there is a problem. Again, could really be a void function
 */
int add_node_to_list(node** head_ref, node* to_insert);

/**
 * This adds a node to a special and smaller list of the jobs that came in from stdin.
 * Its purpose is to store the input in a useable way.
 * Returns 0, could be a void function
 */
int add_node_to_job_list(node** head_ref, node* to_insert);

/**
 * Takes in a refernce to the time slices list, reverses it, traverses it looking for first occurance of given job, then reverses it again when done.
 * Returns index of job if found, 0 otherwise
 */
size_t get_last_index_of_job_2(node** head_ref, job* j);

/**
 * Takes a reversed version of the time slices list and traverses it, looking for the first occurance of the given job
 * If it finds it, returns the index. Otherwise, returns -1
 */
size_t get_last_index_of_job(node* head, job* j);

//-----------------------FORMATTING-----------------------//
/**
 * Cycles through the given string and replaces every instance of whitespace with the given replacement character
 */
void replace_whitespace(char* line, char replacement_char);

//-----------------------CONVERSIONS-----------------------//
/**
 * Converts a string denoting a number into a size_t variable by turning into an unsigned long long then casting it.
 */
size_t strtosizet(char* str);

//-----------------------IMPLEMENTATIONS-----------------------//
/**
 * Calls functions to figure out how a list of jobs should be scheduled according to a shortest job first algorithm.
 */
int main(){

    node* head = NULL; //keeps track of nodes (multiple per job)

    node* job_list_head = NULL; //keeps track of jobs (only used for output, one node per job)

    //------------------------------//
    //  Read Input                  //
    //------------------------------//

    void* line_v = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
    if(line_v == NULL){
        fprintf(stderr, "ERROR in main() : malloc() failed to allocate space for input line\n");
        exit(EXIT_FAILURE);
    }

    char* line = (char*)line_v;
    //read and disregard first line as header
    fgets(line, INITIAL_BUFFER_SIZE, stdin);

    while(fgets(line, INITIAL_BUFFER_SIZE, stdin) != NULL){
        //If last character is a newline, a full line was read (GOOD!)
        // otherwise the buffer wasn't big enough
        if(line && line[strlen(line)-1] == '\n'){

            //line was initially allocated a lot of memory which it probably doesn't need.
            // So, we'll reallocate it to only use what it needs.
            void* err = realloc(line, strlen(line) * sizeof(char));
            if(err == NULL){
                fprintf(stderr, "ERROR in main() : attempt to realloc() space for input line has failed\n");
                exit(EXIT_FAILURE);
            }

            //Now we have a pointer to a line of input, we need to get rid of whitespace.
            replace_whitespace(line, ',');

            //Now that each line is in csv format, we can tokenize it
            char* rest = NULL;
            char* token;
            char* tokens[NUM_TOKENS];
            int i = 0;
            token = strtok_r(line, ",", &rest);
            while(token != NULL){
                tokens[i] = token;
                token = strtok_r(NULL, ",", &rest);
                i++;
            }

            //Now we have an array with all the information we need to create a job
            char* person_name = tokens[0];
            char* job_name = tokens[1];
            size_t arrival_time = strtosizet(tokens[2]);
            size_t duration = strtosizet(tokens[3]);

            job* j = create_job(person_name, job_name, arrival_time, duration);
            if(j == NULL){
                fprintf(stderr, "ERROR in main() : Could not allocate space for job\n");
                exit(EXIT_FAILURE);
            }

            node* n = create_node(j);
            if(n == NULL){
                fprintf(stderr, "ERROR in main() : Could not allocate space for node\n");
                exit(EXIT_FAILURE);
            }

            //this node will get added to the job_list so we can print it out later. The other node will be used for more important purposes
            node* n_copy = create_node(j);
            if(n_copy == NULL){
                fprintf(stderr, "ERROR in main() : Could not allocate space for node copy\n");
                exit(EXIT_FAILURE);
            }

            add_node_to_job_list(&job_list_head, n_copy);

            add_node_to_list(&head, n);

        }else{
            //255 characters is more than enough for any feasible input
            // So, if that's not enough, the input must be beyond recovery so we exit()
            fprintf(stderr, "ERROR in main() : Input buffer was not big enough : Are you sure you are inputting the right data?\n");
            exit(EXIT_FAILURE);
        }

    }

    //At this point we have a list of correctly scheduled jobs!
    //Add one final IDLE job to the very end
    add_node_to_list_end(&head, create_idle_node());

    //print_full_list(head);
    print_output(head, job_list_head);
    destroy_list(head);

    return 0;
}

//-----------------------JOB IMPLEMENTATIONS-----------------------//

job* create_job(char* person_name, char* job_name, size_t arrival_time, size_t duration){
    void* to_return_v = malloc(sizeof(job));
    if(to_return_v == NULL){
        fprintf(stderr, "ERROR in create_job() : Could not allocate space for job name\n");
        return NULL;
    }
    job* to_return = (job*)to_return_v;

    void* p_name_v = malloc(strlen(person_name) * sizeof(char));
    if(p_name_v == NULL){
        fprintf(stderr, "ERROR in create_job() : Could not allocate space for person_name\n");
        free(to_return);
        return NULL;
    }
    char* p_name = (char*)p_name_v;
    strcpy(p_name, person_name);

    void* j_name_v = malloc(strlen(job_name) * sizeof(char));
    if(j_name_v == NULL){
        fprintf(stderr, "ERROR in create_job() : Could not allocate space for job name\n");
        free(p_name);
        free(to_return);
        return NULL;
    }
    char* j_name = (char*)j_name_v;
    strcpy(j_name, job_name);

    to_return->person_name = p_name;
    to_return->job_name = j_name;
    to_return->arrival_time = arrival_time;
    to_return->duration = duration;

    return to_return;
}

int is_job_idle(job* j){
    if(strncmp(j->job_name, IDLE_JOB_NAME, strlen(IDLE_JOB_NAME)) == 0){
        return 1;
    }

    return 0;
}

int add_node_to_job_list(node** head_ref, node* to_insert){
    //for now just add one node to end

    if(*head_ref == NULL){
        //list is empty
        *head_ref = to_insert;
    }else{
        //go to end
        node* curr_node = *head_ref;
        while(curr_node->next != NULL){
            curr_node = curr_node->next;
        }
        curr_node->next = to_insert;
    }

    return 0;
}

void print_job(job* j){
    char* person_name = j->person_name;
    char* job_name = j->job_name;
    size_t arrival_time = j->arrival_time;
    size_t duration = j->duration;

    fprintf(stdout, "\n"
                    "//-------JOB-------//\n"
                    "Name:\t%s\n"
                    "Job:\t%s\n"
                    "Arrived:\t%zu\n"
                    "Duration:\t%zu\n"
                    "\n",
                    person_name, job_name, arrival_time, duration
            );
}

void destroy_job(job* j){
    //free(j->person_name);
    //free(j->job_name);
    free(j);
}

//-----------------------LINKED LIST IMPLEMENTATIONS-----------------------//

node* create_node(job* j){
    void* to_return_v = malloc(sizeof(node));
    if(to_return_v == NULL){
        fprintf(stderr, "ERROR in create_node() : Could not allocate space for node struct\n");
        return NULL;
    }
    node* to_return = (node*)to_return_v;
    to_return->job = j;
    to_return->next = NULL;

    return to_return;
}

node* create_idle_node(){
    void* to_return_v = malloc(sizeof(node));
    if(to_return_v == NULL){
        fprintf(stderr, "ERROR in create_idle_node() : Could not allocate space for node struct\n");
        return NULL;
    }
    node* to_return = (node*)to_return_v;

    void* idle_job_v = malloc(sizeof(job));
    if(idle_job_v == NULL){
        fprintf(stderr, "ERROR in create_idle_node() : Could not allocate space for idle job struct\n");
        return NULL;
    }
    job* idle_job = (job*)idle_job_v;

    idle_job->arrival_time = 0; //Arrival time and duration's values don't matter, they will never be read.
    idle_job->duration = 0;
    idle_job->job_name = IDLE_JOB_NAME;
    idle_job->person_name = IDLE_JOB_NAME;

    to_return->job = idle_job;
    to_return->next = NULL;
    return to_return;
}

size_t get_length_list(node* head){
    node* curr_node = head;
    size_t index = 0;
    while(curr_node != NULL){
        index++;
        curr_node = curr_node->next;
    }
    return index;
}

void reverse_list(node** head){
    node* prev = NULL;
    node* next = NULL;
    node* curr_node = *head;

    while(curr_node != NULL){
        next = curr_node->next;
        curr_node->next = prev;
        prev = curr_node;
        curr_node = next;
    }
    *head = prev;
}

size_t get_last_index_of_job_2(node** head_ref, job* j){
    //First, reverse list
    reverse_list(head_ref);
    node* curr_node = *head_ref;
    size_t index = get_length_list(*head_ref);
    while(curr_node != NULL){
        if(strncmp(curr_node->job->job_name, j->job_name, strlen(j->job_name)) == 0){
            //We found a match!
            reverse_list(head_ref);
            return index;
        }
        index--;
        curr_node = curr_node->next;
    }
    reverse_list(head_ref);
    return 0;
}

size_t get_last_index_of_job(node* head, job* j){
    //We need to find the last node with a given job in it.
    node* curr_node = head;
    size_t index = get_length_list(head);
    while(curr_node != NULL){
        if(strncmp(curr_node->job->job_name, j->job_name, strlen(j->job_name)) == 0){
            //We found a match!
            return index;
        }
        index--;
        curr_node = curr_node->next;
    }
    return 0;
}

void print_full_list(node* head){
    node* curr_node = head;
    size_t index = 0;
    fprintf(stdout, "TIME\tJOB\n");
    while(curr_node != NULL){
        char* job_name = curr_node->job->job_name;
        fprintf(stdout, "%zu\t%s\n", index, job_name);
        index++;
        curr_node = curr_node->next;
    }
}

void print_output(node* head, node* job_list_head){
    //Print header
    fprintf(stdout, "Time\tJob\n");

    //Print out the list from the first non-idle node to the end
    // so first, find the first non-idle node
    node* curr_node = head;
    size_t index = 0;
    while(is_job_idle(curr_node->job)){
        index++;
        curr_node = curr_node->next;
    }
    //Now curr_node is pointing to the first non-idle node so print out list
    while(curr_node != NULL){
        char* job_name = curr_node->job->job_name;
        fprintf(stdout, "%zu\t\t%s\n", index, job_name);
        index++;
        curr_node = curr_node->next;
    }

    //Now print out summary header
    fprintf(stdout, "\nSummary\n");

    //Cultivate a list of unique jobs, where only the latest job from a person is included
    node* cultivated_list_head = NULL;
    curr_node = job_list_head;
    while(curr_node != NULL){
        node* copy = create_node(curr_node->job);
        if(copy == NULL){
            fprintf(stderr, "ERROR in print_output : Could not allocate space for copy node\n");
            exit(EXIT_FAILURE);
        }
        add_node_to_cultivated_list(&cultivated_list_head, &head, copy);
        curr_node = curr_node->next;
    }

    //Now print out some stuff
    curr_node = cultivated_list_head;
    while(curr_node != NULL){
        size_t final_index = get_last_index_of_job_2(&head, curr_node->job);
        fprintf(stdout, "%s \t%zu\n", curr_node->job->person_name, final_index);
        curr_node = curr_node->next;
    }
}

void print_node(node* n){
    job* j = n->job;
    print_job(j);
    if(n->next == NULL){
        fprintf(stdout, "next is NULL\n");
    }
}

void destroy_list(node* head){
    while(head != NULL){
        node* to_free = head;
        head = head->next;
        destroy_node(to_free);
    }
}

void destroy_node(node* n){
    destroy_job(n->job);
    free(n);
}

size_t get_index_of_node(node* head, node* n){
    node* curr_node = head;
    size_t curr_index = 0;
    int found = 0;
    while(curr_node != NULL && (found == 0)){
        if(curr_node == n){
            found = 1;
            return curr_index;
        }
        curr_index++;
        curr_node = curr_node->next;
    }
    return -1;
}

void add_node_to_cultivated_list(node** cultivated_list_head, node** head_ref, node* to_insert){
    if(*cultivated_list_head == NULL){
        (*cultivated_list_head) = to_insert;
    }else{
        node* curr_node = *cultivated_list_head;
        while(curr_node->next != NULL){
            if(strncmp(curr_node->job->person_name, to_insert->job->person_name, strlen(to_insert->job->person_name)) == 0){
                //If the current job has the same person_name, compare their final indices
                size_t presiding_final_index = get_last_index_of_job_2(head_ref, curr_node->job);
                size_t incumbent_final_index = get_last_index_of_job_2(head_ref, to_insert->job);
                if(presiding_final_index < incumbent_final_index){
                    //New node ends later than curr_node, so replace jobs.
                    curr_node->job = to_insert->job;
                    return;
                }else{
                    return;
                }
            }
            curr_node = curr_node->next;
        }
        //If we're here, we didn't find a job with the same person_name, so add this job to the end of the list
        curr_node->next = to_insert;
    }
}

int add_node_to_list_end(node** head_ref, node* to_insert){
    //for now just add one node to end

    if(*head_ref == NULL){
        //list is empty
        *head_ref = to_insert;
    }else{
        //go to end
        node* curr_node = *head_ref;
        while(curr_node->next != NULL){
            curr_node = curr_node->next;
        }
        curr_node->next = to_insert;
    }

    return 0;
}

int add_node_to_list(node** head_ref, node* to_insert){
    /* We need to create one job per time slice
        if the job has duration 5, we need 5 jobs of durations 5, 4, 3, 2, 1
        We then insert each job individually into the list, that way this scenario is handled
            name    arrival duration
            B       6       1
            A       5       6
        We get
            time job duration
            ...
            5    A   6
            6    B   1
            7    A   5
            ...
        Which means each job will have an updated arrival time based on where the previous job was inserted

        There are a few scenarios for inserting
        1. The specified arrival time is beyond the length of the list currently
            ex: the first job we get arrives at t=17. We need to insert it at index 17,
                so we need to build the list up to 17 first. Times 0-16 will have NULL job entries
        2. There is a job at the specified arrival time
            Compare the duration of the incumbent job and the presiding one
                if incumbent is shorter, insert it before presiding job
                else check the next time slice and check for scenario 2 or 3
        3. There is no job at the specified arrival time
            Just put the job there!
        Once a time slice has been assigned to the job, all the remaining jobs must be added after it,
        which means the remaining jobs' arrival times need to be updated to account for the incumbent job's placement.
            Otherwise, job A's duration 4 slice will be put before its duration 5 slice and that's not what we want.
    */
    char* person_name = to_insert->job->person_name;
    char* job_name = to_insert->job->job_name;
    size_t arrival_time = to_insert->job->arrival_time;
    size_t duration = to_insert->job->duration;
    size_t num_nodes_to_add = duration;

    node* nodes_to_add[num_nodes_to_add];

    nodes_to_add[0] = to_insert;
    int i;
    for(i = 1; i < num_nodes_to_add; i++){

        arrival_time++;
        duration--;

        job* job_to_add = create_job(person_name, job_name, arrival_time, duration);
        if(job_to_add == NULL){
            fprintf(stderr, "ERROR in add_node_to_list() : Could not create job\n");
            exit(EXIT_FAILURE);
        }
        node* node_to_add = create_node(job_to_add);
        if(node_to_add == NULL){
            fprintf(stderr, "ERROR in add_node_to_list : Could not create node\n");
            exit(EXIT_FAILURE);
        }

        nodes_to_add[i] = node_to_add;
    }

    //We now have an array with all the nodes we need to add to the list
    // Once one job is added, cycle through the remaining jobs, updating their arrival times
    // to be greater than the job we just added
    node* curr_node;
    for(i = 0; i < num_nodes_to_add; i++){
        //For each node to add, traverse through the list to the desired location and attempt to add it
        curr_node = *head_ref;
        size_t curr_index = 0;
        size_t desired_index = nodes_to_add[i]->job->arrival_time;

        if(desired_index > get_length_list(curr_node)){
            //This means we're trying to add a node beyond the end of the list
            // so we need to build the list up from the last node to that point

            //If head is null, list is empty, add idle node to head
            if(curr_node == NULL){
                node* idle_node = create_idle_node();
                if(idle_node == NULL){
                    fprintf(stderr, "ERROR in add_node_to_list : Could not create idle node (1)\n");
                    exit(EXIT_FAILURE);
                }
                (*head_ref) = idle_node;
                curr_node = idle_node;
            }

            while(curr_node->next != NULL){
                curr_node = curr_node->next;
                curr_index++;
            }
            //curr_node now points to the last node in the list
            while((curr_index + 1) < desired_index){
                node* idle_node = create_idle_node();
                if(idle_node == NULL){
                    fprintf(stderr, "ERROR in add_node_to_list : Could not create idle node (2)\n");
                    exit(EXIT_FAILURE);
                }
                curr_node->next = idle_node;
                curr_node = curr_node->next;
                curr_index++;
            }
        }
        //At this point the list is long enough to add the item to the list
        // so let's move curr_node so that curr_node->next is where we want to add the node
        curr_node = *head_ref;
        curr_index = 0;

        //special case: head->next is null
        // !! I'm not sure these are actual cases, since the point of putting idle nodes in makes this impossible(?) !!
        if(curr_node == NULL){
            //List is empty (head points to null)
            // so just add nodes_to_add[i]
            (*head_ref)->next = nodes_to_add[i];
        }else if(curr_node->next == NULL){
            //There's nothing at the place where we want to add the node, just add it
            curr_node->next = nodes_to_add[i];
        }else{
            //The curr_index + 1 is because we want curr_index->next to point to the desired location,
            // not curr_index itself.
            while((curr_index + 1) < desired_index){
                curr_node = curr_node->next;
                curr_index++;
            }
            //Now we're at the place where we want to add the node
            // is it NULL? If not, is there an idle job or a normal job?
            if(curr_node->next == NULL){
                curr_node->next = nodes_to_add[i];
            }else{
                if(is_job_idle(curr_node->next->job)){
                    //Replace idle node with nodes_to_add[i]
                    node* temp = curr_node->next->next;
                    nodes_to_add[i]->next = temp;
                    curr_node->next = nodes_to_add[i];
                }else{
                    //There is a valid job at the spot where we want to add the node
                    // so compare durations
                    size_t incumbent_duration = nodes_to_add[i]->job->duration;
                    size_t presiding_duration = curr_node->next->job->duration;

                    if(incumbent_duration < presiding_duration){
                        //New job belongs before presiding job
                        nodes_to_add[i]->next = curr_node->next;
                        curr_node->next = nodes_to_add[i];

                        //Now we need to update the arrival times of every job after the one we just entered
                        curr_node = curr_node->next->next;
                        while(curr_node != NULL){
                            curr_node->job->arrival_time = curr_node->job->arrival_time + 1;
                            curr_node = curr_node->next;
                        }

                        //Done inserting node!
                    }else{
                        //Presiding job belongs before new job, find new place for incumbent job
                        // Search for the first available idle space, the first job that's longer than it, or the end of the list
                        curr_index = 0;
                        int found_spot = 0;
                        while(found_spot == 0){
                            if(curr_node->next == NULL){
                                //Add node to end of list
                                curr_node->next = nodes_to_add[i];
                                found_spot = 1;
                            }else if(is_job_idle(curr_node->next->job)){
                                //Replace first found idle spot
                                node* temp = curr_node->next->next;
                                nodes_to_add[i]->next = temp;
                                curr_node->next = nodes_to_add[i];
                                found_spot = 1;
                            }else if(curr_node->next->job->duration > incumbent_duration){
                                //node belongs before curr_node->next
                                nodes_to_add[i]->next = curr_node->next;
                                curr_node->next = nodes_to_add[i];

                                //Now we need to update the arrival times of every job after the one we just entered
                                curr_node = curr_node->next->next;
                                while(curr_node != NULL){
                                    curr_node->job->arrival_time = curr_node->job->arrival_time + 1;
                                    curr_node = curr_node->next;
                                }

                                found_spot = 1;
                            }
                            curr_node = curr_node->next;

                            if(found_spot){
                                //Update the arrival_times of the remaining nodes in nodes_to_add to be after the node we just added
                                size_t index_added_at = get_index_of_node(*head_ref, nodes_to_add[i]);
                                if(index_added_at == -1){
                                    //This means the node we're looking for wasn't added, but if(found_spot) means it was added, so something terrible has happened
                                    fprintf(stderr, "ERROR in add_node_to_list : get_index_of_node returned -1 for an unknown reason. Seriously, there's a check to make sure this never happens.\n");
                                    exit(EXIT_FAILURE);
                                }
                                int j;
                                for(j = 0; j < num_nodes_to_add; j++){
                                    nodes_to_add[j]->job->arrival_time = index_added_at + (j);
                                }
                            }
                        }
                    }
                }
            }
        }


    }
    return 0;
}

//-----------------------FORMATTING IMPLEMENTATION-----------------------//

void replace_whitespace(char* line, char replacement_char){

    //Cycle through string, replace every ' ' or '\t' with replacement_char
    int i;
    for(i = 0; i < strlen(line); i++){
        if(line[i] == ' ' || line[i] == '\t'){
            line[i] = replacement_char;
        }
    }
}

//-----------------------CONVERSION IMPLEMENTATION-----------------------//

/**
 * This function was made with the help of the strtol and strtoull man pages
 *  https://linux.die.net/man/3/strtol
 *  https://linux.die.net/man/3/strtoull
 */
size_t strtosizet(char* str){
    errno = 0;
    char* strtoull_error_check;
    int base = 10;
    unsigned long long int val = strtoull(str, &strtoull_error_check, base);

    //Error check template courtesy of strtol man page
    // https://linux.die.net/man/3/strtol
    if(
        (errno == ERANGE && (val == ULLONG_MAX || val == 0))
        || (errno != 0 && val == 0)
    ){
        fprintf(stderr, "ERROR in strtosizet : strtoull failed\n");
        exit(EXIT_FAILURE);
    }

    if(strtoull_error_check == str){
        fprintf(stderr, "ERROR in strtosizet : No digits were found\n");
        exit(EXIT_FAILURE);
    }

    size_t to_return = val;
    return to_return;
}
