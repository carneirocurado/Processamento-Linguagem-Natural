#!/bin/bash

ARQ_IN=$1
ARQ_SAIDA="./corpus/"$ARQ_IN
DIV=$2
QTD_WORDS_TOT=$(more $ARQ_IN | wc -w)
QTD_DIVS=$(($QTD_WORDS_TOT/$DIV+1))

echo Quantidade de palavras $QTD_WORDS_TOT - Qtd divs: $QTD_DIVS

for ((i=1;i<=$QTD_DIVS;i++)); do
	QTD_WORDS=$(($i*$DIV))
	echo Loop $i de $QTD_DIVS - $QTD_WORDS palavras de $QTD_WORDS_TOT
	echo Arquivo Saida: $ARQ_SAIDA'_'$QTD_WORDS
	awk -v n=$QTD_WORDS 'n==c{exit}n-c>=NF{print;c+=NF;next}{for(i=1;i<=n-c;i++)printf "%s ",$i;print x;exit}' $ARQ_IN > $ARQ_SAIDA'_'$QTD_WORDS
done

#awk -v n=$1 'n==c{exit}n-c>=NF{print;c+=NF;next}{for(i=1;i<=n-c;i++)printf "%s ",$i;print x;exit}'

