/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: ag096092
 *
 * Created on 27 mars 2019, 15:20
 */

#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <string>

using namespace std;

//Fonctions
void init();
void initGlouton();
int nbVoisins(int x);
int calculDegres();
void chargerGraphe(std::string nomGraphe);
int SommetMax(std::string nomGraphe);
bool convient(int x, int c);
int sommetDeDegreMaxNonColorie();
int dsatMax();
void updateDSAT(int x);
int plusPetiteCouleur(int x);
bool ResteNonColorie();
int DSATUR();
int ColorGlouton();
int ColorGloutonAmeliore();
int PGCD(int a, int b);
void calculRatio(int couleurmax);
void afficheGraphe();
void AfficheCouleur();
void ordonnerDegres();

// Variables
//const int n = 9;            //Graphe graphe_simple2.txt
const int n = 4039;         //Graphe facebook_combined.txt
//const int n = 8298;           //Graphe Wiki-Vote.txt
//const int n = 13866;            //Graphe athletes_edges.csv
//const int n = 26197;            //Graphe CA-GrQc.txt
//const int n = 36692;          //Graphe Email-Enron.txt
//const int n = 54573;        //Graphe HR_edges.csv
//const int n = 82168;            //Graphe Slashdot0902.txt
//const int n = 265214;           //Graphe Email-EuAll.txt

//Variables générales
//int adj[n][n];      // matrice d'adjacence, représentation du graphe
vector<vector<int>> adj(n,vector<int>(n,0));

//Variables propres à DSATUR
vector<int> couleur(n);     // contient la couleur de chaque sommet
vector<int> degres(n);      // contient le degré de chaque sommet
vector<int> dsat(n);        // tableau dsat pour l'algo DSATUR
vector<int> ni(n);          // tableau qui contient le numéro du premier sommet ayant utilisé la couleur i
int degresOrd[n][2];    //tableau ordonné des degrés des sommets avec comme première valeur le degré du sommet et comme deuxième son numéro/indice/nom

/*----------------------------- Main -----------------------------*/

int main(int argc, char** argv) {
    /*std::string graphe1 = "Graphes/Email-EuAll.txt";
    cout << SommetMax(graphe1) << endl;//*/
    
    std::string graphes[9] = { "graphe_simple2.txt", "facebook_combined.txt", "Wiki-Vote.txt", "athletes_edges.csv", "CA-GrQc.txt", "Email-Enron.txt", "HR_edges.csv", "Slashdot0902.txt", "Email-EuAll.txt" };
    std::string graphe = "Graphes/" + graphes[1];
    
    init();
    chargerGraphe(graphe);
    calculDegres();
    
    cout << "----- DSATUR" << endl;
    
    int couleurmax = DSATUR();  
    calculRatio(couleurmax);//*/
    
    initGlouton();
    
    cout << "----- Glouton" << endl;
    
    int couleurmaxG = ColorGlouton();  
    calculRatio(couleurmaxG);//*/
    
    initGlouton();
    
    cout << "----- Glouton Amélioré" << endl;
    
    int couleurmaxGA = ColorGloutonAmeliore();  
    calculRatio(couleurmaxGA);//*/
    
    return 0;
}

/*----------------------------- Fonctions de coloration -----------------------------*/

//Colorie le graphe grâce à l'algorithme DSATUR
int DSATUR(){
    int ppc = 1;        //Plus petite couleur
    int ppcmax = ppc;   //Plus grande couleur utilisée (équivalent du nombre chromatique)
    int compteur = 1;
    
    while(ResteNonColorie()){
        int x = dsatMax();
        if(x != -1) { 
            ppc = plusPetiteCouleur(x); 
            if(ppc > ppcmax) { ni[ppc] = compteur; ppcmax = ppc; }
            couleur[x] = ppc; 
        }
        compteur++;
        updateDSAT(x);
    }
    
    return ppcmax;
}

