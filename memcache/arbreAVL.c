#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "arbreAVL.h"

/* Fonction utilitaire pour recuperer la hauteur d'un arbre
   NB : Renvoie 0 si node est NULL
*/
static int height(const struct Node *node) {
  if (!node)
    return 0;
  return node->height;
}

/* Fonction utilitaire renvoyant le maximum de deux entiers
 */
static int max(const int a, const int b) {
  return (a > b)? a : b;
}

/* Procedure qui recalcule le champs height de node a partir de la
   hauteur de ses noeuds fils
*/
static void updateHeight(struct Node *node) {
  node->height = 1 + max(height(node->left),
			 height(node->right));
}

/* Fonction (interne) renvoyant le noeud contenant la cle key dans le 
   sous-arbre node.
   NB : Renvoie NULL si la cle n'a pas ete trouvee
*/
static struct Node *getKeyNode(struct Node* node, const char *key) {
  if (!node)
    return NULL;
  int res = strcmp(key, node->key);
  if (res < 0)
    return getKeyNode(node->left, key);
  else if (res > 0)
    return getKeyNode(node->right, key);
  else {
    return node;
  }
}

/* Fonction renvoyant le nombre de noeuds dans le sous-arbre node
   NB : Renvoie 0 si l'arbre ne contient pas de noeud
*/
int nbNode(const struct Node* node) {
  return node ?
    1 + nbNode(node->left) + nbNode(node->right) :
    0;
}

/* Fonction liberant tous les elements du sous-arbre node
   (y compris node)
   Renvoie systematiquement NULL
*/
struct Node *release(struct Node *node) {
  if (node) {
    node->left = release(node->left);
    node->right = release(node->right);
    free(node->key);
    node->key = NULL;
    free(node->value);
    node->value = NULL;
    free(node);
  }
  return NULL;
}

/* Fonction (interne) pour creer un noeud pour stocker la cle
   NB :
      - La cle est recopiee dans une zone memoire allouee dans newNode
      - Les pointeurs left et right sont a NULL
      - Le champ height est initialise a 1 (ce noeud est considere comme 
        une feuille autonome).
*/
static struct Node* newNode(const char *key, const char* value) {
  struct Node* node = malloc(sizeof(struct Node));
  assert(node);
  node->key = malloc(strlen(key)+1);
  assert(node->key);
  strcpy(node->key, key);
  node->value = malloc(strlen(value)+1);
  assert(node->value);
  strcpy(node->value, value);
  node->left   = NULL;
  node->right  = NULL;
  node->height = 1;  // Ce nouveau noeud est considere comme une feuille
  return node;
}

// Rotation droite d'un sous-arbre de racine z
static struct Node *rightRotate(struct Node *z) {
  // Pour info, les noms de variable sont inspires des noms
  // des noeuds dans les figures de l'enonce.
  struct Node *y = z->left;
  struct Node *T3 = y->right;
 
  // Rotation
  y->right = z;
  z->left = T3;
 
  // MAJ des hauteurs
  updateHeight(z);
  updateHeight(y);
 
  // On renvoie la nouvelle racine du sous-arbre
  return y;
}

// Rotation gauche d'un sous-arbre de racine z
static struct Node *leftRotate(struct Node *z) {
  // Pour info, les noms de variable sont inspires des noms
  // des noeuds dans les figures de l'enonce.
  struct Node *y = z->right;
  struct Node *T2 = y->left;
 
  // Rotation
  y->left = z;
  z->right = T2;
 
  // MAJ des hauteurs
  updateHeight(z);
  updateHeight(y);
 
  // On renvoie la nouvelle racine du sous-arbre
  return y;
}

/* Fonction recursive qui insere key dans le sous-arbre de racine node.
   Renvoie la nouvelle racine de ce sous-arbre.
   NB : 1) un arbre binaire ne contient jamais deux fois la meme valeur.
           De ce fait, si la valeur existe deja, insert renvoie node, sans creer
           de nouveau noeud.
        2) Si node vaut NULL, renvoie un arbre constitue uniquement d'un noeud
           contenant cet arbre.
*/
struct Node* insert(struct Node* node, const char* key, const char *value) {
  /* 1. Insertion du noeud */
  if (!node)
    return newNode(key, value);

  int res = strcmp(key, node->key);
  if (res < 0)
    node->left  = insert(node->left, key, value);
  else if (res > 0)
    node->right = insert(node->right, key, value);
  else {
    // key est deja present : on met à jour value si necessaire
    if (strcmp(value, node->value) != 0) {
      node->value = realloc(node->value, strlen(value)+1);
      assert(node->value);
      strcpy(node->value, value);
    }
    return node;
  }
 
  /* 2. MAJ de la taille du noeud courant */
  updateHeight(node);
 
  /* 3. Calcul du facteur balance du noeud courant */
  int balance = height(node->left) - height(node->right);

  // S'il est devenu desequilibre, il y a 4 cas

  // Cas Gauche-Gauche
  if (balance > 1 && strcmp(key, node->left->key) < 0)
    return rightRotate(node);

  // Cas Droite-Droite
  if (balance < -1 && strcmp(key, node->right->key) > 0)
    return leftRotate(node);

  // Cas Gauche-Droite
  if (balance > 1 && strcmp(key, node->left->key) > 0) {
    node->left =  leftRotate(node->left);
    return rightRotate(node);
  }

  // Cas Droite-Gauche
  if (balance < -1 && strcmp(key, node->right->key) < 0) {
    node->right = rightRotate(node->right);
    return leftRotate(node);
  }

  /* Retourne le pointeur node */
  return node;
}

/* Fonction renvoyant la hauteur de la cle key dans le sous-arbre node.
   NB : Renvoie 0 si la cle n'a pas ete trouvee
*/
int getKeyHeight(struct Node* node, const char *key) {
  struct Node *correspondingNode = getKeyNode(node, key);
  return correspondingNode ? height(correspondingNode) : 0;
}

/* Fonction (interne) renvoyant le nombre de noeuds trouves avant le
   noeud contenant key dans le sous-arbre key ou bien -1 si key n'est pas
   trouve.
*/
static void countNodesUntilKey(const struct Node* node,
			       const char *key,
			       int *nbNodesBeforeKey,
			       bool *found) {
  if (!node)
    return;
  countNodesUntilKey(node->left, key, nbNodesBeforeKey, found);
  int res = strcmp(key, node->key);
  if (res > 0) {
    *nbNodesBeforeKey += 1;
    countNodesUntilKey(node->right, key, nbNodesBeforeKey, found);
  } else if (res == 0) {
    *found = true;
  }
}

/* Fonction renvoyant le rang de la cle key dans le sous-arbre node.
   Par exemple :
     - Renvoie 0 si le noeud de la cle key est le noeud le plus en bas
       à gauche de l'arbre
     - Renvoie le nombre de noeuds dans l'arbre - 1 si le noeud de la cle
       key est le noeud le plus en bas à droite de l'arbre
   NB : Renvoie -1 si la cle n'a pas ete trouvee
*/
int getKeyRank(const struct Node* node, const char *key) {
  if (!node)
    return -1;
  bool found = false;
  int nbNodesBeforeKey = 0;
  countNodesUntilKey(node, key, &nbNodesBeforeKey, &found);
  return found ? nbNodesBeforeKey : -1;
}

/* Fonction renvoyant la valeur associee a une cle 
   (et NULL si la cle n'est pas trouvee)
*/
char *getValue(struct Node *node, const char *key) {
  struct Node *correspondingNode = getKeyNode(node, key);
  return correspondingNode ? correspondingNode->value : NULL;
  return NULL;
}

