#include <iostream>
#include <pthread.h>

#include "meta_database.h"

using namespace std;

void Meta_Database::Insert_Index_by_Entry(Index_Entry * entry){
                         
            if (this->Meta_index.count(entry->Get_key())==0)
                                         this->Meta_index[entry->Get_key()] = new vector<Index_Entry * >;
          
            this->Meta_index[entry->Get_key()]->push_back(entry);
            
            this->Entry_count ++;
            
            if (entry->Get_abundance() > dom_t)
               this->Abundance ++;
           
           };     

void Meta_Database::Insert_Group_by_Entry(Index_Entry * entry){
     
           if (this->Meta_groups.count(entry->Get_group())==0)
                                                this->Meta_groups[entry->Get_group()] = new vector<Index_Entry *>;
                                                
           this->Meta_groups[entry->Get_group()]->push_back(entry);
     

     } 

void Meta_Database::Insert_Index_by_File(string infilename, unsigned int group){
          
          string keys[this->Key_dim];
          float weight[this->Key_dim];
                    
          Get_Key_Weight((infilename + "/classification.txt").c_str(), keys, weight, this->Key_dim);
          
          Index_Entry * entry = new Index_Entry(this->Entry_number, group, keys, weight, this->Key_dim, infilename, i_min);
          
          this->Entry_number ++; 
          
          this->Insert_Index_by_Entry(entry);
          
          this->Insert_Group_by_Entry(entry);
          
          }
      
int Meta_Database::Make_Index(string path){
    
    //if (this->Meta_index.empty())
    //   this->Meta_index.clear();
       
    //this->Entry_count = 0;
    //this->Entry_number = 0;
    
    //this->Path = path;

    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(path.c_str());
    if (pDir == NULL){
	   cerr<<"Error: Cannot Open Path : " << path << endl;
	   exit(0);
	   }
 
    while ((ent=readdir(pDir)) != NULL)
          if ((ent->d_type == 4) && (ent->d_name[0]!= '.')){
                           
             string inpathname = path + "/" + ent->d_name;
             
             string infilename = inpathname + "/classification.txt";
             ifstream infile(infilename.c_str(), ifstream::in); 
              
             if (infile != NULL) 
                                 {
                                 infile.close();
                                 infile.clear();
                                 //cout << inpathname << endl;
                                 this->Insert_Index_by_File(inpathname, 0);
                                                                                                   
                                 }
             else this->Make_Index(inpathname);
           }
           
    closedir(pDir);
          
    return this->Entry_count;
    }


int Meta_Database::Make_Index_Add(string path, unsigned int group){

    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(path.c_str());
    if (pDir == NULL){
	   cerr<<"Error: Cannot Open Path : " << path << endl;
	   exit(0);
	   }
 
    unsigned int _entry_count = 0;
    
    while ((ent=readdir(pDir)) != NULL)
          if ((ent->d_type == 4) && (ent->d_name[0]!= '.')){
             
             string inpathname = path + "/" + ent->d_name;
             
             string infilename = inpathname + "/classification.txt";
             ifstream infile(infilename.c_str(), ifstream::in); 
              
             if (infile != NULL) 
                                 {
                                 infile.close();
                                 infile.clear();
                                 //cout << inpathname << endl;
                                 this->Insert_Index_by_File(inpathname, group);
                                 _entry_count ++;
                                                                                                   
                                 }
             else _entry_count += this->Make_Index_Add(inpathname, group);
           }
    closedir(pDir);       	      
    return _entry_count;
    }


unsigned int Meta_Database::Get_Entry_List(Index_Entry * * list, int group){
         
         unsigned int count = 0;
         
         map<string, vector<Index_Entry *> *>::iterator map_iter;
         
         for (map_iter = this->Meta_index.begin(); map_iter != this->Meta_index.end(); map_iter ++){
             
                      vector<Index_Entry *>::iterator vect_iter;
                      for (vect_iter = map_iter->second->begin(); vect_iter != map_iter->second->end(); vect_iter++)
                          if (((*vect_iter)->Get_group() == group)||(group < 0))     
                                          {
                                           
                                           list[count] = *vect_iter;
                          
                                           count ++;
                                           }
                 }
         
         
        return count; 
         
        }

unsigned int Meta_Database::Get_Entry_List(Index_Entry * * list, string query_key, int group){
         
         unsigned int count = 0;
         
         if (this->Meta_index.count(query_key) == 0) return 0;
         
         vector<Index_Entry *>::iterator vect_iter;
         
         for (vect_iter = this->Meta_index[query_key]->begin(); vect_iter != this->Meta_index[query_key]->end(); vect_iter ++)
                        if (((*vect_iter)->Get_group() == group) || (group < 0))     
                                          {
                                           
                                           list[count] = *vect_iter;
                          
                                           count ++;
                                           }
         
         return count;
         
         }

