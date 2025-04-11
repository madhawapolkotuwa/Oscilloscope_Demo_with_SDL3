#pragma once
#include <iostream>
#include <vector>
#include <fstream>

class SignalGeneration 
{
    std::vector<float> m_siganl;
public:
    SignalGeneration(){            

    }

    int OpenSiganlFile(const char* path){
        std::ifstream file(path);
        if(!file){
            std::cerr << "Error opening file!\n";
            return -1;
        }

        float value;
        while(file >> value){
            m_siganl.push_back(value);
        }
    }

};