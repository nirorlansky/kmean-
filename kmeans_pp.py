import numpy as np
import pandas as pd
import sys
import mykmeanssp as mk ;

ERROR_MESSAGE = "An Error Has Occurred"
np.random.seed(1234)

def validate_K(K, N):
    try:
        K = float(K)
    except ValueError:
        return False
    return int(float(K)) == float(K) and N > int(float(K)) > 1

def validate_eps(eps):
    try:
        eps = float(eps)
    except ValueError:
        return False
    return eps >= 0

def validate_iter(iter):
    try:
        iter = float(iter)
    except ValueError:
        return False
    return int(iter) == iter and 1000 > iter > 1


def validate_input_data(input_data):
    return input_data.endswith(".txt") or input_data.endswith(".csv")

def validate_args(K, eps, data1, data2, iter = 300):
    if not validate_input_data(data1) or not validate_input_data(data2):
        return 0, 0, [], 0
    
    data = extract_data(data1, data2)

    if validate_K(K, len(data)):
        K = int(float(K))
    else:
        print("Invalid number of clusters!")
        return 0, 0, [], 0
    if validate_iter(iter):
        iter = int(float(iter))
    else:
        print("Invalid maximum iteration!")
        return 0, 0, [], 0
    if validate_eps(eps):  # check if eps is a valid number
        eps = float(eps)
    else:
        print("Invalid epsilon!")
        return 0, 0, [], 0
    
    return K, eps, data, iter

def extract_data(data1, data2):
    data1 = pd.read_csv(data1, header=None)
    data2 = pd.read_csv(data2, header=None)
    data = pd.merge(data1, data2, on=0, how='inner') # merge the two dataframes
    data = data.sort_values(by=0)
    data = data.drop(columns=0) # drop the first column
    data = data.to_numpy()
    return data

def initial_k_centroids(data, k):

    centroids = []
    centroids.append(np.random.choice(len(data)))
    for i in range(1, k):
        dists = []
        for vector in data:
            dist = np.inf
            for centroid in centroids:
                dist = min(dist, np.linalg.norm(vector - data[centroid]))
            dists.append(dist)
        dists = np.array(dists)
        dists = dists / np.sum(dists)
        centroids.append(np.random.choice(len(data), p=dists))
    return centroids


def kmeans_pp(K, eps, data1, data2, iter = 300):
    K, eps, vectors, iter = validate_args(K, eps, data1, data2, iter)
    if K == 0:
        return
    centroids_index = initial_k_centroids(vectors, K)
    print(*centroids_index, sep=",")

    vectors = list(vectors)
    vectors = [list(vector) for vector in vectors]
    centroids = [vectors[i].copy() for i in centroids_index]

    l = mk.fit(K, iter, eps, vectors, centroids)
    print(l)

    # Print the initial centroids
    print("Initial centroids:")
    for centroid in centroids:
        print(centroid)



if __name__ == '__main__':
    if len(sys.argv) == 5:
        kmeans_pp(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
    elif len(sys.argv) == 6:
        kmeans_pp(sys.argv[1], sys.argv[3],sys.argv[4], sys.argv[5], sys.argv[2])
    else:
        print(ERROR_MESSAGE)
