#include<fstream>
#include<iostream>
using namespace std;

// 0 -> add_newline
// 1 -> filename
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        cerr << "Usage: add_newline [filename]" << endl;
        return 0;
    }
    cerr << argv[1] << endl;
    auto input_fp = freopen(argv[1], "r", stdin);
    if (input_fp == NULL)
    {
        cerr << "File does not exist" << endl;
        return 0;
    }
    char ch=fgetc(input_fp);
    if(ch==EOF)
        return 0;
    fseek(input_fp, -1, SEEK_END);
    char last_char;
    fread(&last_char, 1, 1, input_fp);
    if(last_char != '\n'){
        fclose(input_fp);
        input_fp = freopen(argv[1], "a+", stdout);
        fseek(input_fp, 0, SEEK_END);
        fprintf(input_fp,"\n");
    }
    fclose(input_fp);
    return 0;
}