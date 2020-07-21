#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#include "arbreAVL.h"

static void test_insert_et_nbNode(void **state) {
  struct Node *root = NULL;

  // Test unitaire du comportement de nbNode() quand root==NULL
  // TODO A completer

  // Inserer une cle fonctionne
  root = insert(root, "Leon", "le pinson");
  assert_int_equal(1, nbNode(root));

  // Inserer une deuxieme fois la meme cle ne cree pas de nouveau noeud
  root = insert(root, "Leon", "le pinson");
  assert_int_equal(1, nbNode(root));

  // Inserer une cle ("Leo") qui ressemble a une autre cle ("Leon")
  // cree bien un nouveau noeud
  root = insert(root, "Leo", "le bonobo");
  assert_int_equal(2, nbNode(root));
  
  // Verifie que la cle fournie est recopiee par insert, pas juste pointee
  // NB : l'erreur sera revelee avec l'utilisation de Valgrind.
  #define GERALDINE "Geraldine"
  char *keyGeraldine = malloc(strlen(GERALDINE) + 1);
  strcpy(keyGeraldine, GERALDINE);
  root = insert(root, keyGeraldine, "la sardine");
  free(keyGeraldine);
  keyGeraldine = NULL;
  assert_int_equal(3, nbNode(root));
  // L'insert permet d'acceder aux cles stockees dans l'arbre. Si le stockage
  // n'est pas correctement fait, valgrind signalera une tentative d'acces
  // a une zone de memoire liberee.
  root = insert(root, GERALDINE, "la sardine");

  // La liberation de l'arbre retire bien tous ses noeuds
  // NB : la liberation memoire de ses noeuds sera verifiee avec Valgrind
  root = release(root);
  assert_int_equal(0, nbNode(root));
}

static void test_release(void **state) {
  struct Node *root = NULL;
  
  // Test unitaire du comportement de release() quand root==NULL
  // TODO A completer

  // Tests unitaires des autres cas
  root = insert(root, "Leon", "le pinson");
  root = insert(root, "Leo", "le bonobo");
  root = insert(root, "Michelle", "la gazelle");
  root = insert(root, "Kelly", "la souris");
  root = insert(root, "Quentin", "le lamentin");

  root = release(root);
  assert_null(root);
}

static void test_getKeyHeight(void **state) {
  struct Node *root = NULL;

  // Test unitaire du comportement de getKeyHeight() quand root==NULL
  // TODO A completer

  // Tests unitaires des autres cas
  root = insert(root, "Leon", "le pinson");
  root = insert(root, "Leo", "le bonobo");
  root = insert(root, "Michelle", "la gazelle");

  assert_int_equal(2, getKeyHeight(root, "Leon"));
  assert_int_equal(1, getKeyHeight(root, "Leo"));
  assert_int_equal(1, getKeyHeight(root, "Michelle"));
  assert_int_equal(0, getKeyHeight(root, "CleInexistante"));

  root = release(root);
}

static void test_getKeyRank(void **state) {
  struct Node *root = NULL;

  // Test unitaire du comportement de getKeyRank() quand root==NULL
  // TODO A completer

  // Tests unitaires des autres cas
  root = insert(root, "Leon", "le pinson");
  root = insert(root, "Eric", "le porc-epic");
  root = insert(root, "Michelle",  "la gazelle");
  root = insert(root, "Bernadette", "la chouette");
  root = insert(root, "Leo", "le bonobo");
  root = insert(root, "Kelly", "la souris");
  root = insert(root, "Quentin", "le lamentin");

  assert_int_equal(-1, getKeyRank(root, "CleInexistante"));
  assert_int_equal(0, getKeyRank(root, "Bernadette"));
  assert_int_equal(1, getKeyRank(root, "Eric"));
  assert_int_equal(2, getKeyRank(root, "Kelly"));
  assert_int_equal(3, getKeyRank(root, "Leo"));
  assert_int_equal(4, getKeyRank(root, "Leon"));
  assert_int_equal(5, getKeyRank(root, "Michelle"));
  assert_int_equal(6, getKeyRank(root, "Quentin"));

  root = release(root);
}

static void test_equilibrageGaucheGauche(void **state) {
  struct Node *root = NULL;

  root = insert(root, "Michelle", "la gazelle");
  root = insert(root, "Quentin", "le lamentin");
  root = insert(root, "Kelly", "la souris");
  root = insert(root, "Leon", "le pinson");
  root = insert(root, "Eric", "le porc-epic");
  root = insert(root, "Bernadette", "la chouette");

  assert_int_equal(0, getKeyRank(root, "Bernadette"));
  assert_int_equal(1, getKeyRank(root, "Eric"));
  assert_int_equal(2, getKeyRank(root, "Kelly"));
  assert_int_equal(3, getKeyRank(root, "Leon"));
  assert_int_equal(4, getKeyRank(root, "Michelle"));
  assert_int_equal(5, getKeyRank(root, "Quentin"));

  assert_int_equal(3, getKeyHeight(root, "Kelly"));
  assert_int_equal(2, getKeyHeight(root, "Eric"));
  assert_int_equal(2, getKeyHeight(root, "Michelle"));
  assert_int_equal(1, getKeyHeight(root, "Bernadette"));
  assert_int_equal(1, getKeyHeight(root, "Leon"));
  assert_int_equal(1, getKeyHeight(root, "Quentin"));

  root = release(root);
}

