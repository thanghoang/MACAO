/*
 * Utils.hpp
 *
 *      Author: thanghoang
 */
#include "Utils.hpp"

Utils::Utils()
{
}

Utils::~Utils()
{
}

#if defined(NTL_LIB)
sp_inverse Utils::PINV = PrepMulMod(P);
#endif

void Utils::print_ucharstring(const unsigned char *pInput,
                              int in_len)
{
    int indx;

    if (pInput != NULL && in_len > 0)
    {
        cout << "0x";
        for (indx = 0; indx < in_len; indx++)
        {
            if ((indx % 72 == 0) && (indx != 0))
            {
                cout << "\n";
            }
            printf("%.2x", pInput[indx]);
        }
    }
    else
    {
        cout << "Input string is NULL" << endl;
    }
}

int Utils::write_list_to_file(string filename, string path, vector<unsigned long int> lst)
{
    if (lst.size() == 0)
        return 0;

    std::ofstream output;
    output.open(path + filename, std::ios_base::app);
    TYPE_INDEX i;
    for (i = 0; i < lst.size(); i++)
    {
        output << lst[i] << " ";
    }
    output << "\n";
    output.close();
    return 0;
}

int Utils::write_list_to_file(string filename, string path, unsigned long int *lst, int lst_size)
{

    std::ofstream output;
    output.open(path + filename, std::ios_base::app);
    TYPE_INDEX i;
    for (i = 0; i < lst_size; i++)
    {
        output << lst[i] << " ";
    }
    output << "\n";
    output.close();
    return 0;
}

void Utils::writeArrayToFile(string filename, string path, double *arr, TYPE_INDEX size)
{
    std::ofstream output;
    TYPE_INDEX i;
    output.open(path + filename, std::ios_base::app);

    for (i = 0; i < size; i++)
    {
        output << arr[i] << " ";
    }
    output << endl;
    output.close();
}
