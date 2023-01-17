<h1>Lab 3 : Display the accelerometer data on the 7 segments</h1>

Lors de ce dernier TP, un système plus avancé que les deux premiers va être mis en place. L’élément principal du système est l’accéléromètre avec lequel nous communiquerons par I2C. Les valeurs obtenues vont permettre le calcul en milli g de l’accélération. Celles-ci seront affichées à la fois sur l’UART mais aussi, pour un axe à la fois, sur les 7-segments avec un bouton poussoir qui va permettre de changer l’axe affiché. 


<h2>I.	Architecture du système</h2>

Comme pour toute architecture de co-design, les IPs de softcore NIOS II et de la mémoire de type RAM sont présentes. Un module UART est ajouté pour afficher les valeurs de l’accélération sur les trois axes (X, Y et Z). Ces valeurs sont également affichées sur les six 7-segment qui sont donc reliés à un décodeur binaire à 7-segment pour chaque 7-segment puis à une IP PIOs (Parallel I/O). Le décodeur 7-segment est nécessaire pour convertir un nombre binaire en BCD. Ce fichier VHDL correspond à une simple table de vérité. Pour pouvoir compter de 0 à 9 soit 10 chiffres, 4 bits sont nécessaires (2^3=8 - > X ; 2^4=16 - > V). Comme nous avons 10 combinaisons utilisées sur les 16, il reste donc 6 combinaisons libres. Pour afficher des nombres négatifs, on rajoute également une possibilité d’affiché le signe « - ».  Pour gérer tous (c’est-à-dire 6) les 7-segment dont nous avons besoin, chaque 7-segment se voit attribuer 4 bits du PIO qui doit donc avoir 6 x 4 = 24 bits. Une IP de communication I2C, qui nous a été fourni, est ajouté pour communiquer avec l’accéléromètre ADXL345. Ces IPs communiquent par le bus Avalon Memory Map c’est-à-dire que chaque périphérique est contrôlé en lisant ou en écrivant à ses adresses. Il y a également un timer relié directement au softcore par une ligne d’interruption. Ce dernier va servir de compteur pour une prise de mesure de l’accélération selon les 3 axes toute les 250 millisecondes. Le système de co-design est résumé sur la Figure 1.  

