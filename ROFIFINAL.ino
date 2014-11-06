#include <Servo.h> 
#include <SPI.h>
#include <Adb.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define FALSE 0
#define TRUE  1
// Commands to Numeric Value Mapping
//               Data[0] =   CMD TYPE | Qual
//                        bit  7654321   0      
#define MOVE         0x01   // 0000000   1        
#define CAMERA_MOVE  0x02   // 0000001   0                  
#define CAMERA_HOME  0x04   // 0000010   0
#define LED_1        0x40   // 0000000   1 
#define LED_2        0x41   // 0000000   1 
#define LED_3        0x42   // 0000000   1 
#define LEDcount  3


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const int numberOfServos             = 12;    // the number of servos
const int numberOfJoints             = 12;
const int numberOfFramesForward      = 126;
const int playbackDelayForward       = 10;    // Delay between the forward walk frames to achieve a stable speed
const int numberOfFramesRight        = 35;    // Delay between the right turn frames to achieve a stable speed
const int playbackDelayRight         = 25;
const int ultrasonicConstant         = 58;    // Calculation for the distance detected by the ultrasonic sensor
                                              // Speed of sound is about 340 m/s -> 29 microseconds/cm, divide
                                              // the time taken for the ultrasonic sensor to detect by 2 to get
                                              // the time taken by the pulse to reach the object, then divide by
                                              // the ultrasonic constant to get the distance in centimeters.
const int pingPin                    = 4;     // Ultrasonic sensor pin

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Action Frames
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Joints positions are in degrees * 100 (home position is 0 degrees)
/////////////////////////////////////////////
//JOINT INDEXES
//
//RIGHT LEG JOINTS
// 0 Right Ankle (roll)
// 1 Right Lower Leg 
// 2 Right Knee 
// 3 Right Middle Leg 
// 4 Right Upper Leg 
// 5 Right Hip (roll)
//LEFT LEG JOINTS
// 6 Left Ankle (roll)
// 7 Left Lower Leg 
// 8 Left Knee 
// 9 Left Middle Leg 
// 10 Left Upper Leg 
// 11 Left Hip (roll)
/////////////////////////////////////////////
int servoPins[numberOfServos] = {22, 24, 26, 28, 30, 32, 38, 40, 42, 44, 46, 48};  // the pin for each servo 

