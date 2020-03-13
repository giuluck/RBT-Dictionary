#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lib1617.h"

// MACRO
#define minimum(a,b) ((a<b) ? a : b)    // minore fra due valori
#define isLeftChild(n) (n->father->children[LEFT] == n) // ritorna 1 se un nodo è figlio sinistro

/// FUNZIONI STATICHE PER RICERCA AVANZATA

// calcolo della distanza di Damerau-Levenshtein fra due stringhe
static int DL_distance(char s1[], char s2[])
{
    int i, j, current_cost, matrix[MAX_WORD + 1][MAX_WORD + 1];

    for(j=0; s2[j] != '\0'; j++)
        matrix[0][j] = j;
    matrix[0][j] = j; // devo riempire anche la casella al posto len(s2)

    // al termine, i = len(s1) e j = len(s2)
    for(i=0; s1[i] != '\0'; i++)
    {
        matrix[i+1][0] = i+1;

        for(j=0; s2[j] != '\0'; j++)
        {
            current_cost = (s1[i] != s2[j]);    // cost = 1 <--> s1[i] != s2[j]

            // se la casella in alto a sinistra di 2 posti esiste, considero anche l'operazione di swap
            if(i>0 && j>0 && s1[i] == s2[j-1] && s1[i-1] == s2[j])
                matrix[i+1][j+1] = minimum( minimum(matrix[i][j+1], matrix[i+1][j]) + 1,
											minimum(matrix[i][j], matrix[i-1][j-1]) + current_cost );
            // altrimenti significa che sto considerando il primo carattere di una stringa, quindi lo swap non è ammissibile
            else
                matrix[i+1][j+1] = minimum( minimum(matrix[i][j+1], matrix[i+1][j]) + 1, matrix[i][j] + current_cost );
        }
    }

    return matrix[i][j]; // la distanza di Damerau-Levenshtein si trova nell'ultima posizione della matrice
}

// inserimento ordinato di una parola nel vettore delle parole simili
static void orderedInsertion(int dist, char *word, int *d, char **r, int len)
{
    int i;

    // se la distanza di Damerau-Levenshtein è maggiore della parola più distante esco
    if(dist > d[0]) return;

    // altrimenti sposto indietro le parole finché non mi trovo alla posizione giusta
    for(i=1; i<len; i++)
    {
        if(dist < d[i])
        {
            d[i-1] = d[i];
            strcpy_s(r[i-1], sizeof(r[i - 1]), r[i]);
        }
        else
            break;
    }

    // e alla fine inserisco la parola nella posizione corretta
    d[i-1] = dist;
    strcpy_s(r[i-1], sizeof(r[i - 1]), word);
}

// trova le tre parole dell'albero con minore distanza di Damerau-Levenshtein da quella inserita
static void spellCheck(NODO *dictionary, char *w, int *d, char **r)
{
    int dist;

    if(strcmp(dictionary->word, SENTINEL) == 0) return; // caso base e chiamate ricorsive
    spellCheck(dictionary->children[LEFT], w, d, r);
    spellCheck(dictionary->children[RIGHT], w, d, r);

    // calcolo la distanza di Damerau-Levenshtein fra la parola corrente e quella inserita
    dist = DL_distance(dictionary->word, w);

    // inserisco la parola nel vettore che contiene le 3 parole più simili (dalla meno simile alla più simile)
    orderedInsertion(dist, dictionary->word, d, r, 3);
}

/// FUNZIONI STATICHE PER CODIFICA DI HUFFMAN

//  ALLOCAZIONE E DEALLOCAZIONE DEI NODI
static HuffNode* huffAlloc(char l, int f)
{
    HuffNode* n;

    n = (HuffNode *) malloc(sizeof(HuffNode));
    if(n == NULL) return NULL;

    n->letter = l;
    n->frequence = f;
    n->children[LEFT] = NULL;
    n->children[RIGHT] = NULL;
    return n;
}

static void huffDealloc(HuffNode *h)
{
    if(h == NULL) return;

    huffDealloc(h->children[LEFT]);
    huffDealloc(h->children[RIGHT]);
    free(h);
}

