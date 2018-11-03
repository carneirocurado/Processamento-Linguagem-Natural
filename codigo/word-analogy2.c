//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

const long long max_size = 2000;         // max length of strings
const long long N = 1000;                // number of closest words that will be shown
const long long max_w = 50;              // max length of vocabulary entries
const long long max_cat = 100;           // quantidade maxima de categorias de palavras que serao tratadas

int read_words(char st[100][max_size]);
int find_words (char st[100][max_size], long long *bi, char *vocab, long long words);
int find_nearest (long long words, float *vec, char *vocab, float *M, long long *bi, long long size, float *bestd, char bestw[N][max_size]);

int main(int argc, char **argv) {
  FILE *f, *arq_out;
  time_t hora, hora_aux;
  struct tm *hora_local;
//  char st1[max_size];
  char bestw[N][max_size], cat_name[max_cat][max_w];
  char wordspace_file[max_size], arq_saida_file[max_size], st[100][max_size];
  float erro, mse, rmsd, cat_mse[max_cat], cat_rmsd[max_cat], len, bestd[N], vec[max_size];
  long long words, size, a, b, cn, bi[100], qtde_out_dictionary, qtde_out_score, cat_qtde_out_dictionary[max_cat], cat_qtde_out_score[max_cat], cat_qtde_total[max_cat], qtde_total, qtde_cat;
  float *M;
  char *vocab;
  if (argc < 3) {
    printf("Utilizacao: ./word-analogy2 <WORDSPACE> <ARQ_SAIDA> < <ARQ_ENTRADA>\nonde:\n\tWORDSPACE contem projecoes de palavras no formato binario\n\tARQ_SAIDA indica o nome do arquivo de saida para armazenar o resultado do processamento\n\t< ARQ_ENTRADA indica o redirecionamento da entrada padrao para um arquivo com dados a serem processados.");
    return 0;
  }
  strcpy(wordspace_file, argv[1]);
  f = fopen(wordspace_file, "rb");
  if (f == NULL) {
    printf("Arquivo %s nao encontrado.\n", argv[1]);
    return -1;
  }
  strcpy(arq_saida_file, argv[2]);
  arq_out = fopen(arq_saida_file, "a");
  if (f == NULL) {
    printf("Arquivo %s nao encontrado.\n", argv[2]);
    return -1;
  }
  fscanf(f, "%lld", &words);
  fscanf(f, "%lld", &size);
  vocab = (char *)malloc((long long)words * max_w * sizeof(char));
  M = (float *)malloc((long long)words * (long long)size * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
    return -1;
  }
  for (b = 0; b < words; b++) {
    a = 0;
    while (1) {
      vocab[b * max_w + a] = fgetc(f);
      if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
      if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
    }
    vocab[b * max_w + a] = 0;
    for (a = 0; a < max_w; a++) vocab[b * max_w + a] = toupper(vocab[b * max_w + a]);
    for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, f);
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++) M[a + b * size] /= len;
  }
  fclose(f);

  mse = 0;
  qtde_out_dictionary = 0;
  qtde_out_score = 0;
  qtde_total = 0;
  qtde_cat = -1;
  hora = time(NULL);
  if ( hora == (time_t)-1 ) printf ("WARNING: Erro resgatando hora local\n");
  hora_local = localtime(&hora);
  printf ("%d-%02d-%02d %02d:%02d:%02d - Iniciando Processamento\n", hora_local->tm_year+1900, hora_local->tm_mon+1, hora_local->tm_mday, hora_local->tm_hour, hora_local->tm_min, hora_local->tm_sec);
  if ( ftell(arq_out) == 0 )
    if ( (fprintf(arq_out, "wordspace;categoria;qtde_total;qtde_analisada;qtde_fora_dicionario;qtde_fora_ranking;mse;rmsd\n")) < 0 ) {
      printf ("\nERROR: Falha escrevendo no arquivo de Saida\n");
      return -1;
    }
  while (1) {
    cn = read_words(st);
    if ( cn < 0 ) {
      if ( qtde_cat >= 0 ) {
        // Calculando o MSE e o RMSD do processamento da categoria anterior
        if ( cat_qtde_total[qtde_cat] > 0 ) {
          cat_mse[qtde_cat] /= cat_qtde_total[qtde_cat];
          cat_rmsd[qtde_cat] = sqrtf(cat_mse[qtde_cat]);
        }
        else {
          cat_mse[qtde_cat] = 0;
          cat_rmsd[qtde_cat] = 0;
        }
        printf ("\rTermino da Categoria: %s - Quantidade Total: %lld - Analisada: %lld (%lld fora do dicionario e %lld fora do ranking) - MSE: %f - RMSD: %f          \n", cat_name[qtde_cat], cat_qtde_total[qtde_cat]+cat_qtde_out_dictionary[qtde_cat]+cat_qtde_out_score[qtde_cat], cat_qtde_total[qtde_cat], cat_qtde_out_dictionary[qtde_cat], cat_qtde_out_score[qtde_cat], cat_mse[qtde_cat], cat_rmsd[qtde_cat]);

        if ( (fprintf(arq_out, "%s;%s;%lld;%lld;%lld;%lld;%f;%f\n", wordspace_file, cat_name[qtde_cat], cat_qtde_total[qtde_cat]+cat_qtde_out_dictionary[qtde_cat]+cat_qtde_out_score[qtde_cat], cat_qtde_total[qtde_cat], cat_qtde_out_dictionary[qtde_cat], cat_qtde_out_score[qtde_cat], cat_mse[qtde_cat], cat_rmsd[qtde_cat])) < 0 ) {
          printf ("\nERROR: Falha escrevendo no arquivo de Saida\n");
          return -1;
        }
      }
      break;
    }
    if ( !cn ) {
      if ( qtde_cat >= 0 ) {
        // Calculando o MSE e o RMSD do processamento da categoria anterior
        if ( cat_qtde_total[qtde_cat] > 0 ) {
          cat_mse[qtde_cat] /= cat_qtde_total[qtde_cat];
          cat_rmsd[qtde_cat] = sqrtf(cat_mse[qtde_cat]);
        }
        else {
          cat_mse[qtde_cat] = 0;
          cat_rmsd[qtde_cat] = 0;
        }
        printf ("\rTermino da Categoria: %s - Quantidade Total: %lld - Analisada: %lld (%lld fora do dicionario e %lld fora do ranking) - MSE: %f - RMSD: %f          \n", cat_name[qtde_cat], cat_qtde_total[qtde_cat]+cat_qtde_out_dictionary[qtde_cat]+cat_qtde_out_score[qtde_cat], cat_qtde_total[qtde_cat], cat_qtde_out_dictionary[qtde_cat], cat_qtde_out_score[qtde_cat], cat_mse[qtde_cat], cat_rmsd[qtde_cat]);

        if ( (fprintf(arq_out, "%s;%s;%lld;%lld;%lld;%lld;%f;%f\n", wordspace_file, cat_name[qtde_cat], cat_qtde_total[qtde_cat]+cat_qtde_out_dictionary[qtde_cat]+cat_qtde_out_score[qtde_cat], cat_qtde_total[qtde_cat], cat_qtde_out_dictionary[qtde_cat], cat_qtde_out_score[qtde_cat], cat_mse[qtde_cat], cat_rmsd[qtde_cat])) < 0 ) {
          printf ("\nERROR: Falha escrevendo no arquivo de Saida\n");
          return -1;
        }
      }
      qtde_cat++;
      if ( qtde_cat > max_cat ) {
        qtde_cat = max_cat;
        printf ("\nWARNING: Quantidade maxima de categorias estourada!\n");
      }

      hora = time(NULL);
      if ( hora == (time_t)-1 ) printf ("\nWARNING: Erro resgatando hora local\n");
      hora_local = localtime(&hora);
      printf ("-----------------------------\n%d-%02d-%02d %02d:%02d:%02d - Iniciando Categoria: %s\n", hora_local->tm_year+1900, hora_local->tm_mon+1, hora_local->tm_mday, hora_local->tm_hour, hora_local->tm_min, hora_local->tm_sec, st[0]);

      strncpy(cat_name[qtde_cat],st[0],max_w-1);
      cat_name[qtde_cat][max_w-1] = '\0';
      cat_mse[qtde_cat] = 0;
      cat_rmsd[qtde_cat] = 0;
      cat_qtde_total[qtde_cat] = 0;
      cat_qtde_out_score[qtde_cat] = 0;
      cat_qtde_out_dictionary[qtde_cat] = 0;
      continue;
    }

    qtde_total += 1;
    cat_qtde_total[qtde_cat]++;

    hora_aux = time(NULL);
    if ( hora_aux == (time_t)-1 ) printf ("\nWARNING: Erro resgatando hora local\n");
    hora_local = localtime(&hora_aux);
    printf ("\r%d-%02d-%02d %02d:%02d:%02d - Quantidade Total: %lld - Analisada: %lld (%lld fora do dicionario e %lld fora do ranking) - tempo de processamento na categoria %f seg", hora_local->tm_year+1900, hora_local->tm_mon+1, hora_local->tm_mday, hora_local->tm_hour, hora_local->tm_min, hora_local->tm_sec, cat_qtde_total[qtde_cat]+cat_qtde_out_dictionary[qtde_cat]+cat_qtde_out_score[qtde_cat], cat_qtde_total[qtde_cat], cat_qtde_out_dictionary[qtde_cat], cat_qtde_out_score[qtde_cat], difftime(hora_aux,hora));
    fflush(stdout);


    if ( find_words(st, bi, vocab, words) <= 0 ) {
      qtde_out_dictionary++;
      cat_qtde_out_dictionary[qtde_cat]++;
      qtde_total--;
      cat_qtde_total[qtde_cat]--;
      continue;
    }

    find_nearest (words, vec, vocab, M, bi, size, bestd, bestw);

    erro = -1;
    for (a = 0; a < N; a++) {
      if (!strcmp(bestw[a], st[3])) {
        erro = powf(bestd[a] - bestd[0],2);
        mse += erro;
        cat_mse[qtde_cat] += erro;
        break;
      }
    }
    if ( erro < 0 ) {
      qtde_out_score++;
      cat_qtde_out_score[qtde_cat]++;
      qtde_total--;
      cat_qtde_total[qtde_cat]--;
    }
  }

  if ( qtde_total > 0 ) {
    mse /= qtde_total;
    rmsd = sqrtf(mse);
  }
  else {
    mse = 0;
    rmsd = 0;
  }

  printf ("-----------------------------\n%d-%02d-%02d %02d:%02d:%02d - Termino da Execucao - Quantidade Total: %lld - Analisada: %lld (%lld fora do dicionario e %lld fora do ranking) - MSE: %f - RMSD: %f\n-----------------------------\n", hora_local->tm_year+1900, hora_local->tm_mon+1, hora_local->tm_mday, hora_local->tm_hour, hora_local->tm_min, hora_local->tm_sec, qtde_total+qtde_out_score+qtde_out_dictionary, qtde_total, qtde_out_dictionary, qtde_out_score, mse, rmsd);

  if ( (fprintf(arq_out, "%s;TOTAL;%lld;%lld;%lld;%lld;%f;%f\n", wordspace_file, qtde_total+qtde_out_score+qtde_out_dictionary, qtde_total, qtde_out_dictionary, qtde_out_score, mse, rmsd)) < 0 ) {
    printf ("\nERROR: Falha escrevendo no arquivo de Saida\n");
    return -1;
  }

  fclose(arq_out);

  return 0;
}

