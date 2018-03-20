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

The design is divided into three main component:

- **Main Memory**
- **Cache**
- **Predicted Write Count Buffer** (pwcBuf)
- **Patter Fifo**















### Prediction Algorithm

```flow
st=>start: Fetch A
op=>operation: Check in pwcBuf
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



### Results

### Analysis

### Conclusion
