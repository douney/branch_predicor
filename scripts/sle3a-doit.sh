#!/bin/bash
# Production et génération bench par bench des graphes des résultats des prédicteurs de branchement
# (c) Frédéric Pétrot <frederic.petrot@univ-grenoble-alpes.fr>
# This software is open source, feel free to use it and modify it at your own risk
##################################################################################

num_parallel_jobs=8

# Paramètres que l'on cherche à faire varier
count_sizes=($(seq 1 3))
array_sizes=(4 6 8 10 12 14 16)

if test 1 -ne 0 ; then
   # Nombre de bits du compteur
   for i in ${count_sizes[*]} ; do
      # Taille du tableau
      for j in ${array_sizes[*]} ; do
         ./runall.pl -s ../sim/predictor -w sle -f  $num_parallel_jobs -d ../results/${i}-${j} -p $j $i
      done
   done
fi

# Liste des benchs
benchs=$(ls -1 ../results/1-4 | awk -F\. '{print $1}')

# Fichier temporaire
# tmpfile=$(mktemp)
tmpfile=1
rm -f 1

for i in ${count_sizes[*]} ; do
   for j in ${array_sizes[*]} ; do
      ./getdata.pl -w sle -d ../results/${i}-${j} >> ${tmpfile}_$i
   done
done

# Format des résultats
# ResultDirs ==>                	./results/2-16	
# SHORT_MOBILE-56     	       1.930 	
# SHORT_MOBILE-59     	       0.030 

# Récupération des résultats et écriture en python
t=$(mktemp)
for i in ${count_sizes[*]} ; do
   rm -f $t
   for bench in $benchs ; do
      echo -n "$bench = [" | tr '-' '_'                       >> $t
      grep $bench ${tmpfile}_$i | awk '{printf("%s, ", $NF)}' >> $t
      echo "]"                                                >> $t
   done
   rm -f ${tmpfile}_$i
   echo "import matplotlib.pyplot as plt"                    > plot_$i.py
   cat $t                                                   >> plot_$i.py
   rm $t                                                    
   echo "f = plt.figure()"                                  >> plot_$i.py
   echo "plt.xlabel('log2(table size)')"                    >> plot_$i.py
   echo "plt.ylabel('miss prediction per k instructions')"  >> plot_$i.py
   echo "i = 0"                                             >> plot_$i.py
   echo "colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']" >> plot_$i.py
   for bench in $benchs ; do
      b=$(echo -n "$bench" | tr '-' '_')
      echo "plt.plot([4, 6, 8, 10, 12, 14, 16], $b, marker='x', linestyle='--', color=colors[i%7], label='"$b"')" >> plot_$i.py
      echo "i = i + 1"                                      >> plot_$i.py
   done                                                    
   echo "plt.title('counter size" $i"')"                    >> plot_$i.py
   echo "plt.legend(loc='upper right')"                     >> plot_$i.py
   echo "plt.subplot().set_yscale('log', nonposy='clip')"   >> plot_$i.py
   echo "plt.legend(loc='upper right')"                     >> plot_$i.py
   echo "f.savefig('"graph_$i"', bbox_inches='tight')"      >> plot_$i.py
   python plot_$i.py
   rm plot_$i.py
done
