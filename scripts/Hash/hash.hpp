#ifndef HASH_HPP
#define HASH_HPP

#include "../Bucket/bucket.hpp"
#include "../B_Plus_Tree/bpt.hpp"
#include <climits> // pelo INT_MAX
#include <string>

using namespace std;

//Estrutura da Hash, que é composta por um vetor de Buckets
typedef struct HashTable{
  Bucket *table[HASH_SIZE];
} HashTable;

//Função que cria a Hash e a insere vazia no arquivo
HashTable* createHash(ofstream &binDataFile){
  HashTable *hashTable = new HashTable();
  for(int i = 0; i < HASH_SIZE; i++){
    hashTable->table[i] = createBucket(binDataFile);
  }
  return hashTable;
}

//Função que transforma a chave em um índice da Hash. Essa função é baseada na MurMurHash
int hashFunction(int key) {
    uint32_t ukey = static_cast<uint32_t>(key);

    ukey ^= ukey >> 16;
    ukey *= 0x85ebca6b;
    ukey ^= ukey >> 13;
    ukey *= 0xc2b2ae35;
    ukey ^= ukey >> 16;

    int result = static_cast<int>(ukey % HASH_SIZE);
    return result;
}

int hashStringFunction(string str){
  unsigned long hash = 0;

  for (char c : str)
      hash = c + (hash << 6) + (hash << 16) - hash;

  return hash % (INT_MAX + 1ULL);
}

//Função que insere o Registro na Hash
pair<node*, node*> insertRegistroHashTable(Registro registro, ofstream &dataFileWrite, ifstream &dataFileRead, node *root1, node *root2){
  //Descobre a chave do registro na Hash (seu índice no vetor)
  int key = hashFunction(registro.id);

  //Itera em cada bloco do Bucket
  for(int blocoId = 0; blocoId < BUCKET_SIZE; blocoId++){

    //Calcula o endereço do bloco
    int blockAddress = (blocoId * BLOCO_SIZE) + (key * BUCKET_SIZE * BLOCO_SIZE);

    //Carrega o bloco para a memória RAM
    Bloco *bloco = loadBloco(blockAddress, dataFileRead);
    
    //Verifica se o registro cabe no bloco. Se couber, insere o registro no bloco e retorna true. Se não couber, deleta o bloco da RAM e carrega o próximo
    if(bloco->header.espacoLivre >= (registro.tamanhoRegistro + sizeof(int))){ //Somamos sizeof(int) para considerar o offset que o registro terá ao ser inserido no bloco
      //Inserir nas árvores
      if(root1 == nullptr && root2 == nullptr){
        //Criando árvore 1
        block *b1 = create_block(blockAddress);
        root1 = create_tree(registro.id, b1);

        //Criando árvore 2
        block *b2 = create_block(blockAddress);
        root2 = create_tree(hashStringFunction(registro.titulo), b2);
      }else{
        root1 = insert(root1, registro.id, blockAddress);
        root2 = insert(root2, hashStringFunction(registro.titulo), blockAddress);
      }

      //Inserindo o registro no bloco
      writeRegistroBucket(blockAddress, bloco, registro, dataFileWrite);
      
      //Apagamos o bloco da memória RAM
      delete bloco;
      return make_pair(root1, root2);
    }
    delete bloco; // Liberando o bloco antes de carregar o próximo
    bloco = NULL;
  }
  
  return make_pair(root1, root2);
}

//Busca um registro na Hash
Registro * searchRegistroById(int registroId, ifstream &dataFileRead){
  //Descobre a chave do registro na Hash (seu índice no vetor)
  int key = hashFunction(registroId);
  int blockAddress;
  Bloco *bloco = NULL;
  Registro *registro = NULL;

  //Itera em cada bloco do Bucket
  for(int blocoId = 0; blocoId < BUCKET_SIZE; blocoId++){
    //Calcula o endereço do bloco
    blockAddress = (blocoId * BLOCO_SIZE) + (key * BUCKET_SIZE * BLOCO_SIZE);

    //Carrega o bloco para a memória RAM
    bloco = loadBloco(blockAddress, dataFileRead);

    //Verifica se o bloco está vazio. Se estiver, retorna NULL. Pois se chegamos até um bloco que o registro deveria estar, mas ele está vazio, quer dizer que não existe o registro
    if(bloco->header.numeroRegistros == 0) {
      delete bloco;
      return NULL;
    }

    //Busca o registro no bloco
    registro = searchRegistroBloco(bloco, registroId);
    
    //Se o registro foi encontrado, retorna o registro
    if(registro != NULL){
      cout << endl << "Foi percorrido " << blocoId+1 << " blocos para encontrar o Registro!" << endl;
      //imprimir a quantidade total de blocos no arquivo de dados
      cout << endl << "O arquivo de dados contém " << (BUCKET_SIZE * HASH_SIZE) << " blocos!" << endl;
      return registro;
    }
  }
  //Se o registro não foi encontrado, retorna NULL
  delete bloco;
  return NULL;
}

#endif