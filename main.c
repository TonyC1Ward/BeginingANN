#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>

#define NUMPAT 4
#define NUMIN  2
#define NUMHID 2
#define NUMOUT 1
#define SUCCESS 0.0002
#define MAX_EPOCH 2000000

#define rando() ((double)rand()/((double)RAND_MAX+1))

int NumHidden = NUMHID,NumInput = NUMIN,NumOutput = NUMOUT, epoch,NumPattern = NUMPAT;;
double DeltaWeightIH[NUMIN+1][NUMHID+1],DeltaWeightHO[NUMHID+1][NUMOUT+1];
double WeightIH[NUMIN+1][NUMHID+1],WeightHO[NUMHID+1][NUMOUT+1];
double Output[NUMPAT+1][NUMOUT+1];
double smallwt = 0.5;
double Input[NUMPAT+1][NUMIN+1] = { {0, 0, 0},
                                    {0, 0, 0},
                                    {0, 1, 0},
                                    {0, 0, 1},
                                    {0, 1, 1} };
double Target[NUMPAT+1][NUMOUT+1] = {   {0, 0},
                                        {0, 0},
                                        {0, 1},
                                        {0, 1},
                                        {0, 0} };

/**< Function prototypes */
void InitialiseWeightIH();
void InitialiseWeightHO();
void PrintNetwork();
float sigmoid(float);

int main() {
    int    i, j, k, p, np, op, ranpat[NUMPAT+1];
    double SumH[NUMPAT+1][NUMHID+1],  Hidden[NUMPAT+1][NUMHID+1];
    double SumO[NUMPAT+1][NUMOUT+1];
    double DeltaO[NUMOUT+1], SumDOW[NUMHID+1], DeltaH[NUMHID+1];
    double Error;
    double eta = 0.9; /*Changing this from 0.5 to 0.9 made a massive difference to solving the XOR but why?*/
    double alpha = 0.9;

    InitialiseWeightIH();
    InitialiseWeightHO();
    PrintNetwork();
    for( epoch = 0 ; epoch < MAX_EPOCH ; epoch++) {    /* iterate weight updates */
        for( p = 1 ; p <= NumPattern ; p++ ) {    /* randomize order of training patterns */
            ranpat[p] = p ;
        }
        for( p = 1 ; p <= NumPattern ; p++) {
            np = p + rando() * ( NumPattern + 1 - p ) ;
            op = ranpat[p] ; ranpat[p] = ranpat[np] ; ranpat[np] = op ;
        }
        Error = 0.0 ;
        for( np = 1 ; np <= NumPattern ; np++ ) {    /* repeat for all the training patterns */
            p = ranpat[np];
            for( j = 1 ; j <= NumHidden ; j++ ) {    /* compute hidden unit activations */
                SumH[p][j] = WeightIH[0][j] ;
                for( i = 1 ; i <= NumInput ; i++ ) {
                    SumH[p][j] += Input[p][i] * WeightIH[i][j] ;
                }
                Hidden[p][j] = sigmoid(SumH[p][j]);      /*1.0/(1.0 + exp(-SumH[p][j])) ;*/
            }
            for( k = 1 ; k <= NumOutput ; k++ ) {    /* compute output unit activations and errors */
                SumO[p][k] = WeightHO[0][k] ;
                for( j = 1 ; j <= NumHidden ; j++ ) {
                    SumO[p][k] += Hidden[p][j] * WeightHO[j][k] ;
                }
                Output[p][k] = sigmoid(SumO[p][k]);    /*1.0/(1.0 + exp(-SumO[p][k])) ;  Sigmoidal Outputs */
/*              Output[p][k] = SumO[p][k];      Linear Outputs */
                Error += 0.5 * (Target[p][k] - Output[p][k]) * (Target[p][k] - Output[p][k]) ;   /* SSE */
/*              Error -= ( Target[p][k] * log( Output[p][k] ) + ( 1.0 - Target[p][k] ) * log( 1.0 - Output[p][k] ) ) ;    Cross-Entropy Error */
                DeltaO[k] = (Target[p][k] - Output[p][k]) * Output[p][k] * (1.0 - Output[p][k]) ;   /* Sigmoidal Outputs, SSE */
/*              DeltaO[k] = Target[p][k] - Output[p][k];     Sigmoidal Outputs, Cross-Entropy Error */
/*              DeltaO[k] = Target[p][k] - Output[p][k];     Linear Outputs, SSE */
            }
            for( j = 1 ; j <= NumHidden ; j++ ) {    /* 'back-propagate' errors to hidden layer */
                SumDOW[j] = 0.0 ;
                for( k = 1 ; k <= NumOutput ; k++ ) {
                    SumDOW[j] += WeightHO[j][k] * DeltaO[k] ;
                }
                DeltaH[j] = SumDOW[j] * Hidden[p][j] * (1.0 - Hidden[p][j]) ;
            }
            for( j = 1 ; j <= NumHidden ; j++ ) {     /* update weights WeightIH */
                DeltaWeightIH[0][j] = eta * DeltaH[j] + alpha * DeltaWeightIH[0][j] ;
                WeightIH[0][j] += DeltaWeightIH[0][j] ;
                for( i = 1 ; i <= NumInput ; i++ ) {
                    DeltaWeightIH[i][j] = eta * Input[p][i] * DeltaH[j] + alpha * DeltaWeightIH[i][j];
                    WeightIH[i][j] += DeltaWeightIH[i][j] ;
                }
            }
            for( k = 1 ; k <= NumOutput ; k ++ ) {    /* update weights WeightHO */
                DeltaWeightHO[0][k] = eta * DeltaO[k] + alpha * DeltaWeightHO[0][k] ;
                WeightHO[0][k] += DeltaWeightHO[0][k] ;
                for( j = 1 ; j <= NumHidden ; j++ ) {
                    DeltaWeightHO[j][k] = eta * Hidden[p][j] * DeltaO[k] + alpha * DeltaWeightHO[j][k] ;
                    WeightHO[j][k] += DeltaWeightHO[j][k] ;
                }
            }
        }
        if( epoch%100 == 0 ) fprintf(stdout, "\nEpoch %-5d :   Error = %f", epoch, Error) ;
        if( Error < SUCCESS ) break ;  /* stop learning when 'near enough' */
    } /*End of main Epoch loop*/
    PrintNetwork();
    return 1 ;
}

