/*   (C) Copyright 2004, 2005, 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 *   (C) Ziggurat method Copyright 2005 Jochen Voss.
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#include <sys/types.h>
#if ( __linux__ || (__APPLE__ && __MACH__) )
#include <unistd.h>
#else 
#include <windows.h>
#include <tlhelp32.h>
#include <process.h>
#endif
#include <stdio.h>
#include <time.h>

#include "rand.h"
#include "math.h"
#include "types.h"

#if (_WIN32 || _WIN64)
DWORD getppid()
{
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe32;
    DWORD ppid = 0, pid = GetCurrentProcessId();

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return -1;

    ZeroMemory(&pe32, sizeof(pe32));
    pe32.dwSize = sizeof(pe32);
    if (!Process32First(hSnapshot, &pe32)) return -1;

    do {
        if (pe32.th32ProcessID == pid) {
            ppid = pe32.th32ParentProcessID;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    return ppid;
}

int getpid()
{
    return GetCurrentProcessId();
}

void srandom(unsigned long long x)
{
    long long id = (x >> 32) ^ x;
    srand((unsigned int)id);
}

int random()
{
    return rand();
}
#endif

unsigned long mcxSeed(unsigned long long i)
{
#if (__linux__ ||  (__APPLE__ && __MACH__) )
    pid_t   p = getpid();
    pid_t   pp = getppid();
#else 
    unsigned long p = getpid(), pp = getppid();
#endif

    time_t  t = time(NULL);
    unsigned long  s = (p ^ p << 4 ^ p << 16 ^ p << 28)
                           ^  (pp ^ pp << 8 ^ pp << 24)
                           ^  (t ^ t << 12 ^ t << 20)
                           ^  (i ^ i << 3 ^ i << 23 ^ i << 26)

         /* I have no solid evidence backing up the usefulness of the xors.
          * They won't increase entropy anyway of course.
          * Anyway, the xors do seem useful in order to spread input
          * bits out over the output space, as seen from some hashing
          * experiments.
         */
   ;  return s
;  }


   /* Box-Muller transform */
double mcxNormalBoxMuller
(  void
)
   {  double a = 1.0 - (rand() * 1.0) / (RAND_MAX + 1.0)
   ;  double b = 1.0 - (rand() * 1.0) / (RAND_MAX + 1.0)
   ;  return sqrt( -2.0 * log(a)) * cos(2*3.14159265358979323846*b)
;  }


double mcxNormal
(  void
)
   {  return mcxNormalZiggurat()
;  }


double mcxNormalCut
(  double radius
,  double stddev
)
   {  dim d
   ;  if (radius < 0)
      radius = -radius
   ;  for (d=0;d<256;d++)
      {  double r = stddev * mcxNormal()
      ;  if (r >= -radius && r <= radius)
         return r
   ;  }
      return 0.0
;  }


double mcxNormalSample
(  double radius
,  double stddev
)
   {  int n_try = 0
   ;  double r = 2 * radius * (((1.0 * rand()) / RAND_MAX) - 0.5)
   ;  while (n_try++ < 1000)
      {  double n = exp( - (r * r) / (2 * stddev * stddev)) / (2.5066282746 * stddev)
      ;  double p = (1.0 * rand()) / RAND_MAX
      ;  if (n >= p)
         break
      ;  r = 2 * radius * (((1.0 * rand()) / RAND_MAX) - 0.5)
   ;  }
      return r
;  }


/* Ziggurat method
 *
 * Copyright (C) 2005  Jochen Voss.
 *
 * For details see the following article.
 *
 *     George Marsaglia, Wai Wan Tsang
 *     The Ziggurat Method for Generating Random Variables
 *     Journal of Statistical Software, vol. 5 (2000), no. 8
 *     http://www.jstatsoft.org/v05/i08/
*/

/* position of right-most step */
#define PARAM_R 3.44428647676

