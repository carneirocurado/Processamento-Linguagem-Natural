#!/bin/bash

ARQ_IN=$1
ARQ_SAIDA=$2

timestamp(){
	date +"%Y-%m-%d %T"
}

printf "\n%s: Iniciando Execucao..." "$(timestamp)"

QTDE_FILES=$(ls -1 ./corpus | wc -l)
qtde_processada=0

for arquivo in ./corpus/*; do
	qtde_processada=$(($qtde_processada+1))
	printf "\n\n%s: Corpus de Entrada: %s - %d de %d\n" "$(timestamp)" "$(basename $arquivo)" "$qtde_processada" "$QTDE_FILES"
	for ((i=1; i<=50; i++)); do
		if [ $i -gt 3 ]
		then
			i=$(($i+4))
			printf "\n\n%s: Termino da Execucao\n" "$(timestamp)"
			exit 0
		fi
		echo
		ARQ_WORDSPACE=./wordspaces/$(basename $arquivo)_cbow_context-$i.bin
		printf "\n------------------------------\n%s: Iniciando Construcao do Wordspace: %s\n" "$(timestamp)" "$ARQ_WORDSPACE"
		./word2vec/word2vec -train $arquivo -output $ARQ_WORDSPACE -cbow 1 -size 200 -window $i -negative 25 -threads 20 -binary 1 -iter 15
		printf "\n\n%s: Iniciando Analise: %s\n" "$(timestamp)" "$ARQ_WORDSPACE"
		./word2vec/word-analogy2 $ARQ_WORDSPACE $ARQ_SAIDA < $ARQ_IN

		ARQ_WORDSPACE=./wordspaces/$(basename $arquivo)_skip-gram_context-$i.bin
		printf "\n------------------------------\n%s: Iniciando Construcao do Wordspace: %s\n" "$(timestamp)" "$ARQ_WORDSPACE"
		./word2vec/word2vec -train $arquivo -output $ARQ_WORDSPACE -cbow 0 -size 200 -window $i -negative 25 -threads 20 -binary 1 -iter 15
		printf "\n\n%s: Iniciando Analise: %s\n" "$(timestamp)" "$ARQ_WORDSPACE"
		./word2vec/word-analogy2 $ARQ_WORDSPACE $ARQ_SAIDA < $ARQ_IN

	done
done

printf "\n\n%s: Termino da Execucao\n" "$(timestamp)"
