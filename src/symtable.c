#include "symtable.h"

int height(tSymNode *n) { return n ? n->height : 0; }

int max(int a, int b) { return (a > b) ? a : b; }

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

void free_node(tSymNode *node)
{
    if (!node) return;
    free(node->key);
    if (node->data.kind == SYM_FUNC) {
        free(node->data.paramTypes);
        if (node->data.paramNames) {
            for (int i = 0; i < node->data.paramCount; i++) {
                free(node->data.paramNames[i]);
            }
            free(node->data.paramNames);
        }
    }
    free(node);
}

tSymNode *rotate_right(tSymNode *y)
{
    tSymNode *x = y->left;

    y->left = x->right;
    x->right = y;

    y->height = 1 + max(height(y->left), height(y->right));
    x->height = 1 + max(height(x->left), height(x->right));

    return x;
}

tSymNode *rotate_left(tSymNode *x)
{
    tSymNode *y = x->right;

    x->right = y->left;
    y->left = x;

    x->height = 1 + max(height(x->left), height(x->right));
    y->height = 1 + max(height(y->left), height(y->right));

    return y;
}

int get_balance(tSymNode *n) {
    return n ? height(n->left) - height(n->right) : 0;
}

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

tSymbolData *find_rec(tSymNode *node, const char *key) 
{
    if (!node) return NULL;
    int cmp = strcmp(key, node->key);
    if (cmp == 0) return &node->data;
    return (cmp < 0) ? find_rec(node->left, key) : find_rec(node->right, key);
}

void free_rec(tSymNode *node) 
{
    if (!node) return;
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

tSymbolData *symtable_find(tSymTable *t, char *key) 
{
    return find_rec(t->root, key);
}
