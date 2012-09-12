#pragma once
#include<cstdlib>
#include<vector>
#include<cstdio>
#include<iostream>
#include<algorithm>
#include<sys/mman.h>
#include<fcntl.h>


namespace isan{

typedef Feature_String Dict_Item;
typedef Feature_String Word;
typedef int Score;
typedef char Character;


bool item_cmp(const std::pair<Dict_Item, Score>& i,
                const std::pair<Dict_Item, Score>& j){
    int ind=0;
    while(true){
        if(ind>=i.first.length)return 1;
        if(ind>=j.first.length)return 0;
        if((i.first.pt[ind])<(j.first.pt[ind]))return 1;
        if((i.first.pt[ind])>(j.first.pt[ind]))return 0;
        ind++;
    };
    return 0;
};


class DAT{
public:
    struct Entry{
        int base;
        int check;
    };

    void* mmap_ptr;
    Entry* dat;
    size_t dat_size;
    DAT():mmap_ptr(NULL){};
    DAT(const char* filename){

        FILE * pFile=fopen(filename,"r+b");
        if(!pFile){
            fprintf(stderr,"[ERROR] DAT file %s not found\n",filename);
        }
        fseek(pFile,0,SEEK_END);
        dat_size=ftell(pFile)/sizeof(Entry);
        rewind(pFile);
        int rtn;
        fclose(pFile);
        
        int fd=open(filename,O_RDWR);
        mmap_ptr=mmap(NULL,sizeof(Entry)*dat_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
        dat=(Entry*)mmap_ptr;
        close(fd);
    }

    void save_as(const char* filename){
        FILE * pFile=fopen(filename,"wb");
        fwrite(dat,sizeof(Entry),dat_size,pFile);
        fclose(pFile);
    }
    inline int get(const Word& word)const{
        register int ind=0;
        register int base=0;
        for(int i=0;i<word.size();i++){
            ind=dat[ind].base+word[i];
            if((ind>=dat_size)||dat[ind].check!=base)return 0;
            base=ind;
        }
        ind=dat[base].base;
        if((ind<dat_size)&&(dat[ind].check==base)){
            return dat[ind].base;
        }
        return 0;
    }
    ~DAT(){
        if(this->mmap_ptr){
            msync(this->mmap_ptr,sizeof(Entry)*this->dat_size,MS_ASYNC);
            munmap(this->mmap_ptr,sizeof(Entry)*this->dat_size);
        }else{
            free(dat);
        }
    }
    inline int get_index(int base,const Character& character){
        int ind=dat[base].base+character;
        if((ind>=dat_size)||dat[ind].check!=base)return -1;
        return ind;
    };
    /*return -base or number of matched characters*/
    int get_info(Word& prefix){
        register int ind=0;
        register int base=0;
        for(size_t i=0;i<prefix.size();i++){
            ind=dat[ind].base+prefix[i];
            if((ind>=dat_size)||dat[ind].check!=base)return i;
            base=ind;
        }
        return -base;
    }
};


class DATMaker: public DAT{
public:
    typedef std::pair<Word, int> KeyValue;
    static bool compare_words (const DATMaker::KeyValue& first, const DATMaker::KeyValue& second)
    {
        const Word& first_word=first.first;
        const Word& second_word=second.first;
        size_t min_size=(first_word.size()<second_word.size())?first_word.size():second_word.size();
        for(int i=0;i<min_size;i++){
            if(first_word[i]>second_word[i])return false;
            if(first_word[i]<second_word[i])return true;
        }
        
      return (first_word.size()<second_word.size());
    }
 
    
public:
    int head;
    int tail;
    DATMaker(){
        Entry init;
        dat_size=1;
        dat=(Entry*)calloc(sizeof(Entry),dat_size);
        dat[0].base=1;dat[0].check=-1;
        head=0;
        tail=0;
    };
    void use(int ind){//use [ind] as an entry
        if(dat[ind].check>=0)printf("cell reused!!\n");
        if(dat[ind].base==1){
            head=dat[ind].check;
        }else{
            dat[-dat[ind].base].check=dat[ind].check;
        };
        if(dat[ind].check==-dat_size){
            tail=dat[ind].base;
        }else{
            dat[-dat[ind].check].base=dat[ind].base;
        };
        dat[ind].check=ind;
    };
    void extends(){
        int old_size=dat_size;
        dat_size*=2;
        dat=(Entry*)realloc(dat,sizeof(Entry)*dat_size);
        for(int i=0;i<old_size;i++){
            dat[old_size+i].base=-(old_size+i-1);
            dat[old_size+i].check=-(old_size+i+1);
        };
        dat[old_size].base=tail;
        if(-tail>=0)dat[-tail].check=-old_size;
        tail=-(old_size*2-1);
    }
    void shrink(){//thrink之后双向链表就不需要保持了
        int last=dat_size-1;
        while(dat[last].check<0)last--;
        dat_size=last+1;
        dat=(Entry*)realloc(dat,sizeof(Entry)*dat_size);
    }

