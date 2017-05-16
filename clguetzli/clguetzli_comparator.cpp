#include <stdint.h>
#include <algorithm>
#include "clguetzli_comparator.h"
#include "guetzli\idct.h"


typedef int16_t coeff_t;

const double* NewSrgb8ToLinearTable() {
	double* table = new double[256];
	int i = 0;
	for (; i < 11; ++i) {
		table[i] = i / 12.92;
	}
	for (; i < 256; ++i) {
		table[i] = 255.0 * std::pow(((i / 255.0) + 0.055) / 1.055, 2.4);
	}
	return table;
}

const double* Srgb8ToLinearTable() {
	static const double* const kSrgb8ToLinearTable = NewSrgb8ToLinearTable();
	return kSrgb8ToLinearTable;
}

static const int kCrToRedTable[256] = {
	-179, -178, -177, -175, -174, -172, -171, -170, -168, -167, -165, -164,
	-163, -161, -160, -158, -157, -156, -154, -153, -151, -150, -149, -147,
	-146, -144, -143, -142, -140, -139, -137, -136, -135, -133, -132, -130,
	-129, -128, -126, -125, -123, -122, -121, -119, -118, -116, -115, -114,
	-112, -111, -109, -108, -107, -105, -104, -102, -101, -100,  -98,  -97,
	-95,  -94,  -93,  -91,  -90,  -88,  -87,  -86,  -84,  -83,  -81,  -80,
	-79,  -77,  -76,  -74,  -73,  -72,  -70,  -69,  -67,  -66,  -64,  -63,
	-62,  -60,  -59,  -57,  -56,  -55,  -53,  -52,  -50,  -49,  -48,  -46,
	-45,  -43,  -42,  -41,  -39,  -38,  -36,  -35,  -34,  -32,  -31,  -29,
	-28,  -27,  -25,  -24,  -22,  -21,  -20,  -18,  -17,  -15,  -14,  -13,
	-11,  -10,   -8,   -7,   -6,   -4,   -3,   -1,    0,    1,    3,    4,
	6,    7,    8,   10,   11,   13,   14,   15,   17,   18,   20,   21,
	22,   24,   25,   27,   28,   29,   31,   32,   34,   35,   36,   38,
	39,   41,   42,   43,   45,   46,   48,   49,   50,   52,   53,   55,
	56,   57,   59,   60,   62,   63,   64,   66,   67,   69,   70,   72,
	73,   74,   76,   77,   79,   80,   81,   83,   84,   86,   87,   88,
	90,   91,   93,   94,   95,   97,   98,  100,  101,  102,  104,  105,
	107,  108,  109,  111,  112,  114,  115,  116,  118,  119,  121,  122,
	123,  125,  126,  128,  129,  130,  132,  133,  135,  136,  137,  139,
	140,  142,  143,  144,  146,  147,  149,  150,  151,  153,  154,  156,
	157,  158,  160,  161,  163,  164,  165,  167,  168,  170,  171,  172,
	174,  175,  177,  178
};

static const int kCbToBlueTable[256] = {
	-227, -225, -223, -222, -220, -218, -216, -214, -213, -211, -209, -207,
	-206, -204, -202, -200, -198, -197, -195, -193, -191, -190, -188, -186,
	-184, -183, -181, -179, -177, -175, -174, -172, -170, -168, -167, -165,
	-163, -161, -159, -158, -156, -154, -152, -151, -149, -147, -145, -144,
	-142, -140, -138, -136, -135, -133, -131, -129, -128, -126, -124, -122,
	-120, -119, -117, -115, -113, -112, -110, -108, -106, -105, -103, -101,
	-99,  -97,  -96,  -94,  -92,  -90,  -89,  -87,  -85,  -83,  -82,  -80,
	-78,  -76,  -74,  -73,  -71,  -69,  -67,  -66,  -64,  -62,  -60,  -58,
	-57,  -55,  -53,  -51,  -50,  -48,  -46,  -44,  -43,  -41,  -39,  -37,
	-35,  -34,  -32,  -30,  -28,  -27,  -25,  -23,  -21,  -19,  -18,  -16,
	-14,  -12,  -11,   -9,   -7,   -5,   -4,   -2,    0,    2,    4,    5,
	7,    9,   11,   12,   14,   16,   18,   19,   21,   23,   25,   27,
	28,   30,   32,   34,   35,   37,   39,   41,   43,   44,   46,   48,
	50,   51,   53,   55,   57,   58,   60,   62,   64,   66,   67,   69,
	71,   73,   74,   76,   78,   80,   82,   83,   85,   87,   89,   90,
	92,   94,   96,   97,   99,  101,  103,  105,  106,  108,  110,  112,
	113,  115,  117,  119,  120,  122,  124,  126,  128,  129,  131,  133,
	135,  136,  138,  140,  142,  144,  145,  147,  149,  151,  152,  154,
	156,  158,  159,  161,  163,  165,  167,  168,  170,  172,  174,  175,
	177,  179,  181,  183,  184,  186,  188,  190,  191,  193,  195,  197,
	198,  200,  202,  204,  206,  207,  209,  211,  213,  214,  216,  218,
	220,  222,  223,  225,
};

