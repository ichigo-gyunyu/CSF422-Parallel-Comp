#include "huffman.h"

#define OUT       "encoder"
#define BLOCKSIZE 1048576 // 1MB
// #define BLOCKSIZE 1073741824 // 1GB

static uint32_t        frequencies[256];
static HuffmanEncoding hfencoding[256];
static HuffmanTree     root;

typedef struct pqele {
    char             c;
    u_int32_t        fq;
    HuffmanTreeNode *node;
} pqele;

void usage();
void perror_and_exit(const char *msg);
void constructHuffmanTree();
void parseHuffmanTree(const HuffmanTreeNode *node, const uint8_t depth, const uint32_t encoding);
bool cmp(const void *e1, const void *e2);
int  encode(FILE *fp, const long offset, char *buf);
void saveEncodings();
void printEncoding(const uint32_t enc, const uint8_t len);
void writeEncoding(FILE *fp, const int len, const char *buf);
void doParallel(FILE *fp, FILE *fpout); // All open MPI related calls

int main(int argc, char **argv) {

    if (argc != 3)
        usage();

    FILE *fp    = fopen(argv[1], "r");
    FILE *fpout = fopen(argv[2], "w");
    if (fp == NULL)
        perror_and_exit("could not open input file");
    if (fpout == NULL)
        perror_and_exit("could not open output file");

    int c;
    while ((c = fgetc(fp)) != EOF)
        frequencies[c]++;

    constructHuffmanTree();
    parseHuffmanTree(root, 0, 0);
    saveEncodings();

    doParallel(fp, fpout);

    fclose(fp);
    fclose(fpout);
}

void usage() {
    printf("Usage: " OUT " <input_file> <output_file>\n");
    exit(EXIT_FAILURE);
}

void constructHuffmanTree() {
    PriorityQueue *pq = pq_init(cmp, 256);

    for (int i = 0; i < 256; i++) {
        if (frequencies[i] == 0)
            continue;

        pqele *e = malloc(sizeof *e);

        *e = (pqele){
            .c    = i,
            .fq   = frequencies[i],
            .node = malloc(sizeof(HuffmanTreeNode)),
        };

        *(e->node) = (HuffmanTreeNode){
            .val    = frequencies[i],
            .leaf   = true,
            .c      = i,
            .left   = NULL,
            .right  = NULL,
            .parent = NULL,
        };

        pq_push(pq, e);
    }

    while (pq->size > 1) {
        pqele *r = pq_pop(pq);
        pqele *l = pq_pop(pq);

        HuffmanTreeNode *node = malloc(sizeof *node);

        *node = (HuffmanTreeNode){
            .val    = r->fq + l->fq,
            .leaf   = false,
            .c      = 0,
            .left   = l->node,
            .right  = r->node,
            .parent = NULL,
        };

        l->node->parent = node;
        r->node->parent = node;

        pqele *e = malloc(sizeof *e);

        *e = (pqele){
            .fq   = node->val,
            .c    = 0,
            .node = node,
        };

        pq_push(pq, e);

        free(r);
        free(l);
    }

    pqele *e = pq_pop(pq);
    root     = e->node;

    free(e);
    pq_free(pq);
}

void parseHuffmanTree(const HuffmanTreeNode *node, const uint8_t depth, const uint32_t encoding) {

    if (node == NULL)
        return;

    if (node->leaf) {
        hfencoding[(int)node->c] = (HuffmanEncoding){
            .c   = node->c,
            .len = depth,
            .enc = encoding,
        };

        return;
    }

    uint32_t shift = 31 - depth;
    uint32_t left  = encoding | (1 << shift);
    uint32_t right = encoding & ~(1 << shift);
    parseHuffmanTree(node->left, depth + 1, left);
    parseHuffmanTree(node->right, depth + 1, right);
}

int encode(FILE *fp, const long off, char *buf) {
    fseek(fp, off, SEEK_SET);

    if (fgetc(fp) == EOF) {
        return 0;
    }

    fseek(fp, off, SEEK_SET);

    int idx = 0, pos = 0;

    int c;
    for (int j = 0; j < BLOCKSIZE; j++) {
        if ((c = fgetc(fp)) == EOF)
            break;
        uint8_t  len = hfencoding[c].len;
        uint32_t enc = hfencoding[c].enc;
        for (int i = 0; i < len; i++) {
            uint32_t shift = 31 - i;
            if (enc & (1 << shift))
                buf[idx] |= (1 << (7 - pos));

            pos++;
            if (pos == 8) {
                pos = 0;
                idx++;
            }
        }
    }

    return idx + 1;
}

void writeEncoding(FILE *fp, const int len, const char *buf) {
    fprintf(fp, "%d\n", len);
    for (int i = 0; i < len; i++) {
        fprintf(fp, "%c", buf[i]);
    }
    fprintf(fp, "\n");
}

void saveEncodings() {
    FILE *fp = fopen("encodings", "wb");
    if (fp == NULL)
        perror_and_exit("could not open file to save encodings");

    fwrite(frequencies, sizeof(frequencies[0]), 256, fp);
    fwrite(hfencoding, sizeof(HuffmanEncoding), 256, fp);

    fclose(fp);
}

void printEncoding(const uint32_t enc, const uint8_t len) {
    for (int i = 0; i < len; i++) {
        uint32_t shift = 31 - i;
        if (enc & (1 << shift))
            printf("1");
        else
            printf("0");
    }

    printf(" (%d)\n", len);
}

bool cmp(const void *e1, const void *e2) { return (((pqele *)e1)->fq <= ((pqele *)e2)->fq); }

void perror_and_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void doParallel(FILE *fp, FILE *fpout) {
    MPI_Init(NULL, NULL);

    int world_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char *buf = calloc(BLOCKSIZE + 2, sizeof *buf);
    int   len = encode(fp, (long)rank * BLOCKSIZE, buf);

    if (rank != 0) {
        MPI_Send(&len, 1, MPI_INT, 0, rank, MPI_COMM_WORLD);
        MPI_Send(buf, len, MPI_CHAR, 0, rank, MPI_COMM_WORLD);
    }

    else {
        writeEncoding(fpout, len, buf);
        int   recvlen;
        char *recvbuf = calloc(BLOCKSIZE + 2, sizeof *recvbuf);
        for (int i = 1; i < world_size; i++) {
            MPI_Recv(&recvlen, 1, MPI_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(recvbuf, recvlen, MPI_CHAR, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (recvlen != 0)
                writeEncoding(fpout, recvlen, recvbuf);
        }
        free(recvbuf);
    }

    free(buf);
    MPI_Finalize();
}
