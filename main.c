#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<assert.h>
#include<string.h>
#include<math.h>

#define die(...) fprintf(stderr, "erro: "); fprintf(stderr, __VA_ARGS__); exit(1)
#define warn(...) fprintf(stderr, "aviso: "); fprintf(stderr, __VA_ARGS__)

void mat_transpose(float *num, float *fac, int r, float *ret);
void mat_cofactor(float *num, float *ret);
float mat_determinant(float *a, int k);

float *key = NULL;
int *buf = NULL;
float *calcMat = NULL;
float *tempMat = NULL;
float *resMat = NULL;
int keysize;
int blocksize;

float *alloc_shaped_matrix() {
    return malloc(sizeof(float)*blocksize);
}

void usage() {
    fprintf(stderr, "matcrypt {enc,dec} k1 k2 ... kn\n");
    fprintf(stderr, " algoritmo de criptografia usando propriedades de matrizes\n");
    fprintf(stderr, " POR FAVOR, NÃO USE PARA NADA SÉRIO, ou use, tu que sabe!\n");
}

void decode_matrix(int *buf, float *mat, int size) {
    for (int i = 0; i < size; i++) {
        mat[i] = buf[i];
    }
}

void encode_matrix(int *buf, float *mat, int size) {
    for (int i = 0; i < size; i++) {
        buf[i] = mat[i];
    }
}
void mat_mul(float *a, float *b, float *res, int magnitude) {
    memset(res, 0, sizeof(float)*magnitude*magnitude); // zera a matriz de saída
    for (int i = 0; i < magnitude; i++) {
        for (int j = 0; j < magnitude; j++) {
            for (int k = 0; k < magnitude; k++) {
                res[(magnitude*i)+j] += a[(magnitude*i) + k] * b[(magnitude*k) + j];
            }
        }
    }
}

void mat_invert(float *min, float *ret) {
    if (mat_determinant(min, keysize) == 0) {
        die("chave inválida, determinante = 0");
    }
    mat_cofactor(min, ret);
}

int read_buffer_int(int *buf, int size) {
    for (int i = 0; i < size * (sizeof(int)/sizeof(char)); i++) {
        int c = getc(stdin);
        if (c == EOF) {
            return i / (sizeof(int)/sizeof(char));
        }
        *((char*)buf + i) = c;
    }
    return size;
}

int read_buffer_char(int *buf, int size) {
    for (int i = 0; i < size; i++) {
        int c = getc(stdin);
        if (c == EOF) {
            return i;
        }
        buf[i] = c;
    }
    return size;
}

int write_buffer_char(int *buf, int size) {
    for (int i = 0; i < size; i++) {
        fwrite(buf + i, sizeof(char), 1, stdout);
    }
    return size;
}

int write_buffer_int(int *buf, size_t size) {
    return fwrite(buf, sizeof(int), size, stdout);
}

void handle_enc() {
    while(!feof(stdin)) {
        int nbytes = read_buffer_char(buf, blocksize);
        if (!nbytes) continue;
        for (int i = nbytes; i < blocksize; i++) {
            buf[i] = -1; // completa o que falta com -1
        }
        decode_matrix(buf, calcMat, blocksize);
        // matrix multiplication, transformation to buffer and writing to stdout
        mat_mul(calcMat, key, tempMat, blocksize);
        encode_matrix(buf, tempMat, blocksize);
        write_buffer_int(buf, blocksize);
    }
}

