#ifndef BPT_HPP
#define BPT_HPP

#include<iostream>
#include "../Constantes/cte.hpp"
#include <bits/stdc++.h>

typedef struct block{
    int offset;
}block;

typedef struct node{

    bool is_leaf; //verifica se é folha
    void **ptrs; //vetor para os nodos filhos, caso não seja folha 
    int *keys; //vetor para as chaves
    struct node *parent; //ponteiro para o nodo pai
    int num_keys; //número de chaves alocadas
    
} node;

typedef struct nodeDisk{

    bool is_leaf; //verifica se é folha
    unsigned int ptrs[2*m+1]; //vetor para os nodos filhos, caso não seja folha 
    unsigned int keys[2*m]; //vetor para as chaves
    unsigned int parent; //ponteiro para o nodo pai
    int num_keys; //número de chaves alocadas
    
} NodeDisk;

int cut(int size){
    if(size % 2 == 0){return size / 2;}
    else{return size/2 + 1;}
}

node *create_node(){
    
    node *new_node = (node *)malloc(sizeof(node));
    if(new_node == NULL) {
        perror("Erro ao criar novo nó\n");
        exit(EXIT_FAILURE);
    }

    new_node->keys = (int*) malloc((m*2)*sizeof(int));
    if(new_node == NULL) {
        perror("Erro ao criar vetor de chaves do novo nó\n");
        exit(EXIT_FAILURE);
    }

    new_node->ptrs = (void**) malloc((2*m + 1)*sizeof(void*));
    if(new_node->ptrs == NULL) {
        perror("Erro ao criar vetor de ponteiros do nó\n");
        exit(EXIT_FAILURE);
    }

    new_node->is_leaf = false;
    new_node->parent = NULL;
    new_node->num_keys = 0;

    return new_node;
}

node *create_leaf(){
    node *leaf = create_node();
    leaf->is_leaf = true;
    return leaf;
}

node *create_tree(int key, block *b){

    node *root = create_leaf();
    root->keys[0] = key;
    root->ptrs[0] = b;
    root->parent = NULL;
    root->num_keys++;
    root->ptrs[2*m] = NULL; //seta o utlimo ponteiro para NULL, já que nao tem filho

    return root;
}

block *create_block(int offset){
    block *b = (block*) malloc(sizeof(block));
    if(b == NULL){
        perror("Erro ao criar bloco !\n");
        exit(EXIT_FAILURE);
    }
    else{b -> offset = offset;}
    return b;
}

node *find_leaf(node* root, int key){

    int i = 0;
    node *current = root; //começa a busca na raiz

    while(!current->is_leaf){     // enquanto nó atual não for folha
        i = 0;
        while(i < current->num_keys){
            if(key >= current->keys[i]){i++;}
            else{break;}
        }
        current = (node *)current->ptrs[i];
    }
    return current;
}

node *insert_leaf(node *leaf, int key, block* b){

    for(int i = 0; i < leaf->num_keys; i++){
        if(key < leaf->keys[i]){
            for(int j = leaf->num_keys; j > i ; j--){
                leaf->keys[j] = leaf->keys[j - 1];
                leaf->ptrs[j] = leaf->ptrs[j-1];
            }
            leaf->keys[i] = key;
            leaf->ptrs[i] = b; 
            leaf->num_keys++;
            return leaf;
        }
    }
    leaf->keys[leaf->num_keys] = key;
    leaf->ptrs[leaf->num_keys] = b;
    leaf->num_keys++;
    return leaf;
}

node *insert_new_root(node *left, int key, node *right){

    node *root = create_node();
    root->keys[0] = key;
    root->ptrs[0] = left;
    root->ptrs[1] = right;
    root->num_keys++;
    root->parent = NULL;
    left->parent = root;
    right->parent = root;
    return root;
}

int get_left_index(node *parent, node *left){
    int r = 0;
    while(r <= parent->num_keys && (node*) parent->ptrs[r] != left){r++;}
    return r;
}