int framesForward[numberOfFramesForward][numberOfJoints] = {
                      {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 },
                      {  -29,  -521,     0,     0,  -675,   304,  -239,   261,   174,     0,     0,  -500 },
                      {  -58, -1042,     0,     0, -1350,   609,  -479,   522,   348,     0,     0, -1001 },
                      {  -88, -1563,     0,     0, -2025,   914,  -718,   783,   522,     0,     0, -1502 },
                      { -117, -2084,     0,     0, -2700,  1219,  -958,  1045,   696,     0,     0, -2003 },
                      {   24, -2372,     0,     0, -3100,  1461, -1141,  1152,   800,     0,   -61, -2300 },
                      {  378, -2326,     0,     0, -3100,  1598, -1232,  1045,   800,     0,  -198, -2300 },
                      {  731, -2280,     0,     0, -3100,  1735, -1323,   939,   800,     0,  -335, -2300 },
                      { 1085, -2234,     0,     0, -3100,  1872, -1414,   832,   800,     0,  -472, -2300 },
                      { 1439, -2187,     0,     0, -3100,  2009, -1506,   726,   800,     0,  -609, -2300 },
                      { 1793, -2141,     0,     0, -3100,  2146, -1597,   619,   800,     0,  -746, -2300 },
                      { 2146, -2095,     0,     0, -3100,  2282, -1688,   513,   800,     0,  -882, -2300 },
                      { 2347, -1908,  -150,     0, -2962,  2400, -1900,   474,   800,     0,  -787, -2300 },
                      { 2530, -1694,  -326,     0, -2800,  2517, -2135,   445,   800,     0,  -654, -2300 },
                      { 2714, -1481,  -503,     0, -2638,  2635, -2371,   416,   800,     0,  -522, -2300 },
                      { 2898, -1267,  -680,     0, -2476,  2753, -2607,   386,   800,     0,  -389, -2300 },
                      { 3081, -1054,  -857,     0, -2314,  2871, -2842,   357,   800,     0,  -257, -2300 },
                      { 3265,  -841, -1033,     0, -2152,  2989, -3078,   327,   800,     0,  -124, -2300 },
                      { 3426,  -664, -1203,    25, -1974,  3101, -3300,   307,   829,    30,    14, -2301 },
                      { 3192, -1147, -1283,   537, -1462,  3135, -3300,   455,  1420,   632,   298, -2335 },
                      { 2958, -1630, -1363,  1049,  -950,  3169, -3300,   603,  2012,  1235,   582, -2369 },
                      { 2725, -2113, -1442,  1560,  -439,  3204, -3300,   750,  2603,  1838,   867, -2404 },
                      { 2491, -2596, -1522,  2072,    72,  3238, -3300,   898,  3194,  2440,  1151, -2438 },
                      { 2257, -3079, -1601,  2584,   584,  3272, -3300,  1046,  3786,  3043,  1435, -2472 },
                      { 2023, -3562, -1681,  3095,  1095,  3306, -3300,  1194,  4377,  3646,  1719, -2506 },
                      { 1789, -4045, -1761,  3607,  1607,  3340, -3300,  1342,  4968,  4248,  2004, -2540 },
                      { 1555, -4528, -1840,  4119,  2119,  3374, -3300,  1490,  5560,  4851,  2288, -2574 },
                      { 1400, -4778, -1875,  4420,  2480,  3397, -3295,  1587,  5875,  5190,  2432, -2595 },
                      { 1474, -4344, -1775,  4100,  2400,  3387, -3275,  1537,  5375,  4750,  2162, -2575 },
                      { 1548, -3909, -1675,  3779,  2320,  3377, -3254,  1487,  4875,  4310,  1892, -2555 },
                      { 1623, -3474, -1575,  3460,  2240,  3367, -3235,  1437,  4375,  3870,  1622, -2535 },
                      { 1697, -3039, -1475,  3140,  2160,  3357, -3215,  1387,  3875,  3429,  1352, -2515 },
                      { 1771, -2604, -1375,  2820,  2080,  3347, -3195,  1337,  3375,  2990,  1082, -2495 },
                      { 1845, -2169, -1275,  2500,  2000,  3337, -3175,  1287,  2875,  2550,   812, -2475 },
                      { 1919, -1735, -1175,  2180,  1920,  3327, -3155,  1237,  2375,  2110,   542, -2455 },
                      { 1993, -1300, -1075,  1860,  1839,  3317, -3135,  1187,  1875,  1670,   272, -2435 },
                      { 2067,  -865,  -975,  1539,  1760,  3307, -3115,  1137,  1375,  1229,     2, -2415 },
                      { 2071,  -517,  -935,  1267,  1697,  3217, -3050,  1040,   980,   877,  -270, -2370 },
                      { 1865,  -429, -1075,  1137,  1687,  2887, -2850,   800,   900,   787,  -550, -2250 },
                      { 1658,  -342, -1215,  1007,  1677,  2557, -2650,   560,   819,   697,  -830, -2130 },
                      { 1451,  -254, -1355,   877,  1667,  2227, -2450,   319,   740,   607, -1110, -2010 },
                      { 1244,  -166, -1495,   747,  1657,  1897, -2250,    80,   660,   517, -1390, -1889 },
                      { 1037,   -79, -1635,   617,  1647,  1567, -2050,  -160,   580,   427, -1670, -1770 },
                      {  830,     8, -1775,   487,  1637,  1237, -1850,  -400,   500,   337, -1950, -1650 },
                      {  623,    96, -1914,   357,  1627,   907, -1650,  -639,   420,   247, -2230, -1530 },
                      {  417,   183, -2054,   227,  1617,   577, -1450,  -879,   340,   157, -2510, -1410 },
                      {  210,   271, -2195,    97,  1607,   247, -1250, -1120,   259,    67, -2790, -1289 },
                      {   52,   316, -2307,    20,  1655,     5, -1102, -1290,   229,     0, -2972, -1182 },
                      {   43,   231, -2337,   100,  1875,    25, -1112, -1250,   350,     0, -2862, -1112 },
                      {   35,   147, -2367,   180,  2095,    45, -1122, -1210,   470,     0, -2752, -1042 },
                      {   26,    63, -2397,   260,  2315,    65, -1132, -1170,   590,     0, -2642,  -972 },
                      {   17,   -21, -2427,   340,  2535,    85, -1142, -1130,   710,     0, -2532,  -902 },
                      {    8,  -105, -2457,   420,  2755,   105, -1152, -1090,   830,     0, -2422,  -832 },
                      {    0,  -189, -2487,   500,  2975,   125, -1162, -1050,   950,     0, -2312,  -762 },
                      {   -9,  -273, -2517,   580,  3195,   145, -1172, -1010,  1070,     0, -2202,  -692 },
                      {  -18,  -358, -2547,   660,  3415,   165, -1182,  -969,  1189,     0, -2092,  -622 },
                      {  -27,  -442, -2577,   740,  3635,   185, -1192,  -930,  1310,     0, -1982,  -552 },
                      {  -86,  -475, -2497,   775,  3685,   124, -1142,  -944,  1357,     0, -1936,  -409 },
                      { -299,  -353, -2088,   679,  3228,  -175,  -914, -1124,  1189,     0, -2080,   -48 },
                      { -511,  -231, -1679,   583,  2771,  -476,  -685, -1304,  1021,     0, -2224,   311 },
                      { -724,  -110, -1270,   487,  2314,  -777,  -457, -1483,   852,     0, -2369,   672 },
                      { -936,    11,  -862,   391,  1857, -1077,  -228, -1663,   684,     0, -2513,  1033 },
                      {-1149,   132,  -453,   294,  1400, -1378,     0, -1842,   515,     0, -2657,  1394 },
                      {-1361,   254,   -44,   198,   943, -1679,   228, -2022,   347,     0, -2802,  1755 },
                      {-1573,   376,   364,   102,   486, -1979,   456, -2202,   179,     0, -2946,  2115 },
                      {-1786,   497,   773,     6,    29, -2280,   685, -2381,    10,     0, -3090,  2476 },
                      {-1793,   432,   800,     0,     0, -2416,   903, -2336,     0,     0, -3100,  2470 },
                      {-1787,   354,   800,     0,     0, -2539,  1118, -2275,     0,     0, -3100,  2440 },
                      {-1780,   277,   800,     0,     0, -2661,  1332, -2215,     0,     0, -3100,  2409 },
                      {-1774,   199,   800,     0,     0, -2784,  1547, -2155,     0,     0, -3100,  2378 },
                      {-1767,   122,   800,     0,     0, -2906,  1762, -2094,     0,     0, -3100,  2348 },
                      {-1761,    44,   800,     0,     0, -3029,  1976, -2034,     0,     0, -3100,  2317 },
                      {-1866,     0,   800,     0,     0, -3046,  2188, -1909,     0,     0, -3100,  2353 },
                      {-2108,     0,   800,     0,     0, -2929,  2385, -1709,     0,     0, -3100,  2470 },
                      {-2350,     0,   800,     0,     0, -2811,  2582, -1509,     0,     0, -3100,  2588 },
                      {-2592,     0,   800,     0,     0, -2693,  2779, -1309,     0,     0, -3100,  2706 },
                      {-2833,     0,   800,     0,     0, -2575,  2977, -1108,     0,     0, -3100,  2824 },
                      {-3075,     0,   800,     0,     0, -2457,  3174,  -908,     0,     0, -3100,  2942 },
                      {-3317,     0,   800,     0,     0, -2340,  3371,  -708,     0,     0, -3100,  3059 },
                      {-3380,    31,  1066,   362,   110, -2189,  3280,  -851,  -125,   133, -2981,  2989 },
                      {-3349,    79,  1475,   919,   281, -2018,  3037, -1176,  -318,   337, -2799,  2818 },
                      {-3319,   127,  1884,  1476,   452, -1847,  2793, -1502,  -512,   542, -2617,  2647 },
                      {-3289,   175,  2294,  2033,   622, -1677,  2550, -1827,  -705,   747, -2435,  2477 },
                      {-3258,   223,  2703,  2591,   793, -1506,  2307, -2152,  -898,   951, -2253,  2306 },
                      {-3228,   271,  3112,  3148,   963, -1336,  2064, -2477, -1092,  1156, -2072,  2136 },
                      {-3197,   319,  3522,  3705,  1134, -1165,  1821, -2802, -1285,  1361, -1890,  1965 },
                      {-3167,   367,  3931,  4262,  1304,  -995,  1578, -3128, -1478,  1565, -1708,  1795 },
                      {-3136,   415,  4341,  4819,  1475,  -824,  1334, -3453, -1672,  1770, -1526,  1624 },
                      {-3135,   500,  4221,  4679,  1585,  -902,  1393, -2956, -1113,  2157,  -989,  1719 },
                      {-3138,   591,  4011,  4419,  1685, -1021,  1503, -2316,  -423,  2577,  -390,  1859 },
                      {-3141,   683,  3801,  4159,  1785, -1142,  1613, -1676,   266,  2997,   209,  1998 },
                      {-3144,   774,  3591,  3899,  1885, -1262,  1723, -1036,   956,  3417,   810,  2139 },
                      {-3147,   866,  3381,  3639,  1985, -1382,  1833,  -396,  1646,  3837,  1409,  2279 },
                      {-3150,   957,  3171,  3379,  2085, -1502,  1943,   243,  2336,  4257,  2009,  2419 },
                      {-3154,  1048,  2961,  3119,  2185, -1622,  2053,   884,  3026,  4677,  2610,  2559 },
                      {-3157,  1140,  2751,  2859,  2285, -1742,  2163,  1524,  3716,  5097,  3210,  2699 },
                      {-3160,  1231,  2541,  2599,  2385, -1862,  2273,  2164,  4406,  5517,  3810,  2839 },
                      {-3163,  1323,  2331,  2339,  2485, -1982,  2383,  2804,  5096,  5937,  4410,  2979 },
                      {-3099,  1161,  2104,  2104,  1972, -2119,  2281,  2704,  4655,  5481,  4126,  2915 },
                      {-3022,   955,  1874,  1874,  1352, -2259,  2140,  2474,  4015,  4871,  3685,  2815 },
                      {-2946,   748,  1644,  1644,   732, -2399,  2000,  2244,  3375,  4261,  3245,  2715 },
                      {-2869,   542,  1414,  1414,   112, -2539,  1860,  2014,  2735,  3651,  2805,  2614 },
                      {-2793,   336,  1184,  1184,  -507, -2679,  1720,  1784,  2095,  3041,  2365,  2514 },
                      {-2717,   129,   954,   954, -1126, -2818,  1581,  1554,  1456,  2431,  1926,  2415 },
                      {-2640,   -76,   724,   724, -1746, -2958,  1441,  1324,   816,  1821,  1486,  2315 },
                      {-2564,  -283,   494,   494, -2366, -3098,  1301,  1094,   176,  1211,  1046,  2215 },
                      {-2487,  -489,   264,   264, -2986, -3239,  1161,   864,  -463,   601,   606,  2115 },
                      {-2411,  -696,    34,    34, -3607, -3379,  1021,   634, -1104,    -8,   166,  2014 },
                      {-2298,  -741,   119,     0, -3547, -3272,   915,   557, -1319,   -23,   414,  1847 },
                      {-2178,  -758,   259,     0, -3367, -3122,   815,   507, -1459,    66,   784,  1667 },
                      {-2058,  -776,   399,     0, -3187, -2972,   715,   457, -1599,   156,  1154,  1487 },
                      {-1938,  -793,   539,     0, -3007, -2822,   615,   407, -1739,   246,  1524,  1307 },
                      {-1818,  -810,   679,     0, -2827, -2672,   515,   357, -1879,   336,  1894,  1127 },
                      {-1698,  -828,   819,     0, -2647, -2522,   415,   307, -2018,   426,  2264,   947 },
                      {-1578,  -845,   959,     0, -2467, -2372,   314,   257, -2159,   516,  2634,   766 },
                      {-1458,  -862,  1099,     0, -2286, -2222,   214,   207, -2299,   606,  3004,   586 },
                      {-1338,  -880,  1239,     0, -2107, -2072,   115,   157, -2439,   696,  3374,   407 },
                      {-1218,  -897,  1379,     0, -1927, -1922,    15,   107, -2579,   786,  3744,   226 },
                      {-1077,  -807,  1256,     0, -1705, -1705,     0,    89, -2334,   718,  3411,   179 },
                      { -932,  -699,  1088,     0, -1477, -1477,     0,    77, -2021,   621,  2954,   155 },
                      { -788,  -591,   920,     0, -1248, -1248,     0,    65, -1708,   525,  2497,   131 },
                      { -644,  -483,   751,     0, -1020, -1020,     0,    53, -1396,   429,  2040,   107 },
                      { -500,  -375,   583,     0,  -791,  -791,     0,    41, -1083,   333,  1583,    83 },
                      { -355,  -266,   414,     0,  -563,  -563,     0,    29,  -770,   237,  1126,    59 },
                      { -211,  -158,   246,     0,  -334,  -334,     0,    17,  -457,   140,   669,    35 },
                      {  -67,   -50,    78,     0,  -106,  -106,     0,     5,  -145,    44,   212,    11 }
};

