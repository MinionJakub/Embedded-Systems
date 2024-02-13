#include <inttypes.h>
#include <stdio.h>

char decode(int8_t len, int8_t sign){
    if(len == 1){
        if(sign&1) return 'T';
        return 'E';
    }
    else if(len == 2){
        if(sign&1){
            if(sign&2) return 'M';
            else return 'N';
        }
        else{
            if(sign&2) return 'A';
            else return 'I';
        }
    }
    else if(len == 3){
        if(sign&1){
            if(sign&2){
                if(sign&4) return 'O';
                else return 'G';
            }
            else{
                if(sign&4) return 'K';
                else return 'D';
            }
        }
        else{
            if(sign&2){
                if(sign&4) return 'W';
                else return 'R';
            }
            else{
                if(sign&4) return 'U';
                else return 'S';
            }
        }
    }
    else{
        if(sign == 8+4) return 'Z';
        if(sign == 8+2+1) return 'Y';
        if(sign == 8+2) return 'C';
        if(sign == 8+1) return 'X';
        if(sign == 8) return 'B';
        if(sign == 7) return 'J';
        if(sign == 6) return 'P';
        if(sign == 4) return 'L';
        if(sign == 2) return 'F';
        if(sign == 1) return 'V';
        return 'H';
    }
}

int main(){
    printf("%c",decode(3,0));
}