/* tabulated values for the heigt of the Ziggurat levels */
static const double ytab[128] =
{  1.000000000000  , 0.963598623011  , 0.936280813353  , 0.913041104253
,  0.892278506696  , 0.873239356919  , 0.855496407634  , 0.838778928349
,  0.822902083699  , 0.807732738234  , 0.793171045519  , 0.779139726505
,  0.765577436082  , 0.752434456248  , 0.739669787677  , 0.727249120285
,  0.715143377413  , 0.703327646455  , 0.691780377035  , 0.680482768910
,  0.669418297233  , 0.658572339120  , 0.647931876189  , 0.637485254896
,  0.627221991450  , 0.617132611532  , 0.607208517467  , 0.597441877296
,  0.587825531465  , 0.578352913803  , 0.569017984198  , 0.559815170911
,  0.550739320877  , 0.541785656682  , 0.532949739145  , 0.524227434628
,  0.515614886373  , 0.507108489253  , 0.498704867478  , 0.490400854812
,  0.482193476986  , 0.474079936010  , 0.466057596125  , 0.458123971214
,  0.450276713467  , 0.442513603171  , 0.434832539473  , 0.427231532022
,  0.419708693379  , 0.412262232120  , 0.404890446548  , 0.397591718955
,  0.390364510382  , 0.383207355816  , 0.376118859788  , 0.369097692334
,  0.362142585282  , 0.355252328834  , 0.348425768415  , 0.341661801776
,  0.334959376311  , 0.328317486588  , 0.321735172063  , 0.315211514970
,  0.308745638367  , 0.302336704338  , 0.295983912320  , 0.289686497571
,  0.283443729739  , 0.277254911560  , 0.271119377649  , 0.265036493387
,  0.259005653912  , 0.253026283183  , 0.247097833139  , 0.241219782932
,  0.235391638239  , 0.229612930649  , 0.223883217122  , 0.218202079518
,  0.212569124201  , 0.206983981709  , 0.201446306496  , 0.195955776745
,  0.190512094256  , 0.185114984406  , 0.179764196185  , 0.174459502324
,  0.169200699492  , 0.163987608600  , 0.158820075195  , 0.153697969964
,  0.148621189348  , 0.143589656295  , 0.138603321143  , 0.133662162669
,  0.128766189309  , 0.123915440582  , 0.119109988745  , 0.114349940703
,  0.109635440230  , 0.104966670533  , 0.100343857232  , 0.0957672718266
,  0.0912372357329 , 0.0867541250127 , 0.082318375932  , 0.0779304915295
,  0.0735910494266 , 0.0693007111742 , 0.065060233529  , 0.0608704821745
,  0.0567324485840 , 0.0526472709800 , 0.0486162607163 , 0.0446409359769
,  0.0407230655415 , 0.0368647267386 , 0.0330683839378 , 0.0293369977411
,  0.0256741818288 , 0.0220844372634 , 0.0185735200577 , 0.0151490552854
,  0.0118216532614 , 0.00860719483079, 0.00553245272614, 0.00265435214565
}  ;


/* tabulated values for 2^24 times x[i]/x[i+1],
 * used to accept for U*x[i+1]<=x[i]
 * without any floating point operations
*/

static const unsigned long ktab[128] =
{  0        , 12590644  , 14272653  , 14988939
,  15384584 , 15635009  , 15807561  , 15933577
,  16029594 , 16105155  , 16166147  , 16216399
,  16258508 , 16294295  , 16325078  , 16351831
,  16375291 , 16396026  , 16414479  , 16431002
,  16445880 , 16459343  , 16471578  , 16482744
,  16492970 , 16502368  , 16511031  , 16519039
,  16526459 , 16533352  , 16539769  , 16545755
,  16551348 , 16556584  , 16561493  , 16566101
,  16570433 , 16574511  , 16578353  , 16581977
,  16585398 , 16588629  , 16591685  , 16594575
,  16597311 , 16599901  , 16602354  , 16604679
,  16606881 , 16608968  , 16610945  , 16612818
,  16614592 , 16616272  , 16617861  , 16619363
,  16620782 , 16622121  , 16623383  , 16624570
,  16625685 , 16626730  , 16627708  , 16628619
,  16629465 , 16630248  , 16630969  , 16631628
,  16632228 , 16632768  , 16633248  , 16633671
,  16634034 , 16634340  , 16634586  , 16634774
,  16634903 , 16634972  , 16634980  , 16634926
,  16634810 , 16634628  , 16634381  , 16634066
,  16633680 , 16633222  , 16632688  , 16632075
,  16631380 , 16630598  , 16629726  , 16628757
,  16627686 , 16626507  , 16625212  , 16623794
,  16622243 , 16620548  , 16618698  , 16616679
,  16614476 , 16612071  , 16609444  , 16606571
,  16603425 , 16599973  , 16596178  , 16591995
,  16587369 , 16582237  , 16576520  , 16570120
,  16562917 , 16554758  , 16545450  , 16534739
,  16522287 , 16507638  , 16490152  , 16468907
,  16442518 , 16408804  , 16364095  , 16301683
,  16207738 , 16047994  , 15704248  , 15472926
}  ;