static const int kCrToGreenTable[256] = {
	5990656,  5943854,  5897052,  5850250,  5803448,  5756646,  5709844,  5663042,
	5616240,  5569438,  5522636,  5475834,  5429032,  5382230,  5335428,  5288626,
	5241824,  5195022,  5148220,  5101418,  5054616,  5007814,  4961012,  4914210,
	4867408,  4820606,  4773804,  4727002,  4680200,  4633398,  4586596,  4539794,
	4492992,  4446190,  4399388,  4352586,  4305784,  4258982,  4212180,  4165378,
	4118576,  4071774,  4024972,  3978170,  3931368,  3884566,  3837764,  3790962,
	3744160,  3697358,  3650556,  3603754,  3556952,  3510150,  3463348,  3416546,
	3369744,  3322942,  3276140,  3229338,  3182536,  3135734,  3088932,  3042130,
	2995328,  2948526,  2901724,  2854922,  2808120,  2761318,  2714516,  2667714,
	2620912,  2574110,  2527308,  2480506,  2433704,  2386902,  2340100,  2293298,
	2246496,  2199694,  2152892,  2106090,  2059288,  2012486,  1965684,  1918882,
	1872080,  1825278,  1778476,  1731674,  1684872,  1638070,  1591268,  1544466,
	1497664,  1450862,  1404060,  1357258,  1310456,  1263654,  1216852,  1170050,
	1123248,  1076446,  1029644,   982842,   936040,   889238,   842436,   795634,
	748832,   702030,   655228,   608426,   561624,   514822,   468020,   421218,
	374416,   327614,   280812,   234010,   187208,   140406,    93604,    46802,
	0,   -46802,   -93604,  -140406,  -187208,  -234010,  -280812,  -327614,
	-374416,  -421218,  -468020,  -514822,  -561624,  -608426,  -655228,  -702030,
	-748832,  -795634,  -842436,  -889238,  -936040,  -982842, -1029644, -1076446,
	-1123248, -1170050, -1216852, -1263654, -1310456, -1357258, -1404060, -1450862,
	-1497664, -1544466, -1591268, -1638070, -1684872, -1731674, -1778476, -1825278,
	-1872080, -1918882, -1965684, -2012486, -2059288, -2106090, -2152892, -2199694,
	-2246496, -2293298, -2340100, -2386902, -2433704, -2480506, -2527308, -2574110,
	-2620912, -2667714, -2714516, -2761318, -2808120, -2854922, -2901724, -2948526,
	-2995328, -3042130, -3088932, -3135734, -3182536, -3229338, -3276140, -3322942,
	-3369744, -3416546, -3463348, -3510150, -3556952, -3603754, -3650556, -3697358,
	-3744160, -3790962, -3837764, -3884566, -3931368, -3978170, -4024972, -4071774,
	-4118576, -4165378, -4212180, -4258982, -4305784, -4352586, -4399388, -4446190,
	-4492992, -4539794, -4586596, -4633398, -4680200, -4727002, -4773804, -4820606,
	-4867408, -4914210, -4961012, -5007814, -5054616, -5101418, -5148220, -5195022,
	-5241824, -5288626, -5335428, -5382230, -5429032, -5475834, -5522636, -5569438,
	-5616240, -5663042, -5709844, -5756646, -5803448, -5850250, -5897052, -5943854,
};