int framesRight[numberOfFramesRight][numberOfJoints] = {
                      {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 },
                      { -323,  -148,  -320,  -180,   400,  -340,   340,     0,     0,     0,   200,   240 },
                      { -646,  -296,  -640,  -360,   800,  -680,   680,     0,     0,     0,   400,   480 },
                      { -970,  -444,  -960,  -540,  1200, -1019,  1019,     0,     0,     0,   600,   719 },
                      {-1293,  -593, -1280,  -720,  1600, -1360,  1360,     0,     0,     0,   800,   960 },
                      {-1616,  -741, -1600,  -900,  2000, -1700,  1700,     0,     0,     0,  1000,  1200 },
                      {-2001,  -741, -1660,  -560,  1839, -1760,  1920,   -20,     0,   460,  1120,  1480 },
                      {-2385,  -741, -1720,  -219,  1680, -1820,  2140,   -40,     0,   920,  1240,  1760 },
                      {-2769,  -741, -1780,   119,  1520, -1880,  2360,   -60,     0,  1380,  1360,  2039 },
                      {-3153,  -741, -1839,   460,  1360, -1939,  2580,   -80,     0,  1840,  1480,  2320 },
                      {-3538,  -741, -1900,   800,  1200, -2000,  2800,  -100,     0,  2300,  1600,  2600 },
                      {-3538,  -741, -1900,   800,  1200, -1800,  2800,  -100,     0,  2300,  1600,  2600 },
                      {-3538,  -741, -1900,   800,  1200, -1600,  2800,  -100,     0,  2300,  1600,  2600 },
                      {-3538,  -741, -1900,   800,  1200, -1400,  2800,  -100,     0,  2300,  1600,  2600 },
                      {-3538,  -741, -1900,   800,  1200, -1200,  2800,  -100,     0,  2300,  1600,  2600 },
                      {-3538,  -741, -1900,   800,  1200, -1000,  2800,  -100,     0,  2300,  1600,  2600 },
                      {-3403,  -573, -1880,   480,  1140, -1320,  2520,   580,   220,  1760,  1900,  2420 },
                      {-3268,  -404, -1860,   159,  1080, -1639,  2240,  1260,   440,  1220,  2200,  2240 },
                      {-3133,  -235, -1839,  -159,  1019, -1960,  1960,  1939,   660,   680,  2500,  2060 },
                      {-2998,   -67, -1820,  -480,   960, -2280,  1679,  2620,   880,   139,  2800,  1880 },
                      {-2864,   101, -1800,  -800,   900, -2600,  1400,  3300,  1100,  -400,  3100,  1700 },
                      {-2708,  -215, -1800,  -640,   800, -2600,  1340,  3379,  1100,  -400,  3020,  1700 },
                      {-2553,  -532, -1800,  -480,   700, -2600,  1280,  3460,  1100,  -400,  2940,  1700 },
                      {-2398,  -849, -1800,  -320,   600, -2600,  1220,  3540,  1100,  -400,  2860,  1700 },
                      {-2243, -1166, -1800,  -159,   500, -2600,  1160,  3620,  1100,  -400,  2780,  1700 },
                      {-2088, -1483, -1800,     0,   400, -2600,  1100,  3700,  1100,  -400,  2700,  1700 },
                      {-1671, -1186, -1440,     0,   320, -2080,   880,  2960,   880,  -320,  2160,  1360 },
                      {-1253,  -889, -1080,     0,   240, -1560,   660,  2220,   660,  -240,  1620,  1019 },
                      { -835,  -593,  -720,     0,   160, -1040,   440,  1480,   440,  -160,  1080,   680 },
                      { -417,  -296,  -359,     0,    79,  -519,   219,   739,   219,   -79,   539,   339 },
                      {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 },
                      {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 },
                      {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 },
                      {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 },
                      {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 }
};