void * Meta_Database::Parallel_Query_Static(void * args){
     
     string infilename = ((Argument *)args)->infilename;
     Index_Entry * * list = ((Argument *)args)->list;
     int thread = ((Argument *)args)->thread;
     int per_number = ((Argument *)args)->per_number;
     int count = ((Argument *)args)->count;
     //string * tree_file = ((Argument *)args)->tree_file;
       
     Meta_Result * buffer = ((Argument *)args)->buffer;
     
     for (int i =0; i < per_number; i++){//Loop in each thread
                                               int sam_file =  thread * per_number + i;
                                               if (sam_file >= count) break;
                                                                                                                
                                               string entry_filename = list[sam_file]->Get_filename() + "/classification.txt";
                                               
                                               buffer[sam_file].m_value = Comp_sam(infilename.c_str(), entry_filename.c_str());
                                 
                                               buffer[sam_file].entry = list[sam_file];
                                               
                                               //cout << (float)(j + 1) / (float)(per_number) * 100.0 << "%\tfinished" << endl;
                                                                                              
                                               }
                                               
                                           pthread_exit(0);
     
     }

unsigned int Meta_Database::Parallel_Query(string infilename, Index_Entry * * list, Meta_Result * results, int n, int t_number, int count){
                                             
                  t_number = (t_number < count)? t_number : count;
                              
                  if (t_number == 0 || t_number > sysconf(_SC_NPROCESSORS_CONF))
                           t_number = sysconf(_SC_NPROCESSORS_CONF);
                                         
                   //Allocation tasks for threads
                        
                  int per_number = count /t_number;
                  
                  if (count % t_number !=0) 
                            per_number++;
                        
                  //Meta_Result * pmap = (Meta_Result *)mmap(NULL, sizeof(Meta_Result) * count, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);
                  
                  Meta_Result buffer[count];
                  
                  Argument args[t_number];
                  pthread_t t[t_number];
                                                       
                  for (int i = 0; i< t_number; i++){//Loop each thread        
                                           //cout<< "Process " << i << " starts" << endl;
                                           
                                           args[i].infilename = infilename;
                                           args[i].list = list;
                                           args[i].thread = i;
                                           args[i].per_number = per_number;
                                           args[i].count = count;
                                           args[i].buffer = buffer;
                                           //args[i].tree_file = &Tree_file;
                                            
                                           pthread_create(&(t[i]),NULL, Parallel_Query_Static,&(args[i]));
                                           }
               
                  for (int i = 0; i < t_number; i++)
                           pthread_join(t[i], NULL);             
                                       
                  for (int i = 0; i < count; i++)
                      if (buffer[i].m_value > results[n-1].m_value){
                                      results[n-1] =  buffer[i];
                                      for (int j = n-1; j>0; j--)
                                          if (results[j].m_value > results[j-1].m_value){
                                                             Meta_Result temp = results[j];
                                                             results[j] = results[j-1];
                                                             results[j-1] = temp;                                                             
                                                             }
                                          else break;                                                                              
                                       }
                  
                  if (n < count) return n;
                  else return count;                     
         
         }


 unsigned int Meta_Database::Parallel_Indexed_Query(string infilename, string query_key, Meta_Result * results, int n, int t_number, int group){
                  
                  unsigned int count = 0;                                 
                  
                  if (this->Meta_index.count(query_key) == 0) return 0;
                  
                  else  count = this->Meta_index[query_key]->size();
                                                                
                  //To get the list of entry
                  Index_Entry * list[count];
                  
                  count = this->Get_Entry_List(list, query_key, group);
                  
                  //for (int i = 0; i < count; i++)
                    //  cout << list[i]->Get_filename() << endl;
                                                                                          
                  n = this->Parallel_Query(infilename, list, results, n, t_number, count);
                  
                  return n;
                          
                 }
                        
