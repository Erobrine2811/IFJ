#include "scanner.h"

/** 
 * kto si chce vyskusat tuto cast scanneru tak to tymtom prelozte a spustite
 * $ gcc scanner_test.c ../src/scanner.c ../src/helper.c -std=c99 -pedantic -Wall -Wextra -g -o scanner_test
 * $ ./scanner_test 
*/



void scannerDebug()
{
    FILE *file_test = fopen("../tests/test_code_scanner", "r"); // ak chcete iny subor tak len prpiste cestu
    if (file_test == NULL)
    {
        printf("nepodarilo sa otvorit test_code_scanner file\n");
        return;
    }

    //char c = ' ';
    tToken token = NULL;

    printf("\n________________TEST_SCANNER_______________\n");
    /*while (c != EOF)
    {
        c = getc(file_test);
        printf("%c", c);
    }*/

    printf("\n\n\n");

    getTokenList(file_test, &token);
    
    printTokenList(token);

    printf("\n\n\n");

    printf("\n________________END_OF_TEST_______________\n");
    fclose(file_test);
}

int main()
{
    scannerDebug();
    return 0;
}