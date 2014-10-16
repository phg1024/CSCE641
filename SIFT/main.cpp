#include "sift.h"
#include "siftgui.h"
#include <QApplication>
#include <cstring>
#include <string>
#include <list>
#include <iostream>

#define GUI_VERSION 1

void printHelp(const string& program)
{
    cout << "Usage: " << program << " [-vgdeh] " << " filename1 ... filenamX" << endl;
    cout << " -v : verbose mode, output everything." << endl;
    cout << " -g : output gaussian pyramid." << endl;
    cout << " -d : output difference of gaussian pyramid." << endl;
    cout << " -e : output extrema images." << endl;
    cout << " -h : print help information." << endl;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

#if GUI_VERSION
    SiftGui gui;
    gui.setVisible(true);
    return app.exec();
#else
    list<string> ops;
    list<string> imgs;
    SiftOperator op;

    for(int i=1;i<argc;i++)
    {
        string arg = argv[i];
        if(arg.at(0) == '-')
        {
            for(size_t j=1;j<arg.size();j++)
            {char option = arg.at(j);
                switch(option)
                {
                case 'v':
                case 'g':
                case 'd':
                case 'e':
                {
                    op.setMode(option);
                    break;
                }
                case 'h':
                {
                    printHelp(argv[0]);
                    return 0;
                }
                default:
                    break;
                }
            }
        }
        else
            imgs.push_back(arg);
    }    

    list<string>::iterator it = imgs.begin();
    while(it!=imgs.end())
    {
        op.process((*it));
        ++it;
    }
    cout << endl << "all input processed!" << endl;
    return 0;
#endif
}
