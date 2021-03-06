#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
//#include "tree_class.h"
//#include "make_tree.h"

#ifndef COMP_H
#define COMP_H

#define LeafN 4938
#define OrderN 4937

using namespace std;

float Dist_1[OrderN];
float Dist_2[OrderN];
     
int Order_1[OrderN];
int Order_2[OrderN];
int Order_d[OrderN];
    
int Id[LeafN];

string Check_Env(){    
                 
                 if (getenv("MetaStorms") == NULL) {
                                          
                                          cerr << "Please set the environment variable \"MetaStorms\" to the directory" << endl;
                                          exit(0);
                                          }
                 
                 string path = getenv("MetaStorms");
                 return path;
                                 
    }

/*
float Comp_tree(string * tree_file, map<string, _PerRate> * table){
    
    Node node;
    node.Node_calc(*tree_file, 0, table);
    
    //cout << node.Get_same()<<"%"<<endl;
    //cout << node.Get_evalue().value << endl;

    
    return node.Get_same();
    
    }


float Comp_sam(string * tree_file, const char * sam1, const char * sam2){
    
    
    map<string, _PerRate> table;
    
    Make_tree(sam1, sam2, &table);
    
    float result = Comp_tree(tree_file, &table);
    
    return result;
    }
*/

void Load_id(const char * infilename, int * id){
     
     ifstream infile(infilename, ifstream::in);
     if (!infile){
                  cerr << "Open id file error, please check : " << infilename << endl;
                  exit(0);
                  
                  }
     
     for (int i = 0; i < LeafN; i++)
         infile >> id[i];
     
     infile.close();
     infile.clear();
     }

int Load_abd(const char * infilename, int * id, float * abd){
    
    map<int, int> hash;
     
    ifstream infile(infilename, ifstream::in);
    if (!infile){
                  cerr << "Cannot open file : " << infilename << endl;
                  exit(0);
                  
                  }
    
    string buffer;
    int line_count = 0;
    getline(infile, buffer); //label
    
    while(getline(infile, buffer)){
                          
                          if (buffer.size() == 0) continue;
                          
                          line_count ++;
                          
                          string seq_id;
                          int current_id;                          
                          
                          stringstream strin(buffer);
                          
                          strin >> seq_id >> current_id;
                          
                          if (hash.count(current_id) == 0)
                                                     hash[current_id] = 1;
                          else hash[current_id] ++;
                          }
    
    for (int i = 0; i < LeafN; i++)
        if (hash.count(id[i]) == 0)
           abd[i] = 0;
        else abd[i] = (float)hash[id[i]] * 100 / (float)line_count;
    
    infile.close();
    infile.clear();
        
    return hash.size();
    }


void Load_order(const char * infilename, int * Order_1, float * Dist_1, int * Order_2, float * Dist_2, int * Order_d){
    ifstream infile(infilename, ifstream::in);
    if (!infile){
                  cerr << "Open order file error, please check : " << infilename << endl;
                  exit(0);
                  
                  }
    
    for (int i = 0; i < OrderN; i++){
        
        infile >> Order_1[i] >> Dist_1[i] >> Order_2[i] >> Dist_2[i] >> Order_d[i];
        
        }
        
    infile.close();
    infile.clear();    
    }

float Calc_sim(float * Abd_1, float * Abd_2, int * Order_1, float * Dist_1, int * Order_2, float * Dist_2, int * Order_d){
      
      float Reg_1[10];
      float Reg_2[10];
      
      float total = 0;
      int root;
      
      for(int i = 0; i < OrderN; i++){
              
              int order_1 = Order_1[i];
              int order_2 = Order_2[i];
              int order_d = Order_d[i] + 10;
              
              float dist_1 = 1- Dist_1[i];
              float dist_2 = 1- Dist_2[i];
              
              float c1_1;
              float c1_2;
              
              float c2_1;
              float c2_2;
                                
              if (order_1 >= 0){
                          
                          c1_1 = Abd_1[order_1];
                          c1_2 = Abd_2[order_1];
                          
                          }
              else {
                   c1_1 = Reg_1[order_1 + 10];
                   c1_2 = Reg_2[order_1 + 10];
                   }
              
              if (order_2 >= 0){
                          
                          c2_1 = Abd_1[order_2];
                          c2_2 = Abd_2[order_2];
                          
                          }
              else {
                   c2_1 = Reg_1[order_2 + 10];
                   c2_2 = Reg_2[order_2 + 10];
                   }
              //min
              float min_1 = (c1_1 < c1_2)?c1_1:c1_2;
              float min_2 = (c2_1 < c2_2)?c2_1:c2_2;
              
              total += min_1;
              total += min_2;
              
              //reduce
              Reg_1[order_d] = (c1_1 - min_1) * dist_1 + (c2_1 - min_2) * dist_2;
              Reg_2[order_d] = (c1_2 - min_1) * dist_1 + (c2_2 - min_2) * dist_2;
              
              root = order_d;
              }
      
      total += (Reg_1[root] < Reg_2[root])?Reg_1[root]:Reg_2[root];
      
      return total;
      
      }

void Comp_init(){
      
      string path = Check_Env();
      string id_file = path + "/treefile/id.txt";
      string order_file = path + "/treefile/order.txt";
          
      Load_id(id_file.c_str(), Id);
      Load_order(order_file.c_str(), Order_1, Dist_1, Order_2, Dist_2, Order_d);
      
      }

float Comp_sam(const char * sam1, const char * sam2){
      
      float * Abd_1 = new float[LeafN];
      float * Abd_2 = new float[LeafN];
      
      Load_abd(sam1, Id, Abd_1);
      Load_abd(sam2, Id, Abd_2);
      
      return Calc_sim(Abd_1, Abd_2, Order_1, Dist_1, Order_2, Dist_2, Order_d);
      
      }

#endif