// GESTIONE DELLE CODE A PRIORITA' CON MIN-HEAP
static void heapify(HuffNode *h[], int dim, int pos)
{
    // left/right = posizione figli sinistro/destro, minIndex = posizione del nodo minimo
    int left, right, minIndex;
    HuffNode *temp;

    left = 2*pos + 1;
    right = 2*pos + 2;

    // confronto con il figlio sinistro, se è maggiore allora la sua posizione è maxIndex, altrimenti è quella di pos
    if(left < dim && h[left]->frequence < h[pos]->frequence)
        minIndex = left;
    else
        minIndex = pos;
    // poi confronto il nodo destro con il massimo fra i due, se è maggiore allora la sua posizione è maxIndex
    if(right < dim && h[right]->frequence < h[minIndex]->frequence)
        minIndex = right;

    // se pMax è p allora i nodi sono nel giusto ordine
    if(minIndex == pos)
        return;

    // altrimenti scambio il nodo p con il suo figlio massimo e chiamo heapify sul nodo sceso (posizione pMax)
    temp = h[minIndex];
    h[minIndex] = h[pos];
    h[pos] = temp;
    heapify(h, dim, minIndex);
}

static void insert(HuffNode *h[], HuffNode *key, int *dim)
{
    int pos;

    // vado a considerare l'ultimo elemento e suppongo di inserire il nuovo nodo in fondo
    // se il padre (pos/2) ha valore maggiore lo sposto in basso per creare uno spazio per il nodo da inserire
    pos = *dim;
    while(pos>0 && h[pos/2]->frequence > key->frequence)
    {
        h[pos] = h[pos/2];
        pos/=2;
    }

    // al termine inserisco il nodo nel posto giusto o, se è il valore minore, in testa e aumento la dimensione dell'heap
    h[pos] = key;
    (*dim)++;
}

static HuffNode* extractMin(HuffNode *h[], int *dim)
{
    HuffNode *m;

    // caso di un heap vuoto
    if(*dim < 1) return NULL;

    (*dim)--;
    m = h[0];

    // sposto l'ultimo nodo in testa e lo faccio scendere alla posizione corretta, poi ritorno il minimo che avevo salvato
    h[0] = h[*dim];
    heapify(h, *dim, 0);
    return m;
}

// FUNZIONI PER LA CODIFICA
static void getFrequences(NODO *n, HuffNode *f[])
{
    int i, pos;

    if(strcmp(n->word, SENTINEL) == 0) return;  // caso base e chiamate ricorsive
    getFrequences(n->children[LEFT], f);
    getFrequences(n->children[RIGHT], f);

    // in ogni posizione i è contenuto l'i-esimo carattere in codifica ASCII
    for(i=0; n->word[i] != '\0'; i++)
    {
        pos = (int) n->word[i];

        // se non ho già allocato il nodo lo alloco con frequenza 1
        if(f[pos] == NULL)
        {
            f[pos] = huffAlloc(pos, 1);
            if(f[pos] == NULL) return;
        }
        // altrimenti aumento la frequenza
        else
            f[pos]->frequence++;
    }
    // ripeto lo stesso procedimento per la definizione
    for(i=0; n->def[i] != '\0'; i++)
    {
        pos = (int) n->def[i];

        // se non ho già allocato il nodo lo alloco con frequenza 1
        if(f[pos] == NULL)
        {
            f[pos] = huffAlloc(pos, 1);
            if(f[pos] == NULL) return;
        }
        // altrimenti aumento la frequenza
        else
            f[pos]->frequence++;
    }
    f['[']->frequence++;    // aggiungo le frequenze per le [] intorno alla definizione
    f[']']->frequence++;
}

static HuffNode *createHuffmanEncTree(HuffNode *f[], int dim)
{
    HuffNode *n;

    // finché non è rimasto un solo nodo nella coda a priorità
    while(dim != 1)
    {
        // estraggo i due nodi con priorità più alta (quindi con frequenza più bassa) e alloco un nuovo nodo che sarà padre
        // dei due frecedenti e che ha come frequenza la somma delle frequenze
        n = huffAlloc('\0', 0);
        if(n == NULL) return NULL;
        n->children[LEFT] = extractMin(f, &dim);;
        n->children[RIGHT] = extractMin(f, &dim);
        n->frequence = n->children[LEFT]->frequence + n->children[RIGHT]->frequence;

        // inserisco il nuovo nodo nella coda a priorità
        insert(f, n, &dim);
    }

    // l'ultimo nodo rimasto è la testa dell'albero
    return extractMin(f, &dim);
}