//Colorie le graphe grâce à l'algorithme glouton
int ColorGlouton(){
    int ppc = 1, ppcmax = ppc, compteur = 1;
        
    //On définit un ordre quelconque de sommets à parcourir (ici l'ordre croissant)
    for(int i=0; i<n; i++){                         
        //On applique la plus petite couleur non utilisée par un de ses voisins au sommet i
        ppc = plusPetiteCouleur(i);                 
        if(ppc > ppcmax) { ni[ppc] = compteur; ppcmax = ppc; }
        couleur[i] = ppc;
        compteur++;
    }
    
    return ppcmax;
}

//Colorie le graphe grâce à une version améliorée de l'algorithme glouton
int ColorGloutonAmeliore(){
    int ppc = 1, ppcmax = ppc, compteur = 1, x;
        
    //L'ordre ne sera cette fois-ci pas quelconque mais décroissant sur le degré des sommets
    ordonnerDegres();
    for(int i=0; i<n; i++){    
        x = degresOrd[i][1];
        //On applique la plus petite couleur non utilisée par un de ses voisins au sommet i
        ppc = plusPetiteCouleur(x);                 
        if(ppc > ppcmax) { ni[ppc] = compteur; ppcmax = ppc; }
        couleur[x] = ppc;
        compteur++;
    }
    
    return ppcmax;
}

/*----------------------------- Fonctions utiles à DSATUR -----------------------------*/

//Vérifie s'il reste au moins un sommet non colorié dans le graphe
bool ResteNonColorie(){
    bool res = false;
    for(int i=0; i<n; i++) if(couleur[i] == 0) res = true;
    return res;
}

//Détermine la plus petit couleur qui peut être mise sur un sommet
int plusPetiteCouleur(int x){
    int c=1;
    //La couleur ne peut pas être de valeur supérieure au degré du sommet
    //Et ne doit pas être utilisée par un des voisins
    while(c <= degres[x] && !convient(x,c)) c++;
    return c;
}

//Met à jour le tableau dsat utilisé par DSATUR
//x est le dernier sommet colorié
void updateDSAT(int x){
    vector<int> couleurs;
    vector<int>::iterator it;
    int valeur[1];
    
    //On détermine la plus grande couleur des voisins
    for(int j=0;j<n;j++){ if(adj[x][j]) { 
        //Pour chaque voisin du sommet
        couleurs.clear();
        for(int v=0;v<n;v++){ if(adj[j][v]) { 
            //On regarde le nombre de couleurs interdites chez ses voisins
            valeur[0] = couleur[v];
            if(valeur[0] != 0){
                //Si on croise une nouvelle couleur (autre que 0), on l'ajoute à la liste
                it = std::search(couleurs.begin(),couleurs.end(),valeur,valeur+1);
                if(it==couleurs.end()) { couleurs.push_back(couleur[v]); }
            }
        }}
        if(couleur[j] == 0) {  dsat[j] = couleurs.size(); }
    }}
}

//Détermine si une couleur c peut être appliquée sur un sommet x
bool convient(int x, int c)
{
    //On regarde si les voisins de x ne possèdent pas la couleur
    for(int i=0;i<n;i++) { if(adj[x][i] && (couleur[i]==c)) { return false; }}
    return true;
}

//Cherche le sommet de degré maximum toujours pas colorié
int sommetDeDegreMaxNonColorie(){
    int i=0, degmax = 0, res=-1;
    
    for(i; i<n; i++){
        if(couleur[i] == 0){   //S'il n'est pas colorié
            if(degres[i] > degmax){ res = i; degmax = degres[i]; }
        }
    }
    
    return res;
}

//Détermine le sommet qui possède le dsat maximum
int dsatMax(){
    int res = -1, max = -1;
    
    for(int i=0; i<n; i++){
        if(couleur[i] == 0){   //S'il n'est pas colorié
            if(dsat[i] > max){ res = i; max = dsat[i]; }
            //On sélectionne le sommet s'il possède un dsat équivalent mais qu'il a plus de voisins
            else if(dsat[i] == max && degres[i] > degres[res]){ res = i; max = dsat[i]; }
        }
    }
    
    return res;
}