static const int kCbToGreenTable[256] = {
	2919680,  2897126,  2874572,  2852018,  2829464,  2806910,  2784356,  2761802,
	2739248,  2716694,  2694140,  2671586,  2649032,  2626478,  2603924,  2581370,
	2558816,  2536262,  2513708,  2491154,  2468600,  2446046,  2423492,  2400938,
	2378384,  2355830,  2333276,  2310722,  2288168,  2265614,  2243060,  2220506,
	2197952,  2175398,  2152844,  2130290,  2107736,  2085182,  2062628,  2040074,
	2017520,  1994966,  1972412,  1949858,  1927304,  1904750,  1882196,  1859642,
	1837088,  1814534,  1791980,  1769426,  1746872,  1724318,  1701764,  1679210,
	1656656,  1634102,  1611548,  1588994,  1566440,  1543886,  1521332,  1498778,
	1476224,  1453670,  1431116,  1408562,  1386008,  1363454,  1340900,  1318346,
	1295792,  1273238,  1250684,  1228130,  1205576,  1183022,  1160468,  1137914,
	1115360,  1092806,  1070252,  1047698,  1025144,  1002590,   980036,   957482,
	934928,   912374,   889820,   867266,   844712,   822158,   799604,   777050,
	754496,   731942,   709388,   686834,   664280,   641726,   619172,   596618,
	574064,   551510,   528956,   506402,   483848,   461294,   438740,   416186,
	393632,   371078,   348524,   325970,   303416,   280862,   258308,   235754,
	213200,   190646,   168092,   145538,   122984,   100430,    77876,    55322,
	32768,    10214,   -12340,   -34894,   -57448,   -80002,  -102556,  -125110,
	-147664,  -170218,  -192772,  -215326,  -237880,  -260434,  -282988,  -305542,
	-328096,  -350650,  -373204,  -395758,  -418312,  -440866,  -463420,  -485974,
	-508528,  -531082,  -553636,  -576190,  -598744,  -621298,  -643852,  -666406,
	-688960,  -711514,  -734068,  -756622,  -779176,  -801730,  -824284,  -846838,
	-869392,  -891946,  -914500,  -937054,  -959608,  -982162, -1004716, -1027270,
	-1049824, -1072378, -1094932, -1117486, -1140040, -1162594, -1185148, -1207702,
	-1230256, -1252810, -1275364, -1297918, -1320472, -1343026, -1365580, -1388134,
	-1410688, -1433242, -1455796, -1478350, -1500904, -1523458, -1546012, -1568566,
	-1591120, -1613674, -1636228, -1658782, -1681336, -1703890, -1726444, -1748998,
	-1771552, -1794106, -1816660, -1839214, -1861768, -1884322, -1906876, -1929430,
	-1951984, -1974538, -1997092, -2019646, -2042200, -2064754, -2087308, -2109862,
	-2132416, -2154970, -2177524, -2200078, -2222632, -2245186, -2267740, -2290294,
	-2312848, -2335402, -2357956, -2380510, -2403064, -2425618, -2448172, -2470726,
	-2493280, -2515834, -2538388, -2560942, -2583496, -2606050, -2628604, -2651158,
	-2673712, -2696266, -2718820, -2741374, -2763928, -2786482, -2809036, -2831590,
};

static const uint8_t kRangeLimitLut[4 * 256] = {
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
	16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
	32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
	48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
	64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
	80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
	96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
};

static const uint8_t* kRangeLimit = kRangeLimitLut + 384;

void CoeffToIDCT(coeff_t *block, uint8_t *idct)
{
	guetzli::ComputeBlockIDCT(block, idct);
}

void IDCTToImage(const uint8_t idct[8 * 8], uint16_t *pixels_)
{
	const int block_x = 0;
	const int block_y = 0;
	const int width_ = 8;
	const int height_ = 8;

	for (int iy = 0; iy < 8; ++iy) {
		for (int ix = 0; ix < 8; ++ix) {
			int x = 8 * block_x + ix;
			int y = 8 * block_y + iy;
			if (x >= width_ || y >= height_) continue;
			int p = y * width_ + x;
			pixels_[p] = idct[8 * iy + ix] << 4;
		}
	}
}

