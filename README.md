Projet par Félix CHOI et Thomas KNIEBIHLER en JUIN 2019
original work on gitlab : https://gitlab.com/felixchoi/planetarium

Pour utiliser le client il faut tapper :

./client \<ip-pc1>  \<ip-pc2>  \<ip-pc3>  \<ip-pc4>  \<ip-pc5>

où \<ip-pcN> est l’adresse IP du N-eme pc.

Pour le server :
on compile avec la commande make (attention a bien changer les adresses du dossir dans le makefile et le CMakeCache).

on execute avec :
bin/MyExecutableName \<port> \<angle>

où \<port> est le port du pc. Les ports commencent à 32000 et chaque pc doit avoir un port différent (on peut aller de 32000 a 32004)
\<angle> est l’angle de rotation selon l’ecran. Il faut mettre des multiple de 72. Chaque ecran couvre 72 degrée.

Au debut de l’experience, il est conseiller d’envoyer « S » depuis le client pour la position des camera, puis « p » pour mettre en marche le systeme solaire.
