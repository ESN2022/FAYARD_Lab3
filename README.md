<h1>Lab 3 : Display the accelerometer data on the 7 segments</h1>

Lors de ce dernier TP, un système plus avancé que les deux premiers va être mis en place. L’élément principal du système est l’accéléromètre avec lequel nous communiquerons par I2C. Les valeurs obtenues vont permettre le calcul en milli g de l’accélération. Celles-ci seront affichées à la fois sur l’UART mais aussi, pour un axe à la fois, sur les 7-segments avec un bouton poussoir qui va permettre de changer l’axe affiché. 


<h2>I.	Architecture du système</h2>

Comme pour toute architecture de co-design, les IPs de softcore NIOS II et de la mémoire de type RAM sont présentes. Un module UART est ajouté pour afficher les valeurs de l’accélération sur les trois axes (X, Y et Z). Ces valeurs sont également affichées sur les six 7-segment qui sont donc reliés à un décodeur binaire à 7-segment pour chaque 7-segment puis à une IP PIOs (Parallel I/O). Le décodeur 7-segment est nécessaire pour convertir un nombre binaire en BCD. Ce fichier VHDL correspond à une simple table de vérité. Pour pouvoir compter de 0 à 9 soit 10 chiffres, 4 bits sont nécessaires (2^3=8 - > X ; 2^4=16 - > V). Comme nous avons 10 combinaisons utilisées sur les 16, il reste donc 6 combinaisons libres. Pour afficher des nombres négatifs, on rajoute également une possibilité d’affiché le signe « - ».  Pour gérer tous (c’est-à-dire 6) les 7-segment dont nous avons besoin, chaque 7-segment se voit attribuer 4 bits du PIO qui doit donc avoir 6 x 4 = 24 bits. Une IP de communication I2C, qui nous a été fourni, est ajouté pour communiquer avec l’accéléromètre ADXL345. Ces IPs communiquent par le bus Avalon Memory Map c’est-à-dire que chaque périphérique est contrôlé en lisant ou en écrivant à ses adresses. Il y a également un timer relié directement au softcore par une ligne d’interruption. Ce dernier va servir de compteur pour une prise de mesure de l’accélération selon les 3 axes toute les secondes. Le système de co-design est résumé sur la Figure 1.  

![arch_sys3](https://user-images.githubusercontent.com/103188608/212535648-385c2efc-a917-499e-991c-5a27e5c678ca.png)


<h2>II.	Progression et résultat</h2>

https://user-images.githubusercontent.com/103188608/212535822-35af9aca-e10e-43ac-9d87-5a8ae0e9a9dd.mov




https://user-images.githubusercontent.com/103188608/212535852-d0ec6811-ad91-406b-92b4-3cfc9c0eca25.mov





https://user-images.githubusercontent.com/103188608/212535865-99426d56-8d7f-4bb4-bd06-295a3c52bfff.mov




<h2>Conclusion</h2>
