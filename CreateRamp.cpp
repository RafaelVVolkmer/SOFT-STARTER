#include <iostream>
#include <fstream>

#define SIZE 180

static void Fill_Angle(uint16_t *angles, uint16_t arr);
static void Calculate_ARR(uint16_t *angles, uint16_t *val_arr, uint16_t arr);

int main(void)
{
    uint16_t Val_Angles[SIZE];
    uint16_t Val_ARR[SIZE];
    
    uint16_t arr;
    
    std::cout << "Enter the value of ARR: ";
    std::cin >> arr;

    Fill_Angle(Val_Angles, arr);
    Calculate_ARR(Val_Angles, Val_ARR, arr);

    std::ofstream file("ramp_vector.h");

    if (file.is_open())
    {
        file << "#ifndef _RAMP_VECTOR_H_" << std::endl;
        file << "#define _RAMP_VECTOR_H_" << std::endl << std::endl;

        file << "// ANGLE TABLE CORRESPONDING TO THE SELECTED ARR: " << arr << std::endl;

        file << "uint16_t Ramp_SoftStarter[" << SIZE << "] = {" << std::endl;

        for (int i = SIZE - 1; i >= 0; i--)
        {
            file << "    " << *(Val_ARR + i);

            if (i > 0)
            {
                file << ",";
            }

            file << std::endl;
        }

        file << "};" << std::endl << std::endl;

        file << "#endif /* _RAMP_VECTOR_H_ */" << std::endl;

        file.close();

        std::cout <<  std::endl << "File written." << std::endl;
    }
    else
    {
        std::cout << std::endl << "Error opening the file." << std::endl;

        return 1;
    }

    return 0;
}

static void Fill_Angle(uint16_t *angles, uint16_t arr)
{
    for (uint16_t i = 0; i < SIZE; i++)
    {
        *(angles + i) = i + 1;
    }
}

static void Calculate_ARR(uint16_t *angles, uint16_t *val_arr, uint16_t arr)
{
    for (uint16_t i = 0; i < SIZE; i++)
    {
        uint16_t X_arr = ((arr * *(angles + i)) / 180);

        *(val_arr + i) = X_arr;
    }
}
