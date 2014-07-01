/**************************************************/
/* Simulation of the 2 neuronal population model. */
/*           **************************************/
/* Version 0 */
/*************/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include <limits.h>
#include<string.h>
#include<gsl/gsl_rng.h>		/* GNU Scientific library */
#include<gsl/gsl_randist.h>
#include<omp.h>		        /* Parallel OMP library */
#include <signal.h>


/* Global variables declaration */
int debug = 0,			/* Boolean: debugging activator */
  d_level = 0;			/* Debugging level */
char DATA_FILE[50] = "input_data.conf";
char mesg[1024];
char mesg2[1024];


/* Additional libraries (custom libraries) */
#include "common.h"		/* This must be first */
#include "utils.h"
#include "file.h"
#include "nrutil.h"
#include "nr.h"
#include "field.h"
#include "qif.h"

/* Functions declaration. */
void WarningFPE(int sig);


/* === FUNCTION  main ====================
 * Description:  Program Body
 *   Variables:  It accepts external arguments
 * ======================================= */

int main(int argc, char **argv) {

#ifdef _OPENMP    		/* Compilation with OMP */
  int numthreads;
  numthreads = omp_get_max_threads();
  omp_set_num_threads(numthreads);
#endif  

  (void) signal(SIGFPE,WarningFPE);

  /* +++++++++++++++++ External Things ++++++++++++++++++++++ */

  time_t ti;   struct tm *tm;  ti = time(NULL); tm = localtime(&ti);
  if((argc >1) && (argv[1][1] == 'd')) {debug = 1; d_level = 1;} /* Initial debugging */

  srand(time(NULL));
  gsl_rng_env_setup();		/* Random generator initialization */
  gsl_rng_default_seed = rand()*RAND_MAX;
  sprintf(mesg,"Seed: %ld ",gsl_rng_default_seed);  DEBUG(mesg);
  
  /* Date variables */
  char day[100], hour[100];  strftime(day, 100, "%m-%d-%Y",tm);  sprintf(hour,"%d.%02d",tm->tm_hour,tm->tm_min);

  /* Data store */
  Create_Dir("results");

  /* Parameters (all the parameters are stored in a structure) */
  t_data *d,*data;
  d = malloc(sizeof(t_data));
  data = malloc(sizeof(t_data));

  /* Time and scanning */
  double TIME = 0; int t = 0; int t_max = 0;
  int Nscan = 0;

  /* ++++++++++++++++++++++++++++++++++++++++++++++++++ */
  
  /* Creating data files */
  sprintf(d->file,"%s_%s",day,hour);
  sprintf(mesg2,"results/%s",d->file);
  Create_Dir(mesg2);
  Create_Dir("temp");

  /*********************************/
  /* Program arguments assignation */
  /*********************************/
  *data = Scan_Data(DATA_FILE,*d);
  d = data;


  while((argc--) != 1) {	/* Terminal arguments can be handled */
    *data = Arg(argv[argc], *d);
    d = data;
  }


#ifdef _OPENMP			/* Work is divided between the cores */
  int chunksize = d->l/numthreads;
  if(chunksize > 5) chunksize = 5;
#endif

  /**********************************/
  /* New simulations can start here */
  /**********************************/

  return 0;
  do {        
    
    printf("\n\n%3d%%",(int)(t*100.0/t_max));
    
    t = 0;
    TIME = 0;
    /* Run */
    do {			/* Tiempo */
      if(t%(t_max/100) == 0) { /* Control point */
	printf("\b\b\b\b%3d%%",(int)(t*100.0/t_max));
	fflush(stdout);
      }

	
      t++;
      TIME = t*d->dt;
    } while(TIME < d->TT);	/* Paso temporal (se puede hacer de la misma manera que en el QIF */

  } while (d->scan < Nscan);  /* Simulation ends here */

  system("rm -r ./temp");
  printf("\n");

  return 0;  
}


void
WarningFPE(int sig) {

  printf("\nEp, ep, ep, ... Floating Point exception\n");
  exit(sig);
}
