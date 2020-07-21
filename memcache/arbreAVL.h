#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Noeud de notre arbre AVL
struct Node {
  char *key;
  char *value;
  struct Node *left;
  struct Node *right;
  int height;
};

/* Fonction renvoyant la hauteur de la cle key dans le sous-arbre node.
   NB : Renvoie 0 si la cle n'a pas ete trouvee
*/
int getKeyHeight(struct Node* node, const char *key);

/* Fonction renvoyant le rang de la cle key dans le sous-arbre node.
   Par exemple :
     - Renvoie 1 si le noeud de la cle key est le noeud le plus en bas
       à gauche de l'arbre
     - Renvoie le nombre de noeuds dans l'arbre si le noeud de la cle
       key est le noeud le plus en bas à droite de l'arbre
   NB : Renvoie 0 si la cle n'a pas ete trouvee
*/
int getKeyRank(const struct Node* node, const char *key);

// Fonction renvoyant le nombre de noeuds dans le sous-arbre node
// NB : Renvoie 0 si l'arbre ne contient pas de noeud
int nbNode(const struct Node* node);

/* Fonction recursive qui insere key dans le sous-arbre de racine node.
   Renvoie la nouvelle racine de ce sous-arbre.
   NB : 1) un arbre binaire ne contient jamais deux fois la meme valeur.
           De ce fait, si la valeur existe deja, insert renvoie node, sans creer
           de nouveau noeud.
        2) Si node vaut NULL, renvoie un arbre constitue uniquement d'un noeud
           contenant cet arbre.
*/
struct Node* insert(struct Node* node, const char* key, const char* value);

// Fonction liberant tous les elements du sous-arbre node
// (y compris node)
// Renvoie systematiquement NULL
struct Node *release(struct Node *node);

/* Fonction renvoyant la valeur associee a une cle 
   (et NULL si la cle n'est pas trouvee)
*/
char *getValue(struct Node *node, const char *key);


