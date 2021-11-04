#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<assert.h>
#include<string.h>

#define die(...) fprintf(stderr, "erro: "); fprintf(stderr, __VA_ARGS__); exit(1)
#define warn(...) fprintf(stderr, "aviso: "); fprintf(stderr, __VA_ARGS__)

float *key = NULL;
char *buf = NULL;
float *calcMat = NULL;
float *tempMat = NULL;
int keysize;
int blocksize;

void usage() {
    fprintf(stderr, "matcrypt {enc,dec} k1 k2 ... kn\n");
    fprintf(stderr, " algoritmo de criptografia usando propriedades de matrizes\n");
    fprintf(stderr, " POR FAVOR, NÃO USE PARA NADA SÉRIO, ou use, tu que sabe!\n");
}

void encode_buffer(char *buf, float *mat, int size) {
    for (int i = 0; i < size; i++) {
        mat[i] = buf[i];
    }
}

void decode_matrix(char *buf, float *mat, int size) {
    for (int i = 0; i < size; i++) {
        buf[i] = mat[i];
    }
}

void handle_enc() {
    while(!feof(stdin)) {
        int nbytes = fread(buf, sizeof(char), blocksize, stdin);
        for (int i = nbytes; i < blocksize; i++) {
            buf[i] = -1; // completa o que falta com -1
        }
        encode_buffer(buf, calcMat, blocksize);
        // matrix multiplication, transformation to buffer and writing to stdout
    }
}

void handle_dec() {
    while(!feof(stdin)) {
        int nbytes = fread(buf, sizeof(char), blocksize, stdin);
        int writeSize = blocksize;
        for (int i = blocksize - 1; i >= 0; i--) {
            if (buf[i] == -1) {
                writeSize--;
            }
        }
        decode_matrix(buf, calcMat, blocksize);
        // matrix multiplication with the inverse of the key, transformation to buffer and writing to stdout
    }
}

int is_perfect_square(int n) {
    int i = 0;
    do {
        i++;
    }
    while (i*i < n);
    return i*i == n;
}

int sqrt_int(int n) {
    int i = 0;
    do {
        i++;
    }
    while (i*i < n);
    return i;
}

void healthcheck() {
    // is_perfect_square
    for (int i = 1; i < 10; i++) {
        int res = is_perfect_square(i * i);
        fflush(stdout);
        fflush(stderr);
        assert(res);
    }
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        usage();
    }
    char *command = argv[1];
    argc -= 2;
    argv += 2;
    if (!is_perfect_square(argc)) {
        die("o tamanho da chave tem que ser um quadrado perfeito\n");
    }
    keysize = sqrt_int(argc);
    if (keysize < 4) {
        warn("%i é definitivamente um tamanho muito pequeno para a chave mas você que sabe\n", keysize);
    }
    blocksize = keysize * keysize;
    key = malloc(sizeof(float) * argc);
    calcMat = malloc(sizeof(float) * argc);
    tempMat = malloc(sizeof(float) * argc);
    buf = malloc(sizeof(char) * argc);
    for (int i = 0; i < argc; i++) {
        key[i] = strtof(argv[i], NULL);
        /* printf("%f\n", key[i]); */
    }
    if (strcmp(command, "enc")) {
        handle_enc();
    } else if (strcmp(command, "dec")) {
        handle_dec();
    }
    printf("%i\n", argc);
    free(key);
    free(buf);
    free(calcMat);
    free(tempMat);
    return 0;
}