unsigned int Meta_Database::Parallel_Indexed_Query(ostream & out, string infilename, int n, int t_number, int group){
         
                        string keys[this->Key_dim];
                        
                        float weight[this->Key_dim];
          
                        Get_Key_Weight(infilename.c_str(), keys, weight, this->Key_dim);
                                                                        
                        Meta_Result results[n];
                                     
                        int n_count = 0;
                        
                        
                        for (int i = this->Key_dim; i>0; i--)
                            if (weight[i-1] >= q_min){
                                          
                                          string temp_keys[i];
                                          float temp_weight[i];
                                          
                                          for (int j = 0; j < i; j++){
                                              
                                              temp_keys[j] = keys[j];
                                              temp_weight[j] = weight[j];
                                              
                                              }
                                          
                                          for (int j = 0; j < i-1; j++)
                                              for (int k = j+1; k < i; k++)
                                                  if ((temp_keys[j] > temp_keys[k])&&(abs(temp_weight[j] - temp_weight[k]) < delta)){
                                                                 
                                                                 string temp_k = temp_keys[j];
                                                                 temp_keys[j] = temp_keys[k];
                                                                 temp_keys[k] = temp_k;
                                                                 
                                                                 float temp_w = temp_weight[j];
                                                                 temp_weight[j] = temp_weight[k];
                                                                 temp_weight[k] = temp_w;
                                                                 
                                                                 }
                                          
                                          string query_key = "";
                                          
                                          for (int j = 0; j < i; j++)
                                              query_key += temp_keys[j];
                                          
                                          n_count += this->Parallel_Indexed_Query(infilename, query_key, results, n, t_number, group);
                                          
                                          //cout << i << "\t" << n_count << endl;
                                          
                                          if (weight[i-1] >= q_max) break;
                                          }
                        
                        if (n_count == 0){
                                                            
                                          out << "#Meta-Storms" << endl;
                                          out << "#Not hits" << endl;
                            
                                          return 0; 
                                         }
                                                                        
                        n = (n < n_count) ? n : n_count;
                           
                        for (int i = 0; i < n; i++){
                           
                           out << "#Meta-Storm" << endl;
                           out << "Match " << i+1 << endl;
                           out << "Similarity: " << results[i].m_value << "%" << endl;
                           out << "Sample Group: " << results[i].entry->Get_group() << endl;
                           out << "Sample path: " << results[i].entry->Get_filename() << endl;                           
                           
                           }
                           
                        return n;
         
         }

unsigned int Meta_Database::Parallel_Exhaustive_Query(ostream & out, string infilename, int n, int t_number, int group){
                  
                  unsigned int count = 0;
                  
                  if (group < 0) count = this->Entry_count;
                  else if (this->Meta_groups.count(group) > 0)
                       count = this->Meta_groups[group]->size();

                  if (count == 0) {
                            
                            out << "#Meta-Storms" << endl;
                            out << "#Not hits" << endl;
                            
                            return 0; 
                            }                                              
                  //To get the list of entry
                  Index_Entry * list[count];
                                    
                  this->Get_Entry_List(list, group);
                  
                  Meta_Result results[n];                  
                  
                  n = this->Parallel_Query(infilename, list, results, n, t_number, count);
                  
                  for (int i = 0; i < n; i++){
                           
                           out << "#Meta-Storm" << endl;
                           out << "Match " << i+1 << endl;
                           out << "Similarity: " << results[i].m_value << "%" << endl;
                           out << "Sample Group: " << results[i].entry->Get_group() << endl;
                           out << "Sample path: " << results[i].entry->Get_filename() << endl;                           
                           
                           }                                          
                  
                  return n; 
                 
         }

                         
int Meta_Database::Out_Index(string outfilename){
    
    mkdir(outfilename.c_str(), 0755);
    string indexfile = outfilename + "/index.txt";
    string nametable = outfilename + "/nametable.txt";
    
    ofstream out_index(indexfile.c_str(), ofstream::out);
    if (!out_index){
                  cerr << "Error : Cannot Open Index File : " << indexfile << endl;
                  exit(0);
                  }
    
    ofstream out_name(nametable.c_str(), ofstream::out);
    if (!out_name){
                  cerr << "Error : Cannot Open Name Table : " << nametable << endl;
                  exit(0);
                  }
        
    map<string, vector<Index_Entry *> *>::iterator miter = this->Meta_index.begin();
    
    out_index << "Database : " << this->Name << "\t" << "Total Entry Count : " << this->Entry_count << "\t"  << "Total Key Count : " << this->Meta_index.size() << "\t" << "Key Dim : " << this->Key_dim << "\t"<< "Total Group Count : " << this->Meta_groups.size() << endl;
    
    out_name << "Database : " << this->Name << "\t" << "Total Entry Count : " << this->Entry_count << "\t"  << "Total Key Count : " << this->Meta_index.size() << "\t" << "Key Dim : " << this->Key_dim << "\t"<< "Total Group Count : " << this->Meta_groups.size() << endl;
    out_name << "Databasae ID\tGroup ID\tPath\tTop phyla abundence" << endl; 
    
    while(miter!=this->Meta_index.end()){
                                         out_index << ">"<< miter->first << endl;
                                         vector<Index_Entry *>::iterator viter;
                                         for (viter = miter->second->begin(); viter != miter->second->end(); viter ++){
                                             out_index << (*viter)->Get_id() << "\t";
                                             out_name << (*viter)->Get_id() << "\t" << (*viter)->Get_group() << "\t" << (*viter)->Get_filename();  
                                             for (int i = 0; i< this->Key_dim; i++)
                                                 out_name << "\t" << (*viter)->Get_key(i) << "\t" << (*viter)->Get_weight(i);                                       
                                             out_name << endl;
                                             }
                                         out_index << endl;
                                         
                                         miter++;
                                         }
    
    out_index.close();
    out_index.clear();
    out_name.close();
    out_name.clear();
    
    return this->Entry_count;
    
    }    