node *insert_node(node *root, node *n, int left_index, int key, node *right){

    for(int i = n->num_keys; i > left_index; i--){
        n->ptrs[i + 1] = n->ptrs[i];
        n->keys[i] = n->keys[i-1];
    }

    n->ptrs[left_index + 1] = right;
    n->keys[left_index] = key;
    n->num_keys++;
    return root;
}

node *insert_parent(node* root, node *left, int key, node *right);

node *insert_node_after_split(node *root, node *old_node, int left_index, int key, node *right){

    int *temp_keys; //vetor temporário para guardar as chaves
    void **temp_ptrs; // vetor temporário para guardar os ponteiros dos blocks

    temp_keys = (int *) malloc((2*m+1)*sizeof(int));
    if(temp_keys == NULL){
        perror("Erro ao criar vetor de chaves temporário !\n");
        exit(EXIT_FAILURE);
    }

    temp_ptrs = (void**)malloc((2*m+2)*sizeof(void *));
    if(temp_ptrs == NULL){
        perror("Erro ao criar vetor de ponteiros para blocks temporário !\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) {
        if (j == left_index + 1){j++;}
        temp_ptrs[j] = old_node->ptrs[i];
    }

    for (int i = 0, j = 0; i < old_node->num_keys; i++, j++) {
        if (j == left_index){j++;}
        temp_keys[j] = old_node->keys[i];
    }

    temp_ptrs[left_index + 1] = right;
    temp_keys[left_index] = key;

    int split = cut(2*m+1);
    node * new_node = create_node();
    old_node->num_keys = 0;
    int i = 0;
    for (i = 0; i < split - 1; i++) {
        old_node->ptrs[i] = temp_ptrs[i];
        old_node->keys[i] = temp_keys[i];
        old_node->num_keys++;
    }
    old_node->ptrs[i] = temp_ptrs[i];
    int k_prime = temp_keys[split - 1];
    int j = 0;
    for (++i, j = 0; i < 2*m+1; i++, j++) {
        new_node->ptrs[j] = temp_ptrs[i];
        new_node->keys[j] = temp_keys[i];
        new_node->num_keys++;
    }
    new_node->ptrs[j] = temp_ptrs[i];

    free(temp_keys);
    free(temp_ptrs);
    
    new_node->parent = old_node->parent;
    for (i = 0; i <= new_node->num_keys; i++) {
        node *child = (node *)new_node->ptrs[i];
        child->parent = new_node;
    }

    return insert_parent(root, old_node, k_prime, new_node);
}

node *insert_parent(node* root, node *left, int key, node *right){

    node *parent = left->parent;

    if(parent == NULL){return insert_new_root(left, key, right);}

    int left_index = get_left_index(parent, left); //indice do ponteiro do nó à esquerda

    if(parent->num_keys < 2*m){
        return insert_node(root, parent, left_index, key, right);
    }

    return insert_node_after_split(root, parent, left_index, key, right);

}

node *insert_leaf_after_split(node *root, node *leaf, int key, block *b){

    node *new_leaf = create_leaf(); //cria nova folha para o split
    int *temp_keys; //vetor temporário para guardar as chaves
    void **temp_ptrs; // vetor temporário para guardar os ponteiros dos blocks

    temp_keys = (int *) malloc((2*m+1)*sizeof(int));
    if(temp_keys == NULL){
        perror("Erro ao criar vetor de chaves temporário !\n");
        exit(EXIT_FAILURE);
    }

    temp_ptrs = (void **) malloc((2*m+1)*sizeof(void *));
    if(temp_ptrs == NULL){
        perror("Erro ao criar vetor de ponteiros para blocks temporário !\n");
        exit(EXIT_FAILURE);
    }

    int insert = 0;
    while(insert < 2*m && key > leaf->keys[insert]){
        insert ++; // enquanto não chegar ao fim e a chave na posição insert for >= a chave nova, avança
    }

    for(int i, j = 0 ; i < leaf->num_keys; i++, j++){
        if(j == insert){j++;}

        temp_keys[j] = leaf->keys[i];
        temp_ptrs[j] = leaf->ptrs[i]; 
    }

    temp_keys[insert] = key;
    temp_ptrs[insert] = b;

    leaf->num_keys = 0;

    int split = cut(2*m);

    for(int i = 0; i < split; i++){
        leaf->keys[i] = temp_keys[i];
        leaf->ptrs[i] = temp_ptrs[i];
        leaf->num_keys++;
    }

    for(int i = split, j = 0; i < 2*m + 1; i++, j++){
        new_leaf->keys[j] = temp_keys[i];
        new_leaf -> ptrs[j] = temp_ptrs[i];
        new_leaf -> num_keys++;
    }

    free(temp_keys);
    free(temp_ptrs);

    new_leaf->ptrs[2*m] = leaf->ptrs[2*m];
    leaf->ptrs[2*m] = new_leaf;

    for(int i = leaf->num_keys; i <  2*m; i++){leaf->ptrs[i] = NULL;}

    for (int i = new_leaf->num_keys; i < 2*m; i++){new_leaf->ptrs[i] = NULL;}

    new_leaf->parent = leaf->parent;
    int new_key = new_leaf->keys[0];

    return insert_parent(root, leaf, new_key, new_leaf);
}

node *insert(node *root, int key, int offset){

    block *b = create_block(offset); //ponteiro onde o b esta alocado
    node *leaf = find_leaf(root, key); // acha a folha onde a chave com o offset do bloco vao ser inseridos

    if(leaf -> num_keys < 2*m){ //ainda tem espaço na folha
        leaf = insert_leaf(leaf, key, b);
        return root;
    }
    return insert_leaf_after_split(root, leaf, key, b);
}

int pathToLeaves(node *const root, node *child) {
  int length = 0;
  node *c = child;
  while (c != root) {
    c = c->parent;
    length++;
  }
  return length;
}

void imprime_node(NodeDisk no){

    cout << "is_leaf: " << no.is_leaf << endl;
    cout << "num_keys: " << no.num_keys << endl;
    cout << "parent: " << no.parent << endl;
    //cout << "next: " << no.next <<  endl;
    if(no.num_keys > 0){
        for(int i = 0; i < no.num_keys; i++){
            cout << "key " << i << " = " << no.keys[i] << endl;
        }
        for(int i = 0; i <= no.num_keys; i++){
            int x = no.ptrs[i];
            cout << "block " << i << " = " << x << endl;
        }
    }
}

void printTree(node *const root){

    queue<node*> q;
    node *c = NULL;
    int level = 0;
    int new_level = 0;

    q.push(root);

    while(!q.empty()){
        c = q.front();
        q.pop();

        if (c->parent != NULL && c == c->parent->ptrs[0]) {
            new_level = pathToLeaves(root, c);
            if (new_level != level) {
                level = new_level;
                printf("\n");
            }
        }
        for(int i = 0; i < c->num_keys; i++){
            cout << c->keys[i] << " ";
        }
        if(!c->is_leaf){
            for(int i = 0; i <= c->num_keys; i++){
                q.push((node *)c->ptrs[i]);
            }
        }
        cout << "| ";
    }
    cout << endl;
}

unsigned int gravaTree(node *root, unsigned int parent, FILE *arq){
    
    NodeDisk temp;
    fseek(arq, 0, SEEK_END);
    unsigned int pos = ftell(arq);
    temp.is_leaf = root->is_leaf;
    temp.num_keys = root->num_keys;
    temp.parent = parent;

    for (unsigned int i=0; i < root->num_keys; i++)
        temp.keys[i] = root->keys[i];

    fwrite(&temp, sizeof(NodeDisk), 1, arq);

    if (root->is_leaf) return pos;

    for (unsigned int i = 0; i <= root->num_keys; i++) {
        temp.ptrs[i] = gravaTree((node *)root->ptrs[i], pos, arq);
    }

    fseek(arq, pos, SEEK_SET);
    fwrite(root, sizeof(node), 1, arq);

    return pos;
}

























#endif