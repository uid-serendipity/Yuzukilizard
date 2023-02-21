const unsigned char cifar10_pic[32*32*3]={\
 67, 41, 27,  61, 38, 22,  62, 46, 23,  86, 77, 44,  86, 86, 50,  91, 96, 65, 103,110, 89, 134,140,123, 148,149,133, 135,127,114, 146,128,117, 192,168,148, 159,135, 99, 169,144,102, 175,145,118, 181,144,136, 208,176,165, 184,150,137, 165,136,121, 134,119,100, 128,128,110, 219,223,212, 204,196,196, 180,163,167, 193,179,180, 144,142,132, 129,132,117,  73, 64, 55,  91, 66, 70, 109, 79, 90, 114,102,108,  95,100, 99, 
 54, 40, 18,  60, 46, 24,  67, 53, 30,  86, 78, 48,  97, 94, 63,  87, 88, 62,  95, 97, 77, 117,119,100, 112,112, 94, 114,107, 92, 109, 97, 85, 149,134,118, 157,144,118, 150,139,111, 149,133,121, 113, 93, 98, 123,104,101, 111, 88, 86, 137,115,110, 129,119,109, 158,159,149, 242,246,240, 235,229,230, 195,180,184, 172,160,160, 145,144,134, 128,132,120, 131,127,122, 121,101,106,  94, 70, 82, 131,120,128, 123,127,128, 
 57, 53, 25,  67, 60, 35,  63, 56, 31,  70, 63, 36,  79, 75, 47,  76, 74, 50,  80, 79, 58,  80, 79, 59,  98, 94, 75, 127,122,107, 109,102, 93, 133,130,122, 143,147,136, 125,132,125, 123,129,136, 117,121,140, 138,137,141, 105, 95,101, 118,104,108,  86, 78, 79, 106,107,105, 222,224,224, 204,199,201, 183,169,171, 126,114,110, 112,111, 97, 100,103, 88, 134,131,123, 127,113,117,  99, 82, 93, 115,105,111, 116,116,116, 
 51, 52, 26,  56, 55, 29,  55, 49, 26,  59, 53, 30,  57, 51, 28,  71, 67, 43,  75, 73, 49,  66, 61, 40,  97, 89, 72, 129,120,110,  93, 88, 87, 115,116,120, 108,118,125, 103,120,129, 109,128,141, 126,143,162, 154,162,162, 169,167,173, 145,136,146,  68, 59, 69,  73, 71, 77, 134,135,139, 196,193,195, 133,121,121, 112,100, 90, 132,125,106, 125,121,102, 144,140,129, 192,180,180, 170,155,159, 131,121,121,  80, 77, 69, 
 49, 48, 28,  47, 43, 24,  54, 47, 28,  55, 48, 29,  51, 45, 26,  64, 62, 38,  75, 74, 48,  83, 78, 57, 118,107, 93, 162,147,144, 124,113,116, 138,134,146, 106,110,128,  91,104,120, 108,123,132, 169,186,189, 206,212,207, 227,224,226, 201,192,202, 100, 91,104,  56, 54, 66,  91, 93,103, 147,150,155,  79, 75, 74, 111,102, 92, 177,167,149, 173,166,146, 188,180,167, 239,230,226, 241,231,231, 163,154,150,  90, 84, 73, 
 44, 39, 24,  46, 37, 23,  55, 44, 30,  45, 35, 25,  54, 48, 35,  62, 60, 42,  75, 74, 53,  99, 93, 74, 137,123,111, 166,147,142, 135,120,118, 151,140,148,  99, 92,107, 101, 97,109,  93, 91, 90, 159,159,145, 200,198,190, 214,209,208, 201,192,202, 112,105,120,  69, 69, 83,  85, 91,102, 148,157,166,  88, 93, 96,  80, 77, 72, 183,177,164, 178,170,157, 189,184,175, 246,242,241, 254,252,252, 169,166,162, 132,130,120, 
 50, 41, 27,  56, 45, 31,  63, 48, 39,  53, 38, 35,  60, 51, 48,  57, 55, 45,  65, 65, 47,  85, 79, 60, 108, 94, 76, 128,108, 90, 133,116, 97, 187,170,161, 120,103,106, 133,112,120, 121,102, 95, 132,111, 89, 220,208,202, 248,236,236, 184,174,180,  96, 87, 97,  62, 58, 64,  63, 66, 70, 149,156,159, 101,109,109,  72, 75, 73, 108,104, 99, 104, 98, 93, 140,136,135, 226,229,233, 250,254,255, 197,202,201, 123,123,117, 
 49, 39, 22,  51, 38, 22,  56, 41, 32,  63, 48, 46,  60, 49, 51,  59, 55, 50,  77, 77, 61, 104, 98, 75, 133,121, 93, 149,133, 97, 130,113, 74, 188,172,143, 127,104, 96,  92, 63, 66, 137,105,100, 160,127,108, 207,184,182, 218,198,197, 160,145,143, 116,100, 93, 135,119,106, 137,125,107, 147,140,123, 106,104, 93,  66, 64, 56,  52, 48, 43,  45, 38, 35,  72, 67, 68, 170,173,177, 247,252,255, 203,205,205, 163,160,155, 
 43, 34, 20,  49, 39, 22,  46, 32, 20,  52, 37, 34,  50, 38, 36,  62, 57, 48,  63, 62, 42,  84, 77, 50, 133,121, 87, 140,125, 79, 115,101, 49, 176,160,118, 168,143,127,  79, 47, 48, 111, 75, 75, 127, 92, 79, 122, 91, 94, 137,112,110, 140,118,107, 131,107, 85, 184,154,119, 193,161,120, 187,161,124, 114,100, 72,  60, 50, 32,  50, 42, 29,  41, 34, 25,  59, 55, 50, 147,145,144, 246,247,245, 189,185,180, 222,213,204, 
 34, 25, 15,  34, 25, 12,  33, 21,  9,  37, 25, 15,  30, 21, 11,  31, 27,  9,  47, 43, 18,  84, 79, 48, 145,133, 97, 155,140, 94, 140,123, 72, 171,151,110, 178,153,133, 134,103,100, 126, 92, 92, 121, 88, 79, 111, 78, 85, 106, 79, 82, 116, 94, 83, 139,112, 85, 201,166,123, 198,156,109, 203,168,125, 112, 89, 57,  45, 34, 12,  36, 29, 14,  34, 31, 17,  47, 45, 35, 110,107,102, 221,216,213, 183,174,170, 186,172,166, 
 30, 18, 12,  24, 14,  4,  27, 15,  3,  34, 22, 10,  32, 23,  9,  31, 24,  5,  51, 45, 22,  90, 83, 56, 111,100, 68, 144,128, 91, 153,134, 91, 173,147,110, 172,139,119, 167,131,123, 154,119,115, 136,104, 98, 112, 85, 95,  93, 70, 78,  75, 57, 56, 116, 95, 80, 187,157,128, 175,140,106, 154,124, 95, 103, 86, 67,  35, 29, 16,  17, 16,  6,  24, 25, 15,  39, 40, 30,  78, 78, 72, 147,143,142, 116,108,109, 109, 96, 98, 
 43, 30, 22,  29, 17,  7,  24, 12,  0,  27, 15,  3,  30, 18,  6,  38, 26, 14,  50, 42, 25,  66, 59, 39,  83, 74, 47, 151,138,106, 141,117, 81, 183,150,117, 196,155,132, 183,141,128, 158,119,111, 126, 92, 86, 101, 78, 83,  68, 50, 57,  52, 38, 44,  87, 73, 74, 142,123,115, 100, 78, 66,  79, 62, 53,  68, 61, 58,  33, 33, 33,  17, 23, 18,  38, 41, 32,  59, 60, 50,  59, 60, 51,  89, 87, 86,  75, 68, 71,  94, 84, 90, 
 43, 30, 16,  33, 19,  7,  29, 17,  7,  36, 20, 13,  38, 22, 15,  56, 40, 34,  60, 49, 41,  60, 53, 38,  80, 76, 52, 124,114, 84, 117, 93, 63, 178,138,109, 217,170,143, 225,178,157, 147,104, 95,  91, 53, 51,  85, 61, 55,  66, 45, 47,  46, 32, 38,  85, 72, 80, 118,107,110,  58, 48, 48,  36, 30, 31,  36, 35, 39,  44, 45, 49,  32, 33, 31,  49, 49, 37,  66, 66, 50,  52, 50, 39,  95, 92, 84, 115,108,105, 159,149,149, 
 40, 25,  9,  34, 21,  7,  35, 23, 13,  42, 27, 18,  40, 23, 14,  53, 36, 27,  56, 44, 32,  55, 51, 32,  70, 71, 45,  86, 79, 52,  76, 52, 28, 161,119, 96, 222,170,147, 236,189,167, 152,114,102,  64, 32, 33,  94, 74, 57,  63, 44, 37,  42, 22, 27,  59, 44, 52,  55, 43, 49,  32, 26, 27,  31, 26, 27,  35, 32, 34,  38, 33, 35,  26, 17, 14,  34, 25, 15,  47, 39, 22,  49, 42, 25,  65, 58, 43,  85, 77, 64, 137,128,115, 
 36, 22, 10,  34, 22, 12,  37, 25, 15,  43, 30, 14,  51, 36, 17,  55, 41, 19,  55, 47, 24,  61, 63, 34,  64, 70, 39,  66, 62, 38,  60, 37, 22, 151,106, 95, 239,189,171, 226,183,162, 116, 86, 75,  59, 38, 40,  79, 64, 48,  61, 42, 37,  50, 30, 35,  33, 14, 23,  26, 12, 16,  13,  7,  2,  11,  7,  2,  28, 23, 20,  37, 25, 25,  30, 15, 13,  28, 15,  7,  35, 26, 13,  44, 39, 24,  28, 24,  6,  56, 50, 31, 119,112, 92, 
 32, 16,  9,  37, 23, 17,  43, 33, 23,  57, 46, 26,  81, 69, 41,  74, 61, 29,  59, 52, 19,  58, 62, 26,  43, 53, 17,  38, 37, 16,  63, 39, 33, 167,121,120, 255,205,193, 196,159,139,  75, 53, 41,  54, 44, 44,  68, 62, 49,  55, 42, 40,  41, 22, 31,  38, 18, 30,  18,  5,  7,  14,  8,  1,   9,  7,  0,  12,  3,  0,  28, 14, 15,  37, 19, 20,  30, 15, 13,  50, 40, 33,  40, 38, 27,  30, 30, 14,  52, 49, 28, 100, 96, 71, 
 29, 14,  5,  39, 22, 13,  42, 27, 18,  55, 47, 30,  77, 71, 48,  65, 54, 26,  62, 49, 23,  54, 46, 29,  33, 32, 22,  26, 20, 13,  78, 59, 46, 184,148,130, 244,205,183, 209,177,164, 102, 78, 80,  82, 62, 75, 101, 85, 86,  70, 49, 51,  51, 29, 31,  35, 18, 21,  18, 11, 14,   4,  3,  5,   3,  0,  4,  10,  3,  8,  12,  3,  6,  21, 16, 15,  16, 13,  8,  32, 27, 18,  42, 33, 23,  32, 24, 11,  35, 36, 20,  72, 82, 65, 
 39, 20, 12,  34, 15,  7,  39, 21, 14,  39, 30, 17,  41, 34, 15,  48, 37, 15,  60, 48, 30,  42, 33, 23,  26, 23, 19,  24, 18, 13,  96, 71, 61, 192,150,131, 219,173,149, 231,189,170, 172,136,130, 116, 82, 88,  94, 74, 69,  70, 47, 45,  49, 28, 27,  42, 26, 27,  20, 11, 14,   8,  1,  6,  12,  2,  8,  10,  0,  2,   8,  0,  1,  17, 12,  9,  21, 18, 13,  34, 29, 20,  34, 23, 15,  28, 15,  7,  29, 22, 13,  44, 43, 33, 
 36, 17,  9,  32, 13,  6,  35, 16, 11,  35, 24, 16,  34, 27, 12,  41, 31, 14,  49, 36, 22,  40, 30, 23,  33, 28, 27,  31, 19, 17, 108, 78, 67, 191,141,119, 184,128, 99, 197,141,116, 205,154,138, 163,113,107, 111, 81, 70,  66, 39, 29,  42, 22, 17,  52, 38, 39,  32, 21, 24,  14,  2,  8,  18,  2,  9,  19,  2,  5,  17,  5,  3,  10,  7,  0,  17, 16,  6,  28, 23, 14,  29, 15,  9,  26,  7,  4,  23,  8,  6,  30, 17, 15, 
 22,  6,  0,  31, 15,  9,  30, 16, 10,  23, 14,  5,  29, 22,  7,  45, 35, 18,  41, 28, 14,  34, 23, 15,  41, 32, 28,  44, 28, 22, 149,114,100, 206,152,127, 175,114, 82, 155, 95, 65, 191,134,113, 205,149,138, 167,115,102,  78, 40, 28,  50, 28, 22,  57, 47, 47,  28, 22, 27,  12,  1,  9,  20,  3,  7,  22,  4,  5,  23,  8,  5,  13,  6,  0,  20, 15,  6,  28, 18, 11,  31, 16, 13,  30,  9, 11,  26,  7, 10,  28, 11, 14, 
 21, 11,  4,  23, 12,  8,  28, 17, 13,  25, 18,  9,  24, 19,  4,  47, 37, 19,  47, 31, 14,  33, 20,  6,  36, 26, 16,  56, 38, 27, 188,152,134, 219,165,140, 179,119, 89, 142, 86, 57, 150, 97, 77, 198,143,134, 194,124,111, 131, 80, 70,  77, 52, 48,  39, 34, 35,  30, 28, 34,  18, 10, 17,  17,  2,  6,  22,  4,  5,  19,  4,  1,  19,  9,  2,  19, 10,  6,  21, 12,  9,  25, 13, 13,  27, 13, 15,  45, 31, 33,  40, 27, 29, 
 21, 15, 10,  15,  6,  3,  21, 12,  9,  27, 21, 14,  26, 23,  9,  43, 35, 18,  48, 32, 19,  39, 27, 17,  43, 33, 26,  71, 53, 46, 163,129,116, 198,149,127, 184,130,105, 143, 90, 69, 115, 66, 52, 165,113,107, 191,112,103, 129, 74, 67,  80, 55, 51,  53, 48, 50,  34, 34, 40,   9,  2,  9,  21,  7, 13,  20,  7,  9,  15,  5,  5,  18, 14, 13,  13, 10, 12,  20, 16, 21,  23, 19, 25,  17, 11, 16,  54, 46, 47,  55, 45, 45, 
 14,  5,  2,  18,  6,  6,  19,  6,  8,  17,  9,  9,  24, 19, 16,  41, 32, 28,  47, 35, 33,  47, 36, 38,  46, 39, 42,  66, 53, 55, 120, 91, 87, 174,129,116, 197,145,128, 163,113, 95, 129, 77, 65, 162,107,102, 168, 96, 86, 127, 75, 68,  83, 58, 56,  50, 41, 44,  35, 28, 35,  20,  9, 17,  22, 10, 16,   7,  0,  5,   9,  8, 12,  19, 22, 30,  20, 28, 41,  24, 35, 49,  43, 52, 65,  25, 30, 39,  36, 33, 35,  58, 46, 46, 
 16,  2,  4,  21,  4,  8,  27, 11, 18,  34, 26, 33,  42, 38, 44,  45, 38, 45,  48, 39, 49,  55, 46, 59,  48, 43, 58,  62, 50, 62, 138,111,115, 190,146,139, 222,170,157, 213,161,145, 178,123,110, 190,129,119, 184,127,112, 131, 89, 77,  86, 61, 57,  57, 44, 46,  45, 32, 40,  30, 14, 25,  14,  3, 13,   7,  4, 13,   0,  6, 19,  20, 30, 48,  29, 45, 68,  19, 39, 64,  60, 81,103,  48, 61, 75,  28, 25, 34,  69, 55, 57, 
 32, 18, 24,  40, 23, 32,  57, 41, 52,  54, 44, 56,  43, 37, 48,  45, 37, 48,  63, 53, 66,  62, 52, 68,  45, 38, 53,  34, 23, 33, 100, 73, 76, 170,128,121, 228,179,165, 230,178,161, 184,128,111, 139, 79, 63, 113, 69, 52, 147,112, 99, 125,103, 98,  73, 58, 62,  44, 30, 41,  19,  9, 22,  10,  6, 18,   9, 12, 27,   6, 12, 31,  18, 25, 52,  31, 42, 74,  23, 40, 73,  29, 50, 78,  67, 82,101,  47, 45, 57,  62, 48, 54, 
 68, 59, 69,  75, 61, 72,  67, 53, 65,  50, 40, 52,  46, 41, 50,  61, 52, 62,  75, 61, 72,  59, 48, 58,  32, 24, 31,  31, 17, 21,  49, 24, 22, 111, 75, 65, 205,162,147, 241,198,179, 205,157,139, 121, 66, 51,  72, 33, 18, 178,146,135, 161,138,136,  80, 63, 72,  30, 20, 36,   9,  7, 26,  11, 15, 34,  15, 20, 41,  25, 24, 50,  39, 33, 64,  44, 40, 76,  36, 40, 75,  44, 55, 85,  75, 89,108,  49, 50, 64,  60, 49, 57, 
 85, 77, 87,  84, 73, 83,  80, 66, 77,  84, 75, 85,  81, 74, 81,  75, 64, 72,  72, 58, 64,  57, 45, 51,  48, 39, 42,  49, 38, 40,  55, 34, 32,  60, 30, 25, 120, 87, 78, 218,186,175, 210,174,164, 129, 86, 77,  92, 56, 46, 192,160,155, 171,145,151,  88, 71, 85,  35, 28, 49,  26, 30, 54,  26, 34, 57,  33, 35, 59,  70, 59, 85, 110, 90,119, 108, 87,119, 112,102,132, 148,150,174,  78, 85,102,  39, 40, 50,  54, 48, 53, 
 84, 72, 78,  91, 75, 82,  95, 78, 87, 100, 87, 95,  70, 61, 71,  51, 41, 53,  62, 50, 62,  67, 57, 69,  67, 61, 72,  63, 58, 67,  64, 52, 58,  80, 60, 65,  95, 73, 75, 152,131,130, 193,165,165, 112, 78, 78,  73, 40, 37, 164,131,135, 122, 92,105,  63, 43, 62,  47, 36, 62,  44, 45, 71,  49, 50, 76,  63, 57, 80, 150,132,155, 192,163,188, 182,156,179, 194,180,198, 160,157,172,  50, 52, 60,  41, 40, 44,  58, 51, 54, 
 98, 80, 81,  79, 57, 59,  75, 53, 58,  88, 71, 80,  68, 58, 70,  60, 50, 67,  70, 59, 79,  65, 58, 79,  62, 61, 81,  67, 66, 86,  80, 72, 89,  96, 80, 97, 125,108,121, 136,117,126, 155,131,139, 128, 95,102, 103, 73, 78, 145,114,121, 130,100,113, 108, 82,100, 108, 92,115, 106, 97,118, 101, 92,113, 138,122,140, 192,168,186, 204,177,193, 184,164,177, 152,141,151,  73, 69, 75,  32, 28, 33,  54, 48, 49,  63, 52, 54, 
 71, 51, 46,  29,  6,  4,  33,  9, 11,  71, 53, 60,  77, 65, 77,  74, 63, 79,  69, 58, 78,  50, 43, 64,  32, 30, 50,  50, 49, 69,  70, 61, 81,  88, 74, 92,  87, 72, 86, 113, 96,107, 157,130,139, 151,117,124, 151,124,128, 174,143,150, 188,157,166, 185,158,168, 176,157,166, 154,138,149, 152,132,144, 151,125,138, 153,123,136, 156,130,143, 123,108,116,  60, 52, 59,  40, 33, 40,  34, 27, 32,  50, 41, 44,  54, 43, 46, 
 50, 34, 28,  28,  8,  3,  39, 18, 16,  51, 33, 34,  42, 28, 32,  39, 26, 34,  44, 28, 39,  32, 18, 30,  23, 15, 25,  46, 38, 48,  45, 34, 44,  65, 48, 59,  44, 27, 36,  70, 50, 55, 101, 74, 77, 112, 78, 79, 100, 68, 69,  97, 65, 66, 122, 91, 90,  98, 76, 71,  87, 71, 65,  66, 52, 46,  81, 60, 58,  93, 62, 65, 101, 68, 75,  97, 72, 82,  66, 52, 63,  42, 34, 45,  36, 28, 39,  28, 20, 30,  40, 32, 42,  46, 38, 48, 
 59, 49, 42,  43, 27, 21,  52, 33, 28,  49, 32, 29,  44, 31, 29,  54, 38, 39,  56, 38, 39,  42, 24, 23,  36, 23, 21,  50, 38, 36,  38, 23, 21,  42, 21, 23,  49, 28, 30,  59, 39, 38,  72, 44, 43,  96, 61, 58,  81, 45, 45,  73, 38, 34,  98, 69, 60,  62, 43, 28,  45, 35, 17,  54, 44, 27, 111, 89, 78,  97, 64, 61,  81, 45, 51,  56, 29, 39,  40, 23, 36,  49, 40, 53,  31, 21, 37,  26, 16, 32,  41, 36, 51,  42, 42, 56, 
};
