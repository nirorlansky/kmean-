#define PY_SSIZE_T_CLEAN

#include <Python.h>
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

centroid* find_min_dist(centroid* centroids, cord* cords);
void add_vector_to_centroid_sum(cord* sum, cord* vec);
void comupte_avg(cord* sum, int size);
void sum_to_zeros(cord* sum);
void free_cords(cord* cords);
void free_memory(vector* vectors, centroid* centroids);
centroid* compute_centroids(vector* vectors, long K, long iter, double epsilon, centroid* centroids);
static PyObject* fit(PyObject* self, PyObject* args);
cord* line_to_cord(PyObject* line);
vector* file_to_vectors(PyObject* vectors);
centroid* file_to_centroids(PyObject* centroids);
double find_distance(cord* centroid, cord* vec);
void copy_cords(cord* from, cord* to);
cord* create_zeros_vector();
void print_vector(cord* cord);
// static PyObject* c_to_py_list(double c_list[]);
void free_matrix(double** matrix, int K);
double** centroids_to_matrix(centroid* centroids, int K, int d);


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

centroid* compute_centroids(vector* vectors, long K, long iter, double epsilon, centroid* centroids){
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

void free_matrix(double** matrix, int K) {
    for (int i = 0; i < K; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

static PyObject* fit(PyObject* self, PyObject* args){
    PyObject* vectors;
    PyObject* centroids;
    struct vector* vectors_list;
    struct centroid* centroids_list;
    
    long K;
    long iter;
    double epsilon;
    double** matrix;
    int i, j;

    if(!PyArg_ParseTuple(args, "lldOO", &K, &iter, &epsilon, &vectors, &centroids)){    
        return NULL;
    }
 
    vectors_list = file_to_vectors(vectors);
    centroids_list = file_to_centroids(centroids);
    
    centroids_list = compute_centroids(vectors_list, K, iter, epsilon, centroids_list);
    matrix = centroids_to_matrix(centroids_list, K, d);

    PyObject* py_matrix = PyList_New(K);
    for (i= 0; i < K; i++) {
        PyObject* row = PyList_New(d);
        for (j = 0; j < d; j++) {
            if (i == 3 && j == 4) {
                printf("%f\n", matrix[i][j]);
            }
            PyList_SetItem(row, j, PyFloat_FromDouble(matrix[i][j]));
        }
        PyList_SetItem(py_matrix, i, row);
    }

    free_memory(vectors_list, centroids_list);
    free_matrix(matrix, K);
    return py_matrix;
}


double** centroids_to_matrix(centroid* centroids, int K, int d) {
    double** matrix = (double**)malloc(K * sizeof(double*));
    if (!matrix) {
        printf(ERROR_MESSAGE);
        return NULL;
    }

    for (int i = 0; i < K; i++) {
        matrix[i] = (double*)malloc(d * sizeof(double));
        if (!matrix[i]) {
            printf(ERROR_MESSAGE);
            for (int j = 0; j < i; j++) {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
    }

    centroid* curr_centroid = centroids;
    for (int i = 0; i < K && curr_centroid != NULL; i++) {
        cord* curr_cord = curr_centroid->cords;
        for (int j = 0; j < d && curr_cord != NULL; j++) {
            matrix[i][j] = curr_cord->value;
            curr_cord = curr_cord->next;
        }
        curr_centroid = curr_centroid->next;
    }

    return matrix;
}

cord* line_to_cord(PyObject* line){
    struct cord* head_cord;
    struct cord* curr_cord;
    int i;
    PyObject* cor;
    head_cord = malloc(sizeof(struct cord));
    head_cord->next = NULL;
    curr_cord = head_cord;
    d = PyObject_Length(line);
    for (i = 0; i < d; i++)
    {
        struct cord* new_cord;
        cor = PyList_GetItem(line, i);
        new_cord = malloc(sizeof(struct cord));
        new_cord->next = NULL;
        new_cord->value = PyFloat_AsDouble(cor);
        curr_cord->next = new_cord;
        curr_cord = new_cord;
    }
    curr_cord = head_cord->next;
    free(head_cord);
    return curr_cord;
}

vector* file_to_vectors(PyObject* vectors) {
    struct vector* head_vector;
    struct vector* curr_vector;
    PyObject* vec;
    N = PyObject_Length(vectors);
    int i =0;
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
        vec = PyList_GetItem(vectors, i);
        curr_cord = line_to_cord(vec);
        curr_vector->cords = curr_cord;
        curr_vector->next = NULL;
        i++;

    }while (i<N);
    curr_vector = head_vector->next;
    free(head_vector);
    return curr_vector;
}

centroid* file_to_centroids(PyObject* centroids) {
    struct centroid* head_centroid;
    struct centroid* curr_centroid;
    PyObject* cent;
    int K = PyObject_Length(centroids);
    int i =0;
    head_centroid = malloc(sizeof(struct centroid));
    head_centroid->next = NULL;
    curr_centroid = head_centroid;
    do
    {
        struct centroid* new_centroid;
        struct cord* curr_cord;
        new_centroid = malloc(sizeof(struct centroid));
        curr_centroid->next = new_centroid;
        curr_centroid = new_centroid;
        cent = PyList_GetItem(centroids, i);
        curr_cord = line_to_cord(cent);
        curr_centroid->cords = curr_cord;
        curr_centroid->next = NULL;
        curr_centroid->group_size=0;
        curr_centroid->sum = create_zeros_vector();
        i++;

    }while (i<K);
    curr_centroid = head_centroid->next;
    free(head_centroid);
    return curr_centroid;
}

static PyMethodDef fitMethods[] = {
    {"fit", 
    (PyCFunction) fit,
    METH_VARARGS,
    PyDoc_STR("calculate the kmeans centroids given vector and initial centroids")
    },
    {NULL, NULL, 0 ,NULL}
};

static struct PyModuleDef fitmodule = {
    PyModuleDef_HEAD_INIT, 
    "mykmeanssp",
    NULL,
    -1,
    fitMethods
};

PyMODINIT_FUNC PyInit_mykmeanssp(void){
    PyObject *m;
    m = PyModule_Create(&fitmodule);
    if (!m)
    {
        return NULL;
    }
    return m;
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



double find_distance(cord* centroid, cord* vec){
    double sum = 0;
    while(centroid != NULL && vec != NULL){
        sum += pow(centroid->value - vec->value, 2);
        centroid = centroid->next;
        vec = vec->next;
    }
    return sqrt(sum);
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




// int main(int argc, char **argv){
//     long K;
//     long iter;
//     struct vector* vectors;
//     struct centroid* centroids, *centroids_iter;
//     vectors = file_to_vectors();
//     centroids = compute_centroids(vectors, K, iter, 0.001);
//     centroids_iter = centroids;
//     while (centroids_iter != NULL){
//         print_vector(centroids_iter->cords);
//         centroids_iter = centroids_iter->next;
//     }
//     free_memory(vectors, centroids);
//     return 0;
// }