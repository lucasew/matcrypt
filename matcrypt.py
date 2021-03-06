#!/usr/bin/env python3

import numpy as np
import pickle
from sys import argv, stdout
from base64 import standard_b64encode, standard_b64decode

def is_perfect_square(n):
    sq = n ** (1/2)
    return abs(sq - float(int(sq))) < 0.0001

class Encoder:
  def __init__(self, key = None, ndim = 4):
    if key is not None:
        if not is_perfect_square(len(key)):
            raise Exception("tamanho da chave não é quadrado perfeito")
        self.ndim = int(len(key)**(1/2))
        self._key = np.reshape(np.asarray(key, dtype=int), (self.ndim, self.ndim))
    else:
        self._key = self.generate_key(ndim)
        self.ndim = ndim
  @property
  def key(self):
    return self._key
  @property
  def dekey(self):
    return np.linalg.inv(self._key)
  def generate_key(self, ndim):
    return np.random.randint(1, high = 256**2 - 1, size=(ndim, ndim))
  def encode_string(self, s: str):
    encoded = np.array(list(map(ord, s)))
    encoded_len = len(encoded)
    if encoded_len % self.ndim != 0:
      final_len = encoded_len - (encoded_len % self.ndim) + self.ndim
      base = np.zeros(final_len, dtype = int) - 1
      base[0:encoded_len] = encoded
      return np.reshape(base, (-1, self.ndim))
    return np.reshape(encoded, (-1, self.ndim))
  def decode_string(self, m):
    chars = np.reshape(m, (1, -1))[0]
    minusOnes = 0
    for i in range(1, self.ndim):
      if chars[-i] == -1:
        minusOnes = i
    if minusOnes != 0:
      chars = chars[0:-minusOnes]
    return "".join(list(map(chr, chars)))
  def encrypt_string(self, s: str):
    encoded = self.encode_string(s)
    return np.matmul(encoded, self._key, dtype = int)
  def decrypt_string(self, mat):
    decrypted = np.rint(np.matmul(mat, self.dekey)).astype(int)
    return self.decode_string(decrypted)

def usage():
    print("""matcrypt: criptografia matricial

matcrypt comando chave_1 chave_2 ... chave_n payload

* comando pode ser enc ou dec que significam criptografar e descriptografar respectivamente
* as chaves são elementos de uma matriz, a quantidade de chaves deve ser um quadrado perfeito
    * quadrados perfeitos são números que a raiz quadrada é um número inteiro. Ex: 1, 4, 9, 16, 25, ...
* payload é o texto a ser criptografado
""")

if len(argv) < (1 + 1 + 1 + 1): # base, command, smallest key, payload
    usage()
    raise Exception("parâmetros insuficientes")

cmd = argv[1]
keys = argv[2:-1]
payload = argv[-1]

encoder = Encoder(key = keys)

if cmd == "enc":
    encrypted = encoder.encrypt_string(payload).dumps()
    encoded = standard_b64encode(encrypted)
    print(str(encoded, encoding='utf-8'))
elif cmd == "dec":
    encoded_bytes = payload.encode('utf-8')
    decoded_bytes = standard_b64decode(encoded_bytes)
    arr = pickle.loads(decoded_bytes)
    decrypt = encoder.decrypt_string(arr)
    print(decrypt)
else:
    usage()
    raise Exception("comando não encontrado")