/* tabulated values of 2^{-24}*x[i] */
static const double wtab[128] =
{  1.62318314817e-08 , 2.16291505214e-08  , 2.54246305087e-08, 2.84579525938e-08
,  3.10340022482e-08 , 3.33011726243e-08  , 3.53439060345e-08, 3.72152672658e-08
,  3.89509895720e-08 , 4.05763964764e-08  , 4.21101548915e-08, 4.35664624904e-08
,  4.49563968336e-08 , 4.62887864029e-08  , 4.75707945735e-08, 4.88083237257e-08
,  5.00063025384e-08 , 5.11688950428e-08  , 5.22996558616e-08, 5.34016475624e-08
,  5.44775307871e-08 , 5.55296344581e-08  , 5.65600111659e-08, 5.75704813695e-08
,  5.85626690412e-08 , 5.95380306862e-08  , 6.04978791776e-08, 6.14434034901e-08
,  6.23756851626e-08 , 6.32957121259e-08  , 6.42043903937e-08, 6.51025540077e-08
,  6.59909735447e-08 , 6.68703634341e-08  , 6.77413882848e-08, 6.86046683810e-08
,  6.94607844804e-08 , 7.03102820203e-08  , 7.11536748229e-08, 7.19914483720e-08
,  7.28240627230e-08 , 7.36519550992e-08  , 7.44755422158e-08, 7.52952223703e-08
,  7.61113773308e-08 , 7.69243740467e-08  , 7.77345662086e-08, 7.85422956743e-08
,  7.93478937793e-08 , 8.01516825471e-08  , 8.09539758128e-08, 8.17550802699e-08
,  8.25552964535e-08 , 8.33549196661e-08  , 8.41542408569e-08, 8.49535474601e-08
,  8.57531242006e-08 , 8.65532538723e-08  , 8.73542180955e-08, 8.81562980590e-08
,  8.89597752521e-08 , 8.97649321908e-08  , 9.05720531451e-08, 9.13814248700e-08
,  9.21933373471e-08 , 9.30080845407e-08  , 9.38259651738e-08, 9.46472835298e-08
,  9.54723502847e-08 , 9.63014833769e-08  , 9.71350089201e-08, 9.79732621669e-08
,  9.88165885297e-08 , 9.96653446693e-08  , 1.00519899658e-07, 1.01380636230e-07
,  1.02247952126e-07 , 1.03122261554e-07  , 1.04003996769e-07, 1.04893609795e-07
,  1.05791574313e-07 , 1.06698387725e-07  , 1.07614573423e-07, 1.08540683296e-07
,  1.09477300508e-07 , 1.10425042570e-07  , 1.11384564771e-07, 1.12356564007e-07
,  1.13341783071e-07 , 1.14341015475e-07  , 1.15355110887e-07, 1.16384981291e-07
,  1.17431607977e-07 , 1.18496049514e-07  , 1.19579450872e-07, 1.20683053909e-07
,  1.21808209468e-07 , 1.22956391410e-07  , 1.24129212952e-07, 1.25328445797e-07
,  1.26556042658e-07 , 1.27814163916e-07  , 1.29105209375e-07, 1.30431856341e-07
,  1.31797105598e-07 , 1.33204337360e-07  , 1.34657379914e-07, 1.36160594606e-07
,  1.37718982103e-07 , 1.39338316679e-07  , 1.41025317971e-07, 1.42787873535e-07
,  1.44635331499e-07 , 1.46578891730e-07  , 1.48632138436e-07, 1.50811780719e-07
,  1.53138707402e-07 , 1.55639532047e-07  , 1.58348931426e-07, 1.61313325908e-07
,  1.64596952856e-07 , 1.68292495203e-07  , 1.72541128694e-07, 1.77574279496e-07
,  1.83813550477e-07 , 1.92166040885e-07  , 2.05295471952e-07, 2.22600839893e-07
}  ;


double mcxNormalZiggurat
(  void
)
   {  unsigned long  U, sign, i, j
   ;  double  x = 0.0, y

   ;  while (1)
      {  U = rand()
      ;  i = U & 0x0000007F         /* 7 bit to choose the step */
      ;  sign = U & 0x00000080      /* 1 bit for the sign */
      ;  j = rand() & 0x00FFFFFF  /* 24 bit for the x-value */

      ;  x = j*wtab[i]
      ;  if (j < ktab[i])
         break

      ;  if (i<127)
         {  double  y0, y1
         ;  y0 = ytab[i]
         ;  y1 = ytab[i+1]
         ;  y = y1+(y0-y1) * mcxUniform0
      ;  }
         else
         {  x = PARAM_R - log(mcxUniform1)/PARAM_R
         ;  y = exp(-PARAM_R*(x-0.5*PARAM_R)) * mcxUniform0
      ;  }
         if (y < exp(-0.5*x*x))
         break
   ;  }

      return  sign ? x : -x
;  }


