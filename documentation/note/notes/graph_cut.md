# Graph Cut (图割)

# Definition

- Set of edges whose removal makes a graph **disconnected**
- Cost of a cut: `sum` of weights of cut edges:
    
    $$
    cut(A,B)=\sum_{p\in A, q\in B}{w_{pq}}
    $$
    

# Applicaiton

- A graph cut gives us a segmentation

# Algorithms (Core)

What is a “good” graph cut and how do we find one?

- Max-Flow or Min-Cut Algorithms
- Fast min-cut algorithms exist