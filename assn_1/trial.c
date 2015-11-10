#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void myprint(char s[]);

int main(){
	char s[10]="Anish";
	myprint(s);
}

void myprint(char s[]){
	print("%s\n",s);
}