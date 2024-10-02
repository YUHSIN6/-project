#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
using namespace std;
bool debug[10] = {0};
bool research_mode = 0;

int main(int argc, char *argv[]) {
    research_mode = 0;
    //pass
    if (argc != 4) {
        cout << "command error" << endl;
        return 0; 
    }
    //cache_name = argv[1];
    //reference_name = argv[2];
    //index_name = argv[3];  

    int cachedata;
    string key;
    ifstream fin;


    //cache---------------------------------------------------------------------------------------------------------
    int Address_bits;
    int Offset_bits;
    int Index_bits;
    int Tag_bits;
    int Block_size;
    int Cache_sets;
    int Associativity;

    fin.open(argv[1],ios::in);
    if(!fin.is_open()){
        cerr << "cache error" << endl;
        return 1;
    }

    while(fin >> key >> cachedata){
        if(key == "Address_bits:"){
            Address_bits = cachedata;
        }else if(key == "Block_size:"){
            Block_size = cachedata;
        }else if(key == "Cache_sets:"){
            Cache_sets = cachedata;
        }else if(key == "Associativity:"){
            Associativity = cachedata;
        }
    }
    Offset_bits = log2(Block_size);
    Index_bits = log2(Cache_sets);
    Tag_bits = Address_bits - Offset_bits - Index_bits;

    if(debug[0]){
        cout << "Address_bits = " << Address_bits << endl;
        cout << "Block_size = " << Block_size << endl;
        cout << "Cache_sets = " << Cache_sets << endl;
        cout << "Associativity = " << Associativity << endl;
        cout << "Offset_bits = " << Offset_bits << endl;
        cout << "Index_bits = " << Index_bits << endl;
        cout << "Tag_bits = " << Tag_bits << endl;
    }

    fin.close();



    //reference---------------------------------------------------------------------------------------------------------
    fin.open(argv[2],ios::in);
    if(!fin.is_open()){
        cerr << "reference error" << endl;
        return 1;
    }

    int ref_num = 0;
    string casename;
    string linedata;
    string ref_list[100];
    while(getline(fin,linedata)){

        if(linedata == ".end"){
            break;
        }
        else if(linedata.substr(0,10) == ".benchmark"){
            casename = linedata;
        }else{
            ref_list[ref_num++] = linedata;
        }

    }
    
    if(debug[1]){
        cout << casename << endl;
        for (int i = 0; i<ref_num; i++){
            cout << i << " " << ref_list[i] << endl;
        }
    }

//research--------------------------------------------------------------------------------------------------
        int idx_chosenbits[Index_bits];
if(research_mode){
        int idx_chosen = 0;
        bool bit_use[Address_bits-Offset_bits];//unchoosebit
        for(int i = 0;i<Address_bits-Offset_bits;i++){
            bit_use[i] = 1;
        }
        //Q-------------------------------------------

        double Qi[Address_bits-Offset_bits];
        for(int i = 0;i<Address_bits-Offset_bits;i++){
            int Zi = 0;
            int Oi = 0;
            for(int j = 0;j<ref_num;j++){
                if(ref_list[j][i] == '1'){
                    Oi++;
                }else if(ref_list[j][i] == '0'){
                    Zi++;
                }
                if(debug[4]){
                    cout <<ref_list[j][i];
                }
            }

            Qi[i] = (double)min(Oi,Zi)/(double)max(Oi,Zi);


            if(debug[4]){
                cout <<" "<< Oi << " " << Zi <<  " " << Qi[i] << "\n";
            }
        }
        //find maxQ
        int maxQ_bit = 0;
        for(int i = 0;i<Address_bits-Offset_bits;i++){
            if(Qi[i]>Qi[maxQ_bit]){
                maxQ_bit = i;
            }
        }
        if(debug[4]){
            cout << "i= " << maxQ_bit << " Q= " <<Qi[maxQ_bit]<<"\n";
        }
        idx_chosenbits[0] = maxQ_bit;
        idx_chosen++;
        bit_use[maxQ_bit] = 0;

        //C------------------------------------

        double Ci[Address_bits-Offset_bits];
        for(int idx_num = 1;idx_num<Index_bits;idx_num++){//choose the rest idx by Cij
            for(int i = 0;i<Address_bits-Offset_bits;i++){//get Eij,Dij
                int Eij = 0;
                int Dij = 0;
                for(int j = 0;j<ref_num;j++){
                    if(ref_list[j][i]==ref_list[j][maxQ_bit]){
                        Eij++;
                    }else{
                        Dij++;
                    }
                }
                Ci[i] = (double)min(Eij,Dij)/(double)max(Eij,Dij);
                Qi[i] = (double)Qi[i]*(double)Ci[i];//New Qi
                if(debug[5]){
                    cout << "C" <<i<<"= "<< Ci[i]<< " Q" <<i<<"= "<< Qi[i]<<"\n";
                }
            }

            maxQ_bit = 0;//reset max_bit
            while(bit_use[maxQ_bit]==0){
                maxQ_bit++;
            }
            for(int i = 0;i<Address_bits-Offset_bits;i++){
                if(Qi[i]>Qi[maxQ_bit] && bit_use[i] == 1){
                    maxQ_bit = i;
                }
            }
            idx_chosenbits[idx_num] = maxQ_bit;//get new index bit
            idx_chosen++;
            bit_use[maxQ_bit] = 0;
            if(debug[5]){
                cout << idx_num << ".idx= " << maxQ_bit << " Q= " << idx_chosenbits[idx_num] << "\n";
            }
        }

        if(debug[6]){
            for(int i=0;i<Address_bits-Offset_bits;i++){
                cout<<bit_use[i];
            }
            cout<<"\n";
            for(int i = 0;i<idx_chosen;i++){
                cout << i << idx_chosenbits[i]<<"\n";
            }
        }


        //repalce indexbit to back
        string ref_listT[100];
        for(int i = 0;i<ref_num;i++){
            ref_listT[i] = ref_list[i];
        }


        for(int i = 0;i<ref_num;i++){

            bool N_bit_use[Address_bits-Offset_bits];
            for(int j = 0;j<Address_bits-Offset_bits;j++){
                N_bit_use[j] = bit_use[j];
                if(debug[0]){
                    cout << N_bit_use[j];
                }
            }
            if(debug[9]){
                cout << "\n";
            }


            int tag_nowbit = 0;
            for(int j = 0;j<Address_bits-Offset_bits;j++){
                if(N_bit_use[j]== 1){
                    N_bit_use[j] = 0;
                    ref_list[i][tag_nowbit] = ref_listT[i][j];
                    tag_nowbit++;
                    if(debug[9]){
                        cout <<ref_list[i][tag_nowbit-1];
                    }
                }
            }
            if(debug[9]){
                cout << "\n";
            }
        }

        for(int j = 0;j<ref_num;j++){
            for(int i = 0;i<Index_bits;i++){
                ref_list[j][i+Tag_bits] = ref_listT[j][idx_chosenbits[i]];
                if(debug[8]){
                    cout<< ref_list[j][i+Tag_bits];
                }
            }
            if(debug[8]){
                cout<<"\n";
            }
        }    


            if(debug[7]){
                //cout << "Tag_bits_n = " <<Tag_bits <<" New_tag_bits_n = " << tag_nowbit;
                for(int i = 0;i<ref_num;i++){
                    cout << ref_list[i] << " " << ref_listT[i] << "\n";
                }
            }
}

//nru--------------------------------------------------------------------------------------------------------
    //initial
    int cache_miss = 0;
    int cache_hit[100] = {0};//all miss at begin
    string cache[Cache_sets][Associativity];
    bool NRU[Cache_sets][Associativity];
    for(int i = 0; i < Cache_sets; i++){
        for(int j = 0; j < Associativity; j++){
            NRU[i][j] = 1;
        }
    }



    //play reference one by one
    for(int run_num = 0; run_num<ref_num; run_num++){
        //find tag&index
        string tag_adr = ref_list[run_num].substr(0, Tag_bits);
        string index_adr = ref_list[run_num].substr(Tag_bits, Index_bits);
        
        //check if it hit
        bool hit = 0;
        int index_adr_dec = stoi(index_adr,0,2);

        if(debug[2]){
            cout << "tag= " << tag_adr << " index= " << index_adr << " index_dec= " << index_adr_dec << " \n";
        }

        for(int check_ass = 0; check_ass<Associativity; check_ass++){
            if(tag_adr == cache[index_adr_dec][check_ass]){

                if(debug[3]){
                    cout << "hit! tag= " << tag_adr << " cache = " << cache[index_adr_dec][check_ass] << " index= " << index_adr_dec << " ass= " << check_ass << "\n";
                }

                cache_hit[run_num]=1;
                NRU[index_adr_dec][check_ass]=0;
                hit=1;
                break;
            }
        }
        //if hit,go ahead for next one
        if(hit){
            continue;
        }

        //miss
        cache_miss++;
        for(int check_ass = 0; check_ass<Associativity; check_ass++){
            //turn the first (NRU=0)'s cache to tag
            if(NRU[index_adr_dec][check_ass] == 1){
                NRU[index_adr_dec][check_ass]=0;
                cache[index_adr_dec][check_ass] = tag_adr;
                break;
            //if all = 0 turn back to 1 and the first one is tag
            }else if(check_ass == Associativity-1 && NRU[index_adr_dec][check_ass] == 0){
                for(int clean_ass = 1; clean_ass<Associativity; clean_ass++){
                    NRU[index_adr_dec][check_ass] = 1;
                }
                cache[index_adr_dec][0] = tag_adr;
            }
        }
    }        


//output--------------------------------------------------------------------------------------------------------
    ofstream fout;
    fout.open(argv[3],ios::out);
    if(!fout.is_open()){
        cerr << "index error" << endl;
        return 1;
    }
    fout << "Address bits: "<< Address_bits << "\n";
    fout << "Block size: "<< Block_size << "\n";
    fout << "Cache sets: "<< Cache_sets << "\n";
    fout << "Associativity: " << Associativity << "\n\n";


    fout << "Offset bit count: " << Offset_bits << "\n";
    fout << "Indexing bit count: " << Index_bits << "\n";
    fout <<"Indexing bits:" ;
    if(research_mode){
        for(int i = 0; i<Index_bits;i++){
            fout << " " <<7-idx_chosenbits[i]; 
        }
    }else{
        int indexing_bit = Offset_bits + Index_bits - 1;
        for(int i = 0; i<Index_bits;i++){
            fout << " " << indexing_bit--;
        }
    }
    
    
    fout << "\n\n" << casename << "\n";
    for(int i = 0; i<ref_num; i++){
        fout << ref_list[i] << " ";
        if(cache_hit[i]){
            fout << "hit\n";
        }else{
            fout << "miss\n";
        }
    }
        fout << ".end\n\n";
        fout << "Total cache miss count: "<<cache_miss;

    return 0;
}