/*----------------------------- Calcul du ratio -----------------------------*/

//Calcule le PGCD des nombres a et b
int PGCD(int a, int b){
    int c;
    while(b!=0){
        c=a%b;
        a=b;
        b=c;
    }
    return a;
}

//Calcule le ratio du graphe en fonction de la couleur max qui a été utilisée pour le colorier
void calculRatio(int couleurmax){
    int pgcd = PGCD(ni[couleurmax],n);
    cout << "Taille du graphe : " << n << endl;
    cout << "Nombre chromatique : " << couleurmax << endl;
    cout << "Ratio p : " << ni[couleurmax]/pgcd << "/" << n/pgcd << endl;
    cout << endl;
}

/*----------------------------- Initialisation et chargement du graphe -----------------------------*/

//Initialise les tableaux
void init(){
    for(int i=0; i<n; i++) {
        couleur[i] = 0;
        degres[i] = 0;
        dsat[i] = 0;
        ni[i] = -1;
    }
}

//Ré-initialise les tableaux pour relancer un algo glouton
void initGlouton(){
    for(int i=0; i<n; i++) {
        couleur[i] = 0;
        ni[i] = -1;
    }
}

//Détermine le nombre de sommets du sommet x
int nbVoisins(int x){
    int compt = 0;
    
    for(int i=0; i<n; i++){ if(adj[x][i]) compt++; }
    
    return compt;
}

//Calcule le degré de tous les sommets du graphe
int calculDegres(){
    for(int i =0; i<n; i++) degres[i] = nbVoisins(i);
}

//Lit le fichier SNAP qui représente le graphe
void chargerGraphe(std::string nomGraphe){
    int s1,s2;
    std::fstream infile(nomGraphe);
    while(infile >> s1 >> s2)
    {
        adj[s1][s2]=1;
        adj[s2][s1]=1;
    }
}

//Lit un fichier de graphe et détermine le sommet maximum
int SommetMax(std::string nomGraphe){
    int s1,s2,max = -1;
    std::fstream infile(nomGraphe);
    while(infile >> s1 >> s2)
    {
        if(s1 > max){ max = s1; }
        else if(s2 > max){ max = s2; }
    }
    return max;
}

/*----------------------------- Fonctions d'affichage -----------------------------*/

//Affiche dans la console les voisins de chaque sommet
void afficheGraphe()
{
    int i=0;
    int j=0;
    for(i=0;i<n;i++)
    {
        cout << "\nSommet "<< i << " :";
        for(j=0;j<n;j++)
        {
            if(adj[i][j]==1)
            {
                cout << j << " " ;
            }
        }
    }
}

//Affiche la couleur des sommets du graphe
void AfficheCouleur(){
    for(int i=0; i<n; i++) cout << "Sommet " << i << " : " << couleur[i] << endl;
}

//Remplit une version triée du tableau des degrés
void ordonnerDegres(){
    bool tableau_trie = false;
    int i = n-1;
    int temp[2];
    
    for(int k=0; k<n ;k++) { degresOrd[k][0] = degres[k]; degresOrd[k][1] = k; }
    while(i > 0 && !tableau_trie){
        tableau_trie = true;
        for(int j=0; j<i; j++){
            if(degresOrd[j+1][0] > degresOrd[j][0]){
                temp[0] = degresOrd[j][0]; temp[1] = degresOrd[j][1];
                degresOrd[j][0] = degresOrd[j+1][0]; degresOrd[j][1] = degresOrd[j+1][1];
                degresOrd[j+1][0] = temp[0]; degresOrd[j+1][1] = temp[1];
                tableau_trie = false;
            }
        }
        i--;
    }
}