    int alloc(std::vector<int>& offsets){
        size_t size=offsets.size();
        register size_t base=-head;
        while(1){
            if(base==dat_size)extends();
            if(size)
                while(base+offsets[size-1]>=dat_size)
                    extends();
            register int flag=true;
            if(dat[base].check>=0){
                flag=false;
            }else{
                for(register int i=0;i<size;i++){
                    if(dat[base+offsets[i]].check>=0){//used
                        flag=false;
                        break;
                    }
                }
            }
            if(flag){
                use(base);
                for(int i=0;i<size;i++)use(base+offsets[i]);
                return base;//got it and return it
            }
            if(dat[base].check==-dat_size)extends();
            base=-dat[base].check;
        }
    }
    void gen_children(std::vector<KeyValue>& lexicon,int start,const Word& prefix,std::vector<int>&children){
        children.clear();
        size_t l=prefix.size();
        for(size_t ind=start;ind<lexicon.size();ind++){
            Word& word=lexicon[ind].first;
            if(word.size()<l)return;
            for(int i=0;i<l;i++)if(word[i]!=prefix[i])return;
            if(word.size()>l){
                if(children.empty()||word[l]!=children.back())
                    children.push_back(word[l]);
            }
        }
    }
    int assign(int check,std::vector<int>& offsets,int is_word=false){
        int base=alloc(offsets);
        //base
        dat[base].base=0;
        if(is_word){//如果是词
            dat[base].check=check;
        }else{//如果不是词
            dat[base].check=base;
        }
           
        for(int i=0;i<(int)offsets.size();i++){
            dat[base+offsets[i]].base=0;
            dat[base+offsets[i]].check=check;
        }
        dat[check].base=base;
        //printf("is_word %d  base %d\n",is_word,base);
        return base;
    }
    void make_dat(std::vector<KeyValue>& lexicon,int no_prefix=0){
        std::sort(lexicon.begin(),lexicon.end(),&compare_words);

        int size=(int)lexicon.size();
        //std::cout<<size<<"\n";
        std::vector<int> children;
        Word prefix;
        //prefix.clear();
        gen_children(lexicon,0,prefix,children);
        //std::cout<<"children size: "<<children.size()<<"\n";
        int base=assign(0,children,true);
        dat[0].base=base;
        for(int i=0;i<(int)lexicon.size();i++){
            //std::cout<<"i "<<i<<"\n";
            Word& word=lexicon[i].first;//得到第一个词
            int off=this->get_info(word);
            //std::cout<<"off: "<<off<<" length of word: "<<word.size()<<"\n";
            //for(int j=0;j<word.size();j++){
            //    std::cout<<(int)word[j]<<" ";
            //};std::cout<<"\n";
            if(off<=0)off=(int)word.size();
            for(int offset=off;offset<=(int)word.size();offset++){
                //for(int j=0;j<offset;j++)prefix.push_back(word[j]);
                prefix=Word(word.pt,offset);
                int p_base=-this->get_info(prefix);
                //std::cout<<"p_base "<<p_base<<"\n";
                    
                gen_children(lexicon,i,prefix,children);
                int base=assign(p_base,children,offset==(int)word.size());
            }
            off=-this->get_info(word);
            if(no_prefix){
                dat[off].base=lexicon[i].second;
            }else{
                dat[dat[off].base].base=lexicon[i].second;
            }
            if(i&&(i%100000==0))printf("%f\n",(double)i/size);
            //int zkx;
            //std::cin>>zkx;
        }

    }

    void print(){
        printf("head %d, tail %d\n",head,tail);
        for(int i=0;i<(int)dat_size;i++)
            printf("[%d,%d,%d] ",i,dat[i].base,dat[i].check);
        printf("\n");
    }
};


};//end of isan
