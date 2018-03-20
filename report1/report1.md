#  WRITE COUNT PREDICTION FOR ENCRYPTED MAIN MEMORY

## Team 3

### Team Members

- Ajit Mathew
- Daulet 
- Sudha Ravali
- Pranavi
- Xiaolong



### Objective

To create a memory system to predict the write count of a memory location before the location is actually fetched into the main memory.



### Task

- **Implement a lossy hash table to model Last Level Cache in C++**
- **Implement data structures to store write count history, predicted write count**
- Generate address traces using benchmarks
- **Implement Prediction Algorithm based on the history**
- **Vary parameters to find optimal prediction coverage.**

*Note: Tasks in bold has been completed successfully*

### Design

The design is divided into four main components:

- **Main Memory**: Simulation model of the main memory that stores all address blocks from the address trace file.
- **Cache with Write History Holder**: 2MB LLC with a LRU replacement policy and n-way set associativity (n is an adjustable parameter in the model). 
- **Pattern Fifo**: This is used to keep track of cache miss history.
- **Predicted Write Count Buffer** (pwcBuf): This buffer stores the predicted write count. On a cache miss, the buffer is read and compared to actual write count














### Prediction Algorithm

```flow
st=>start: Fetch A
op=>operation: Check in pwcBuf and compare
op1=>operation: update coverage
op2=>operation: Put A in $
op3=>operation: Find element to evict(E)
op4=>operation: Update WC and writeback
cond1=>condition: Check A is in $
cond2=>condition: Check A in pwcBuf
cond3=>condition: Check $ needs eviction
e=>end
e1=>end

st->cond1
cond1(yes)->e
cond1(no)->op(right)->op1
op1->cond3(yes)->op3->op4->op2
cond3(no)->op2->e1
```

```python
def putinCache(A):
	patternFifo.size() == HISTORY_SIZE:
        x = patternFifo.pop()
        UpdateWCHistory(x)
    patternFifo.push(A)
```



### Parameters

Our model is completely parameterized. The parameters we chose to analyse our design are:

- **Write Count History Size**: This determines the size of *write count history holder (WCHH)*.
- **Range Size**: This determines the range of predicted write count history. For example if write count of **B** according to **WCHH** of  **A** is n, the n to n+r values will be used to predict the write count of B.
- **Set Size**: This determines the number of cache lines that can be stored in on cache set (n-way set associativity).

### Results

We ran our model on **NAS Parallel Benchmark** for three different workloads: 

CG - Conjugate Gradient benchmark computes an estimate of the largest eigenvalue of a symmetric positive definite sparse matrix with a random patter of nonzeros [1]. 

MG - Multi-Grid benchmark performs four iteration of the V-cycle multigrid algorithm to obtain an approximate solution to Poisson equation  [1].

EP - Embarrassingly Parallel benchmark generates pairs of Gaussian random deviates and tabulates the number of pairs in successive square annuli [1].



#### Conjugate Gradient

**Prediction Rate vs History Size:**

*Note: Y-axis is a prediction rate/coverage (# of correct predictions/ # of total predictions), not Total Cache Lines.*

![rediction_vs_HistorySiz](../cg_pintrace2/Prediction_vs_HistorySize.png)

**Prediction Rate vs Range Size:**

<img src="../cg_pintrace2/Prediction_vs_RangeSize.png" height="430px"/>

**Prediction Rate vs Set Size:**

<img src="../cg_pintrace2/Prediction_vs_SetSize.png" height="450px"/>

#### Embarrassingly Parallel

*Note: Y-axis is a prediction rate/coverage (# of correct predictions/ # of total predictions), not Total Cache Lines*

**Prediction Rate vs History Size:**

<img src="../ep_plots/Prediction_vs_HistorySize.png" height="430px"/>

**Prediction Rate vs Range Size:**

<img src="../ep_plots/Prediction_vs_RangeSize.png" height="430px"/>

**Prediction Rate vs Set Size:**

<img src="../ep_plots/Prediction_vs_SetSize.png" height="430px"/>

#### Multi-Grid

*Note: Y-axis is a prediction rate/coverage (# of correct predictions/ # of total predictions), not Total Cache Lines.*

**Prediction Rate vs History Size:**

<img src="../mg_plots/Prediction_vs_HistorySize.png" height="430px"/>

**Prediction Rate vs Range Size:**

<img src="../mg_plots/Prediction_vs_RangeSize.png" height="430px"/>

**Prediction Rate vs Set Size:**

<img src="../mg_plots/Prediction_vs_SetSize.png" height="430px"/>





### Analysis

#### Interpreting Results

Our write count prediction model shows fairly good results for Conjugate Gradient and Embarrassingly Parallel benchmarks. CG shows the best results with more than 70% coverage rate for all parameters. Increasing the write count history size (WCHH size) from 1 to 10 results in almost 14% increase in coverage, and increasing the range size from 1 to 10 results in more than 15% increase in coverage. Changing the value of associativity/set size from 1 (direct mapped) to 16 does not have any major effect on coverage. 

#### Cost Analysis

####Future Optimization

### Conclusion