static void saveCharactersMap(char *m[], HuffNode *h, char code[], int pos, FILE *f)
{
	int i;
    // se il nodo non ha figli (h->left == h->right == NULL perché ogni nodo ha solo 0 o 2 figli)
    if(h->children[LEFT] == NULL)
    {
		i = (int) h->letter;
		code[pos] = '\0';

        // copio nella casella corrispondente alla lettera il suo codice (tramite il puntatore c che alloco)
		m[i] = (char *) malloc((pos + 1) * sizeof(char));
        if(m[i] == NULL) return;
        strcpy_s(m[i], (pos + 1) * sizeof(char), code);

        // e salvo nel file la mappa con le associazioni lettera/codice
        fwrite(m[(int) h->letter], sizeof(char), pos, f);
        fwrite(&(h->letter), sizeof(char), 1, f);
        return;
    }

    // altrimenti scendo nei nodi figli (che esistono entrambi) dopo aver aggiunto 0 o 1 al codice
    code[pos] = '0';
    saveCharactersMap(m, h->children[LEFT], code, pos+1, f);

    code[pos] = '1';
    saveCharactersMap(m, h->children[RIGHT], code, pos+1, f);
}

static void addSequenceofBits(char *t, int *k, char *c, char *m[], FILE *f)
{
    int i, j, pos;

    // per ogni lettera in t
    for(i=0; t[i] != '\0'; i++)
    {
        pos = (int) t[i];

        // per ogni bit nel codice
        for(j=0; m[pos][j] != '\0'; j++)
        {
            // shifto a sinistra il carattere c e aggiungo in fondo uno 0 o un 1 a seconda del bit map[pos][j] (= '0' o '1')
            *c <<= 1;
            *c |= (m[pos][j] - '0');
            (*k)++;

            // quando ho completato l'intero carattere lo stampo sul file
            if(*k == BITSEQUENCE_LENGTH)
            {
                fwrite(c, sizeof(char), 1, f);
                *k = 0;
            }
        }
    }
}

// k = bit fino al quale ho scritto i dati nel carattere c; c = carattere su cui sto scrivendo i bit
static void encode(NODO *n, char *m[], int *k, char *c, FILE *f)
{
    if(strcmp(n->word, SENTINEL) == 0) return;  // caso base e chiamate ricorsive
    encode(n->children[LEFT], m, k, c, f);
    encode(n->children[RIGHT], m, k, c, f);

    // codifico la stringa "word[def]"
    addSequenceofBits(n->word, k, c, m, f);
    addSequenceofBits("[", k, c, m, f);
    addSequenceofBits(n->def, k, c, m, f);
    addSequenceofBits("]", k, c, m, f);
}

// FUNZIONI PER LA DECODIFICA
static HuffNode* createHuffmanDecTree(FILE *f)
{
    HuffNode *temp, *head;
    char letter;
    int dir;

    head = huffAlloc('\0', 0);
    temp = head;
    if(head == NULL) return NULL;

    fread(&letter, sizeof(char), 1, f);
    while(letter != DIVIDER || feof(f))    // ripeto finché non trovo il divisore
    {
        // se il carattere letto non fa parte del codice (!= '0','1'), allora sono in una foglia
        if(letter!='0' && letter!='1')
        {
            // salvo nella foglia il valore della lettera trovata
            temp->letter = letter;
            temp->children[LEFT] = NULL;
            temp->children[RIGHT] = NULL;
            temp = head;    // torno alla testa per il prossimo codice
        }
        else
        {
            // in base al valore trovato scendo a sinistra o a destra
            dir = (letter == '1');
            // se il nodo non esiste lo creo, poi mi sposto sul nodo indicato
            if(temp->children[dir] == NULL)
            {
                temp->children[dir] = huffAlloc('\0', 0);
                if(temp->children[dir] == NULL) return NULL;
            }
            temp = temp->children[dir];
        }

        fread(&letter, sizeof(char), 1, f); // passo alla lettera successiva
    }

    return head;
}

static char decodeChar(HuffNode *t, char *c, int *k, FILE *f)
{
    // quando mi trovo su una foglia salvo la lettera e ritorno alla radice
    while(t->children[LEFT] != NULL)
    {
        t = t->children[*c < 0];   // dir è il primo bit del carattere letto (1 se c<0)
        (*k)++;
        *c <<= 1;

        // quando ho leto tutti i bit passo al carattere successivo
        if(*k == BITSEQUENCE_LENGTH)
        {
            *k = 0;
            fread(c, sizeof(char), 1, f);
        }
    }

    return t->letter;
}

/// FUNZIONI STATICHE PER RED-BLACK TREES