int servoCalibrations[numberOfJoints][3] = {
                      {691, 29, -404}, 
                      {-407, -507, -973}, 
                      {100, -700, -1200}, 
                      {600, 100, -300}, 
                      {300, -200, -800}, 
                      {300, -100, -266}, 
                      {500, 0, -335}, 
                      {0, -300, -500}, 
                      {600, -200, -600}, 
                      {400, 400, 100}, 
                      {500, -100, -270}, 
                      {100, -300, -541}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Servo servos[numberOfServos];  // create servo object to control a servo 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
// Distance detected by the ultrasonic sensor
unsigned long cm = DetectDistance(pingPin);
delay(50);
// The robot will walk forward, but will turn right if there is an obstacle 30 cm or less in front of it
if(cm >= 30)
PlayFrames(numberOfFramesForward, playbackDelayForward);
else
{
for (int i = 0; i < 4; i++)
          PlayFrames(numberOfFramesRight, playbackDelayRight);                  
  
}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  //wait for a second to begin (keeps the communication line open in case a new program is being downloaded)
  delay(1000);    
  
  //start up the communication
  Serial.begin(9600);  
  
  //initialize the servos
  initializeServos();
  
  // Apply the servo calibrations to the joint positions.
  ApplyCalibration();

  delay(3000);
  
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long DetectDistance(int pingPin)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  long duration = pulseIn(pingPin, HIGH);

  // convert the time into a distance
  long cm = duration / ultrasonicConstant;
  return cm;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlayFrames(int numberOfFrames, int playbackDelay)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  // Angle value of the servo
  int value;
  // This for loop determines the animation frame the robot is playing through
  for (int framesRowNumber = 0; framesRowNumber < numberOfFrames; framesRowNumber++)
  {
    // This for loop adjusts the position of each servo
    for (int servo = 0; servo < numberOfServos; servo++)
    {   
        // each servo position is sent as a 2 byte value (high byte, low byte) integer (from -32,768 to 32,767)
        // this number is encoding the angle of the servo. The number is 100 * the servo angle.  This allows for the
        // storage of 2 significant digits(i.e. the value can be from -60.00 to 60.00 and every value in between).
        // Also remember that the servos have a range of 120 degrees. The angle is written in positions
        // which range from a minimum of 800 (-60 degrees) and go to a maximum of 2200 (60 degrees)
        
        // This branch determines whether or not the robot is walking forward or turning left based on
        // the parameters passed in the main loop    
        if (numberOfFrames == numberOfFramesForward)    value = framesForward[framesRowNumber][servo];
        else if (numberOfFrames == numberOfFramesRight) value = framesRight[framesRowNumber][servo];
        
        // flip for the left leg.
        if(servo >= numberOfServos/2) value = map(value, -6000,6000,6000,-6000);
        
        // tell servo to go to position in variable 'pos'
        servos[servo].write(map(value, -6000,6000,800,2200));   
        // This delay controls the delay between each servo being updated       
        delay(2);
    }
  // This delay controls the delay between each frame
  // This will vary based on the animation and may need to be changed if you make your own
  // animation with a different speed.
  delay(playbackDelay);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplyCalibration()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{   
  // Apply the servo calibrations to each frame of the animation.
  // This is done before hand to keep from slowing down the playback.
  // The frames could also be stored with the calibrations already
  // applied, however leaving the calibration seperate allows other
  // ROFIs to use this same action with their own calibration.
  
  for(int f = 0; f < numberOfFramesForward; f++)
    for(int s = 0; s < numberOfServos; s++)    
      framesForward[f][s] = CorrectJointAngle(framesForward[f][s], s);
      
  for(int f = 0; f < numberOfFramesRight; f++)
    for(int s = 0; s < numberOfServos; s++)    
      framesRight[f][s] = CorrectJointAngle(framesRight[f][s], s);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CorrectJointAngle(double inputAngle, int servo)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  // The input angle is what the angle should be.
  // The corrected angle is the angle that has to be sent to the servo to achieve the input angle.
  if (inputAngle > 0)
    // Do a two point calibration between the middle and high corrected values.
    return map(inputAngle, 0, 4500, 0 + servoCalibrations[servo][1], 4500 + servoCalibrations[servo][2]);
  else
    // Do a two point calibration between the low and middle corrected values.
    return map(inputAngle, -4500, 0, -4500 + servoCalibrations[servo][0], 0 + servoCalibrations[servo][1]);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initializeServos()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{   
  // Assign the correct pin to each servo.
  for(int s = 0; s < numberOfServos; s++)
    servos[s].attach(servoPins[s]);  
}
