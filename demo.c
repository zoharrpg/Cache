#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

int main(){

    FILE *tfp = fopen("traces/csim/dave.trace","rt");
    if(!tfp){
        fprintf(stderr,"Error opening '%s': %s\n",
                "traces/csim/dave.trace",strerror(errno));
        return 1;
    }
    char linebuf[20];
    int parse_error = 0;
    while (fgets(linebuf, 20,tfp)){
        printf("%c\n",linebuf[0]);
        

    }
    fclose(tfp);
    return parse_error;
    
}

for (size_t i = 0; i < N; i += 64) {

            for (size_t j = 0; j < M; j += 4) {
                for (size_t k = i; k < i + 64; k++) {
                    for (size_t l = j; l < j + 4; l++) {

                        B[l][k] = A[k][l];
                    }
                }
            }
        }