// alloca un nuovo nodo con valori predefiniti
static NODO* nodeAlloc(NODO *root, char *w, char *def)
{
    NODO *node;

    node = (NODO*) malloc(sizeof(NODO));
    if(node == NULL) return NULL;

    strcpy_s(node->word, sizeof(node->word), w);
    strcpy_s(node->def, sizeof(node->def), def);
    node->father = NULL;
    node->children[LEFT] = root; // le foglie puntano alla sentinella, in questo modo si ha una struttura circolare
    node->children[RIGHT] = root; // e non sono necessari i controlli su NULL
    node->color = RED;   // i nodi inseriti hanno inizialmente colore rosso
    node->nodes = 1;    // inizialmente nel sottoalbero radicato in node c'è solo esso stesso
    return node;
}

// inizializza un nuovo RBT
static NODO* init()
{
    NODO *n;

    // inizializzo una sentinella di colore nero con valore SENTINEL (ha se stessa come figli e conta sempre 0 nodi)
    n = nodeAlloc(NULL, SENTINEL, "");
    if(n == NULL) return NULL;

    n->children[0] = n;
    n->children[1] = n;
    n->nodes = 0;
    n->color = BLACK;
    return n;
}

// presa in input la sentinella, ritorna la testa del RBT
static NODO *head(NODO *root)
{
    return root->children[RIGHT]; // poiché la sentinella ha valore "" (0) tutti i suoi figli sono a destra
}

// scambia un nodo con un suo figlio mantenendo le proprietà dei BST (dir = 0 se rotazione sinistra, 1 se rotazione destra)
static void rotate(NODO *f, int dir)
{
    NODO *c, *s;

    c = f->children[!dir]; // c (child) = figlio destro o sinistro di f (father)
    s = c->children[dir]; // s (subTree) = figlio sinistro o destro di c

    // s diventa figlio destro di f
    f->children[!dir] = s;
    s->father = f;

    // c diventa figlio (sinistro o destro) del padre di f (prima nonno di c)
    c->father = f->father;
    f->father->children[!isLeftChild(f)] = c;

    // f diventa figlio sinistro o destro di c
    f->father = c;
    c->children[dir] = f;

    // cambio il numero di nodi nei sottoalberi radicati in c e f
    c->nodes = f->nodes;
    f->nodes -= (1 + c->children[!dir]->nodes);
}

// inserisce un nodo in fondo al RBT con colore rosso
static void insertNode(NODO *root, NODO *node)
{
    int pos;

    // se il valore del nodo da inserire è superiore a quello esaminato scendo nel sottoalbero destro (1 = true)
    pos = strcmp(node->word, root->word) > 0;

    // se mi trovo in una foglia inserisco il nodo, altrimenti ripeto il procedimento nel sottoalbero sinistro
    if(strcmp(root->children[pos]->word, SENTINEL) == 0)
    {
        root->children[pos] = node;
        node->father = root;
    }
    else
        insertNode(root->children[pos], node);

    // avendo inserito il nodo aumento il contatore dei nodi (tranne che nella sentinella)
    if(strcmp(root->word, SENTINEL) != 0)
        root->nodes++;
}

// distribuisce il colore rosso sull'albero in seguito a un inserimento
static void distributeRed(NODO *node)
{
    NODO *father, *grandfather, *uncle, *temp;

    father = node->father;

    // CASO 1: se il nodo è alla radice lo coloro di nero (la radice ha come padre la sentinella)
    if(strcmp(father->word, SENTINEL) == 0)
    {
        node->color = BLACK;
        return;
    }

    // CASO 2: se il nodo ha un padre nero allora l'albero è un RBT
    if(father->color == BLACK)
        return;

    grandfather = father->father;
    uncle = grandfather->children[isLeftChild(father)];
    // CASO 3: se lo zio esiste ed è rosso sposto la colorazione sul nonno e coloro di nero loro due
    if(uncle->color == RED)
    {
        father->color = BLACK;
        uncle->color = BLACK;
        grandfather->color = RED;

        distributeRed(grandfather); // itero sul nonno
        return;
    }

    // CASO 4: se il nodo, il padre e il nonno non sono disposti lungo una retta (quindi il nodo è figlio sinistro e il padre
    // è figlio destro o viceversa) effettuo una rotazione sul padre per fare in modo che lo diventino
    if(isLeftChild(node) != isLeftChild(father))
    {
        // ruoto a destra se node è figlio sinistro o viceversa
        rotate(father, isLeftChild(node));

        // ora nodo e padre sono scambiati, perciò devo cambiare le variabili temporanee che puntano ad essi
        temp = node;
        node = father;
        father = temp;
    }

    // posso ruotare intorno al nonno in modo che il padre si sposti in cima e scambio le colorazioni del padre e del nonno
    grandfather->color = RED;
    father->color = BLACK;
    rotate(grandfather, isLeftChild(father)); // ruoto a destra se il padre è figlio sinistro o viceversa
}

