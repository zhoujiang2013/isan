#pragma once
#include "isan/common/common.hpp"

typedef String<Chinese_Character> Chinese;
typedef String<char> Feature_String;
typedef std::vector<Feature_String> Feature_Vector;


class State_Type: public String<char>{
public:
    PyObject* pack(){
        return PyBytes_FromStringAndSize(pt,length);

    };
    State_Type(){
    };
    
    State_Type(PyObject* py_key){
        char* buffer;
        Py_ssize_t len;
        int rtn=PyBytes_AsStringAndSize(py_key,&buffer,&len);
        length=(size_t)len;
        pt=new char[length];
        memcpy(pt,buffer,length*sizeof(char));        
    };
};

class Default_State_Type: public State_Type{
public:
    inline short* ind2(){
        return (short*)(pt);
    };
    inline Action_Type* last_action2(){
        return (Action_Type*)(pt+sizeof(short));
    };

    inline Action_Type* last_last_action2(){
        return (Action_Type*)(pt+sizeof(short)+sizeof(Action_Type));
    };

    inline short* sep_ind2(){
        return (short*)(pt+sizeof(short)+sizeof(Action_Type)+sizeof(Action_Type));
    };
    
    
    Default_State_Type(){
        length=sizeof(short)+sizeof(Action_Type)+sizeof(Action_Type)+sizeof(short);
        pt=new char[length];
        *ind2()=0;
        *last_action2()='|';
        *last_last_action2()='|';
        *sep_ind2()=0;
    };

    //void operator=(const Default_State_Type& other){
        //memcpy(pt,other.pt,length*sizeof(char));
    //};
    //Default_State_Type(PyObject* py_key): State_Type(){
        //char* buffer;
        //Py_ssize_t len;
        //int rtn=PyBytes_AsStringAndSize(py_key,&buffer,&len);
        //length=(size_t)len;
        //pt=new char[length];
        //memcpy(pt,buffer,length*sizeof(char));
        //return;
        
    //};
    
    
};


typedef Feature_Generator<Chinese,State_Type,Feature_Vector> CWS_Feature_Generator;
typedef State_Generator<Chinese,State_Type,Action_Type> CWS_State_Generator;


class Default_Feature_Generator: public CWS_Feature_Generator{
public:
    struct Three{
        char a;
        Chinese_Character b;
        char c;
    };
    struct Four{
        char a;
        Chinese_Character b;
        Chinese_Character c;
        char d;
    };
    Default_Feature_Generator(){
        this->raw=NULL;
    };
    void operator()(State_Type& super_state, Feature_Vector& fv){
        Default_State_Type& state=(Default_State_Type&)super_state;
        int ind=*(short*)state.pt;
        Action_Type left_action=*state.last_action2();
        Action_Type left_left_action=*state.last_last_action2();
        long sep_ind=*state.sep_ind2();
        
        Chinese_Character char_mid=ind-1>=0?raw->pt[ind-1]:-1;
        Chinese_Character char_right=ind<raw->length?raw->pt[ind]:-1;
        Chinese_Character char_left=ind-2>=0?raw->pt[ind-2]:-1;
        Chinese_Character char_left2=ind-3>=0?raw->pt[ind-3]:-1;
        Chinese_Character char_right2=ind+1<raw->length?raw->pt[ind+1]:-1;
        
        Three f_trans={0,(left_action),(left_left_action)};
        Three f_mid={1,(char_mid),(left_action)};
        Three f_right={2,(char_right),(left_action)};
        Three f_left={3,(char_left),(left_action)};
        
        Four f_mid_right={4,(char_mid),(char_right),(left_action)};
        Four f_left_mid={5,(char_left),(char_mid),(left_action)};
        Four f_left2_left={6,(char_left2),(char_left),(left_action)};
        Four f_right_right2={7,(char_right),(char_right2),(left_action)};
        
        fv.clear();
        fv.push_back(String<char>((char*)&f_trans,sizeof(f_trans)));
        fv.push_back(String<char>((char*)&f_mid,sizeof(f_mid)));
        fv.push_back(String<char>((char*)&f_right,sizeof(f_right)));
        fv.push_back(String<char>((char*)&f_left,sizeof(f_left)));
        
        fv.push_back(String<char>((char*)&f_mid_right,sizeof(f_mid_right)));
        fv.push_back(String<char>((char*)&f_left_mid,sizeof(f_left_mid)));
        fv.push_back(String<char>((char*)&f_left2_left,sizeof(f_left2_left)));
        fv.push_back(String<char>((char*)&f_right_right2,sizeof(f_right_right2)));
        
        fv.push_back(String<char>(1+sizeof(Chinese_Character)*sep_ind));
        fv.back().pt[0]=8;
        for(int i=0;i<sep_ind;i++)
            *(Chinese_Character *) (fv.back().pt+1+i*sizeof(Chinese_Character))= raw->pt[ind-sep_ind+i];
    };
};


class Default_State_Generator: public CWS_State_Generator{
public:

    Default_State_Generator(){
        
    }
    
    void operator()(State_Type& super_key, std::vector<std::pair<Action_Type, State_Type> > & super_nexts){
        Default_State_Type& key=(Default_State_Type&)super_key;
        std::vector<std::pair<Action_Type, Default_State_Type> > & nexts=
                (std::vector<std::pair<Action_Type, Default_State_Type> > &)super_nexts;
        
        nexts.resize(2);
        nexts[0].first='s';
        *(short*)nexts[0].second.pt=(*(short*)key.pt)+1;
        *nexts[0].second.last_action2()='s';
        *nexts[0].second.last_last_action2()=*key.last_action2();
        *nexts[0].second.sep_ind2()=1;
        
        

        nexts[1].first='c';
        *(short*)nexts.back().second.pt=(*(short*)key.pt)+1;
        *nexts.back().second.last_action2()='c';
        *nexts.back().second.last_last_action2()=*key.last_action2();
        *nexts.back().second.sep_ind2()=*key.sep_ind2()+1;
        
    };
};