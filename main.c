#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<assert.h>
#include<string.h>
#include<math.h>

#define die(...) fprintf(stderr, "erro: "); fprintf(stderr, __VA_ARGS__); exit(1)
#define warn(...) fprintf(stderr, "aviso: "); fprintf(stderr, __VA_ARGS__)

float *key = NULL;
int *buf = NULL;
float *calcMat = NULL;
float *tempMat = NULL;
float *resMat = NULL;
int keysize;
int blocksize;

int inverse(float *A, float *inverse);
void adjoint(float *A, float *adj);
float determinant(float *A, int n);
void getCofactor(float *A, float *temp, int p, int q, int n);

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
        mat[i] = (float)buf[i];
    }
}

void encode_matrix(int *buf, float *mat, int size) {
    for (int i = 0; i < size; i++) {
        buf[i] = (int)roundf(mat[i]);
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
    inverse(key, tempMat);
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
    keysize = 3;
    float mtin[] = {1, 2, 3, 0, 1, 4, 5, 6, 0};
    float mout[] = {-24, 18, 5, 20, -15, -4, -5, 4, 1};
    float *mtemp = malloc(sizeof(float)*3*3);
    inverse(mtin, mtemp);
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
    } else {
        die("comando não encontrado");
    }
    /* free(key); */
    /* free(buf); */
    /* free(calcMat); */
    /* free(tempMat); */
    /* free(resMat); */
    fflush(stdout);
    return 0;
}

// Function to get cofactor of A[p][q] in temp[][]. n is current
// dimension of A[][]
void getCofactor(float *A, float *temp, int p, int q, int n)
{
	int i = 0, j = 0;

	// Looping for each element of the matrix
	for (int row = 0; row < n; row++)
	{
		for (int col = 0; col < n; col++)
		{
			// Copying into temporary matrix only those element
			// which are not in given row and column
			if (row != p && col != q)
			{
                temp[i*keysize + j++] = A[row*keysize + col];

				// Row is filled, so increase row index and
				// reset col index
				if (j == n - 1)
				{
					j = 0;
					i++;
				}
			}
		}
	}
}

/* Recursive function for finding determinant of matrix.
n is current dimension of A[][]. */
float determinant(float *A, int n)
{
	float D = 0; // Initialize result

	// Base case : if matrix contains single element
	if (n == 1)
		return A[0];

    float *temp = alloc_shaped_matrix(); // To store cofactors

	int sign = 1; // To store sign multiplier

	// Iterate for each element of first row
	for (int f = 0; f < n; f++)
	{
		// Getting Cofactor of A[0][f]
		getCofactor(A, temp, 0, f, n);
		D += sign * A[f] * determinant(temp, n - 1);

		// terms are to be added with alternate sign
		sign = -sign;
	}
    free(temp);
	return D;
}

// Function to get adjoint of A[N][N] in adj[N][N].
void adjoint(float *A, float *adj)
{
	if (keysize == 1)
	{
		adj[0] = 1;
		return;
	}

	// temp is used to store cofactors of A[][]
	int sign = 1;
    float *temp = alloc_shaped_matrix();

	for (int i=0; i<keysize; i++)
	{
		for (int j=0; j<keysize; j++)
		{
			// Get cofactor of A[i][j]
			getCofactor(A, temp, i, j, keysize);

			// sign of adj[j][i] positive if sum of row
			// and column indexes is even.
			sign = ((i+j)%2==0)? 1: -1;

			// Interchanging rows and columns to get the
			// transpose of the cofactor matrix
			adj[j*keysize + i] = (sign)*(determinant(temp, keysize -1));
		}
	}
}

// Function to calculate and store inverse, returns false if
// matrix is singular
int inverse(float *A, float *inverse)
{
	// Find determinant of A[][]
	int det = determinant(A, keysize);
	if (det == 0)
	{
        die("matriz singular, não é possível calcular a chave de descriptografia");
		return 0;
	}

	// Find adjoint
    float *adj = alloc_shaped_matrix();
	adjoint(A, adj);

	// Find Inverse using formula "inverse(A) = adj(A)/det(A)"
	for (int i=0; i<keysize; i++)
		for (int j=0; j<keysize; j++)
			inverse[i*keysize + j] = adj[i*keysize + j]/det;

	return 1;
}