// rimuove un nodo dal RBT ma senza controllare che le proprietà siano preservate e ritorna un nodo adiacente al nodo rimosso
NODO* deleteNode(NODO* node)
{
    NODO *temp, *child, *father;
    int isRedNode;

    // avendo trovato il nodo, dato che devo rimuoverlo, decremento il valore dei nodi di tutti i suoi avi e anche di se
    // stesso perché potrei dover eliminare il suo successore al suo posto
    temp = node;
    while(strcmp(temp->word, SENTINEL) != 0)
    {
        temp->nodes--;
        temp = temp->father;
    }

    // se il nodo ha entrambi i figli lo sostituisco il suo valore con quello del suo successore e poi rimuovo il successore
    // nel cercare il successore diminuisco di uno il campo nodes di tutti i nodi incontrati
    if(node->nodes > 1)
    {
        // il successore è il nodo più a sinistra del sottoalbero destro
        temp = node->children[RIGHT];
        while(strcmp(temp->children[LEFT]->word, SENTINEL) != 0)
        {
            temp->nodes--;
            temp = temp->children[LEFT];
        }

        strcpy_s(node->word, sizeof(node->word), temp->word);    // scambio il valore del successore con quello del nodo
        node = temp;    // una volta finito, devo eliminare il successore, quindi assegno il suo indirizzo a node
    }

    // collego il padre del nodo al suo eventuale unico figlio (destro o sinistro)
    father = node->father;
    child = node->children[strcmp(node->children[LEFT]->word, SENTINEL) == 0];
    child->father = father;
    father->children[!isLeftChild(node)] = child;

    // infine controllo il colore del nodo e lo dealloco: se è rosso ritorno NULL altrimenti ritorno il figlio del nodo
    // (nel caso fosse la sentinella avrà comunque il padre del nodo eliminato come suo padre)
    isRedNode = (node->color == RED);
    free(node);
    return isRedNode ? NULL : child;
}

// distribuisce il doppio nero sull'albero in seguito a un'eliminazione
static void distributeDoubleBlack(NODO *node)
{
    NODO *father, *sibling;

    father = node->father;
    sibling = father->children[isLeftChild(node)];

    // se il nodo è rosso o è alla radice (il padre è la sentinella) coloro il nodo di nero
    if(node->color == RED || strcmp(father->word, SENTINEL) == 0)
    {
        node->color = BLACK;
        return;
    }

    // CASO 1: il fratello è rosso (quindi entrambi i figli sono neri come anche il padre)
    // mi basta ruotare intorno al padre e al fratello scambiando i loro colori per rientrare in uno dei casi successivi
    if(sibling->color == RED)
    {
        sibling->color = BLACK;
        father->color = RED;
        rotate(father, isLeftChild(sibling));

        sibling = father->children[isLeftChild(node)]; // in seguito alle rotazioni cambia il fratello del nodo
    }

    // CASO 2: il fratello è nero
    // se ha entrambi i figli neri, posso colorare il fratello di rosso
    if(sibling->children[LEFT]->color == BLACK && sibling->children[RIGHT]->color == BLACK)
    {
        sibling->color = RED;

        if(father->color == RED)
            father->color = BLACK;  // nel caso in cui anche il padre fosse rosso, mi basta colorarlo di nero
        else
            distributeDoubleBlack(father);  // altrimenti il padre diventa il nodo con doppio nero
    }
    else
    {
        // se padre e figlio rosso non sono allineati, ruoto intorno ad essi per fare in modo che lo siano
        // a quel punto scambio i loro colori e imposto l'ex-figlio rosso come fratello perché è salito di un livello
        if(sibling->children[!isLeftChild(sibling)]->color != RED)
        {
            sibling->color = RED;
            sibling->children[isLeftChild(sibling)]->color = BLACK; // il nodo rosso è quello non allineato

            // se il fratello è figlio sinistro allora il figlio rosso è figlio destro, quindi ruoto verso sinistra
            rotate(sibling, !isLeftChild(sibling));
            sibling = sibling->father;  // il nuovo "fratello" da esaminare è il nodo padre di quello corrente
        }

        // una volta che il figlio rosso del fratello è allineato con esso, coloro di nero il figlio,
        // poi scambio il colore del padre e del fratello e infine ruoto intorno padre e al fratello
        sibling->color = father->color;
        father->color = BLACK;
        sibling->children[!isLeftChild(sibling)]->color = BLACK;

        // se il fratello è figlio sinistro allora ruoto verso destra, altrimenti verso sinistra
        rotate(father, isLeftChild(sibling));
    }
}

