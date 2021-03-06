#include <iostream>
#include <fstream>
#include <stdio.h>
#include "meta_database.cpp"
#include "version.h"

using namespace std;

int printhelp(){
    
    cout << "Welcome to Meta-Storms Beta " << endl;
    cout << "Version : " << Version << endl;
    cout << "Usage : " << endl;
    cout << "query_index [-option] value" << endl;
    cout << "\toption : " << endl;
    cout << "\t-d database index name" << endl;
    cout << "\t-i query sample path" << endl;
    cout << "\t-o result output file, default is to output on screen" << endl;
    cout << "\t-n hit number, default is 5" << endl;
    cout << "\t-e exhaustive search" << endl;
    cout << "\t-g to assign group number, default is off" << endl;
    cout << "\t-t cpu core number, default is 1" << endl;
    cout << "\t-h help" << endl;
    
    exit(0);
    
    return 0;
    
    };

int main(int argc, char * argv[]){
    //Para
    string indexname = "Default.txt";
    string queryfilename = "classification.txt";
    string outfilename = "";
    
    int t_number = 1;
    int is_index = 1;
    int r_number = 5;
    int group = -1;
    
    //string tree_file;
    Comp_init();
        
    int i = 1;
      
      if (argc ==1) 
		printhelp();
      
      while(i<argc){
         if (argv[i][0] != '-') {
                           printf("Argument # %d Error : Arguments must start with -\n", i);
                           exit(0);
                           };           
         switch(argv[i][1]){
                            case 'd': indexname = argv[i+1]; break;
                            case 'i': queryfilename = argv[i+1]; break;
                            case 't': t_number = atoi(argv[i+1]); break;
                            case 'e': is_index = 0; i--; break;
                            case 'n': r_number = atoi(argv[i+1]); break;
                            case 'o': outfilename = argv[i+1]; break;
                            case 'g': group = atoi(argv[i+1]); break;
                            default : printf("Unrec argument %s\n", argv[i]); printhelp(); break; 
                            }
         i+=2;
         } 
    
    cout << "Welcome to Meta-Storms Beta " << endl;
    
    Meta_Database database(indexname);
    
    //file
    if (outfilename.size() > 0){                           
                           ofstream out(outfilename.c_str(), ofstream::out);
                           if (!out){
                                     cerr << "Open output file error : " << outfilename << endl;
                                     exit(0);
                                     }                                                       
                           
                           if (is_index == 1)
                              database.Parallel_Indexed_Query(out, queryfilename, r_number, t_number, group);
                           else
                              database.Parallel_Exhaustive_Query(out, queryfilename, r_number, t_number, group);
                           
                           out.close();
                           out.clear();
                           
                           } 
    //cout
    else {
         if (is_index == 1)
              database.Parallel_Indexed_Query(cout, queryfilename, r_number, t_number, group);
         else
             database.Parallel_Exhaustive_Query(cout, queryfilename, r_number, t_number, group);
         }
    
    
    return 0;
}
