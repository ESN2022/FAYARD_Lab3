<h1>Lab 3 : Display the accelerometer data on the 7 segments</h1>

Lors de ce dernier TP, un système plus avancé que les deux premiers va être mis en place. L’élément principal du système est l’accéléromètre avec lequel nous communiquerons par I2C. Les valeurs obtenues vont permettre le calcul en milli g de l’accélération. Celles-ci seront affichées à la fois sur l’UART mais aussi, pour un axe à la fois, sur les 7-segments avec un bouton poussoir qui va permettre de changer l’axe affiché. 


<h2>I.	Architecture du système</h2>

Comme pour toute architecture de co-design, les IPs de softcore NIOS II et de la mémoire de type RAM sont présentes. Un module UART est ajouté pour afficher les valeurs de l’accélération sur les trois axes (X, Y et Z). Ces valeurs sont également affichées sur les six 7-segment qui sont donc reliés à un décodeur binaire à 7-segment pour chaque 7-segment puis à une IP PIOs (Parallel I/O). Le décodeur 7-segment est nécessaire pour convertir un nombre binaire en BCD. Ce fichier VHDL correspond à une simple table de vérité. Pour pouvoir compter de 0 à 9 soit 10 chiffres, 4 bits sont nécessaires (2^3=8 - > X ; 2^4=16 - > V). Comme nous avons 10 combinaisons utilisées sur les 16, il reste donc 6 combinaisons libres. Pour afficher des nombres négatifs, on rajoute également une possibilité d’affiché le signe « - ».  Pour gérer tous (c’est-à-dire 6) les 7-segment dont nous avons besoin, chaque 7-segment se voit attribuer 4 bits du PIO qui doit donc avoir 6 x 4 = 24 bits. Une IP de communication I2C, qui nous a été fourni, est ajouté pour communiquer avec l’accéléromètre ADXL345. Ces IPs communiquent par le bus Avalon Memory Map c’est-à-dire que chaque périphérique est contrôlé en lisant ou en écrivant à ses adresses. Il y a également un timer relié directement au softcore par une ligne d’interruption. Ce dernier va servir de compteur pour une prise de mesure de l’accélération selon les 3 axes toute les 250 millisecondes. Le système de co-design est résumé sur la Figure 1.  

![arch_sys3](https://user-images.githubusercontent.com/103188608/212535648-385c2efc-a917-499e-991c-5a27e5c678ca.png)


<h2>II.	Progression et résultat</h2>

La première étape réalisée a été d’établir la communication avec l’accéléromètre via I2C. Voici la communication I2C résumé dans la datasheet de l’accéléromètre :

![I2C](https://user-images.githubusercontent.com/103188608/212663096-04248e05-f540-4c31-9b4a-56a53f6bc39d.png)

Pour rappel, la communication est, dans notre cas, gérée par l’IP « opencoresi2c ». Or elle comporte des fonctions software dédiées à la communication I2C disponible dans : opencores_i2c/HAL/src/opencores_i2c.c (de plus, des exemples sont disponibles dans opencores_i2c/Docs/I2C_tests.c) :
-	L’initialisation de la communication : void I2C_init(alt_u32 base,alt_u32 clk,alt_u32 speed),
-	Le bit de start ainsi que le registre et le write bit: int I2C_start(alt_u32 base, alt_u32 add, alt_u32 read),
-	La lecture : alt_u32 I2C_read(alt_u32 base,alt_u32 last),
-	L’écriture : alt_u32 I2C_write(alt_u32 base,alt_u8 data, alt_u32 last).

Pour ces deux dernières fonctions, on peut remarquer le dernier argument nommé « last » qui permet de dire si la lecture ou l’écriture du registre est la dernière et, si tel est le cas, la fonction va mettre un bit de stop. Autre remarque, les fonctions d’écriture et start retournent un entier qui correspond à l’ « acknowledge » si la valeur est 0 et 1 sinon. C’est ce dernier point qui nous a permis de vérifier la communication entre le FPGA et l’accéléromètre. Après avoir initialisé la communication, la fonction start est lancée en mode écriture (dernier argument à 0) et la valeur retournée est récupéré dans une variable. Si celle-ci est égale à 0 alors la communication avec l’accéléromètre est établie sinon, il y a un problème.

![Capture](https://user-images.githubusercontent.com/103188608/212713493-578d560d-42e6-46ab-b77e-30a30cb24dc1.PNG)

La difficulté principale lors de cette première étape a été de comprendre ce que faisais les fonctions I2C que nous devions utilisée. En effet, la fonction start ne contient pas que le bit de start et ajoute l’adresse de l’esclave (ici l’accéléromètre) ainsi qu’un bit de lecture (1) ou écriture (0). L’explication des fonctions dans opencores_i2c.c et l’étude des exemples ont permis de surmonter cette difficulté.


Le deuxième commit correspond à l’ajout de la lecture du registre correspondant aux valeurs d’accélérations de l’axe X. Pour cela, il y a deux registres un pour les MSB et l’autre les LSB et il faut donc les concaténer.

Là encore, la difficulté a été la compréhension des fonctions I2C.

Le troisième et quatrième commit correspondent à plusieurs changements et ajouts. Tout d’abord, une fonction générale nommée « read_axis » a été créé pour la lecture des deux registres d’un axes et qui prend comme argument le nom de l’axe qui est de type énumération. Cette fonction prend appuie sur une autre fonction nommée « read_byte » et qui permet de réaliser la procédure entière pour lire un seul octet. La fonction « axis_calc » permet de réaliser l’ensemble des opérations mathématiques menant à l’obtention de la valeur de l’accélération en milli g :
-	Concaténation des valeurs de registres MSB et LSB de l’axe,
-	Utilisation de la fonction « comp2 » permet d’obtenir la valeur signée à partir de la non signée,
-	Calcul de la valeur en mg en multipliant par la sensibilité (dans la datasheet de l’accéléromètre, typiquement 3,9 mg/LSB).
La valeur (uniquement de l’axe X dans un premier temps) est affichée sur le 7-segment et l’ensemble des valeurs est affiché via UART.

https://user-images.githubusercontent.com/103188608/212535822-35af9aca-e10e-43ac-9d87-5a8ae0e9a9dd.mov

https://user-images.githubusercontent.com/103188608/212535852-d0ec6811-ad91-406b-92b4-3cfc9c0eca25.mov

Le cinquième commit est l’ajout de l’interruption liée au bouton poussoir 1 (sur le PIO 1). L’appuie sur ce dernier permet de choisir l’axe dont la valeur sera affichée.

https://user-images.githubusercontent.com/103188608/212535865-99426d56-8d7f-4bb4-bd06-295a3c52bfff.mov

Le sixième commit correspond au remplacement du « usleep » par un timer. Celui-ci est choisi à 250 ms pour que les mesures soient assez rapides lors des changements de mouvements mais restent visible pour l’œil. Lors de l’interruption, un flag est mis à 1. Lors du retour dans le main, lorsque le flag est à 1, l’ensemble des opérations (mesures, calculs, affichage, …) est effectuée, puis le flag est remis à 0.

Les derniers commits correspondent à la calibration de l’accéléromètre. Pour cela, une fonction d’écriture sur le bus I2C est ajouté « write_byte ». Pour calibrer l’accéléromètre, la méthode est la suivante : 

https://user-images.githubusercontent.com/103188608/212877440-5a48741a-67ba-4a04-b726-88c73de82f99.mov

https://user-images.githubusercontent.com/103188608/212877510-825924b3-55dd-4231-a083-0070fd6a80e2.mov


<h2>Conclusion</h2>