// out = [YUVYUV....YUVYUV]
void ImageToYUV(uint16_t *pixels_, uint8_t *out)
{
	const int stride = 3;

	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
            int px = y * 8 + x;
			*out = static_cast<uint8_t>((pixels_[px] + 8 - (x & 1)) >> 4);
            out += stride;
		}
	}
}

// pixel = [YUVYUV...YUVYUV] to [RGBRGB...RGBRGB]
void YUVToRGB(uint8_t* pixelBlock)
{
	for (int i = 0; i < 64; i++)
	{
		uint8_t *pixel = &pixelBlock[i*3];

		int y = pixel[0];
		int cb = pixel[1];
		int cr = pixel[2];
		pixel[0] = kRangeLimit[y + kCrToRedTable[cr]];
		pixel[1] = kRangeLimit[y + ((kCrToGreenTable[cr] + kCbToGreenTable[cb]) >> 16)];
		pixel[2] = kRangeLimit[y + kCbToBlueTable[cb]];
	}
}

// block = [R....R][G....G][B.....]
void BlockToImage(coeff_t *block, float* r, float* g, float* b)
{
	uint8_t idct[8 * 8 * 3];
	CoeffToIDCT(&block[0], &idct[0]);
	CoeffToIDCT(&block[8 * 8], &idct[8 * 8]);
	CoeffToIDCT(&block[8 * 8 * 2], &idct[8 * 8 * 2]);

	uint16_t pixels[8 * 8 * 3];

	IDCTToImage(&idct[0], &pixels[0]);
	IDCTToImage(&idct[8*8], &pixels[8*8]);
	IDCTToImage(&idct[8*8*2], &pixels[8*8*2]);

	uint8_t yuv[8 * 8 * 3];

	ImageToYUV(&pixels[0], &yuv[0]);
	ImageToYUV(&pixels[8*8], &yuv[1]);
	ImageToYUV(&pixels[8*8*2], &yuv[2]);

    YUVToRGB(yuv);

	const double* lut = Srgb8ToLinearTable();
	for (int i = 0; i < 8 * 8; i++)
	{
		r[i] = lut[yuv[3 * i]];
		g[i] = lut[yuv[3 * i + 1]];
		b[i] = lut[yuv[3 * i + 2]];
	}
}

namespace guetzli
{
	ButteraugliComparatorEx::ButteraugliComparatorEx(const int width, const int height,
		const std::vector<uint8_t>* rgb,
		const float target_distance, ProcessStats* stats)
		: ButteraugliComparator(width, height, rgb, target_distance, stats)
	{

	}

	void ButteraugliComparatorEx::StartBlockComparisons()
	{
		ButteraugliComparator::StartBlockComparisons();

        const int width = width_;
        const int height = height_;
        const int factor_x = 1;
        const int factor_y = 1;

        const int block_width = (width + 8 * factor_x - 1) / (8 * factor_x);
        const int block_height = (height + 8 * factor_y - 1) / (8 * factor_y);
        const int num_blocks = block_width * block_height;

        const double* lut = Srgb8ToLinearTable();

        imgOpsinDynamicsBlockList.resize(num_blocks * 3 * kDCTBlockSize);
        imgMaskXyzScaleBlockList.resize(num_blocks * 3);
        for (int block_y = 0, block_ix = 0; block_y < block_height; ++block_y)
        {
            for (int block_x = 0; block_x < block_width; ++block_x, ++block_ix)
            {
                float* curR = &imgOpsinDynamicsBlockList[block_ix * 3 * kDCTBlockSize];
                float* curG = curR + kDCTBlockSize;
                float* curB = curG + kDCTBlockSize;

                for (int iy = 0, i = 0; iy < 8; ++iy) {
                    for (int ix = 0; ix < 8; ++ix, ++i) {
                        int x = std::min(8 * block_x + ix, width - 1);
                        int y = std::min(8 * block_y + iy, height - 1);
                        int px = y * width + x;

                        curR[i] = lut[rgb_orig_[3 * px]];
                        curG[i] = lut[rgb_orig_[3 * px + 1]];
                        curB[i] = lut[rgb_orig_[3 * px + 2]];
                    }
                }

                int xmin = block_x * 8;
                int ymin = block_y * 8;

                imgMaskXyzScaleBlockList[block_ix * 3] = mask_xyz_[0][ymin * width_ + xmin];
                imgMaskXyzScaleBlockList[block_ix * 3 + 1] = mask_xyz_[1][ymin * width_ + xmin];
                imgMaskXyzScaleBlockList[block_ix * 3 + 2] = mask_xyz_[2][ymin * width_ + xmin];
            }
        }


	}