static void test_equilibrageDroiteDroite(void **state) {
  struct Node *root = NULL;

  root = insert(root, "Eric", "le porc-epic");
  root = insert(root, "Bernadette", "la chouette");
  root = insert(root, "Leon", "le pinson");
  root = insert(root, "Kelly", "la souris");
  root = insert(root, "Michelle", "la gazelle");
  root = insert(root, "Quentin", "le lamentin");

  assert_int_equal(0, getKeyRank(root, "Bernadette"));
  assert_int_equal(1, getKeyRank(root, "Eric"));
  assert_int_equal(2, getKeyRank(root, "Kelly"));
  assert_int_equal(3, getKeyRank(root, "Leon"));
  assert_int_equal(4, getKeyRank(root, "Michelle"));
  assert_int_equal(5, getKeyRank(root, "Quentin"));

  assert_int_equal(3, getKeyHeight(root, "Leon"));
  assert_int_equal(2, getKeyHeight(root, "Eric"));
  assert_int_equal(2, getKeyHeight(root, "Michelle"));
  assert_int_equal(1, getKeyHeight(root, "Bernadette"));
  assert_int_equal(1, getKeyHeight(root, "Kelly"));
  assert_int_equal(1, getKeyHeight(root, "Quentin"));

  root = release(root);
}

static void test_equilibrageGaucheDroite(void **state) {
  struct Node *root = NULL;

  root = insert(root, "Michelle", "la gazelle");
  root = insert(root, "Quentin", "le lamentin");
  root = insert(root, "Eric", "le porc-epic");
  root = insert(root, "Bernadette", "la chouette");
  root = insert(root, "Leon", "le pinson");
  root = insert(root, "Kelly", "la souris");

  assert_int_equal(0, getKeyRank(root, "Bernadette"));
  assert_int_equal(1, getKeyRank(root, "Eric"));
  assert_int_equal(2, getKeyRank(root, "Kelly"));
  assert_int_equal(3, getKeyRank(root, "Leon"));
  assert_int_equal(4, getKeyRank(root, "Michelle"));
  assert_int_equal(5, getKeyRank(root, "Quentin"));

  assert_int_equal(3, getKeyHeight(root, "Leon"));
  assert_int_equal(2, getKeyHeight(root, "Eric"));
  assert_int_equal(2, getKeyHeight(root, "Michelle"));
  assert_int_equal(1, getKeyHeight(root, "Bernadette"));
  assert_int_equal(1, getKeyHeight(root, "Kelly"));
  assert_int_equal(1, getKeyHeight(root, "Quentin"));

  root = release(root);
}

static void test_equilibrageDroiteGauche(void **state) {
  struct Node *root = NULL;

  root = insert(root, "Eric", "le porc-epic");
  root = insert(root, "Bernadette", "la chouette");
  root = insert(root, "Michelle", "la gazelle");
  root = insert(root, "Quentin", "le lamentin");
  root = insert(root, "Leon", "le pinson");
  root = insert(root, "Kelly", "la souris");
  
  assert_int_equal(0, getKeyRank(root, "Bernadette"));
  assert_int_equal(1, getKeyRank(root, "Eric"));
  assert_int_equal(2, getKeyRank(root, "Kelly"));
  assert_int_equal(3, getKeyRank(root, "Leon"));
  assert_int_equal(4, getKeyRank(root, "Michelle"));
  assert_int_equal(5, getKeyRank(root, "Quentin"));

  assert_int_equal(3, getKeyHeight(root, "Leon"));
  assert_int_equal(2, getKeyHeight(root, "Michelle"));
  assert_int_equal(2, getKeyHeight(root, "Eric"));
  assert_int_equal(1, getKeyHeight(root, "Bernadette"));
  assert_int_equal(1, getKeyHeight(root, "Kelly"));
  assert_int_equal(1, getKeyHeight(root, "Quentin"));

  root = release(root);
}

static void test_getValue(void **state) {
  struct Node *root = NULL;

  root = insert(root, "Leon", "le pinson");
  root = insert(root, "Leo", "le bonobo");
  assert_null(getValue(root, "CleInexistante"));

  char *resultat = getValue(root, "Leon");
  assert_non_null(resultat);
  assert_string_equal("le pinson", resultat);
  
  resultat = getValue(root, "Leo");
  assert_non_null(resultat);
  assert_string_equal("le bonobo", resultat);


  root = insert(root, "Leon", "Leon");
  resultat = getValue(root, "Leon");
  assert_non_null(resultat);
  assert_string_equal("Leon", resultat);

  root = release(root);
}

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_insert_et_nbNode),
    cmocka_unit_test(test_release),
    cmocka_unit_test(test_getKeyHeight),
    cmocka_unit_test(test_getKeyRank),
    cmocka_unit_test(test_equilibrageGaucheGauche),
    cmocka_unit_test(test_equilibrageDroiteDroite),
    cmocka_unit_test(test_equilibrageGaucheDroite),
    cmocka_unit_test(test_equilibrageDroiteGauche),
    cmocka_unit_test(test_getValue),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
