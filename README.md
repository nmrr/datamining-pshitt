# datamining-pshitt

> Je remercie l'équipe du Cyber Hacking Lab (Nabil Bouzerna et Flavien Quesnel) de l'IRT SystemX pour avoir Inspiré ces travaux de R&D ainsi que Babiga Birregah, mon tuteur de projet côté UTT

Ce projet a été réalisé en collaboration entre [**l'UTT (Université de technologie de Troyes)**](https://www.utt.fr/) et [**l'IRT SystemX**](https://www.irt-systemx.fr/) dans le cadre d'une TX (projet étudiant encadré) au printemps 2017. L'objectif de ce projet était d'analyser des logs provenant d'un **pot de miel** (honeypot) : un faux serveur SSH (pshitt) est installé sur un serveur et ce dernier répond systèmatiquement en cas de tentative de connexion que les identifiants tentés sont incorrects. Les informations des tentatives de connexion sont sauvegardées pour y être analysées.

Exemple d'attaque :

#####  {"username": "usr", "try": 1, "src_port": 59880, "software_version": "libssh2_1.4.3", "timestamp": "2017-08-01T00:01:42.666293", "src_ip": "138.197.0.147", "mac": "hmac-sha1", "cipher": "aes128-ctr", "password": "usr"}

Dans cette attaque, l'attaquant a essayé de se connecter avec l'identifiant **usr** et le mot de passe **usr** le **01/08/2017** à **01H42**. Cette attaque vient des **USA**, cette information a été obtenue à partir de son adresse IP.

Dans un premier temps, l'objectif a été de visualiser ces données. **D3.js** a été utilisé pour visualiser les données de manière générale (représentation des attaquants sur une carte géographique, statistiques sur les usernames/passwords les plus utilisés, etc). 

A l'aide de **Linkurious.js** (fork de sigma.js), ces données ont été représentées sous la forme de graphes petits-mondes : des corrélations entre les attaques et leur provenance sont réalisées.

Dans un deuxième temps, un outil de visualisation des nouvelles attaques a été créé. Cet outil permet de déterminer les nouvelles attaques qui, jusqu'à une certaine date n'ont jamais été tentées. Cela permet de déterminer les nouvelles cibles du moment comme un objet connecté vulnérable par exemple.

-----------------------------------

### Installation du logiciel

#### Prérequis 

Certains logiciels sont nécessaires :  **Node.js**, **node-gyp**, **build-essential** et **rapidjson-dev**

Pour installer **Node.js** : https://nodejs.org/en/download/

**Attention !** Ce projet n'a pas été mis à jour de puis longtemps. La dernière version de **Node.js** compatible est la version 10, qui est obsolète depuis début 2021 ! Des changements ont été effectuées concernant les modules compilés, cela explique l'incompatibilité avec les versions supérieures.

Installer **node-gyp**  de manière globale : 
```
sudo npm install node-gyp -g
```

Installer l'environnement de développement : **sudo apt install build-essential rapidjson-dev** (pour une distribution basée sur Debian)

#### Préparation du logiciel

Cloner le repo : 
```
git clone https://github.com/nmrr/datamining-pshitt.git
```

Se placer dans le dossier **datamining-pshitt**

Installer les dépendances : 
```
npm install nan express
```

Compiler le projet :
```
node-gyp configure build
```

Lancer le logiciel :
```
node script.js
```

Ouvrir le navigateur Internet et se rendre à l'adresse suivante : **http://127.0.0.1:8888**

-----------------------------------

Un dataset d'une semaine de récolte de données est présent dans le dossier **ressources**.

-----------------------------------

### Galerie

Visualisation du nombre d'attaques par jour entre 2017 et 2022 :

<img src="https://github.com/nmrr/datamining-pshitt/blob/master/img/pshitt1.png" width=75% height=75%>

Visualisation des attaquants par pays :

<img src="https://github.com/nmrr/datamining-pshitt/blob/master/img/pshitt2.png" width=75% height=75%>

Exemple de graphe petit-monde généré par l'application :

<img src="https://github.com/nmrr/datamining-pshitt/blob/master/img/pshitt3.png" width=75% height=75%>

### Changelog

**09/07/2018** : ajout de la visualisation des IP dans la partie **Visualisation des attaques par pays**

**09/03/2019** : échappement des caractères dans la recherche, ajout d’une limite lors de la génération des graphes (20k nœuds et 50k liens), modifications diverses dans les formulaires