    void ButteraugliComparatorEx::FinishBlockComparisons() {
        ButteraugliComparator::FinishBlockComparisons();

        imgOpsinDynamicsBlockList.clear();
        imgMaskXyzScaleBlockList.clear();
    }

	void ButteraugliComparatorEx::SwitchBlock(int block_x, int block_y, int factor_x, int factor_y)
	{
        block_x_ = block_x;
        block_y_ = block_y;
        factor_x_ = factor_x;
        factor_y_ = factor_y;

		ButteraugliComparator::SwitchBlock(block_x, block_y, factor_x, factor_y);
	}

	double ButteraugliComparatorEx::CompareBlockEx(coeff_t* candidate_block)
	{
        int block_ix = getCurrentBlockIdx();

        float*  block_opsin = &imgOpsinDynamicsBlockList[block_ix * 3 * kDCTBlockSize];

        // 这个内存拷贝待优化，但不是现在
        std::vector< std::vector<float> > rgb0_c;
        rgb0_c.resize(3);
        for (int i = 0; i < 3; i++)
        {
            rgb0_c[i].resize(kDCTBlockSize);
            memcpy(rgb0_c[i].data(), block_opsin + i*kDCTBlockSize, kDCTBlockSize * sizeof(float));
        }

        //
		std::vector<std::vector<float> > rgb1_c(3, std::vector<float>(kDCTBlockSize));
		BlockToImage(candidate_block, rgb1_c[0].data(), rgb1_c[1].data(), rgb1_c[2].data());

        ::butteraugli::OpsinDynamicsImage(8, 8, rgb0_c);
		::butteraugli::OpsinDynamicsImage(8, 8, rgb1_c);

		std::vector<std::vector<float> > rgb0 = rgb0_c;
		std::vector<std::vector<float> > rgb1 = rgb1_c;

		::butteraugli::MaskHighIntensityChange(8, 8, rgb0_c, rgb1_c, rgb0, rgb1);

		double b0[3 * kDCTBlockSize];
		double b1[3 * kDCTBlockSize];
		for (int c = 0; c < 3; ++c) {
			for (int ix = 0; ix < kDCTBlockSize; ++ix) {
				b0[c * kDCTBlockSize + ix] = rgb0[c][ix];
				b1[c * kDCTBlockSize + ix] = rgb1[c][ix];
			}
		}
		double diff_xyz_dc[3] = { 0.0 };
		double diff_xyz_ac[3] = { 0.0 };
		double diff_xyz_edge_dc[3] = { 0.0 };
		::butteraugli::ButteraugliBlockDiff(b0, b1, diff_xyz_dc, diff_xyz_ac, diff_xyz_edge_dc);

		double diff = 0.0;
		double diff_edge = 0.0;
		for (int c = 0; c < 3; ++c) {
            diff      += diff_xyz_dc[c]      * imgMaskXyzScaleBlockList[block_ix * 3 + c];
            diff      += diff_xyz_ac[c]      * imgMaskXyzScaleBlockList[block_ix * 3 + c];
            diff_edge += diff_xyz_edge_dc[c] * imgMaskXyzScaleBlockList[block_ix * 3 + c];
		}
        const double kEdgeWeight = 0.05;
		return sqrt((1 - kEdgeWeight) * diff + kEdgeWeight * diff_edge);
	}


    int ButteraugliComparatorEx::getCurrentBlockIdx(void)
    {
        const int width = width_;
        const int height = height_;
        const int factor_x = 1;
        const int factor_y = 1;

        const int block_width = (width + 8 * factor_x - 1) / (8 * factor_x);
        const int block_height = (height + 8 * factor_y - 1) / (8 * factor_y);

        return block_y_ * block_width + block_x_;
    }
}