//**********************
// Codigos de retorno:
// -1: Final da Execucao
// 0: Termino da leitura de palavras no Grupo
// >0: Quantidade de palavras lidas.
int read_words(char st[100][max_size]) {
  long long a, cn;

  // Leitura da primeira palavra e validacao das condicoes de saida
  scanf("%s", st[0]);
  for (a = 0; a < strlen(st[0]); a++) st[0][a] = toupper(st[0][a]);
  if ((!strcmp(st[0], ":")) || (!strcmp(st[0], "EXIT")) || feof(stdin)) {
    if ( (!strcmp(st[0], "EXIT")) || (feof(stdin)) ) return -1;
    scanf("%s", st[0]);
    if ( (!strcmp(st[0], "EXIT")) || (feof(stdin)) ) return -1;
    return 0;
  }

  // Leitura das palavras seguintes
  for (cn = 1; cn < 4; cn++) {
    scanf("%s", st[cn]);
    if ((!strcmp(st[cn], ":")) || (!strcmp(st[cn], "EXIT")) || feof(stdin)) {
      printf("\nERROR: Foram lidas %lld palavras, enquanto o esperado eram 4.\n", cn);
      return -1;
    }
    for (a = 0; a < strlen(st[cn]); a++) st[cn][a] = toupper(st[cn][a]);
  }

  return 1;

}