// restituisce il nodo con valore cercato
static NODO* nodeSearch(NODO *root, char *w)
{
    // se il nodo vale SENTINEL significa che non ho trovato il valore, quindi ritorno NULL
    if(strcmp(root->word, SENTINEL) == 0) return NULL;

    // se invece trovo il nodo lo ritorno
    if(strcmp(root->word, w) == 0) return root;

    // altrimenti scendo nel sottoalbero sinistro (0 = false) o destro(1 = true) in base al valore dell'elemento cercato
    return nodeSearch(root->children[strcmp(w, root->word) > 0], w);
}

// restituisce il nodo in posizione i-esima
static NODO* nodeAt(NODO *root, int i)
{
    int leftNodes, pos;

    leftNodes = root->children[LEFT]->nodes; // nodi nel sottoalbero sinistro

    // se i = nodi sinistri allora, dato che conto partendo da 0, questo è l'i-esimo nodo
    if(i == leftNodes) return root;

    // se i < al numero di nodi a sinistra, lo cerco nel sottoalbero sinistro
    // altrimenti cerco nel sottoalbero destro in posizione i - nodi - 1
    pos = i > leftNodes;
    if(pos == RIGHT)
        i -= (leftNodes + 1);

    return nodeAt(root->children[pos], i);
}

/// FUNZIONI STATICHE PER DIZIONARIO

// stampa a video delle parole in ordine lessicografico
static void inorderPrint(NODO* dictionary)
{
    if(strcmp(dictionary->word, SENTINEL) == 0) return; // caso base

    inorderPrint(dictionary->children[LEFT]);
    printf("\"%s\" : [%s]\n", dictionary->word, dictionary->def);
    inorderPrint(dictionary->children[RIGHT]);
}

// stampa su file delle parole in ordine lessicografico
static void inorderSave(NODO* dictionary, FILE *f)
{
    if(strcmp(dictionary->word, SENTINEL) == 0) return; // caso base

    inorderSave(dictionary->children[LEFT], f);
    fprintf(f, "\"%s\" : [%s]\n", dictionary->word, dictionary->def);
    inorderSave(dictionary->children[RIGHT], f);
}

// crea un nuovo nodo con la parola e la definizione indicata
static NODO* newWord(NODO *dictionary, char *word, char *def)
{
    NODO *newNode;
    char w[MAX_WORD]; // nuova stringa perché non posso cambiare il valore di una stringa costante
    int i, j;

    // salvo la parola in minuscolo su w[]
    j = 0;
    for(i = 0; word[i] != '\0'; i++)
    {
        // i caratteri ammissibili sono le lettere, il trattino e le parentesi tonde (nella definizione nulla)
        if(isalpha(word[i]) || word[i] == '-' || word[i] == '(' || word[i] == ')')
        {
            w[j] = tolower(word[i]);

            // controllo che la parola non vada in overflow
            if(j < MAX_WORD)
                j++;
            else
                return NULL;
        }
    }
    w[j] = '\0';

    // se la parola è troppo corta o è già presente non la inserisco
    if(j < MIN_WORD || searchDef(dictionary, w) != NULL) return NULL;

    // altrimenti alloco un nuovo nodo con valore "word" e definizione "def"
    newNode = nodeAlloc(dictionary, w, def);
    return newNode;
}

/// FUNZIONI DI LIBRERIA

NODO* createFromFile(char* nameFile)
{
    FILE *f;
    NODO *dictionary;
    char w[MAX_WORD];

    fopen_s(&f, nameFile, "r");   // apro il file e controllo che esista
    if(f == NULL) return NULL;

    // inizializzo un nuovo RBT e inserisco i valori letti finché non arrivo al termine del file
    dictionary = init();
    while(!feof(f))
    {
        fscanf_s(f, "%s", w, sizeof(w));
        insertWord(&dictionary, w);
    }

    fclose(f);
    return dictionary;
}

