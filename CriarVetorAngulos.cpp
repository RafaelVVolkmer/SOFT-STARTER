
#include <iostream>
#include <fstream>

#define ARR 17999

#define TAMANHO 42

uint16_t Val_Angulos[TAMANHO] = {0};

uint16_t Val_ARR[TAMANHO] = {0};

void Preenche_Angulo ( )
{

    for (uint16_t i = 0; i < TAMANHO; i++)
    {
       Val_Angulos[i] = i + 1;
    }

}

void Calcula_ARR ( )
{

    for(uint16_t i = 0; i < TAMANHO; i++)
    {
        uint16_t X_arr = ((ARR*Val_Angulos[i])/360);

        Val_ARR[i] = X_arr;
    }

}

int main (void)
{

    Preenche_Angulo ( );

    Calcula_ARR ( );

    std::ofstream arquivo("Vetor_Rampa.h");

     if ( arquivo.is_open() )
    {
        arquivo << "uint16_t Rampa_SoftStarter[" << TAMANHO << "] = {" << std::endl;

        for (uint16_t i = 0; i < TAMANHO; i++)
        {

            arquivo << "    " << Val_ARR[i];

            if (i < (TAMANHO - 1) )
            {

                arquivo << ",";

            }

            arquivo << std::endl;

        }

        arquivo << "};" << std::endl;

        arquivo.close();

        std::cout << "Arquivo gravado." << std::endl;

    } 
    else
    {
        std::cout << "Erro ao abrir o arquivo." << std::endl;

        return 1;

    }

}
