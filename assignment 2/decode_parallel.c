#include "huffman.h"

#define OUT       "decoder"
#define BLOCKSIZE 1048576 // 1MB
// #define BLOCKSIZE 1073741824 // 1GB

static uint32_t        frequencies[256];
static HuffmanEncoding hfencoding[256];
static HuffmanTree     root;
static int             lengths[256];
static long            offsets[256];

void usage();
void perror_and_exit(const char *msg);
void reconstructHuffman();
int  decode(FILE *fp, const long off, const int len, char *buf);
void printEncoding(const uint32_t enc, const uint8_t len);
void prepareOffsets(FILE *fp);
void writeDecoding(FILE *fp, const int len, const char *buf);
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

    reconstructHuffman();
    prepareOffsets(fp);

    doParallel(fp, fpout);

    fclose(fp);
    fclose(fpout);
}

void usage() {
    printf("Usage: " OUT " <input_file> <output_file>\n");
    exit(EXIT_FAILURE);
}

void reconstructHuffman() {

    FILE *fp = fopen("encodings", "rb");
    if (fp == NULL)
        perror_and_exit("could not encodings file");

    fread(frequencies, sizeof(frequencies[0]), 256, fp);
    fread(hfencoding, sizeof(HuffmanEncoding), 256, fp);

    fclose(fp);

    root  = malloc(sizeof(HuffmanTreeNode));
    *root = (HuffmanTreeNode){
        .leaf   = false,
        .left   = NULL,
        .right  = NULL,
        .parent = NULL,
    };

    for (int i = 0; i < 256; i++) {
        if (frequencies[i] == 0)
            continue;

        HuffmanTreeNode *trav = root;
        uint8_t          len  = hfencoding[i].len;
        uint32_t         enc  = hfencoding[i].enc;
        for (int j = 0; j < len; j++) {
            uint32_t shift = 31 - j;

            if (enc & (1 << shift)) {
                if (trav->left == NULL) {
                    trav->left    = malloc(sizeof(HuffmanTreeNode));
                    *(trav->left) = (HuffmanTreeNode){
                        .leaf   = false,
                        .left   = NULL,
                        .right  = NULL,
                        .parent = trav,
                    };
                }
                trav = trav->left;
            }

            else {
                if (trav->right == NULL) {
                    trav->right    = malloc(sizeof(HuffmanTreeNode));
                    *(trav->right) = (HuffmanTreeNode){
                        .leaf   = false,
                        .left   = NULL,
                        .right  = NULL,
                        .parent = trav,
                    };
                }
                trav = trav->right;
            }
        }

        trav->leaf = true;
        trav->c    = i;
    }
}

int decode(FILE *fp, const long off, const int len, char *buf) {
    fseek(fp, off, SEEK_SET);

    char             c;
    int              idx  = 0;
    HuffmanTreeNode *trav = root;
    for (int i = 0; i < len; i++) {
        c          = fgetc(fp);
        int8_t pos = 7;
        while (pos >= 0) {
            if (c & (1 << pos)) {
                trav = trav->left;
            } else {
                trav = trav->right;
            }

            if (trav->leaf) {
                buf[idx++] = trav->c;
                trav       = root;
            }
            pos--;
        }
    }

    return idx;
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

void perror_and_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void prepareOffsets(FILE *fp) {
    int idx = 0;

    while (!feof(fp)) {
        int len = 4;
        fscanf(fp, "%d", &len);
        if (fgetc(fp) == EOF)
            break;
        lengths[idx] = len;
        offsets[idx] = ftell(fp);
        for (int i = 0; i < len; i++) {
            fgetc(fp);
        }
        fgetc(fp);
        idx++;
    }
}

void writeDecoding(FILE *fp, const int len, const char *buf) {
    for (int i = 0; i < len; i++) {
        fprintf(fp, "%c", buf[i]);
    }
}

void doParallel(FILE *fp, FILE *fpout) {
    MPI_Init(NULL, NULL);

    int world_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char *buf = calloc(BLOCKSIZE + 2, sizeof *buf);
    int   len = decode(fp, offsets[rank], lengths[rank], buf);

    if (rank != 0) {
        MPI_Send(&len, 1, MPI_INT, 0, rank, MPI_COMM_WORLD);
        MPI_Send(buf, len, MPI_CHAR, 0, rank, MPI_COMM_WORLD);
    }

    else {
        writeDecoding(fpout, len, buf);
        int   recvlen;
        char *recvbuf = calloc(BLOCKSIZE + 2, sizeof *recvbuf);
        for (int i = 1; i < world_size; i++) {
            MPI_Recv(&recvlen, 1, MPI_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(recvbuf, recvlen, MPI_CHAR, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (recvlen != 0)
                writeDecoding(fpout, recvlen, recvbuf);
        }
        free(recvbuf);
    }

    MPI_Finalize();
}
