#include <iostream>
#include <fstream>

#define ARR 17400
#define TAMANHO 180

static void Preenche_Angulo(uint16_t *angulos);
static void Calcula_ARR(uint16_t *angulos, uint16_t *val_arr);

int main(void)
{
    uint16_t Val_Angulos[TAMANHO] = {0};
    uint16_t Val_ARR[TAMANHO] = {0};

    Preenche_Angulo(Val_Angulos);

    Calcula_ARR(Val_Angulos, Val_ARR);

    std::ofstream arquivo("Vetor_Rampa.h");

    if (arquivo.is_open())
    {
        arquivo << "#ifndef _VETOR_RAMPA_H_" << std::endl;
        arquivo << "#define _VETOR_RAMPA_H_" << std::endl << std::endl;

        arquivo << "/* TABELA DE ANGULOS CORRESPONDENTES AO ARR SELECIONADO */" << std::endl;

        arquivo << "uint16_t Rampa_SoftStarter[" << TAMANHO << "] = {" << std::endl;

        for (int i = TAMANHO - 1; i >= 0; i--)
        {
            arquivo << "    " << *(Val_ARR + i);

            if (i > 0)
            {
                arquivo << ",";
            }

            arquivo << std::endl;
        }

        arquivo << "};" << std::endl << std::endl;

        arquivo << "#endif /* _VETOR_RAMPA_H_ */" << std::endl;

        arquivo.close();

        std::cout << "Arquivo gravado." << std::endl;
    }
    else
    {
        std::cout << "Erro ao abrir o arquivo." << std::endl;

        return 1;
    }

    return 0;
}

static void Preenche_Angulo(uint16_t *angulos)
{
    for (uint16_t i = 0; i < TAMANHO; i++)
    {
        *(angulos + i) = i + 1;
    }
}

static void Calcula_ARR(uint16_t *angulos, uint16_t *val_arr)
{
    for (uint16_t i = 0; i < TAMANHO; i++)
    {
        uint16_t X_arr = ((ARR * *(angulos + i)) / 180);

        *(val_arr + i) = X_arr;
    }
}
