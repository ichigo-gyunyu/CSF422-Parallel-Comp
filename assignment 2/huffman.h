#include <errno.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "Utils/pq.h"

typedef struct HuffmanTreeNode {
    uint32_t                val;
    bool                    leaf;
    char                    c;
    struct HuffmanTreeNode *left;
    struct HuffmanTreeNode *right;
    struct HuffmanTreeNode *parent;
} HuffmanTreeNode;

typedef HuffmanTreeNode *HuffmanTree;

typedef struct HuffmanEncoding {
    char     c;
    uint8_t  len;
    uint32_t enc;
} HuffmanEncoding;
