#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ERROR_MESSAGE "An Error Has Occurred"

int N = 0;
int d = 0;

struct cord {
    double value;
    struct cord* next;
};

struct vector {
    struct vector* next;
    struct cord* cords;
};

struct centroid
{
    struct cord* cords;
    struct centroid* next;
    int group_size;
    struct cord* sum;
};

typedef struct vector vector;
typedef struct cord cord;
typedef struct centroid centroid;

long validate_K(char* K){
    char* end;
    long k = strtol(K, &end, 10);
    if (*end == '\0' && N > k && k > 1){
        return k;
    }
    return 0;
}

long validate_iter(char* iter){
    char* end;
    long num_iter = strtol(iter, &end, 10);
    if (*end == '\0' && 1000 > num_iter && num_iter > 1){
        return num_iter;
    }
    return 0;
}

void print_vector(cord* cord){
    while(cord != NULL){
        printf("%.4f", cord->value);
        cord = cord->next;
        if(cord != NULL){
            printf(",");
        }
    }
    printf("\n");
}

cord* line_to_cord(char* line){
    char* ptr;
    struct cord* head_cord;
    struct cord* curr_cord;
    int i;
    ptr = line;
    head_cord = malloc(sizeof(struct cord));
    head_cord->next = NULL;
    curr_cord = head_cord;
    for (i = 0; i < d; i++)
    {
        struct cord* new_cord;
        while (*ptr != ',' && *ptr != '\n')
        {
            ptr++;
        }
        new_cord = malloc(sizeof(struct cord));
        new_cord->next = NULL;
        new_cord->value = strtod(line, &ptr);
        ptr++;
        line = ptr;
        curr_cord->next = new_cord;
        curr_cord = new_cord;
    }
    curr_cord = head_cord->next;
    free(head_cord);
    return curr_cord;
}

double find_distance(cord* centroid, cord* vec){
    double sum = 0;
    while(centroid != NULL && vec != NULL){
        sum += pow(centroid->value - vec->value, 2);
        centroid = centroid->next;
        vec = vec->next;
    }
    return sqrt(sum);
}

vector* file_to_vectors() {
    size_t bufsize = 0;
    int is_end_of_file;
    char *line =NULL;
    char* ptr;
    struct vector* head_vector;
    struct vector* curr_vector;
    is_end_of_file = getline(&line, &bufsize, stdin);
    if(is_end_of_file == -1){
        return NULL;
    }
    d = 1;
    ptr = line;
    while(*ptr != '\n'){
        if(*ptr == ','){
            d++;
        }
        ptr++;
    }
    head_vector = malloc(sizeof(struct vector));
    head_vector->next = NULL;
    curr_vector = head_vector;
    do
    {
        struct vector* new_vector;
        struct cord* curr_cord;
        new_vector = malloc(sizeof(struct vector));
        curr_vector->next = new_vector;
        curr_vector = new_vector;
        curr_cord = line_to_cord(line);
        curr_vector->cords = curr_cord;
        curr_vector->next = NULL;
        N++;

    }while ((is_end_of_file = getline(&line, &bufsize, stdin)) != EOF);
    curr_vector = head_vector->next;
    free(head_vector);
    free(line);
    return curr_vector;
}

void copy_cords(cord* from, cord* to){
    while(from != NULL && to != NULL){
        to->value = from->value;
        from = from->next;
        to = to->next;
    }
}

cord* create_zeros_vector(){
    struct cord* zeros;
    struct cord* head;
    int j;
    zeros = malloc(sizeof(struct cord));
    zeros->next = NULL;
    zeros->value = 0;
    head = zeros;
    for (j = 0; j < d-1; j++)
    {
        struct cord* next;
        next = malloc(sizeof(struct cord));
        next->next = NULL;
        next->value = 0;
        zeros->next = next;
        zeros = next;
    }
    return head;
}

