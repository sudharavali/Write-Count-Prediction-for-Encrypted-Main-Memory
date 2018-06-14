# Write-Count-Prediction-for-Encrypted-Main-Memory

Encrypted main memory would help many security applications, however, the decryption adds overhead to the system performance. Predicting the memory write count could save the time for decryption significantly. Challenges in write count prediction need to be identified and explored.

Motivation

CPU executes code and operates on data and addresses in plaintext only, and writes data into memory and reads data from memory, through buses by providing the corresponding logical addresses over the the address bus. Physical and logical attacks on could happen in a lot of levels, the memory is usually the target, so encryption standard like DES and AES are established. So we will encrypt plaintext into ciphertext to be written into memory, while also decrypt the ciphertext to plaintext to be read from memory. However, this takes time and brings overhead to system speed. If we look into this problem, the secret key, addresses and the write count of the memory block, would be needed to decrypt the ciphertext. Write count prediction would be significant to reduce the time for decryption. In Intel SGX(a new CPU extension for executing secure code in Intel processors), it requires write count of memory location before decrypting it.

Key Idea

To be able to successfully predict the write count of a cache miss, the algorithm should be able to do the following:

● Predict the cache miss address This is done using a mechanism similar to linked-list based prefetch. The intuition is that if a cache miss pattern happened in the past, then it is highly likely that this will happen again.
 
● Predict the write count of the miss address To predict the write count, we append each cache block with approximate write count of the next N predicted misses. Also to improve the chances of correct prediction, we use the stored approximate write count to generate a range of predicted write counts.