int Meta_Database::Load_Index(string infilename){
    
    string indexfile = infilename + "/index.txt";
    string nametable = infilename + "/nametable.txt";
    
    ifstream in_index(indexfile.c_str(), ifstream::in);
    if (!in_index){
                 cerr <<"Error: Cannot Open Database Index File : " << indexfile << endl;
                 exit(0);
                 }
    ifstream in_name(nametable.c_str(), ifstream::in);
    if (!in_name){
                 cerr <<"Error: Cannot Open Database Name Table : " << nametable << endl;
                 exit(0);
                 }
    
    if (this->Meta_index.empty())
       this->Meta_index.clear();
    
    map <unsigned int, Index_Entry *> entry_map;
    
    string buffer;
    
    //Nametable
    
    //Label
    getline(in_name, buffer);
    
    //DB Name;
    int begin = buffer.find("Database : ") + 11;
    int end =buffer.find("\t");
    this->Name = buffer.substr(begin, end - begin);
    //Key dim;
    begin = buffer.find("Key Dim : ") + 10;
    end = buffer.size();
    this->Key_dim = atoi((buffer.substr(begin, end - begin)).c_str());
    //Entry_count;
    this->Entry_count = 0;
    //Abundance
    this->Abundance = 0;
    
    //Label
    getline(in_name, buffer);
    
    while (getline(in_name, buffer)){
          
          if (buffer.size() == 0) continue;
          
          unsigned int id;
          unsigned int group;
          string filename;
          
          string keys[this->Key_dim];
          float weight[this->Key_dim];
          
          int begin = 0;
          int end = buffer.find('\t', begin);
          
          id = atoi(buffer.substr(begin, end - begin).c_str());
          
          begin = end + 1;
          end = buffer.find('\t', begin);
          
          group = atoi(buffer.substr(begin, end - begin).c_str());
          
          begin = end + 1;
          end = buffer.find('\t', begin);
          
          filename = buffer.substr(begin, end - begin);
          

          
          for (int i = 0; i < this->Key_dim; i++){
              
              begin = end + 1;
              end = buffer.find('\t', begin);
              keys[i] = buffer.substr(begin, end - begin);
              
              begin = end + 1;
              end = buffer.find('\t', begin);
              weight[i] = atof(buffer.substr(begin, end - begin).c_str());
              
              }
          
          Index_Entry * entry = new Index_Entry(id, group, keys, weight, this->Key_dim, filename, i_min);
          
          entry_map[id] = entry;
          
          if (id >= this->Entry_number)
             this->Entry_number = id + 1;
          /*
          istringstream strin(buffer);
          unsigned int id;
          string filename;
          strin >> id >> filename;
          
          namemap[id] = filename;
          */

          }
    //Index
    
    getline(in_index, buffer);
    
    while (getline(in_index, buffer)){
          
          if (buffer.size() == 0) continue;
          if (buffer[0] == '>'){
                         string key = buffer.substr(1, buffer.size()-1); 
                         getline(in_index, buffer);
                         istringstream strin(buffer);
                         unsigned int id;
                         while (strin >> id){
                               if (entry_map.count(id) == 0){
                                                       
                                                       cerr << "Error : Cannot find Sample ID : " << id << endl;
                                                       exit(0);
                                                       
                                                       }

                               this->Insert_Index_by_Entry(entry_map[id]);
                               this->Insert_Group_by_Entry(entry_map[id]);
                               }
                               }
                         }     
    in_index.close();
    in_index.clear();
    in_name.close();
    in_name.clear();
    return this->Entry_count;
    }

float Meta_Database::Get_Abundance(){
      
      return (float) this->Abundance / (float) this->Entry_count;
      
      }

float Meta_Database::Update_Abundance(){
    
    unsigned int temp_abundance = 0;
    
    unsigned int count = this->Entry_count;
    
    Index_Entry * list[count];
    
    this->Get_Entry_List(list, -1);
    
    for (int i = 0; i < count; i++)
        if (list[i]->Get_abundance() >= dom_t ) temp_abundance ++;
        //else cout << list[i]->Get_abundance() << endl;
    
    this->Abundance = temp_abundance;
    
    return (float)temp_abundance / (float)count;
    
    }
