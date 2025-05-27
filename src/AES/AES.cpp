#include "AES.h"

static byte s_fwd [256] PROGMEM =
{
99, 124, 119, 123, 242, 107, 111, 197, 48, 1, 103, 43, 254, 215, 171, 118, 
202, 130, 201, 125, 250, 89, 71, 240, 173, 212, 162, 175, 156, 164, 114, 192, 
183, 253, 147, 38, 54, 63, 247, 204, 52, 165, 229, 241, 113, 216, 49, 21, 
4, 199, 35, 195, 24, 150, 5, 154, 7, 18, 128, 226, 235, 39, 178, 117, 
9, 131, 44, 26, 27, 110, 90, 160, 82, 59, 214, 179, 41, 227, 47, 132, 
83, 209, 0, 237, 32, 252, 177, 91, 106, 203, 190, 57, 74, 76, 88, 207, 
208, 239, 170, 251, 67, 77, 51, 133, 69, 249, 2, 127, 80, 60, 159, 168, 
81, 163, 64, 143, 146, 157, 56, 245, 188, 182, 218, 33, 16, 255, 243, 210, 
205, 12, 19, 236, 95, 151, 68, 23, 196, 167, 126, 61, 100, 93, 25, 115, 
96, 129, 79, 220, 34, 42, 144, 136, 70, 238, 184, 20, 222, 94, 11, 219, 
224, 50, 58, 10, 73, 6, 36, 92, 194, 211, 172, 98, 145, 149, 228, 121, 
231, 200, 55, 109, 141, 213, 78, 169, 108, 86, 244, 234, 101, 122, 174, 8, 
186, 120, 37, 46, 28, 166, 180, 198, 232, 221, 116, 31, 75, 189, 139, 138, 
112, 62, 181, 102, 72, 3, 246, 14, 97, 53, 87, 185, 134, 193, 29, 158, 
225, 248, 152, 17, 105, 217, 142, 148, 155, 30, 135, 233, 206, 85, 40, 223, 
140, 161, 137, 13, 191, 230, 66, 104, 65, 153, 45, 15, 176, 84, 187, 22
};

static byte s_inv [256] PROGMEM =
{
82, 9, 106, 213, 48, 54, 165, 56, 191, 64, 163, 158, 129, 243, 215, 251,
124, 227, 57, 130, 155, 47, 255, 135, 52, 142, 67, 68, 196, 222, 233, 203,
84, 123, 148, 50, 166, 194, 35, 61, 238, 76, 149, 11, 66, 250, 195, 78,
8, 46, 161, 102, 40, 217, 36, 178, 118, 91, 162, 73, 109, 139, 209, 37,
114, 248, 246, 100, 134, 104, 152, 22, 212, 164, 92, 204, 93, 101, 182, 146,
108, 112, 72, 80, 253, 237, 185, 218, 94, 21, 70, 87, 167, 141, 157, 132,
144, 216, 171, 0, 140, 188, 211, 10, 247, 228, 88, 5, 184, 179, 69, 6,
208, 44, 30, 143, 202, 63, 15, 2, 193, 175, 189, 3, 1, 19, 138, 107,
58, 145, 17, 65, 79, 103, 220, 234, 151, 242, 207, 206, 240, 180, 230, 115,
150, 172, 116, 34, 231, 173, 53, 133, 226, 249, 55, 232, 28, 117, 223, 110,
71, 241, 26, 113, 29, 41, 197, 137, 111, 183, 98, 14, 170, 24, 190, 27,
252, 86, 62, 75, 198, 210, 121, 32, 154, 219, 192, 254, 120, 205, 90, 244,
31, 221, 168, 51, 136, 7, 199, 49, 177, 18, 16, 89, 39, 128, 236, 95,
96, 81, 127, 169, 25, 181, 74, 13, 45, 229, 122, 159, 147, 201, 156, 239,
160, 224, 59, 77, 174, 42, 245, 176, 200, 235, 187, 60, 131, 83, 153, 97,
23, 43, 4, 126, 186, 119, 214, 38, 225, 105, 20, 99, 85, 33, 12, 125
};

static byte u_key [16] =
{
  113, 51, 98, 117, 94, 116, 49, 110, 113, 102, 90, 40, 112, 102, 36, 49
};

static byte d_key [16] =
{
  72, 64, 67, 70, 107, 82, 110, 122, 64, 75, 65, 116, 66, 74, 112, 50
};

#define f2(x) ((x) & 0x80 ? (x << 1) ^ 0x011B : x << 1)
#define d2(x) (((x) >> 1) ^ ((x) & 1 ? 0x008D : 0))
#define f9(x) (f2(f2(f2(x))) ^ x)
#define fb(x) (f2(f2(f2(x))) ^ f2(x) ^ x)
#define fd(x) (f2(f2(f2(x))) ^ f2(f2(x)) ^ x)
#define fe(x) (f2(f2(f2(x))) ^ f2(f2(x)) ^ f2(x))

static byte s_box(byte x)
{
  return pgm_read_byte(&s_fwd[x]);
}

static byte inv_s_box(byte x)
{
  return pgm_read_byte(&s_inv[x]);
}

void copy_n_bytes(byte *d, byte *s, byte nn)
{
  while (nn >= 4)
  {
    *d++ = *s++;
    *d++ = *s++;
    *d++ = *s++;
    *d++ = *s++;
    nn -= 4;
  }
  while (nn--)
    *d++ = *s++;
}

static void copy_and_key(byte *d, byte *s, byte *k)
{
  for (byte i = 0; i < 16; i += 4)
  {
    *d++ = *s++ ^ *k++;
    *d++ = *s++ ^ *k++;
    *d++ = *s++ ^ *k++;
    *d++ = *s++ ^ *k++;
  }
}

