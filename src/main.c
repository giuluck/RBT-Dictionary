#include <stdio.h>
#include <stdlib.h>
#include"lib1617.h"
#include <time.h>

char* randomWord(int dim);
char* randomString(int dim);


int main(void) {
	char* stringTemp;
	srand(time(NULL));
	
	NODO* dictionary = createFromFile("fileTest1617.txt");

	printf("\nSTAMPA DEL DIZIONARIO:\n");
	printDictionary(dictionary);

	printf("\nNumero di parole salvate nel dizionario : %d\n\n", countWord(dictionary));

	insertWord(&dictionary, "tavolo");
	insertWord(&dictionary, "b");
	insertWord(&dictionary, "zoo");

	printf("\nNumero di parole salvate nel dizionario : %d\n\n", countWord(dictionary));
	printf("\nSTAMPA DEL DIZIONARIO dopo 3 inserimenti:\n");
	printDictionary(dictionary);

	cancWord(&dictionary, getWordAt(dictionary, 5));
	cancWord(&dictionary, getWordAt(dictionary, 1));
	cancWord(&dictionary, getWordAt(dictionary, 0));
	printf("\nSTAMPA DEL DIZIONARIO dopo 3 cancellazioni:\n");
	printDictionary(dictionary);
	
	insertDef(dictionary, getWordAt(dictionary, 2), "una definizione");
	insertDef(dictionary, getWordAt(dictionary, 4), "altra definizione");
	printf("\nSTAMPA DEL DIZIONARIO dopo inserimento definizioni:\n");
	printDictionary(dictionary);
	printf("\nNumero di parole salvate nel dizionario : %d\n\n", countWord(dictionary));


	printf("\nRicerca Parola \"%s\" -> definizione : [%s]\n\n", getWordAt(dictionary, 2), searchDef(dictionary, getWordAt(dictionary, 2)));
	printf("\nRicerca Parola \"%s\" -> definizione : [%s]\n\n", getWordAt(dictionary, 7), searchDef(dictionary, getWordAt(dictionary, 7)));
	stringTemp = "eftd";
	printf("\nRicerca Parola \"%s\" -> definizione : [%s]\n\n", stringTemp, searchDef(dictionary, stringTemp));
	


	system("PAUSE");
	return (0);
}



char* randomWord(int dim) {
	int  i;

	char* string = (char*)malloc((1 + dim) * sizeof(char));
	if (string == NULL) return "random";

	for (i = 0; i < dim; i++) {
		string[i] = rand() % 25 + 97;
	}
	string[i] = '\0';
	printf("\t\t-----> randomWord = %s \n", string);
	return string;
}

char* randomString(int dim) {
	int r, i;

	char* string = (char*)malloc((1 + dim) * sizeof(char));
	if (string == NULL) return "random string";

	for (i = 0; i < dim; i++) {

		r = rand() % 40 + 97;
		if (r >122) r = ' ';
		string[i] = r;
	}
	string[i] = '\0';
	printf("\t\t-----> randomString = %s \n", string);
	return string;
}