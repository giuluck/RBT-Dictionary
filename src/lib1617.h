#define MAX_DEF 50
#define MAX_WORD 20
#define MIN_WORD 2

// costanti per dizionario
#define RED 0
#define BLACK 1
#define LEFT 0
#define RIGHT 1
#define SENTINEL ""

// costanti per codifica Huffman
#define ALPHABET 128
#define BITSEQUENCE_LENGTH 8
#define TERMINATOR '*'
#define DIVIDER ';'

// nodo del dizionario
typedef struct NODO
{
    char word[MAX_WORD + 1]; // +1 per terminatore '\0'
    char def[MAX_DEF + 1];
    struct NODO *father;
    struct NODO *children[2]; // children[0] = figlio sinistro, children[1] = figlio destro
    int color;
    int nodes; // numero di nodi del sottoalbero radicato nel nodo
} NODO;

// nodo dell'albero di Huffman
typedef struct _HuffNode
{
    char letter;
    int frequence;
    struct _HuffNode *children[2];
} HuffNode;


// dato il nome del file di testo da cui viene creato un primo dizionario con definizioni assenti ritorna l'indirizzo
// della struttura dati contenente il dizionario ordinato (NULL in caso errore)
NODO* createFromFile(char* nameFile);


// stampa la struttura dati in cui è memorizzato il dizionario
void printDictionary(NODO*  dictionary);


// stampa il numero di parole salvato nel dizionario
int countWord(NODO* dictionary);

// inserisce la parola "word" nel dizionario senza definizione e ritorna 0 in caso di assenza di errori, 1 altrimenti
int insertWord(NODO** dictionary, char* word);

// cancella la parola "word" nel dizionario senza definizione e ritorna 0 in caso di assenza di errori, 1 altrimenti
int cancWord(NODO** dictionary, char* word);

// ritorna la i-esima parola nel dizionario (NULL in caso di errore)
char* getWordAt(NODO* dictionary, int index);

// sostituisce la definizione "def" della parola "word" e ritorna 0 in caso di assenza di errori, 1 altrimenti
int insertDef(NODO* dictionary, char* word, char* def);

// ritorna la definizione di "word" se presente, NULL altrimenti
char* searchDef(NODO* dictionary, char* word);

// salva il dizionario su file con il formato della stampa e ritorna 0 in caso di assenza di errori, -1 altrimenti
int saveDictionary(NODO* dictionary, char* fileOutput);

// crea un dizionario leggendo da file con il formato della stampa e lo ritorna
NODO* importDictionary(char *fileInput);


/*
Input:
    -dictionary: la struttura dati in cui avete memorizzato il dizionario
    -word: la parola per cui si vuole cercare la presenza
    -first,second,third: in queste tre variabili occorre memorizzare
        le tre voci più simili/vicine alla word da cercare.
Output:
    -0 in caso di assenza del termine nel dizionario
    -1 in caso di presenza del termine nel dizionario
    -(-1) in caso di altri errori
*/
int searchAdvance(NODO* dictionary, char* word, char** first, char** second, char** third);



/*
Input:
    -dictionary: la struttura dati in cui avete memorizzato il dizionario
    -fileOutput: il nome del file in cui si vuole salvare il risultato della compressione
Output:
    -0 in caso si successo
    -(-1) in caso di presenza di errori
*/
int compressHuffman(NODO* dictionary, char* fileOutput);


/*
Input:
    -fileInput: il nome del file contenente i dati compressi
    -dictionary : la struttura dati in cui deve essere memorizzato il dizionario
Output:
    -0 in caso si successo
    -(-1) in caso di presenza di errori
*/
int decompressHuffman(char *fileInput, NODO** dictionary);