static void shift_sub_rows(byte st[16])
{
  st[0] = s_box(st[0]);
  st[4] = s_box(st[4]);
  st[8] = s_box(st[8]);
  st[12] = s_box(st[12]);

  byte tt = st[1];
  st[1] = s_box(st[5]);
  st[5] = s_box(st[9]);
  st[9] = s_box(st[13]);
  st[13] = s_box(tt);

  tt = st[2];
  st[2] = s_box(st[10]);
  st[10] = s_box(tt);
  tt = st[6];
  st[6] = s_box(st[14]);
  st[14] = s_box(tt);

  tt = st[15];
  st[15] = s_box(st[11]);
  st[11] = s_box(st[7]);
  st[7] = s_box(st[3]);
  st[3] = s_box(tt);
}

static void inv_shift_sub_rows(byte st[16])
{
  st[0] = inv_s_box(st[0]);
  st[4] = inv_s_box(st[4]);
  st[8] = inv_s_box(st[8]);
  st[12] = inv_s_box(st[12]);

  byte tt = st[13];
  st[13] = inv_s_box(st[9]);
  st[9] = inv_s_box(st[5]);
  st[5] = inv_s_box(st[1]);
  st[1] = inv_s_box(tt);

  tt = st[2];
  st[2] = inv_s_box(st[10]);
  st[10] = inv_s_box(tt);
  tt = st[6];
  st[6] = inv_s_box(st[14]);
  st[14] = inv_s_box(tt);

  tt = st[3];
  st[3] = inv_s_box(st[7]);
  st[7] = inv_s_box(st[11]);
  st[11] = inv_s_box(st[15]);
  st[15] = inv_s_box(tt);
}

static void mix_sub_columns(byte dt[16], byte st[16])
{
  byte j = 5;
  byte k = 10;
  byte l = 15;
  for (byte i = 0; i < 16; i += 4)
  {
    byte a = st[i];
    byte b = st[j];
    j = (j + 4) & 15;
    byte c = st[k];
    k = (k + 4) & 15;
    byte d = st[l];
    l = (l + 4) & 15;
    byte a1 = s_box(a), b1 = s_box(b), c1 = s_box(c), d1 = s_box(d);
    byte a2 = f2(a1), b2 = f2(b1), c2 = f2(c1), d2 = f2(d1);
    dt[i] = a2 ^ b2 ^ b1 ^ c1 ^ d1;
    dt[i + 1] = a1 ^ b2 ^ c2 ^ c1 ^ d1;
    dt[i + 2] = a1 ^ b1 ^ c2 ^ d2 ^ d1;
    dt[i + 3] = a2 ^ a1 ^ b1 ^ c1 ^ d2;
  }
}

static void inv_mix_sub_columns(byte dt[16], byte st[16])
{
  byte j = 5;
  byte k = 10;
  byte l = 15;
  for (byte i = 0; i < 16; i += 4)
  {
    byte a = st[i];
    byte b = st[j];
    j = (j + 4) & 15;
    byte c = st[k];
    k = (k + 4) & 15;
    byte d = st[l];
    l = (l + 4) & 15;
    byte a1 = inv_s_box(a), b1 = inv_s_box(b), c1 = inv_s_box(c), d1 = inv_s_box(d);
    dt[i] = fe(a1) ^ fb(b1) ^ fd(c1) ^ f9(d1);
    dt[i + 1] = f9(a1) ^ fe(b1) ^ fb(c1) ^ fd(d1);
    dt[i + 2] = fd(a1) ^ f9(b1) ^ fe(c1) ^ fb(d1);
    dt[i + 3] = fb(a1) ^ fd(b1) ^ f9(c1) ^ fe(d1);
  }
}

byte AES::set_key(int keytype)
{
  byte hi;
  hi = 11 << 4;
  if (keytype == 1){
    copy_n_bytes(key_sched, d_key, 16);
  }else{
    copy_n_bytes(key_sched, u_key, 16);
  }
  byte t[4];
  byte next = 16;
  for (byte cc = 16, rc = 1; cc < hi; cc += 4)
  {
    for (byte i = 0; i < 4; i++)
      t[i] = key_sched[cc - 4 + i];
    if (cc == next)
    {
      next += 16;
      byte ttt = t[0];
      t[0] = s_box(t[1]) ^ rc;
      t[1] = s_box(t[2]);
      t[2] = s_box(t[3]);
      t[3] = s_box(ttt);
      rc = f2(rc);
    }
    byte tt = cc - 16;
    for (byte i = 0; i < 4; i++)
      key_sched[cc + i] = key_sched[tt + i] ^ t[i];
  }
  return 0;
}

int AES::encrypt(int keytype, byte plain[16], byte cipher[16])
{
  set_key(keytype);
  byte s1[16], r;
  copy_and_key(s1, plain, (byte *)(key_sched));

  for (r = 1; r < 10; r++)
  {
    byte s2[16];
    mix_sub_columns(s2, s1);
    copy_and_key(s1, s2, (byte *)(key_sched + r * 16));
  }
  shift_sub_rows(s1);
  copy_and_key(cipher, s1, (byte *)(key_sched + r * 16));
  return 0;
}

int AES::decrypt(int keytype, byte cipher[16], byte plain[16])
{
  set_key(1); // Use decryption key
  byte s1[16], r;
  copy_and_key(s1, cipher, (byte *)(key_sched + 10 * 16)); // Start with last round key

  inv_shift_sub_rows(s1);
  for (r = 9; r > 0; r--)
  {
    byte s2[16];
    copy_and_key(s2, s1, (byte *)(key_sched + r * 16));
    inv_mix_sub_columns(s1, s2);
  }
  copy_and_key(plain, s1, (byte *)(key_sched));
  return 0;
}
