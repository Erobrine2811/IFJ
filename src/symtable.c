/**
 * @file symtable.c
 *
 * IFJ25 project
 *
 * Implementation of a symbol table using an AVL tree.
 *
 * @author Lukáš Denkócy <xdenkol00>
 */

#include "symtable.h"

/**
 * Gets the height of a node.
 *
 * @param n The node.
 * @return The height of the node, or 0 if the node is NULL.
 */
int height(tSymNode *n)
{
    return n ? n->height : 0;
}

/**
 * Returns the maximum of two integers.
 *
 * @param a The first integer.
 * @param b The second integer.
 * @return The larger of the two integers.
 */
int max(int a, int b)
{
    return (a > b) ? a : b;
}

/**
 * Creates a new symbol table node.
 *
 * @param key The key for the new node.
 * @param data The data for the new node.
 * @return A pointer to the newly created node.
 */
tSymNode *create_node(const char *key, tSymbolData data)
{
    tSymNode *node = safeMalloc(sizeof(tSymNode));
    node->key = safeMalloc(strlen(key) + 1);
    strcpy(node->key, key);
    node->data = data;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

/**
 * Frees a single symbol table node and its associated data.
 *
 * @param node The node to free.
 */
void free_node(tSymNode *node)
{
    if (!node)
        return;
    free(node->data.unique_name);
    if (node->data.kind == SYM_FUNC)
    {
        free(node->data.paramTypes);
        if (node->data.paramNames)
        {
            for (int i = 0; i < node->data.paramCount; i++)
            {
                free(node->data.paramNames[i]);
            }
            free(node->data.paramNames);
        }
    }
    free(node->key);
    free(node);
}

/**
 * Performs a right rotation on a subtree rooted at y.
 *
 * @param y The root of the subtree to rotate.
 * @return The new root of the rotated subtree.
 */
tSymNode *rotate_right(tSymNode *y)
{
    tSymNode *x = y->left;

    y->left = x->right;
    x->right = y;

    y->height = 1 + max(height(y->left), height(y->right));
    x->height = 1 + max(height(x->left), height(x->right));

    return x;
}

/**
 * Performs a left rotation on a subtree rooted at x.
 *
 * @param x The root of the subtree to rotate.
 * @return The new root of the rotated subtree.
 */
tSymNode *rotate_left(tSymNode *x)
{
    tSymNode *y = x->right;

    x->right = y->left;
    y->left = x;

    x->height = 1 + max(height(x->left), height(x->right));
    y->height = 1 + max(height(y->left), height(y->right));

    return y;
}

/**
 * Calculates the balance factor of a node.
 *
 * @param n The node to check.
 * @return The balance factor (height of left subtree - height of right subtree).
 */
int get_balance(tSymNode *n)
{
    return n ? height(n->left) - height(n->right) : 0;
}

/**
 * Recursively inserts a new node into the AVL tree and performs rebalancing.
 *
 * @param node The current node in the recursion.
 * @param key The key to insert.
 * @param data The data for the new symbol.
 * @param inserted A pointer to a boolean that will be set to true on successful insertion.
 * @return The new root of the (potentially modified) subtree.
 */
static tSymNode *insert_rec(tSymNode *node, const char *key, tSymbolData data, bool *inserted)
{
    if (node == NULL)
    {
        *inserted = true;
        return create_node(key, data);
    }

    int cmp = strcmp(key, node->key);
    if (cmp < 0)
        node->left = insert_rec(node->left, key, data, inserted);
    else if (cmp > 0)
        node->right = insert_rec(node->right, key, data, inserted);
    else
    {
        *inserted = false;
        return node;
    }

    node->height = 1 + max(height(node->left), height(node->right));

    int balance = get_balance(node);

    if (balance > 1 && strcmp(key, node->left->key) < 0)
        return rotate_right(node);
    if (balance < -1 && strcmp(key, node->right->key) > 0)
        return rotate_left(node);
    if (balance > 1 && strcmp(key, node->left->key) > 0)
    {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (balance < -1 && strcmp(key, node->right->key) < 0)
    {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    return node;
}

/**
 * Recursively finds a symbol in a subtree by its key.
 *
 * @param node The current node in the recursion.
 * @param key The key of the symbol to find.
 * @return A pointer to the symbol's data if found, otherwise NULL.
 */
tSymbolData *find_rec(tSymNode *node, const char *key)
{
    if (!node)
        return NULL;
    int cmp = strcmp(key, node->key);
    if (cmp == 0)
        return &node->data;
    return (cmp < 0) ? find_rec(node->left, key) : find_rec(node->right, key);
}

/**
 * Compares the function names without considering the parameters count.
 *
 * @param s1 The first string.
 * @param s2 The second string.
 * @return 'true' if the function names are identical, 'false' otherwise.
 */
bool compare_function_names(const char *s1, const char *s2)
{
    if (s1 == NULL || s2 == NULL)
        return false;

    const char *at1 = strrchr(s1, '@');
    const char *at2 = strrchr(s2, '@');

    if (at1 == NULL && at2 == NULL)
        return strcmp(s1, s2) == 0;

    if (at1 == NULL || at2 == NULL)
        return false;

    size_t len1 = at1 - s1;
    size_t len2 = at2 - s2;

    if (len1 != len2)
        return false;

    if (len1 == 0)
        return true;

    return strncmp(s1, s2, len1) == 0;
}

/**
 * Recursively finds if a function with the same base name exists in the tree.
 *
 * @param node The current node in the recursion.
 * @param key The key of the function to find (mangled name).
 * @return True if a function with the same base name is found, false otherwise.
 */
bool find_function(tSymNode *node, const char *key)
{
    if (!node)
        return false;
    bool cmp = compare_function_names(key, node->key);
    int cmpFull = strcmp(key, node->key);
    if (cmp && node->data.kind == SYM_FUNC && node->data.defined)
        return true;
    return (cmpFull < 0) ? find_function(node->left, key) : find_function(node->right, key);
}

/**
 * Recursively frees all nodes in a subtree.
 *
 * @param node The root of the subtree to free.
 */
void free_rec(tSymNode *node)
{
    if (!node)
        return;
    free_rec(node->left);
    free_rec(node->right);
    free_node(node);
}

void symtable_init(tSymTable *t)
{
    t->root = NULL;
}

void symtable_free(tSymTable *t)
{
    free_rec(t->root);
    t->root = NULL;
}

bool symtable_insert(tSymTable *t, char *key, tSymbolData data)
{
    bool inserted = false;
    t->root = insert_rec(t->root, key, data, &inserted);
    return inserted;
}

tSymbolData *symtable_find(tSymTable *t, const char *key)
{
    return find_rec(t->root, key);
}

bool symtable_find_function(tSymTable *t, const char *key)
{
    return find_function(t->root, key);
}