centroid* initial_centroids(vector* vectors, long K){
    struct centroid* head_centroid;
    struct centroid* curr_centroid;
    int i;
    head_centroid = malloc(sizeof(struct centroid));
    head_centroid->next = NULL;
    head_centroid->group_size = 0;
    head_centroid->sum = NULL;
    curr_centroid = head_centroid;
    for (i = 0; i < K; i++)
    {
        struct centroid* new_centroid;
        new_centroid = malloc(sizeof(struct centroid));
        new_centroid->sum = create_zeros_vector();
        new_centroid->cords = create_zeros_vector();
        copy_cords(vectors->cords, new_centroid->cords);
        new_centroid->next = NULL;
        new_centroid->group_size = 0;
        curr_centroid->next = new_centroid;
        curr_centroid = new_centroid;
        vectors = vectors->next;
    }
    curr_centroid = head_centroid->next;

    free(head_centroid);
    return curr_centroid;
}

centroid* find_min_dist(centroid* centroids, cord* cords){
    struct centroid* min_cent;
    double min_dist = -1;
    double curr_dist;
    min_cent = centroids;
    while (centroids != NULL)
    {
        curr_dist = find_distance(centroids->cords, cords);
        if (min_dist > curr_dist || min_dist == -1)
        {
            min_dist = curr_dist;
            min_cent = centroids;
        }
        centroids = centroids->next;
    }
    return min_cent;
}

void add_vector_to_centroid_sum(cord* sum, cord* vec){
    while (sum != NULL && vec != NULL)
    {
        sum->value += vec->value;
        sum = sum->next;
        vec = vec->next;
    }
}

void comupte_avg(cord* sum, int size){
    while (sum != NULL)
    {
        sum->value = sum->value/size;
        sum = sum->next;
    }
}

void sum_to_zeros(cord* sum){
    while(sum!= NULL){
        sum->value = 0;
        sum = sum->next;
    }
}

void free_cords(cord* cords){
    cord* next_cord;
    while(cords != NULL){
        next_cord = cords->next;
        free(cords);
        cords = next_cord;        
    }
}

void free_memory(vector* vectors, centroid* centroids){
    centroid* next_cent;
    vector* next_vec;
    while(centroids != NULL){
        next_cent = centroids->next;
        free_cords(centroids->cords);
        free_cords(centroids->sum);
        free(centroids);
        centroids = next_cent;        
    }
    while(vectors != NULL){
        next_vec = vectors->next;
        free_cords(vectors->cords);
        free(vectors);
        vectors = next_vec;        
    }
}


centroid* compute_centroids(vector* vectors, long K, long iter, double epsilon){
    struct centroid* centroids = initial_centroids(vectors, K);
    struct vector* iter_vec;
    struct centroid* curr_centroid;
    struct centroid* closest_centroid;
    int changed, i;
    for (i = 0; i < iter; i++)
    {
        changed = 0;
        iter_vec = vectors;
        curr_centroid = centroids;
        while (iter_vec != NULL)
        {
            closest_centroid = find_min_dist(centroids, iter_vec->cords);
            closest_centroid->group_size++;
            add_vector_to_centroid_sum(closest_centroid->sum, iter_vec->cords);
            iter_vec = iter_vec->next;
        }
        while(curr_centroid != NULL){
            if(curr_centroid->group_size != 0){
                comupte_avg(curr_centroid->sum, curr_centroid->group_size);
                if(find_distance(curr_centroid->sum, curr_centroid->cords) > epsilon){
                    changed = 1;
                }
                copy_cords(curr_centroid->sum, curr_centroid->cords);
                sum_to_zeros(curr_centroid->sum);
                curr_centroid->group_size = 0;
            }
            curr_centroid = curr_centroid->next;
        }
        if(changed == 0){
            return centroids;
        }
    }
    return centroids; 
}

int main(int argc, char **argv){
    long K;
    long iter;
    struct vector* vectors;
    struct centroid* centroids, *centroids_iter;
    vectors = file_to_vectors();
    if(argc == 1 || argc > 3){
        printf("%s\n", ERROR_MESSAGE);
        return 1;
    }
    if(argc == 2){
        iter = 200;
    }else{
        iter = validate_iter(argv[2]);
        if(iter == 0){
            printf("%s\n", "Invalid maximum iteration!");
            return 1;
        }
    }
    K = validate_K(argv[1]);
    if(K == 0){
        printf("%s\n", "Invalid number of clusters!");
        return 1;
    }
    centroids = compute_centroids(vectors, K, iter, 0.001);
    centroids_iter = centroids;
    while (centroids_iter != NULL){
        print_vector(centroids_iter->cords);
        centroids_iter = centroids_iter->next;
    }
    free_memory(vectors, centroids);
    return 0;
}