# datamining-pshitt

> Je remercie l'équipe du Cyber Hacking Lab (Nabil Bouzerna et Flavien Quesnel) de l'IRT SystemX pour avoir Inspiré ces travaux de R&D

Ce projet a été réalisé en collaboration entre l'UTT (Université de technologie de Troyes) et l'IRT SystemX dans le cadre d'une TX (projet étudiant encadré) au printemps 2017. L'objectif de ce projet était d'analyser des logs provenant d'un pot de miel (honeypot) : un faux serveur SSH (pshitt) est installé sur un serveur et ce logiciel récupère les informations des tentatives d'intrusion.

Exemple d'attaque :

#####  {"username": "usr", "try": 1, "src_port": 59880, "software_version": "libssh2_1.4.3", "timestamp": "2017-08-01T00:01:42.666293", "src_ip": "138.197.0.147", "mac": "hmac-sha1", "cipher": "aes128-ctr", "password": "usr"}

Dans cette attaque, l'attaquant a essayé de se connecter avec l'identifiant **usr** et le mot de passe **usr** le **01/08/2017** à **01H42**. Cette attaque vient des **USA**, cette information a été obtenue à partir de son adresse IP. Ces informations là sont les plus intéressantes, les autres informations n'apportent que peu d'informations.

Dans un premier temps, l'objectif a été de visualiser ces données. **D3.js** a été utilisé pour visualiser de manière générale les données (représentation des attaquants sur une carte géographique, statistiques sur les usernames/passwords les plus utilisés, etc). A l'aide de **Linkurious.js** (sigma.js amélioré), ces données ont été représentées sous la forme d'arbres : des corrélations entre les attaques et leur provenance sont réalisées.
Dans un deuxième temps, un outil de visualisation des nouvelles attaques a été créé. Cet outil permet de déterminer les nouvelles attaques qui, jusqu'à une certaine date n'ont jamais été tentées. Cela permet de déterminer les nouvelles tendances et donc, les nouvelles cibles.

-----------------------------------

### Installation du logiciel

#### Prérequis 

Certains logiciels sont nécessaires :  **Node.js**, **node-gyp** et **build-essential**

Pour installer **Node.js** : https://nodejs.org/en/download/

Installer **node-gyp**  de manière globale : **npm install node-gyp -g**

Pour installer l'environnement de développement : **apt install build-essential** (pour une distribution basée sur Debian)

#### Préparation du logiciel

Cloner le repo : **git clone https://github.com/nmrr/datamining-pshitt.git**

Se placer dans le dossier **datamining-pshitt**

Installer les dépendances : **npm install nan express**

Compiler le projet : **node-gyp configure build**

Lancer le logiciel : **node script.js**

Ouvrir le navigateur Internet et se rendre à l'adresse suivante : **http://127.0.0.1:8888**

-----------------------------------

Un dataset d'une semaine de récolte de données est présent dans le dossier **ressources**.