void printDictionary(NODO* dictionary)
{
    inorderPrint(head(dictionary)); // chiamo head(dictionary) per non considerare la sentinella
}

int countWord(NODO* dictionary)
{
    return head(dictionary)->nodes; // il nodo alla radice contiene il numero di nodi salvati in tutta la struttura dati
}

int insertWord(NODO** dictionary, char* word)
{
    NODO *newNode;

    // alloco un nuovo nodo con definizione predefinita "(null)"
    newNode = newWord(*dictionary, word, "(null)");
    if(newNode == NULL) return 1;

    // se tutto è andato a buon fine inserisco il nuovo nodo e ripristino le proprietà dei RBT
    insertNode(head(*dictionary), newNode);
    distributeRed(newNode);
    return 0;
}

int cancWord(NODO** dictionary, char* word)
{
    NODO* node;

    // cerco il nodo nella struttura dati
    node = nodeSearch(head(*dictionary), word);
    if(node == NULL) return 1;

    // rimuovo il nodo e ripristino le proprietà dei RBT se il nodo eliminato non era rosso (!= NULL)
    node = deleteNode(node);
    if(node != NULL)
        distributeDoubleBlack(node);
    return 0;
}

char* getWordAt(NODO* dictionary,int index)
{
    if(index < 0 || index >= countWord(dictionary)) return NULL;    // controllo che i sia un valore ammissibile

    return nodeAt(head(dictionary), index)->word;   // se lo è ritorno la parola all'i-esimo posto
}

int insertDef(NODO* dictionary, char* word, char* def)
{
    NODO *n;

    // cerco il nodo
    n = nodeSearch(head(dictionary), word);
    if(n == NULL) return 1;

    // se esiste copio la nuova definizione
    strcpy_s(n->def, sizeof(n->def), def);
    return 0;
}

char* searchDef(NODO* dictionary, char* word)
{
    NODO *n;

    n = nodeSearch(head(dictionary), word); // cerco il nodo
    if(n == NULL) return NULL;

    return n->def;  // se esiste ritorno la sua definizione
}

int saveDictionary(NODO* dictionary, char* fileOutput)
{
	FILE *f;
	
	fopen_s(&f, fileOutput, "w");
    if(f == NULL) return -1;

    inorderSave(head(dictionary), f);
    fclose(f);
    return 0;
}

NODO* importDictionary(char *fileInput)
{
    FILE *f;
    NODO *dictionary, *newNode;
    char w[MAX_WORD+1], d[MAX_DEF+1], *c;

    fopen_s(&f, fileInput, "r");
    if(f == NULL) return NULL;

    // inizializzo un nuovo RBT e inserisco i valori letti finché non arrivo al termine del file
    dictionary = init();
    while(!feof(f))
    {
        fscanf_s(f, "%s", w, sizeof(w));         // leggo la "word"
        fgets(d, 5, f);             // leggo " : ["
        fgets(d, MAX_DEF + 1, f);   // leggo la "def" con ']' al termine (uso fgets perché può essere più di una parola)

        // scambio ']' con il terminatore '\0' se presente
        c = strchr(d, ']');
        if (c != NULL)
            *c = '\0';

        // se non ci sono errori nell'allocazione inserisco il nuovo nodo e ripristino le proprietà dei RBT
        newNode = newWord(dictionary, w, d);
        if(newNode != NULL)
        {
            insertNode(head(dictionary), newNode);
            distributeRed(newNode);
        }
    }

    fclose(f);
    return dictionary;
}

int searchAdvance(NODO* dictionary, char* word, char** primoRis, char** secondoRis, char** terzoRis)
{
    // inizialmente le distanze sono settate al valore massimo
    int i, distances[3] = {MAX_WORD + 1, MAX_WORD + 1, MAX_WORD + 1};
    char *results[3];
    for(i=0; i<3; i++)
    {
        results[i] = (char *) calloc(MAX_WORD + 1, sizeof(char));
        if(results[i] == NULL) return -1;
    }

    spellCheck(head(dictionary), word, distances, results);

    // inserisco nei parametri i puntatori alle parole trovate e ritorno true se la parola è presente nel dizionario
    *primoRis = results[2];
    *secondoRis = results[1];
    *terzoRis = results[0];
    return (distances[2] == 0); // se distances[2] == 0 allora la parola è presente nel dizionario, altrimenti non lo è
}