int find_words (char st[100][max_size], long long *bi, char *vocab, long long words) {
//  long long a = 0, b = 0, cn = 0;
  long long cn, b;

  b = cn = 0;

  // Procura pelas palavras no dicionario
  for (cn = 0; cn < 4; cn++) {
    for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[cn])) break;
    bi[cn] = b;
    if (bi[cn] == words) {
      bi[cn] = 0;
      return 0;
    }
//    else
//      printf("\nWord: %s  Position in vocabulary: %lld\n", st[cn], bi[cn]);
  }

  return 1;
}


int find_nearest (long long words, float *vec, char *vocab, float *M, long long *bi, long long size, float *bestd, char bestw[N][max_size]) {
  long long a, b, c, d;
  float len, dist;

  // Calculando o vetor resultante
  for (a = 0; a < size; a++) vec[a] = M[a + bi[1] * size] - M[a + bi[0] * size] + M[a + bi[2] * size];
  // Normalizando o vetor
  len = 0;
  for (a = 0; a < size; a++) len += vec[a] * vec[a];
  len = sqrt(len);
  for (a = 0; a < size; a++) vec[a] /= len;

  for (a = 0; a < N; a++) bestd[a] = 0;
  for (a = 0; a < N; a++) bestw[a][0] = 0;
  for (c = 0; c < words; c++) {
    if (c == bi[0]) continue;
    if (c == bi[1]) continue;
    if (c == bi[2]) continue;
    a = 0;
    for (b = 0; b < 3; b++) if (bi[b] == c) a = 1;
    if (a == 1) continue;
    dist = 0;
    for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
    for (a = 0; a < N; a++) {
      if (dist > bestd[a]) {
        for (d = N - 1; d > a; d--) {
          bestd[d] = bestd[d - 1];
          strcpy(bestw[d], bestw[d - 1]);
        }
        bestd[a] = dist;
        strcpy(bestw[a], &vocab[c * max_w]);
        break;
      }
    }
  }
  return 1;
}