void handle_dec() {
    mat_invert(key, tempMat);
    for (int i = 0; i < blocksize; i++) {
        /* printf("%f <-> %f\n", key[i], tempMat[i]); */
    }
    memcpy(key, tempMat, sizeof(float)*blocksize); // inversão da chave
    while(!feof(stdin)) {
        int nbytes = read_buffer_int(buf, blocksize);
        if (!nbytes) continue;
        if (nbytes != blocksize) {
            die("sucked block with %i bytes but expected a block with exacly %i bytes", nbytes, blocksize);
        }
        int writeSize = blocksize;
        decode_matrix(buf, calcMat, blocksize);
        mat_mul(calcMat, key, tempMat, keysize);
        encode_matrix(buf, tempMat, blocksize);
        for (int i = blocksize - 1; i >= 0; i--) {
            if (buf[i] == -1) {
                writeSize--;
                warn("filling the rest\n");
            }
        }
        write_buffer_char(buf, nbytes);
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
    // inversa
    /* keysize = 3; */
    float mtin[] = {1, 2, 3, 0, 1, 4, 5, 6, 0};
    float mout[] = {-24, 18, 5, 20, -15, -4, -5, 4, 1};
    float *mtemp = malloc(sizeof(float)*3*3);
    mat_cofactor(mtin, mtemp);
    for (int i = 0; i < 3*3; i++) {
        printf("certo = %f, nosso = %f\n", mout[i], mtemp[i]);
    }
    free(mtemp);
}

int main(int argc, char *argv[]) {
    /* healthcheck(); */
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

    key = alloc_shaped_matrix();
    calcMat = alloc_shaped_matrix();
    tempMat = alloc_shaped_matrix();
    resMat = alloc_shaped_matrix();
    buf = malloc(sizeof(int) * argc);

    for (int i = 0; i < argc; i++) {
        key[i] = strtof(argv[i], NULL);
        /* printf("%f\n", key[i]); */
    }
    if (!strcmp(command, "enc")) {
        handle_enc();
    } else if (!strcmp(command, "dec")) {
        handle_dec();
    }
    /* free(key); */
    /* free(buf); */
    /* free(calcMat); */
    /* free(tempMat); */
    /* free(resMat); */
    fflush(stdout);
    return 0;
}

// NOTA para o meu eu do futuro: cuida com as estruturas recursivas aqui, pode dar BO
float mat_determinant(float *a, int k) {
    float s = 1, det = 0;
    float *b = alloc_shaped_matrix();
    int i, j, m, n, c;
    if (k == 1) {
        return a[0];
    } else {
        det = 0;
        for (c = 0; c < k; c++) {
            m = 0;
            n = 0;
            for (i = 0;i < k; i++) {
                for (j = 0 ;j < k; j++) {
                    b[(i*keysize) + j] = 0;
                    if (i != 0 && j != c) {
                        b[(m*keysize) + n] = a[(i*keysize) + j];
                        if (n < (k - 2)) {
                            n++;
                        }
                        else {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            det = det + s * (a[(keysize*0) + c] * mat_determinant(b, k - 1));
            s = -1 * s;
        }
    }
    /* free(b); */
    return det;
}


// function for cofactor calculation
void mat_cofactor(float *num, float *ret) {
    float *b = alloc_shaped_matrix();
    float *fac = alloc_shaped_matrix();
    int p, q, m, n, i, j;
    for (q = 0;q < keysize; q++) {
        for (p = 0;p < keysize; p++) {
            m = 0;
            n = 0;
            for (i = 0;i < keysize; i++) {
                for (j = 0;j < keysize; j++) {
                    if (i != q && j != p) {
                        b[(m*keysize) + n] = num[(i*keysize) + j];
                        if (n < (keysize - 2)) {
                            n++;
                        } else {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            fac[(q*keysize) + p] = pow(-1, q + p) * mat_determinant(b, keysize - 1);
        }
    }
    mat_transpose(num, fac, keysize , ret);
    /* free(b); */
    /* free(fac); */
}


///function to find the transpose of a matrix
void mat_transpose(float *num, float *fac, int r, float *ret) {
    int i, j;
    float *b = alloc_shaped_matrix();
    /* float *inverse = alloc_shaped_matrix(); */
    float d;
    for (i = 0;i < r; i++) {
        for (j = 0;j < r; j++) {
            b[(i*blocksize) + j] = fac[(j*blocksize) + i];
        }
    }
    d = mat_determinant(num, r);
    for (i = 0;i < r; i++) {
        for (j = 0;j < r; j++) {
            /* printf("chegou no preenchimento\n"); */
            ret[(i*blocksize) + j] = b[(i*blocksize) + j] / d;
        }
    }
}