/*******************************************************************************/

void InitialiseWeightIH(){
    int i,j;
    for( j = 1 ; j <= NumHidden ; j++ ) {
        for( i = 0 ; i <= NumInput ; i++ ) {
            DeltaWeightIH[i][j] = 0.0 ;
            WeightIH[i][j] = 2.0 * ( rando() - 0.5 ) * smallwt ;
        }
    }
}

void InitialiseWeightHO(){
    int j,k;
    for( k = 1 ; k <= NumOutput ; k ++ ) {
        for( j = 0 ; j <= NumHidden ; j++ ) {
            DeltaWeightHO[j][k] = 0.0 ;
            WeightHO[j][k] = 2.0 * ( rando() - 0.5 ) * smallwt ;
        }
    }
}

float sigmoid(float x){
     float exp_value,return_value;
     exp_value = exp((double) -x); /*** Exponential calculation ***/
     return_value = 1 / (1 + exp_value); /*** Final sigmoid value ***/
     return return_value;
}

void PrintNetwork(){
    int i,k,p;
    fprintf(stdout, "\n\nNETWORK DATA - EPOCH %d\n\nPat\t", epoch) ;   /* print network outputs */
    for( i = 1 ; i <= NumInput ; i++ ) {
        fprintf(stdout, "Input%-4d\t", i) ;
    }
    for( k = 1 ; k <= NumOutput ; k++ ) {
        fprintf(stdout, "Target%-4d\tOutput%-4d\t", k, k) ;
    }
    for( p = 1 ; p <= NumPattern ; p++ ) {
    fprintf(stdout, "\n%d\t", p) ;
        for( i = 1 ; i <= NumInput ; i++ ) {
            fprintf(stdout, "%f\t", Input[p][i]) ;
        }
        for( k = 1 ; k <= NumOutput ; k++ ) {
            fprintf(stdout, "%f\t%f\t", Target[p][k], Output[p][k]) ;
        }
    }
    fprintf(stdout,"\n\n");
}