![arch_sys3](https://user-images.githubusercontent.com/103188608/212535648-385c2efc-a917-499e-991c-5a27e5c678ca.png)


<h2>II.	Progression et résultat</h2>

Lors de la création du QSYS, certaines particularités par rapport aux TP précédents sont à noter. Ici, le capteur utilisé comporte 6 pins dont 4 à définir absolument (les deux autres étant des interruptions qui ne seront pas utilisées ici). Comme pour toute communication I2C, les pins d’horloge (GSENSOR_SCLK) et de données (GSENSOR_SDI) sont connectées aux pins correspondantes de l’IP opencores i2c. Mais ce n’est pas tout, la pin GSENSOR_CS_n doit être à 1 pour activer a communication I2C et désactiver la SPI et la pin GSENSOR_SDO, qui sert à sélectionner une adresse alternative quand le mode choisis est I2C doit être à 1 également pour garder l’adresse par défaut du capteur. Ces informations sont disponibles aux pages 39 et 40 du manuel de la DE10-Lite.

Une fois l’ensemble du système designé, la partie soft est construite.

La première étape réalisée a été d’établir la communication avec l’accéléromètre via I2C. Voici la communication I2C résumé dans la datasheet de l’accéléromètre :

![I2C](https://user-images.githubusercontent.com/103188608/212663096-04248e05-f540-4c31-9b4a-56a53f6bc39d.png)

Pour rappel, la communication est, dans notre cas, gérée par l’IP « opencoresi2c ». Or elle comporte des fonctions software dédiées à la communication I2C disponible dans : opencores_i2c/HAL/src/opencores_i2c.c (de plus, des exemples sont disponibles dans opencores_i2c/Docs/I2C_tests.c) :
-	L’initialisation de la communication : void I2C_init(alt_u32 base,alt_u32 clk,alt_u32 speed),
-	Le bit de start ainsi que le registre et le read ou write bit: int I2C_start(alt_u32 base, alt_u32 add, alt_u32 read),
-	La lecture : alt_u32 I2C_read(alt_u32 base,alt_u32 last),
-	L’écriture : alt_u32 I2C_write(alt_u32 base,alt_u8 data, alt_u32 last).

Pour ces deux dernières fonctions, on peut remarquer le dernier argument nommé « last » qui permet de dire si la lecture ou l’écriture du registre est la dernière et, si tel est le cas, la fonction va mettre un bit de stop. Autre remarque, les fonctions d’écriture et start retournent un entier qui correspond à l’ « acknowledge » si la valeur est 0 et 1 sinon. C’est ce dernier point qui nous a permis de vérifier la communication entre le FPGA et l’accéléromètre. Après avoir initialisé la communication, la fonction start est lancée en mode écriture (dernier argument à 0) et la valeur retournée est récupéré dans une variable. Si celle-ci est égale à 0 alors la communication avec l’accéléromètre est établie sinon, il y a un problème.

![Capture](https://user-images.githubusercontent.com/103188608/212713493-578d560d-42e6-46ab-b77e-30a30cb24dc1.PNG)

La difficulté principale lors de cette première étape a été de comprendre ce que faisais les fonctions I2C que nous devions utilisées. En effet, la fonction start ne contient pas que le bit de start et ajoute l’adresse de l’esclave (ici l’accéléromètre) ainsi qu’un bit de lecture (1) ou écriture (0). L’explication des fonctions dans opencores_i2c.c et l’étude des exemples ont permis de surmonter cette difficulté.


Le deuxième, troisième et quatrième commits correspondent à la lecture des valeurs bruts de l’accélération et à leur calcul en milli g. Tout d’abord, une fonction générale nommée « read_axis » a été créé pour la lecture des deux registres d’un axes et qui prend comme argument le nom de l’axe qui est de type énumération. Cette fonction prend appuie sur une autre fonction nommée « read_byte » et qui permet de réaliser la procédure entière pour lire un seul octet. La fonction « axis_calc » permet de réaliser l’ensemble des opérations mathématiques menant à l’obtention de la valeur de l’accélération en milli g :
-	Concaténation des valeurs de registres MSB et LSB de l’axe : (DATA1 << 8) & DATA0,
-	Utilisation de la fonction « comp2 » permet d’obtenir la valeur signée à partir de la non signée,
-	Calcul de la valeur en mg en multipliant par la sensibilité (dans la datasheet de l’accéléromètre, page 4, typiquement 3,9 mg/LSB pour le range par défaut de 2g).
La valeur (uniquement de l’axe X dans un premier temps) est affichée sur le 7-segment et l’ensemble des valeurs est affiché via UART.

https://user-images.githubusercontent.com/103188608/212535822-35af9aca-e10e-43ac-9d87-5a8ae0e9a9dd.mov

https://user-images.githubusercontent.com/103188608/212535852-d0ec6811-ad91-406b-92b4-3cfc9c0eca25.mov


Le cinquième commit est l’ajout de l’interruption liée au bouton poussoir 1 (sur le PIO 1). L’appuie sur ce dernier permet de choisir l’axe dont la valeur sera affichée.

https://user-images.githubusercontent.com/103188608/212535865-99426d56-8d7f-4bb4-bd06-295a3c52bfff.mov


Le sixième commit correspond au remplacement du « usleep » par un timer. Celui-ci est choisi à 250 ms pour que les mesures soient assez rapides lors des changements de mouvements mais restent visible pour l’œil. Lors de l’interruption liée au timer, un flag est mis à 1. Lors du retour dans le main, lorsque le flag est à 1, l’ensemble des opérations (mesures, calculs, affichage, …) est effectuée, puis le flag est remis à 0.


Les derniers commits correspondent à la calibration de l’accéléromètre. Ici, la calibration consiste uniquement à régler l’offset. La sensibilité restera à la valeur typique donnée par le constructeur. La nécessité d’une calibration s’explique par les contraintes mécaniques que subit ou a subit l’accéléromètre sur la carte (soudure, …) et qui modifie les valeurs retournées. D’autres facteurs environnementaux (température, …) modifient les valeurs mais ne seront pas traités ici.

Dans la datasheet de l’accéléromètre, page 35, les valeurs théoriques d’accélération à obtenir sont données selon l’orientation du capteur. Lorsque la carte est posée à plat, les valeurs théoriques pour les axes X et Y est de 0 et pour Z de 1 g (=9,81 m/s2) soit 1 000 mg (Figure 3).

![Position_acc](https://user-images.githubusercontent.com/103188608/212972701-1892277d-34e6-4041-9ed3-9c29438a1077.png)

On remarque que les valeurs expérimentales avant réglage de l’offset sont légèrement différentes des théoriques et que celui-ci permet, en moyenne de s’en rapprocher :

![acc_before_after_cal](https://user-images.githubusercontent.com/103188608/212972898-671c0b06-a5eb-4f46-a1ed-e0f021d4610f.png)

Le réglage de l’offset de l’accéléromètre a été réalisé de la manière suivante : 
-	La carte est posée sur une surface plane,
-	Pour chaque axe, 10 valeurs d’accélération en milli g servent à calculer la moyenne,
-	Pour chaque axe, on soustrait à cette moyenne expérimentale, la valeur théorique pour obtenir l’offset actuel (ordonnée à l’origine) nommée OFST :
o	Pour X et Y, la valeur est directement celle de l’offset : OFSTX = Xmoyen, OFSTY = Ymoyen,
o	Pour Z, on soustrait 1 000 : OFSTZ = Zmoyen – 1 000,
-	Pour supprimer l’offset OFST, il faut ajouter à chaque valeur, l’opposé de l’offset soit -OFST. Un registre pour chacun des axes est dédié à cela. Ce registre comprend un nombre signé binaire (complément à 2). Il faut être vigilant ici car l’échelle n’est pas la même que celle des registres d’accélération (3,9 mg/LSB dans notre cas) mais est de 15,6 mg/LSB. Pour convertir notre valeur de mg à un nombre binaire signée que nous allons nommer bin_OFST, il faut donc le diviser par 15,6 :

![calc_bin_offset](https://user-images.githubusercontent.com/103188608/212973090-4c994f9d-6aa1-4289-a59d-92e2087a77a3.png)

Les résultats obtenus avant et après réglage de l’offset sont résumé dans les tableaux ci-dessous :

![mean_acc_before_after_cal](https://user-images.githubusercontent.com/103188608/212973247-23a012a8-527c-46da-a192-24abc37af1c9.png)

Après avoir écrit dans les registres de l’offset de X, Y et Z respectivement +3, +6 et -1 (inverse de l’offset expérimental en binaire signée), on remarque que la moyenne des valeurs d’accélération sont plus proches des valeurs théoriques qu’auparavant. Par exemple, pour X, on passe d’une moyenne de -41,8 mg à 2,4 mg (valeur théorique : 0 mg). Cette dernière valeur correspond à 0,15 en binaire signée pour une échelle de 15,6 mg/LSB soit en arrondissant 0 ce qui signifie que le réglage ne peut pas être amélioré.

https://user-images.githubusercontent.com/103188608/212877440-5a48741a-67ba-4a04-b726-88c73de82f99.mov

https://user-images.githubusercontent.com/103188608/212877510-825924b3-55dd-4231-a083-0070fd6a80e2.mov


<h2>Conclusion</h2>

Lors de ce dernier TP, la nouveauté a été l’ajout d’une IP fournit pour la partie hardware et la communication en I2C pour la partie software. Cette dernière partie comprend non seulement la compréhension du protocole de communication mais également l’exploration des différents registres.
La difficulté principale de ce TP a été l’appropriation des fonctions softwares de communication I2C présentes avec l’IP. La lecture des fonctions ainsi que des exemples permet de lever cette difficulté.