int compressHuffman(NODO* dictionary, char* fileOutput)
{
    FILE *f;
    HuffNode *frequencies[ALPHABET], *tree;
    char c, code[ALPHABET], *map[ALPHABET];
    int i, k, offset, dim;

    fopen_s(&f, fileOutput, "wb");
    if(f == NULL) return -1;

    // inizializzo a NULL l'array di nodi perché molti di essi probabilmente non serviranno
    for(i=0; i<ALPHABET; i++)
        frequencies[i] = NULL;
    frequencies['['] = huffAlloc('[', 0);                   // caratteri speciali che serviranno sicuramente
    frequencies[']'] = huffAlloc(']', 0);
    frequencies[TERMINATOR] = huffAlloc(TERMINATOR, 1);
    if(frequencies[TERMINATOR] == NULL) return -1;

    // ottengo le frequenze e rimuovo quelle nulle (i nodi non inizializzati)
    getFrequences(head(dictionary), frequencies);
    offset = 0;
    for(i=0; i<ALPHABET; i++)
    {
        if(frequencies[i] != NULL)
        {
            frequencies[i - offset] = frequencies[i];
            frequencies[i] = NULL;
        }
        else
            offset++;
    }
    // aggiorno la dimensione dell'alfabeto e creo un heap con le lettere presenti (Build-Heap)
    dim = ALPHABET - offset;
    for(i = dim/2 - 1; i>=0; i--)
        heapify(frequencies, dim, i);

    // inizializzo la mappa con i codici identificativi di ogni lettera, poi vado a riempire la mappa dopo aver creato l'albero
    for(i=0; i<ALPHABET; i++)
        map[i] = NULL;
    tree = createHuffmanEncTree(frequencies, dim);
    saveCharactersMap(map, tree, code, 0, f);
    huffDealloc(tree);

    // inserisco il divisore fra la mappa per decodificare l'albero e il resto del file
    c = DIVIDER;
    fwrite(&c, sizeof(char), 1, f);

    // codifico i dati e al termine aggiungo il terminatore in modo da sapere dove si conclude la codifica
    k = 0;
    c = '\0';
    encode(head(dictionary), map, &k, &c, f);
    code[0] = TERMINATOR;
    code[1] = '\0';
    addSequenceofBits(code, &k, &c, map, f);

    // al termine shifto a sinistra delle BITSEQUENCE_LENGTH - k posizioni rimanenti e stampo l'ultimo carattere
    c <<= (BITSEQUENCE_LENGTH-k);
    fwrite(&c, sizeof(char), 1, f);

    for(i=0; i<ALPHABET; i++)
        if(map[i] != NULL) free(frequencies[i]);

    fclose(f);
    return 0;
}

int decompressHuffman(char *fileInput, NODO** dictionary)
{
    FILE *f;
    HuffNode *tree;
    NODO *newNode;
    char w[MAX_WORD+1], d[MAX_DEF+1], c, temp;
    int readWord, i, k;

    fopen_s(&f, fileInput, "rb");
    if(f == NULL) return -1;

    tree = createHuffmanDecTree(f);
    *dictionary = init();
    readWord = 1;
    i = 0;
    k = 0;
    fread(&c, sizeof(char), 1, f); // leggo il primo carattere

    while(!feof(f))
    {
        temp = decodeChar(tree, &c, &k, f);
        if(temp == '[') // quando trovo una '[' termino la parola e mi preparo a leggere la definizione
        {
            readWord = 0;
            w[i] = '\0';
            i = 0;
        }
        else if(temp == ']') // quando trovo una ']' inserisco il nodo nel dizionario e mi preparo a leggere una nuova parola
        {
            readWord = 1;
            d[i] = '\0';
            i = 0;

            newNode = newWord(*dictionary, w, d);
            if(newNode != NULL)
            {
                insertNode(head(*dictionary), newNode);
                distributeRed(newNode);
            }
        }
        else if(temp == TERMINATOR) // quando incontro il terminatore ho finito di leggere
        {
            fclose(f);
            huffDealloc(tree);
            return 0;
        }
        else
        {
            // inserisco il carattere letto nella parola o nella definizione
            if(readWord)
                w[i] = temp;
            else
                d[i] = temp;

            i++;
        }
    }

    // se arrivo qui significa che non ho incontrato il terminatore per qualche motivo
    fclose(f);
    huffDealloc(tree);
    return -1;
}
